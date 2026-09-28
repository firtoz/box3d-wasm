// Velocity/position integrate and TGS delta apply.

fn modified_cross(a: vec3<f32>, b: vec3<f32>) -> vec3<f32> {
    return vec3<f32>(
        a.y * b.z + a.z * b.y,
        a.z * b.x + a.x * b.z,
        a.x * b.y + a.y * b.x,
    );
}

@compute @workgroup_size(ISLAND_WORKGROUP_SIZE)
fn island_init(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, ISLAND_WORKGROUP_SIZE), 0u, 0u);
    let i = gid.x;
    if (i >= params.body_count) {
        return;
    }
    let s = body_states[i];
    let static_body = (s.flags & (FLAG_STATIC | FLAG_KINEMATIC | FLAG_DISABLED)) != 0u;
    atomicStore(&atom[atom_island_label() + i], select(i, EMPTY, static_body));
    atomicStore(&atom[atom_island_wake() + i], 0u);
    atomicStore(&atom[atom_island_ready() + i], 1u);
}

@compute @workgroup_size(ISLAND_WORKGROUP_SIZE)
fn island_union_edges(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, ISLAND_WORKGROUP_SIZE), 0u, 0u);
    let i = gid.x;
    let nc = min(scratch[SCR_NCONTACTS], pair_cap());
    if (i < nc) {
        let c = contacts[scratch[scr_active_contact() + i]];
        if (c.a != EMPTY && c.count != 0u) {
            island_union(c.a, c.b);
        }
    }
    if (i < params.joint_count) {
        let j = joints[i];
        if (j.kind != JOINT_NONE) {
            island_union(j.a, j.b);
        }
    }
}

@compute @workgroup_size(ISLAND_WORKGROUP_SIZE)
fn island_accumulate_wake(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, ISLAND_WORKGROUP_SIZE), 0u, 0u);
    let i = gid.x;
    if (i >= params.body_count) {
        return;
    }
    if (params.enable_sleep == 0u) {
        body_states[i].flags = body_states[i].flags & (~FLAG_SLEEP);
        body_states[i].sleep_time = 0.0;
        return;
    }
    let root = island_root(i);
    atomicStore(&atom[atom_island_label() + i], root);
    if (root != EMPTY && (body_states[i].flags & FLAG_SLEEP) == 0u) {
        atomicOr(&atom[atom_island_wake() + root], 1u);
    }
}

@compute @workgroup_size(ISLAND_WORKGROUP_SIZE)
fn island_apply_wake(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, ISLAND_WORKGROUP_SIZE), 0u, 0u);
    let i = gid.x;
    if (i >= params.body_count || params.enable_sleep == 0u) {
        return;
    }
    let root = atomicLoad(&atom[atom_island_label() + i]);
    if (root != EMPTY && atomicLoad(&atom[atom_island_wake() + root]) != 0u
        && (body_states[i].flags & FLAG_SLEEP) != 0u) {
        body_states[i].flags = body_states[i].flags & (~FLAG_SLEEP);
        body_states[i].sleep_time = 0.0;
    }
}

@compute @workgroup_size(ISLAND_WORKGROUP_SIZE)
fn island_reset_ready(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, ISLAND_WORKGROUP_SIZE), 0u, 0u);
    if (gid.x < params.body_count) {
        atomicStore(&atom[atom_island_ready() + gid.x], 1u);
    }
}

@compute @workgroup_size(ISLAND_WORKGROUP_SIZE)
fn island_accumulate_ready(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, ISLAND_WORKGROUP_SIZE), 0u, 0u);
    let i = gid.x;
    if (i >= params.body_count || params.enable_sleep == 0u) {
        return;
    }
    let root = atomicLoad(&atom[atom_island_label() + i]);
    if (root == EMPTY) {
        return;
    }
    // Explicitly sleeping bodies stay ready even before the automatic sleep
    // timer expires. Island wake propagation above handles active neighbours.
    let ready = (body_states[i].flags & FLAG_SLEEP) != 0u
        || ((body_states[i].flags & FLAG_SLEEP_ENABLED) != 0u
            && body_states[i].sleep_time >= TIME_TO_SLEEP);
    atomicAnd(&atom[atom_island_ready() + root], select(0u, 1u, ready));
}

@compute @workgroup_size(ISLAND_WORKGROUP_SIZE)
fn island_apply_sleep(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, ISLAND_WORKGROUP_SIZE), 0u, 0u);
    let i = gid.x;
    if (i >= params.body_count || params.enable_sleep == 0u) {
        return;
    }
    let root = atomicLoad(&atom[atom_island_label() + i]);
    if (root == EMPTY) {
        return;
    }
    if (atomicLoad(&atom[atom_island_ready() + root]) != 0u) {
        body_states[i].flags = body_states[i].flags | FLAG_SLEEP;
        body_states[i].vel = vec3<f32>(0.0);
        body_states[i].omega = vec3<f32>(0.0);
    } else {
        body_states[i].flags = body_states[i].flags & (~FLAG_SLEEP);
    }
}

@compute @workgroup_size(64)
fn reset_deltas(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    let i = gid.x;
    if (i >= params.body_count) {
        return;
    }
    var b = body_states[i];
    b.dp = vec3<f32>(0.0);
    b.dq = vec4<f32>(0.0, 0.0, 0.0, 1.0);
    body_states[i] = b;
}

@compute @workgroup_size(64)
fn apply_deltas(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    let i = gid.x;
    if (i >= params.body_count) {
        return;
    }
    var b = body_states[i];
    b.flags = b.flags & (~(FLAG_FAST | FLAG_CCD_NO_HIT));
    if ((b.flags & (FLAG_STATIC | FLAG_DISABLED | FLAG_SLEEP)) == 0u) {
        // Always apply TGS pose. Skipping `dp` while "quiet" left stacks nested
        // inside each other until a later step woke them and popped them apart.
        let base_rotation = b.rot;
        b.origin_valid = 0u;
        b.pos = b.pos + b.dp;
        b.rot = gyro_finish_rotation(b.dq, b.rot);
        let extra = body_extra_offset(i);
        let half = vec3<f32>(scene_f32(extra + 4u), scene_f32(extra + 5u), scene_f32(extra + 6u));
        let local_omega = abs(quat_rotate(quat_inv(base_rotation), b.omega));
        let velocity_arc = modified_cross(local_omega, half);
        let max_velocity = length(b.vel) + length(velocity_arc);
        let local_delta_rotation = abs(quat_rotate(quat_inv(base_rotation), b.dq.xyz));
        let rotation_arc = modified_cross(local_delta_rotation, half);
        let max_delta = length(b.dp) + 2.0 * length(rotation_arc);
        let sleep_velocity = max(max_velocity, 0.5 * max_delta / max(params.step_dt, 1e-8));
        b.sleep_velocity = sleep_velocity;
        let quiet = params.enable_sleep != 0u
            && (b.flags & FLAG_SLEEP_ENABLED) != 0u
            && sleep_velocity <= scene_f32(extra + 7u);
        let min_extent = scene_f32(extra + 3u);
        let max_motion = max(max_delta, max_velocity * params.step_dt);
        if (!quiet && params.enable_continuous != 0u
            && (b.flags & (FLAG_KINEMATIC | FLAG_DISABLED)) == 0u
            && (b.flags & FLAG_KINEMATIC) == 0u && max_motion > 0.5 * min_extent) {
            b.flags = b.flags | FLAG_FAST | FLAG_CCD_NO_HIT;
        }
        if (quiet) {
            b.sleep_time = b.sleep_time + params.step_dt;
        } else {
            b.sleep_time = 0.0;
            b.flags = b.flags & (~FLAG_SLEEP);
        }
    }
    b.dp = vec3<f32>(0.0);
    b.dq = vec4<f32>(0.0, 0.0, 0.0, 1.0);
    body_states[i] = b;
}

fn fused_body(i: u32) -> bool {
    return scratch[fused_flag_base() + i] != 0u;
}

fn integrate_vel_one(i: u32) {
    var b = load_body(i);
    // Kinematic bodies damp their prescribed velocities too. Their zero inverse
    // mass suppresses gravity/forces; static and disabled bodies do not advance.
    if ((b.flags & (FLAG_STATIC | FLAG_DISABLED | FLAG_SLEEP)) == 0u) {
        let g = vec3<f32>(params.gravity_x, params.gravity_y, params.gravity_z);
        let h = params.dt;
        let ld = 1.0 / (1.0 + h * b.linear_damping);
        let ad = 1.0 / (1.0 + h * b.angular_damping);
        let extra = body_extra_offset(i);
        let force = vec3<f32>(scene_f32(extra + 8u), scene_f32(extra + 9u), scene_f32(extra + 10u));
        let torque = vec3<f32>(scene_f32(extra + 12u), scene_f32(extra + 13u), scene_f32(extra + 14u));
        let gravity_scale = select(0.0, b.gravity_scale, b.inv_mass > 0.0);
        if (any(force != vec3<f32>(0.0))) {
            // Native applies the load on every substep, after damping the old velocity.
            let delta = (h * b.inv_mass) * force + (h * gravity_scale) * g;
            b.vel = delta + ld * b.vel;
        } else {
            b.vel = g * (h * gravity_scale) + b.vel * ld;
        }
        if (any(torque != vec3<f32>(0.0))) {
            b.omega = h * world_inv_inertia(b, torque) + ad * b.omega;
        } else {
            b.omega = b.omega * ad;
        }
        gyro_apply_gyro(&b, h);
        if ((b.flags & 256u) != 0u) { b.vel.x = 0.0; }
        if ((b.flags & 512u) != 0u) { b.vel.y = 0.0; }
        if ((b.flags & 1024u) != 0u) { b.vel.z = 0.0; }
        if ((b.flags & 2048u) != 0u) { b.omega.x = 0.0; }
        if ((b.flags & 4096u) != 0u) { b.omega.y = 0.0; }
        if ((b.flags & 8192u) != 0u) { b.omega.z = 0.0; }
    }
    store_body(i, b);
}

fn physics_max_angular_speed(step_dt: f32) -> f32 {
    // Native integration multiplies B3_MAX_ROTATION by the rounded inverse dt.
    return 0.78539816339 * gyro_recip(max(step_dt, 1e-8));
}

fn physics_clamp_speed(v: vec3<f32>, max_speed: f32) -> vec3<f32> {
    let speed2 = gyro_dot3(v, v);
    if (speed2 > max_speed * max_speed) {
        return v * gyro_divide(max_speed, gyro_sqrt(speed2));
    }
    return v;
}

fn integrate_pos_one(i: u32) {
    var b = body_states[i];
    if ((b.flags & FLAG_STATIC) == 0u
        && (b.flags & FLAG_SLEEP) == 0u
        && (b.flags & FLAG_DISABLED) == 0u) {
        if ((b.flags & 256u) != 0u) { b.vel.x = 0.0; }
        if ((b.flags & 512u) != 0u) { b.vel.y = 0.0; }
        if ((b.flags & 1024u) != 0u) { b.vel.z = 0.0; }
        if ((b.flags & 2048u) != 0u) { b.omega.x = 0.0; }
        if ((b.flags & 4096u) != 0u) { b.omega.y = 0.0; }
        if ((b.flags & 8192u) != 0u) { b.omega.z = 0.0; }
        b.vel = physics_clamp_speed(b.vel, params.maximum_linear_speed);
        if ((b.flags & FLAG_ALLOW_FAST_ROTATION) == 0u) {
            b.omega = physics_clamp_speed(b.omega, physics_max_angular_speed(params.step_dt));
        }
        let h = params.dt;
        b.dp = b.dp + b.vel * h;
        b.dq = gyro_integrate_rotation(b.dq, b.omega, h);
    }
    body_states[i] = b;
}

fn solve_tiny_slot(slot: u32, mode: u32) {
    var c = load_solve_contact(slot);
    if (c.a == EMPTY || c.count == 0u) {
        return;
    }
    var ba = load_body(c.a);
    var bb = load_body(c.b);
    solve_contact_chain(slot, &ba, &bb, mode);
    if (!is_immovable(ba)) {
        store_body(c.a, ba);
    }
    if (!is_immovable(bb)) {
        store_body(c.b, bb);
    }
}

fn solve_validated_component_slot(slot: u32, mode: u32) {
    var c = load_solve_contact(slot);
    if (c.a == EMPTY || c.count == 0u) {
        return;
    }
    var ba = load_body(c.a);
    var bb = load_body(c.b);
    solve_validated_contact_chain_store(slot, &ba, &bb, mode, true);
    if (!is_immovable(ba)) {
        store_body(c.a, ba);
    }
    if (!is_immovable(bb)) {
        store_body(c.b, bb);
    }
}

@compute @workgroup_size(64)
fn solve_tiny_islands(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    let i = gid.x;
    if (i >= params.body_count || !fused_body(i)) {
        return;
    }
    let nsub = max(params.sub_step_count, 1u);
    let base = fused_slots_base() + 8u * i;
    for (var sub = 0u; sub < nsub; sub++) {
        integrate_vel_one(i);
        for (var k = 0u; k < 8u; k++) {
            let slot = scratch[base + k];
            if (slot != EMPTY) {
                solve_tiny_slot(slot, 2u);
                solve_tiny_slot(slot, 1u);
            }
        }
        integrate_pos_one(i);
        for (var k = 0u; k < 8u; k++) {
            let slot = scratch[base + k];
            if (slot != EMPTY) {
                solve_tiny_slot(slot, 0u);
            }
        }
    }
    for (var k = 0u; k < 8u; k++) {
        let slot = scratch[base + k];
        if (slot != EMPTY) {
            solve_tiny_slot(slot, 3u);
        }
    }
}

@compute @workgroup_size(64)
fn integrate_vel(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    let i = gid.x;
    if (i >= params.body_count) {
        return;
    }
    integrate_vel_one(i);
}

@compute @workgroup_size(64)
fn integrate_pos(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    let i = gid.x;
    if (i >= params.body_count) {
        return;
    }
    integrate_pos_one(i);
}

// Opt-in, phase-accurate diagnostics. Each dispatch records the body state
// visible to the following pipeline plus aggregate contact persistence data.
@compute @workgroup_size(64)
fn capture_phase(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    if ((params.diagnostic_flags & DIAG_PHASE_CAPTURE) == 0u) {
        return;
    }
    let i = gid.x;
    let phase = params.color_select;
    if (i < params.body_count) {
        let b = body_states[i];
        let base = phase_body_base() + (phase * params.body_count + i) * 8u;
        scratch[base] = bitcast<u32>(b.vel.x);
        scratch[base + 1u] = bitcast<u32>(b.vel.z);
        scratch[base + 2u] = bitcast<u32>(length(b.omega));
        scratch[base + 3u] = b.flags;
        scratch[base + 4u] = bitcast<u32>(b.vel.y);
        scratch[base + 5u] = bitcast<u32>(b.omega.x);
        scratch[base + 6u] = bitcast<u32>(b.omega.y);
        scratch[base + 7u] = bitcast<u32>(b.omega.z);
    }
    if (i != 0u) {
        return;
    }
    var touching = 0u;
    var feature_points = 0u;
    var warm_points = 0u;
    var normal_impulse = 0.0;
    var tangent_impulse2 = 0.0;
    let pair_count = min(scratch[SCR_NCONTACTS], pair_cap());
    for (var j = 0u; j < pair_count; j++) {
        let slot = scratch[scr_active_contact() + j];
        let c = load_contact(slot);
        if (c.a == EMPTY || c.count == 0u) {
            continue;
        }
        touching = touching + 1u;
        for (var p = 0u; p < c.count; p++) {
            feature_points = feature_points + select(0u, 1u, feat_at(c, p) != 0u);
            let impulse = rb_at(c, p).w;
            warm_points = warm_points + select(0u, 1u, impulse > 0.0);
            normal_impulse = normal_impulse + impulse;
        }
        tangent_impulse2 = tangent_impulse2 + dot(c.friction_impulse, c.friction_impulse);
    }
    let summary = phase_summary_base() + phase * 8u;
    scratch[summary] = phase;
    scratch[summary + 1u] = touching;
    scratch[summary + 2u] = feature_points;
    scratch[summary + 3u] = warm_points;
    scratch[summary + 4u] = bitcast<u32>(normal_impulse);
    scratch[summary + 5u] = bitcast<u32>(sqrt(tangent_impulse2));
    scratch[summary + 6u] = pair_count;
    scratch[summary + 7u] = scratch[SCR_BSTRIDE];
}

// Experimental complete-component TGS. Static endpoints are read-only and
// moving immovable endpoints/joints are excluded by the host eligibility check.
fn component_owns_contact(slot: u32, root: u32) -> bool {
    // Other components may already be updating hot contacts. Persistent pair
    // identity and scene shape metadata stay read-only throughout TGS.
    let key=contact_persistent[slot].pair.xy;
    let a=load_shape(key.x).body_index;
    let b=load_shape(key.y).body_index;
    return island_root(a)==root || island_root(b)==root;
}
fn component_integrate(root: u32, position: bool, n: u32, ids: ptr<function, array<u32, 8>>) {
    if (n <= 8u) {
        for (var i=0u; i<n; i++) {
            if (position) { integrate_pos_one((*ids)[i]); } else { integrate_vel_one((*ids)[i]); }
        }
    } else {
        for (var i=0u; i<params.body_count; i++) {
            if (island_root(i) == root) {
                if (position) { integrate_pos_one(i); } else { integrate_vel_one(i); }
            }
        }
    }
}
fn component_phase(root: u32, mode: u32, n: u32, slots: ptr<function, array<u32, 32>>) {
    if (n <= 32u) {
        for (var i=0u; i<n; i++) { solve_validated_component_slot((*slots)[i], mode); }
    } else {
        // Large components retain all constraints, in reference wave order.
        for (var wave=0u; wave<=OVERFLOW_COLOR; wave++) {
            let col = select(wave-1u, OVERFLOW_COLOR, wave==0u);
            for (var i=0u; i<scratch[SCR_COLOR+col]; i++) {
                let slot=listed_index(col,i);
                if (component_owns_contact(slot,root)) { solve_tiny_slot(slot,mode); }
            }
        }
    }
}
override SMALL_COMPONENT_WORKGROUP_SIZE:u32=16u;
@compute @workgroup_size(SMALL_COMPONENT_WORKGROUP_SIZE)
fn solve_complete_components(@builtin(global_invocation_id) dispatch_gid: vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, SMALL_COMPONENT_WORKGROUP_SIZE), 0u, 0u);
    let root=gid.x;
    if (atomicLoad(&query[260u])!=0u) {return;}
    if (root>=params.body_count || island_root(root)!=root) {return;}
    // Island-wide wake propagation has completed. A sleeping root proves all
    // writable members sleep; preserve their bodies and cached impulses.
    if ((body_states[root].flags & FLAG_SLEEP)!=0u) {return;}
    let meta_base=component_meta(root);
    let body_n=atomicLoad(&query[meta_base]);
    let contact_n=atomicLoad(&query[meta_base+1u]);
    if (body_n>8u || contact_n>32u) {return;}
    let body_start=atomicLoad(&query[meta_base+2u]);
    let contact_start=atomicLoad(&query[meta_base+3u]);
    var ids: array<u32,8>;
    for (var i=0u;i<body_n;i++) {ids[i]=atomicLoad(&query[component_bodies()+body_start+i]);}
    var slots: array<u32,32>;
    var keys: array<u32,32>;
    for (var i=0u;i<contact_n;i++) {
        let at=component_contacts()+2u*(contact_start+i);
        let slot=atomicLoad(&query[at]);let key=atomicLoad(&query[at+1u]);
        // No allocation/retirement occurs during this component's TGS dispatch.
        // Reject the complete component before integration if a chain is invalid.
        if (!contact_chain_valid(slot)) { return; }
        var j=i;
        loop {
            if (j==0u) {break;}
            if (keys[j-1u]<=key) {break;}
            keys[j]=keys[j-1u];slots[j]=slots[j-1u];j--;
        }
        keys[j]=key;slots[j]=slot;
    }
    for (var sub=0u; sub<max(params.sub_step_count,1u); sub++) {
        component_integrate(root,false,body_n,&ids);
        component_phase(root,2u,contact_n,&slots);
        component_phase(root,1u,contact_n,&slots);
        component_integrate(root,true,body_n,&ids);
        // Dispatch binds use_bias=1. Keep the relaxed phase uniform-derived,
        // like the global solver: literal-zero specialization changed normal
        // impulse results on NVIDIA and accumulated drift in the comparison.
        component_phase(root,params.use_bias - 1u,contact_n,&slots);
    }
    component_phase(root,3u,contact_n,&slots);
}


var<workgroup> large_component_enabled: u32;
var<workgroup> large_component_color_mask: u32;
fn large_component_integrate(root: u32, lid: u32, position: bool) {
    let meta_base=component_meta(root);let n=atomicLoad(&query[meta_base]);let start=atomicLoad(&query[meta_base+2u]);
    for (var k=lid; k<n; k+=64u) {
        let i=atomicLoad(&query[component_bodies()+start+k]);
        if (position) { integrate_pos_one(i); } else { integrate_vel_one(i); }
    }
    storageBarrier();
    workgroupBarrier();
}
fn large_component_phase(root: u32, lid: u32, mode: u32, color_mask:u32) {
    for (var wave=0u; wave<=OVERFLOW_COLOR; wave++) {
        let col=select(wave-1u,OVERFLOW_COLOR,wave==0u);
        if ((color_mask & (1u<<col))==0u) {continue;}
        let n=scratch[SCR_COLOR+col];
        if (col==OVERFLOW_COLOR) {
            if (lid==0u) {
                for (var i=0u; i<n; i++) {
                    let slot=listed_index(col,i);
                    if (component_owns_contact(slot,root)) {solve_validated_component_slot(slot,mode);}
                }
            }
        } else {
            let count=atomicLoad(&query[component_color_counts()+24u*root+col]);
            let start=atomicLoad(&query[component_color_starts()+24u*root+col]);
            // Each color is contiguous; do not rescan all island contacts for every wave.
            for (var i=lid;i<count;i+=64u) {
                solve_validated_component_slot(atomicLoad(&query[component_contacts()+2u*(start+i)]),mode);
            }
        }
        storageBarrier();
        workgroupBarrier();
    }
}
var<workgroup> large_component_invalid: atomic<u32>;
@compute @workgroup_size(64)
fn solve_large_components(@builtin(workgroup_id) group: vec3<u32>, @builtin(local_invocation_index) lid:u32) {
    let index=group.x+256u*group.y;
    if (lid==0u) {
        large_component_enabled=EMPTY;
        atomicStore(&large_component_invalid,0u);
        if (index<atomicLoad(&query[256u])) {
            let candidate=atomicLoad(&query[component_large_roots()+index]);
            if ((body_states[candidate].flags & FLAG_SLEEP)==0u) {large_component_enabled=candidate;}
        }
    }
    workgroupBarrier();
    let root=workgroupUniformLoad(&large_component_enabled);
    if (root==EMPTY) {return;}
    // Every color, including overflow, occupies this component's compact range.
    // Links, endpoints and point counts cannot change during the TGS dispatch.
    // Reject the entire component before integration, then reuse that proof.
    let meta_base=component_meta(root);
    let contact_count=atomicLoad(&query[meta_base+1u]);
    let contact_start=atomicLoad(&query[meta_base+3u]);
    for (var i=lid;i<contact_count;i+=64u) {
        let slot=atomicLoad(&query[component_contacts()+2u*(contact_start+i)]);
        if (!contact_chain_valid(slot)) {atomicStore(&large_component_invalid,1u);}
    }
    workgroupBarrier();
    if (lid==0u) {
        if (atomicLoad(&large_component_invalid)!=0u) {large_component_enabled=EMPTY;}
        var mask=0u;
        for (var col=0u;col<=OVERFLOW_COLOR;col++) {
            if (atomicLoad(&query[component_color_counts()+24u*root+col])!=0u) {mask|=1u<<col;}
        }
        large_component_color_mask=mask;
    }
    workgroupBarrier();
    if (workgroupUniformLoad(&large_component_enabled)==EMPTY) {return;}
    // Uniform across this workgroup: empty colors need no storage barrier.
    // Lists remain fixed through all substeps; active color order is unchanged.
    let color_mask=workgroupUniformLoad(&large_component_color_mask);
    for (var sub=0u; sub<max(params.sub_step_count,1u); sub++) {
        large_component_integrate(root,lid,false);
        large_component_phase(root,lid,2u,color_mask);
        large_component_phase(root,lid,1u,color_mask);
        large_component_integrate(root,lid,true);
        large_component_phase(root,lid,params.use_bias-1u,color_mask);
    }
    large_component_phase(root,lid,3u,color_mask);
}


// Query words 0..255 retain their existing query/status ABI. This private
// tail is rebuilt after island wake/prepare and remains immutable during TGS.
fn component_meta(root:u32)->u32 {return 261u+4u*root;}
fn component_bodies()->u32 {return 261u+4u*params.body_count;}
fn component_contacts()->u32 {return component_bodies()+params.body_count;}
fn component_large_roots()->u32 {return component_contacts()+2u*params.contact_capacity;}
fn component_color_counts()->u32 {return component_large_roots()+params.body_count;}
fn component_color_starts()->u32 {return component_color_counts()+24u*params.body_count;}
fn component_contact_root(slot:u32)->u32 {
    let key=contact_persistent[slot].pair.xy;
    let a=island_root(load_shape(key.x).body_index);
    let b=island_root(load_shape(key.y).body_index);
    return select(a,b,a==EMPTY);
}
@compute @workgroup_size(64)
fn component_reset(@builtin(global_invocation_id) dispatch_gid:vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    if (gid.x<params.body_count) {
        atomicStore(&query[component_meta(gid.x)],0u);
        atomicStore(&query[component_meta(gid.x)+1u],0u);
        for (var col=0u;col<=OVERFLOW_COLOR;col++) {atomicStore(&query[component_color_counts()+24u*gid.x+col],0u);}
    }
}
// Validate membership in the current graph lists, not just stale color metadata.
// This also handles memoized graphs without reconstructing their liveness rules.
fn component_active_slot(i:u32)->u32 {
    let slot=scratch[scr_active_contact()+i];
    if (slot>=params.contact_capacity) {return EMPTY;}
    let col=contacts[slot].color;
    if (col>OVERFLOW_COLOR) {return EMPTY;}
    let local=contact_persistent[slot].lifecycle.z;
    if (local>=scratch[SCR_COLOR+col]) {return EMPTY;}
    if (listed_index(col,local)!=slot) {return EMPTY;}
    return slot;
}
@compute @workgroup_size(64)
fn component_count(@builtin(global_invocation_id) dispatch_gid:vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    let i=gid.x;
    if (i<params.body_count) {
        let root=island_root(i);
        if (root!=EMPTY) {atomicAdd(&query[component_meta(root)],1u);}
    }
    // Physical dispatch covers capacity; stride also handles candidate lists
    // longer than the root-slot pool without omitting late allocated roots.
    if (i>=params.contact_capacity) {return;}
    for (var candidate_i=i;candidate_i<min(scratch[SCR_NCONTACTS],pair_cap());candidate_i+=params.contact_capacity) {
        let slot=component_active_slot(candidate_i);
        if (slot==EMPTY) {continue;}
        let root=component_contact_root(slot);
        if (root!=EMPTY) {
            let col=contacts[slot].color;
            atomicAdd(&query[component_meta(root)+1u],1u);
            atomicAdd(&query[component_color_counts()+24u*root+col],1u);
        }
    }
}
// One cooperative prefix workgroup: each lane counts a contiguous root range,
// then scans range totals. Root order remains canonical without a serial N scan.
var<workgroup> component_prefix: array<vec3<u32>,256>;
var<workgroup> component_prefix_valid: u32;
@compute @workgroup_size(256)
fn component_offsets(@builtin(local_invocation_index) lid:u32) {
    let first=params.body_count*lid/256u;
    let end=params.body_count*(lid+1u)/256u;
    var counts=vec3<u32>(0u);
    for (var root=first;root<end;root++) {
        let m=component_meta(root);
        let nb=atomicLoad(&query[m]);let nc=atomicLoad(&query[m+1u]);
        counts+=vec3<u32>(nb,nc,select(0u,1u,nb>8u || nc>32u));
    }
    component_prefix[lid]=counts;
    workgroupBarrier();
    for (var stride=1u;stride<256u;stride*=2u) {
        var before=vec3<u32>(0u);
        if (lid>=stride) {before=component_prefix[lid-stride];}
        workgroupBarrier();
        component_prefix[lid]+=before;
        workgroupBarrier();
    }
    let totals=component_prefix[255u];
    if (lid==0u) {
        component_prefix_valid=select(0u,1u,totals.x<=params.body_count && totals.y<=params.contact_capacity);
        atomicStore(&query[260u],1u-component_prefix_valid);
        atomicStore(&query[256u],select(0u,totals.z,component_prefix_valid!=0u));
        atomicStore(&query[257u],select(0u,min(totals.z,256u),component_prefix_valid!=0u));
        atomicStore(&query[258u],max((totals.z+255u)/256u,1u));
        atomicStore(&query[259u],1u);
        if (component_prefix_valid==0u) {record_contact_drop(15u);}
    }
    workgroupBarrier();
    if (workgroupUniformLoad(&component_prefix_valid)==0u) {return;}
    var starts=component_prefix[lid]-counts;
    for (var root=first;root<end;root++) {
        let m=component_meta(root);
        let nb=atomicLoad(&query[m]);let nc=atomicLoad(&query[m+1u]);
        atomicStore(&query[m+2u],starts.x);atomicStore(&query[m+3u],starts.y);
        if (nb>8u || nc>32u) {
            atomicStore(&query[component_large_roots()+starts.z],root);starts.z++;
        }
        starts.x+=nb;starts.y+=nc;
        atomicStore(&query[m],0u);atomicStore(&query[m+1u],0u);
    }
}
@compute @workgroup_size(64)
fn component_scatter(@builtin(global_invocation_id) dispatch_gid:vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    if (atomicLoad(&query[260u])!=0u) {return;}
    let i=gid.x;
    if (i<params.body_count) {
        let root=island_root(i);
        if (root!=EMPTY) {
            let meta_base=component_meta(root);let local=atomicAdd(&query[meta_base],1u);
            atomicStore(&query[component_bodies()+atomicLoad(&query[meta_base+2u])+local],i);
        }
    }
    if (i>=params.contact_capacity) {return;}
    for (var candidate_i=i;candidate_i<min(scratch[SCR_NCONTACTS],pair_cap());candidate_i+=params.contact_capacity) {
        let slot=component_active_slot(candidate_i);
        if (slot==EMPTY) {continue;}
        let root=component_contact_root(slot);
        if (root!=EMPTY) {
            let col=contacts[slot].color;
            let graph_local=contact_persistent[slot].lifecycle.z;
            let meta_base=component_meta(root);atomicAdd(&query[meta_base+1u],1u);
            let local=atomicAdd(&query[component_color_counts()+24u*root+col],1u);
            let at=component_contacts()+2u*(atomicLoad(&query[component_color_starts()+24u*root+col])+local);
            atomicStore(&query[at],slot);
            atomicStore(&query[at+1u],(select(col+1u,0u,col==OVERFLOW_COLOR)<<24u)|graph_local);
        }
    }
}

@compute @workgroup_size(64)
fn component_color_offsets(@builtin(global_invocation_id) dispatch_gid:vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    let root=gid.x;
    if (root>=params.body_count || atomicLoad(&query[260u])!=0u) {return;}
    var start=atomicLoad(&query[component_meta(root)+3u]);
    for (var col=0u;col<=OVERFLOW_COLOR;col++) {
        let index=24u*root+col;
        atomicStore(&query[component_color_starts()+index],start);
        start+=atomicLoad(&query[component_color_counts()+index]);
        atomicStore(&query[component_color_counts()+index],0u);
    }
}

// Count after CCD. Kinematics and any enabled non-static awake body block idle.
@compute @workgroup_size(64)
fn count_active_bodies(@builtin(global_invocation_id) dispatch_gid:vec3<u32>) {
    let gid = vec3<u32>(linear_invocation_id(dispatch_gid, 64u), 0u, 0u);
    if (gid.x>=params.body_count) {return;}
    let flags=body_states[gid.x].flags;
    if ((flags & (FLAG_STATIC|FLAG_DISABLED))==0u && ((flags & FLAG_SLEEP)==0u || (flags & FLAG_KINEMATIC)!=0u)) {
        atomicAdd(&query[72u],1u);
    }
}
