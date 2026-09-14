// TGS contact GS, friction/rolling, overflow, joints.

fn rolling_mass_mul(ba: Body, bb: Body, v: vec3<f32>) -> vec3<f32> {
    let c0 = world_inv_inertia(ba, vec3<f32>(1.0, 0.0, 0.0)) + world_inv_inertia(bb, vec3<f32>(1.0, 0.0, 0.0));
    let c1 = world_inv_inertia(ba, vec3<f32>(0.0, 1.0, 0.0)) + world_inv_inertia(bb, vec3<f32>(0.0, 1.0, 0.0));
    let c2 = world_inv_inertia(ba, vec3<f32>(0.0, 0.0, 1.0)) + world_inv_inertia(bb, vec3<f32>(0.0, 0.0, 1.0));
    return solve3(c0, c1, c2, v);
}

fn motor_linear_mass_mul(ba: Body, bb: Body, rA: vec3<f32>, rB: vec3<f32>, v: vec3<f32>) -> vec3<f32> {
    let m = body_inv_mass(ba) + body_inv_mass(bb);
    let e0 = vec3<f32>(1.0, 0.0, 0.0);
    let e1 = vec3<f32>(0.0, 1.0, 0.0);
    let e2 = vec3<f32>(0.0, 0.0, 1.0);
    let c0 = m * e0
        - cross(rA, world_inv_inertia(ba, cross(rA, e0)))
        - cross(rB, world_inv_inertia(bb, cross(rB, e0)));
    let c1 = m * e1
        - cross(rA, world_inv_inertia(ba, cross(rA, e1)))
        - cross(rB, world_inv_inertia(bb, cross(rB, e1)));
    let c2 = m * e2
        - cross(rA, world_inv_inertia(ba, cross(rA, e2)))
        - cross(rB, world_inv_inertia(bb, cross(rB, e2)));
    return solve3(c0, c1, c2, v);
}

fn clamp_vector_length(v: vec3<f32>, max_length: f32) -> vec3<f32> {
    let length_squared = dot(v, v);
    if (length_squared > max_length * max_length) {
        return v * (max_length / sqrt(length_squared));
    }
    return v;
}

// Stored joint anchors are body-origin. Live lever arms subtract COM.
fn joint_anchor_com(body_index: u32, origin_anchor: vec3<f32>) -> vec3<f32> {
    return origin_anchor - load_body_cold(body_index).local_center;
}

fn joint_lever(body_index: u32, q: vec4<f32>, origin_anchor: vec3<f32>) -> vec3<f32> {
    return quat_rotate(q, joint_anchor_com(body_index, origin_anchor));
}

fn joint_softness(hertz: f32, damping: f32) -> vec3<f32> {
    let h = params.dt;
    let clamped_hertz = min(max(hertz, 0.0), 0.25 / max(h, 1e-8));
    let omega = 6.28318530718 * clamped_hertz;
    let a1 = 2.0 * max(damping, 0.0) + h * omega;
    let a2 = h * omega * a1;
    let a3 = 1.0 / (1.0 + a2);
    return vec3<f32>(
        select(0.0, omega / a1, a1 > 0.0),
        a2 * a3,
        a3,
    );
}

fn spring_softness(hertz: f32, damping: f32) -> vec3<f32> {
    let h = params.dt;
    let omega = 6.28318530718 * max(hertz, 0.0);
    let a1 = 2.0 * max(damping, 0.0) + h * omega;
    let a2 = h * omega * a1;
    let a3 = 1.0 / (1.0 + a2);
    return vec3<f32>(
        select(0.0, omega / a1, a1 > 0.0),
        a2 * a3,
        a3,
    );
}

fn twist_angle(q: vec4<f32>) -> f32 {
    if (q.w < 0.0) {
        return 2.0 * atan2(-q.z, -q.w);
    }
    return 2.0 * atan2(q.z, q.w);
}

fn swing_angle(q: vec4<f32>) -> f32 {
    return 2.0 * atan2(length(q.xy), length(q.zw));
}

fn normalize_or_zero(v: vec3<f32>) -> vec3<f32> {
    let length_squared = dot(v, v);
    return select(vec3<f32>(0.0), v * inverseSqrt(length_squared), length_squared > 0.0);
}

fn delta_quat_to_rotation(q: vec4<f32>, desired: vec4<f32>) -> vec3<f32> {
    let s = select(q, -q, dot(q, desired) < 0.0);
    let diff = desired - s;
    return 2.0 * quat_mul(diff, quat_inv(s)).xyz;
}

fn revolute_axes(frame_a: vec4<f32>, rel: vec4<f32>) -> mat2x3<f32> {
    let axis_x = 0.5 * quat_rotate(
        frame_a,
        rel.w * vec3<f32>(1.0, 0.0, 0.0)
            + cross(rel.xyz, vec3<f32>(1.0, 0.0, 0.0)),
    );
    let axis_y = 0.5 * quat_rotate(
        frame_a,
        rel.w * vec3<f32>(0.0, 1.0, 0.0)
            + cross(rel.xyz, vec3<f32>(0.0, 1.0, 0.0)),
    );
    // WGSL matrices store columns. Keep each Jacobian axis in one column.
    return mat2x3<f32>(axis_x, axis_y);
}

fn revolute_axis_x(axes: mat2x3<f32>) -> vec3<f32> {
    return axes[0];
}

fn revolute_axis_y(axes: mat2x3<f32>) -> vec3<f32> {
    return axes[1];
}

fn perp(n: vec3<f32>) -> vec3<f32> {
    // Box3D `b3Perp`: branch on |x| > 0.5 (not 1/sqrt(3)).
    if (n.x < -0.5 || n.x > 0.5) {
        return normalize(vec3<f32>(n.y, -n.x, 0.0));
    }
    return normalize(vec3<f32>(0.0, n.z, -n.y));
}

fn normal_mass(a: Body, b: Body, rA: vec3<f32>, rB: vec3<f32>, n: vec3<f32>) -> f32 {
    let rnA = cross(rA, n);
    let rnB = cross(rB, n);
    let k = body_inv_mass(a) + body_inv_mass(b) + dot(rnA, world_inv_inertia(a, rnA)) + dot(rnB, world_inv_inertia(b, rnB));
    if (k > 0.0) {
        return 1.0 / k;
    }
    return 0.0;
}

fn current_sep(a: Body, b: Body, rA: vec3<f32>, rB: vec3<f32>, n: vec3<f32>, base: f32) -> f32 {
    let rsA = quat_rotate(a.dq, rA);
    let rsB = quat_rotate(b.dq, rB);
    let ds = (b.dp - a.dp) + (rsB - rsA);
    return base + dot(n, ds);
}

// Validate the complete ownership chain before any impulse is applied. This
// also makes malformed/cyclic lists fail closed instead of double-solving.
fn contact_chain_valid(root: u32) -> bool {
    // The common single-manifold case needs no ownership traversal. Preserve
    // the structural/content failure categories and every general-path check.
    if (root >= params.contact_capacity) {
        record_contact_drop(0u); return false;
    }
    let head = contacts[root];
    if (head.manifold_link.z <= 1u) {
        if (head.manifold_link.y != 0u || head.manifold_link.x != 0u) {
            record_contact_drop(1u); return false;
        }
        let valid = head.a < params.body_count && head.b < params.body_count
            && head.a != head.b && head.count > 0u && head.count <= 4u;
        if (!valid) { record_contact_drop(5u); }
        return valid;
    }
    if (!contact_chain_structure_valid(root)) { return false; }
    let first = contacts[root];
    var valid = first.a < params.body_count && first.b < params.body_count && first.a != first.b;
    var slot = root;
    for (var i = 0u; i < max(first.manifold_link.z, 1u); i++) {
        let c = contacts[slot];
        valid = valid && c.count > 0u && c.count <= 4u;
        if (c.manifold_link.x != 0u) { slot = c.manifold_link.x - 1u; }
    }
    if (!valid) { record_contact_drop(5u); }
    return valid;
}

fn prepare_contact_slot(k: u32) {
    var c = load_solve_contact(k);
    if (c.a == EMPTY || c.count == 0u) {
        return;
    }
    let a = load_body(c.a);
    let b = load_body(c.b);
    c.prepared_normal_mass = vec4<f32>(0.0);
    c.prepared_lever_arm = vec4<f32>(0.0);
    c.total_normal_impulse = vec4<f32>(0.0);
    var relative_velocity = vec4<f32>(0.0);
    var center_a = vec3<f32>(0.0);
    var center_b = vec3<f32>(0.0);
    var friction_weight = 0.0;
    for (var i = 0u; i < c.count; i++) {
        let rA = ra_at(c, i).xyz;
        let rB = rb_at(c, i).xyz;
        c.prepared_normal_mass[i] = normal_mass(a, b, rA, rB, c.n);
        relative_velocity[i] = dot((b.vel + cross(b.omega, rB)) - (a.vel + cross(a.omega, rA)), c.n);
        let separation = ra_at(c, i).w + dot(rB - rA, c.n);
        let weight = clamp(2.0 - separation / SPECULATIVE, MIN_FRICTION_WEIGHT, 1.0);
        center_a = center_a + weight * rA;
        center_b = center_b + weight * rB;
        friction_weight = friction_weight + weight;
    }
    c.center_a = center_a / friction_weight;
    c.center_b = center_b / friction_weight;
    for (var i = 0u; i < c.count; i++) {
        c.prepared_lever_arm[i] = length(ra_at(c, i).xyz - c.center_a);
    }
    c._tail0 = vec4<u32>(
        bitcast<u32>(relative_velocity.x),
        bitcast<u32>(relative_velocity.y),
        bitcast<u32>(relative_velocity.z),
        bitcast<u32>(relative_velocity.w),
    );
    let t1 = perp(c.n);
    let t2 = cross(t1, c.n);
    let rt1a = cross(c.center_a, t1);
    let rt1b = cross(c.center_b, t1);
    let rt2a = cross(c.center_a, t2);
    let rt2b = cross(c.center_b, t2);
    let msum = body_inv_mass(a) + body_inv_mass(b);
    let k11 = msum + dot(rt1a, world_inv_inertia(a, rt1a)) + dot(rt1b, world_inv_inertia(b, rt1b));
    let k22 = msum + dot(rt2a, world_inv_inertia(a, rt2a)) + dot(rt2b, world_inv_inertia(b, rt2b));
    let k12 = dot(rt1a, world_inv_inertia(a, rt2a)) + dot(rt1b, world_inv_inertia(b, rt2b));
    let det = k11 * k22 - k12 * k12;
    if (abs(det) > 1e-8) {
        let inv_det = 1.0 / det;
        c.prepared_tangent_inv = vec4<f32>(k22 * inv_det, -k12 * inv_det, -k12 * inv_det, k11 * inv_det);
    } else {
        c.prepared_tangent_inv = vec4<f32>(
            select(0.0, 1.0 / k11, k11 > 1e-8),
            0.0,
            0.0,
            select(0.0, 1.0 / k22, k22 > 1e-8),
        );
    }
    let soft = contact_softness(is_non_dynamic(a) || is_non_dynamic(b));
    c.prepared_softness = vec4<f32>(
        soft.x,
        soft.y,
        soft.z,
        select(0.0, 1.0, is_non_dynamic(a) || is_non_dynamic(b)),
    );
    if ((params.diagnostic_flags & DIAG_MESH_CANDIDATES) != 0u) {
        trace_solver_values(20u,c.a,c.b,c.prepared_normal_mass,
            vec4<f32>(soft,c.friction),vec4<f32>(c.center_b,c.rolling));
        trace_solver_values(21u,c.a,c.b,
            vec4<f32>(world_inv_inertia(b,vec3<f32>(1.0,0.0,0.0)),0.0),
            vec4<f32>(world_inv_inertia(b,vec3<f32>(0.0,1.0,0.0)),0.0),
            vec4<f32>(world_inv_inertia(b,vec3<f32>(0.0,0.0,1.0)),0.0));
    }
    store_solve_contact(k, c);
}

@compute @workgroup_size(64)
fn prepare_contacts(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    if (i >= min(scratch[SCR_NCONTACTS], PAIR_CAP)) { return; }
    let root = scratch[SCR_ACTIVE_CONTACT + i];
    if (root >= params.contact_capacity || contacts[root].a == EMPTY || contacts[root].count == 0u) { return; }
    if (!contact_chain_valid(root)) { return; }
    let count = max(contacts[root].manifold_link.z, 1u);
    var slot = root;
    for (var j = 0u; j < count; j++) {
        prepare_contact_slot(slot);
        if (j + 1u < count) { slot = contacts[slot].manifold_link.x - 1u; }
    }
}

fn solve_manifold(c: ptr<function, Contact>, ba: ptr<function, Body>, bb: ptr<function, Body>, do_friction: bool) {
    solve_manifold_bias(c, ba, bb, do_friction, params.use_bias);
}

fn solve_manifold_bias(
    c: ptr<function, Contact>,
    ba: ptr<function, Body>,
    bb: ptr<function, Body>,
    do_friction: bool,
    use_bias: u32,
) {
    if ((params.diagnostic_flags & DIAG_MESH_CANDIDATES) != 0u) {
        trace_solver_values(30u+use_bias,(*c).a,(*c).b,
            vec4<f32>((*bb).vel,0.0),vec4<f32>((*bb).omega,0.0),vec4<f32>((*bb).dp,0.0));
    }
    let n = (*c).n;
    let soft = (*c).prepared_softness.xyz;
    var bias_rate = 0.0;
    var mass_scale = 1.0;
    var impulse_scale = 0.0;
    if (use_bias == 1u) {
        bias_rate = soft.y * soft.x;
        mass_scale = soft.y;
        impulse_scale = soft.z;
    }
    let inv_h = 1.0 / max(params.dt, 1e-8);
    let contact_speed = -params.contact_speed;
    var total_jn = 0.0;
    var total_twist = 0.0;
    for (var i = 0u; i < (*c).count; i++) {
        let ra = ra_at(*c, i);
        let rb = rb_at(*c, i);
        let rA = ra.xyz;
        let rB = rb.xyz;
        let s = current_sep(*ba, *bb, rA, rB, n, ra.w);
        var bias: f32;
        var pms = mass_scale;
        var pis = impulse_scale;
        if (s > 0.0) {
            bias = s * inv_h;
            pms = 1.0;
            pis = 0.0;
        } else {
            bias = max(bias_rate * s, contact_speed);
        }
        let vA = (*ba).vel + cross((*ba).omega, rA);
        let vB = (*bb).vel + cross((*bb).omega, rB);
        let vn = dot(vB - vA, n);
        let nm = (*c).prepared_normal_mass[i];
        let j_old = rb.w;
        let neg_imp = nm * (pms * vn + bias) + pis * j_old;
        var j = max(j_old - neg_imp, 0.0);
        let dj = j - j_old;
        (*c).total_normal_impulse[i] = (*c).total_normal_impulse[i] + j;
        let P = n * dj;
        apply_P(ba, rA, P, -1.0);
        apply_P(bb, rB, P, 1.0);
        total_jn = total_jn + j;
        let lever = (*c).prepared_lever_arm[i];
        total_twist = total_twist + lever * j;
        set_point(c, i, ra, vec4<f32>(rB, j));
    }

    // Box3D's SIMD convex path omits friction during the biased pass.
    if (do_friction && use_bias == 0u) {
        let t1 = perp(n);
        let t2 = cross(t1, n);
        let rA = (*c).center_a;
        let rB = (*c).center_b;
        {
            let twist_speed = dot(n, (*bb).omega - (*ba).omega);
            let max_l = (*c).friction * total_twist;
            var k_tw = dot(n, world_inv_inertia(*ba, n) + world_inv_inertia(*bb, n));
            var tm = 0.0;
            if (k_tw > 1e-8) { tm = 1.0 / k_tw; }
            var dtw = -tm * twist_speed;
            var tw = clamp((*c).twist_impulse + dtw, -max_l, max_l);
            dtw = tw - (*c).twist_impulse;
            (*c).twist_impulse = tw;
            let L = n * dtw;
            if (!is_immovable(*ba)) {
                (*ba).omega = (*ba).omega - world_inv_inertia(*ba, L);
            }
            if (!is_immovable(*bb)) {
                (*bb).omega = (*bb).omega + world_inv_inertia(*bb, L);
            }
        }
        if ((*c).rolling > 0.0) {
            // Box3D: deltaImpulse = -inv(iA+iB) * (wB - wA), clamp to rolling * Σjn.
            let dw = (*ba).omega - (*bb).omega;
            var jr = (*c).rolling_impulse + rolling_mass_mul(*ba, *bb, dw);
            let max_r = (*c).rolling * total_jn;
            let lr2 = dot(jr, jr);
            if (lr2 > max_r * max_r + 1.1920929e-7) {
                jr = jr * (max_r / sqrt(lr2));
            }
            let djr = jr - (*c).rolling_impulse;
            (*c).rolling_impulse = jr;
            if (!is_immovable(*ba)) {
                (*ba).omega = (*ba).omega - world_inv_inertia(*ba, djr);
            }
            if (!is_immovable(*bb)) {
                (*bb).omega = (*bb).omega + world_inv_inertia(*bb, djr);
            }
        }
        let vA = (*ba).vel + cross((*ba).omega, rA);
        let vB = (*bb).vel + cross((*bb).omega, rB);
        let vr = vB - vA;
        let relative_tangent_velocity = vr - (*c).tangent_velocity;
        let vt = vec2<f32>(
            dot(relative_tangent_velocity, t1),
            dot(relative_tangent_velocity, t2),
        );
        var jn = (*c).friction_impulse;
        let mt = (*c).prepared_tangent_inv;
        jn = jn - vec2<f32>(mt.x * vt.x + mt.y * vt.y, mt.z * vt.x + mt.w * vt.y);
        let max_j = (*c).friction * total_jn;
        let mag2 = dot(jn, jn);
        if (mag2 > max_j * max_j) {
            jn = jn * (max_j / sqrt(mag2));
        }
        let djt = jn - (*c).friction_impulse;
        (*c).friction_impulse = jn;
        let P = t1 * djt.x + t2 * djt.y;
        apply_P(ba, rA, P, -1.0);
        apply_P(bb, rB, P, 1.0);
    }
    if ((params.diagnostic_flags & DIAG_MESH_CANDIDATES) != 0u) {
        trace_solver_values(40u+use_bias,(*c).a,(*c).b,
            vec4<f32>((*bb).vel,0.0),vec4<f32>((*bb).omega,0.0),vec4<f32>((*bb).dp,0.0));
    }

}

fn apply_restitution(c: ptr<function, Contact>, ba: ptr<function, Body>, bb: ptr<function, Body>) {
    let rest = (*c)._pad_end;
    if (rest <= 0.0) {
        return;
    }
    let n = (*c).n;
    let threshold = params.restitution_threshold;
    let rel = vec4<f32>(
        bitcast<f32>((*c)._tail0.x),
        bitcast<f32>((*c)._tail0.y),
        bitcast<f32>((*c)._tail0.z),
        bitcast<f32>((*c)._tail0.w),
    );
    for (var i = 0u; i < (*c).count; i++) {
        var rv = rel.x;
        if (i == 1u) { rv = rel.y; }
        if (i == 2u) { rv = rel.z; }
        if (i == 3u) { rv = rel.w; }
        if (rv + threshold > 0.0) {
            continue;
        }
        let ra = ra_at(*c, i);
        let rb = rb_at(*c, i);
        if ((*c).total_normal_impulse[i] == 0.0) {
            continue;
        }
        let nm = (*c).prepared_normal_mass[i];
        let vA = (*ba).vel + cross((*ba).omega, ra.xyz);
        let vB = (*bb).vel + cross((*bb).omega, rb.xyz);
        let vn = dot(vB - vA, n);
        let neg_imp = nm * (vn + rest * rv);
        var j = max(rb.w - neg_imp, 0.0);
        let dj = j - rb.w;
        (*c).total_normal_impulse[i] = (*c).total_normal_impulse[i] + dj;
        apply_P(ba, ra.xyz, n * dj, -1.0);
        apply_P(bb, rb.xyz, n * dj, 1.0);
        set_point(c, i, ra, vec4<f32>(rb.xyz, j));
    }
}

fn warm_manifold(c: Contact, ba: ptr<function, Body>, bb: ptr<function, Body>) {
    let n = c.n;
    for (var i = 0u; i < c.count; i++) {
        let rA = ra_at(c, i).xyz;
        let rB = rb_at(c, i).xyz;
        let P = n * rb_at(c, i).w;
        apply_P(ba, rA, P, -1.0);
        apply_P(bb, rB, P, 1.0);
    }
    let t1 = perp(n);
    let t2 = cross(t1, n);
    let Pf = t1 * c.friction_impulse.x + t2 * c.friction_impulse.y;
    apply_P(ba, c.center_a, Pf, -1.0);
    apply_P(bb, c.center_b, Pf, 1.0);
    if (length(c.rolling_impulse) > 0.0) {
        if (!is_immovable(*ba)) {
            (*ba).omega = (*ba).omega - world_inv_inertia(*ba, c.rolling_impulse);
        }
        if (!is_immovable(*bb)) {
            (*bb).omega = (*bb).omega + world_inv_inertia(*bb, c.rolling_impulse);
        }
    }
    if (abs(c.twist_impulse) > 0.0) {
        let L = c.n * c.twist_impulse;
        if (!is_immovable(*ba)) {
            (*ba).omega = (*ba).omega - world_inv_inertia(*ba, L);
        }
        if (!is_immovable(*bb)) {
            (*bb).omega = (*bb).omega + world_inv_inertia(*bb, L);
        }
    }
}

@compute @workgroup_size(64)
fn warm_start(@builtin(global_invocation_id) gid: vec3<u32>) {
    let k = gid.x;
    let maxc = params.contact_capacity;
    if (k >= maxc) {
        return;
    }
    let c = load_solve_contact(k);
    if (c.a == EMPTY || c.count == 0u || c.manifold_link.y != 0u || c.color != params.color_select) {
        return;
    }
    var ba = load_body(c.a);
    var bb = load_body(c.b);
    solve_contact_chain(k, &ba, &bb, 2u);
    store_body(c.a, ba);
    store_body(c.b, bb);
}

@compute @workgroup_size(64)
fn solve_contacts(@builtin(global_invocation_id) gid: vec3<u32>) {
    let k = gid.x;
    let maxc = params.contact_capacity;
    if (k >= maxc) {
        return;
    }
    var c = load_solve_contact(k);
    if (c.a == EMPTY || c.count == 0u || c.manifold_link.y != 0u || c.color != params.color_select) {
        return;
    }
    var ba = load_body(c.a);
    var bb = load_body(c.b);
    solve_contact_chain(k, &ba, &bb, params.use_bias);
    store_body(c.a, ba);
    store_body(c.b, bb);
}

fn listed_index(col: u32, i: u32) -> u32 {
    return scratch[color_contact_base() + col * params.contact_capacity + i];
}

fn solve_contact_chain(root: u32, ba: ptr<function, Body>, bb: ptr<function, Body>, mode: u32) {
    if (!contact_chain_valid(root)) { return; }
    solve_validated_contact_chain(root, ba, bb, mode);
}

// Caller owns an immutable, validated chain for this dispatch only.
fn solve_validated_contact_chain(root: u32, ba: ptr<function, Body>, bb: ptr<function, Body>, mode: u32) {
    solve_validated_contact_chain_store(root, ba, bb, mode, false);
}

fn solve_validated_contact_chain_store(root: u32, ba: ptr<function, Body>, bb: ptr<function, Body>, mode: u32, impulses_only: bool) {
    // Sleeping contacts retain their warm-start history, as in Box3D's sleeping
    // solver sets. Solving two immovable endpoints otherwise decays that cache.
    if (is_immovable(*ba) && is_immovable(*bb)) {return;}
    let count = max(contacts[root].manifold_link.z, 1u);
    var slot = root;
    for (var i = 0u; i < count; i++) {
        var c = load_solve_contact(slot);
        if (c.count > 0u) {
            if (mode == 2u) { warm_manifold(c, ba, bb); }
            else if (mode == 3u) {
                apply_restitution(&c, ba, bb);
                if (impulses_only) { store_contact_impulses(slot, c); }
                else { store_solve_contact(slot, c); }
            } else {
                solve_manifold_bias(&c, ba, bb, true, mode);
                if (impulses_only) { store_contact_impulses(slot, c); }
                else { store_solve_contact(slot, c); }
            }
        }
        if (i + 1u < count) { slot = c.manifold_link.x - 1u; }
    }
}

fn run_contact(k: u32) {
    var c = load_solve_contact(k);
    if (c.a == EMPTY || c.count == 0u) {
        return;
    }
    var ba = load_body(c.a);
    var bb = load_body(c.b);
    solve_contact_chain(k, &ba, &bb, params.use_bias);
    if (!is_immovable(ba)) {
        store_body(c.a, ba);
    }
    if (!is_immovable(bb)) {
        store_body(c.b, bb);
    }
}

var<workgroup> wg_solve_a: array<Body, 64>;
var<workgroup> wg_solve_b: array<Body, 64>;

@compute @workgroup_size(64)
fn solve_color(
    @builtin(global_invocation_id) gid: vec3<u32>,
) {
    let col = params.color_select;
    let n = scratch[SCR_COLOR + col];
    if (col == OVERFLOW_COLOR) {
        if (gid.x == 0u) {
            for (var i = 0u; i < n; i++) {
                run_contact(listed_index(col, i));
            }
        }
        return;
    }
    let i = gid.x;
    if (i >= n) {
        return;
    }
    // Coloring grants exclusive writable endpoints across the entire color.
    // Each lane owns its bodies; no cross-lane staging or barrier is needed.
    let k = listed_index(col, i);
    var c = load_solve_contact(k);
    var ba = load_body(c.a);
    var bb = load_body(c.b);
    solve_contact_chain(k, &ba, &bb, params.use_bias);
    if (!is_immovable(ba)) {
        store_body(c.a, ba);
    }
    if (!is_immovable(bb)) {
        store_body(c.b, bb);
    }
}

// Low-occupancy fast path. Graph compaction enables this dispatch only when
// every non-overflow color has at most one workgroup of contacts. The barriers
// retain overflow-first, then ascending-color Gauss-Seidel ordering while
// replacing 24 separate active dispatches with one workgroup.
@compute @workgroup_size(64)
fn solve_color_wave_one_group(@builtin(local_invocation_index) lid: u32) {
    if (lid == 0u) {
        let overflow_n = scratch[SCR_COLOR + OVERFLOW_COLOR];
        for (var i = 0u; i < overflow_n; i++) {
            run_contact(listed_index(OVERFLOW_COLOR, i));
        }
    }
    storageBarrier();
    workgroupBarrier();

    solve_color_range_one_group(lid, 0u, OVERFLOW_COLOR);
}

// The prefix handled overflow and dynamic colors below color_select. Static
// colors are dispatched in parallel after this dynamic-color tail.
// When the existing whole-wave shortcut ran, skip this tail to avoid solving twice.
@compute @workgroup_size(64)
fn solve_color_tail_one_group(@builtin(local_invocation_index) lid: u32) {
    if (scratch[SCR_INDIRECT_COLLIDE] != 0u) { return; }
    solve_color_range_one_group(lid, params.color_select, DYNAMIC_COLOR_COUNT);
}

fn solve_color_range_one_group(lid: u32, first_color: u32, end_color: u32) {
    for (var col = first_color; col < end_color; col++) {
        let n = scratch[SCR_COLOR + col];
        if (n == 0u) {
            continue;
        }
        var base = 0u;
        loop {
            if (base >= n) {
                break;
            }
            let i = base + lid;
            var k = EMPTY;
            if (i < n) {
                k = listed_index(col, i);
                let c = load_solve_contact(k);
                wg_solve_a[lid] = load_body(c.a);
                wg_solve_b[lid] = load_body(c.b);
            }
            workgroupBarrier();
            if (i < n) {
                var c = load_solve_contact(k);
                var ba = wg_solve_a[lid];
                var bb = wg_solve_b[lid];
                solve_contact_chain(k, &ba, &bb, params.use_bias);
                if (!is_immovable(ba)) {
                    store_body(c.a, ba);
                }
                if (!is_immovable(bb)) {
                    store_body(c.b, bb);
                }
            }
            storageBarrier();
            workgroupBarrier();
            base = base + 64u;
        }
    }
}

fn jac_slot(body: u32, comp: u32) -> u32 {
    return ATOM_JACOBI + body * 6u + comp;
}

fn jac_add(body: u32, dv: vec3<f32>, dw: vec3<f32>) {
    if (is_immovable(load_body(body))) {
        return;
    }
    atomicAdd(&atom[jac_slot(body, 0u)], bitcast<u32>(i32(round(dv.x * 1000.0))));
    atomicAdd(&atom[jac_slot(body, 1u)], bitcast<u32>(i32(round(dv.y * 1000.0))));
    atomicAdd(&atom[jac_slot(body, 2u)], bitcast<u32>(i32(round(dv.z * 1000.0))));
    atomicAdd(&atom[jac_slot(body, 3u)], bitcast<u32>(i32(round(dw.x * 1000.0))));
    atomicAdd(&atom[jac_slot(body, 4u)], bitcast<u32>(i32(round(dw.y * 1000.0))));
    atomicAdd(&atom[jac_slot(body, 5u)], bitcast<u32>(i32(round(dw.z * 1000.0))));
}

@compute @workgroup_size(64)
fn jacobi_clear(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    if (i >= params.body_count) {
        return;
    }
    for (var c = 0u; c < 6u; c++) {
        atomicStore(&atom[jac_slot(i, c)], 0u);
    }
}

@compute @workgroup_size(64)
fn solve_jacobi(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    let np = min(scratch[SCR_NCONTACTS], PAIR_CAP);
    if (i >= np) {
        return;
    }
    let k = scratch[SCR_ACTIVE_CONTACT + i];
    var c = load_solve_contact(k);
    if (c.a == EMPTY || c.count == 0u) {
        return;
    }
    var ba = load_body(c.a);
    var bb = load_body(c.b);
    let va0 = ba.vel;
    let wa0 = ba.omega;
    let vb0 = bb.vel;
    let wb0 = bb.omega;
    solve_contact_chain(k, &ba, &bb, params.use_bias);
    jac_add(c.a, ba.vel - va0, ba.omega - wa0);
    jac_add(c.b, bb.vel - vb0, bb.omega - wb0);
}

@compute @workgroup_size(64)
fn apply_jacobi(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    if (i >= params.body_count) {
        return;
    }
    var b = load_body(i);
    if (is_immovable(b)) {
        return;
    }
    let s = 0.001;
    b.vel.x = b.vel.x + f32(bitcast<i32>(atomicLoad(&atom[jac_slot(i, 0u)]))) * s;
    b.vel.y = b.vel.y + f32(bitcast<i32>(atomicLoad(&atom[jac_slot(i, 1u)]))) * s;
    b.vel.z = b.vel.z + f32(bitcast<i32>(atomicLoad(&atom[jac_slot(i, 2u)]))) * s;
    b.omega.x = b.omega.x + f32(bitcast<i32>(atomicLoad(&atom[jac_slot(i, 3u)]))) * s;
    b.omega.y = b.omega.y + f32(bitcast<i32>(atomicLoad(&atom[jac_slot(i, 4u)]))) * s;
    b.omega.z = b.omega.z + f32(bitcast<i32>(atomicLoad(&atom[jac_slot(i, 5u)]))) * s;
    store_body(i, b);
}

@compute @workgroup_size(1)
fn solve_overflow(@builtin(global_invocation_id) gid: vec3<u32>) {
    if (gid.x != 0u) {
        return;
    }
    let maxc = params.contact_capacity;
    for (var k = 0u; k < maxc; k++) {
        var c = load_solve_contact(k);
        if (c.a == EMPTY || c.count == 0u || c.manifold_link.y != 0u || c.color != OVERFLOW_COLOR) {
            continue;
        }
        var ba = load_body(c.a);
        var bb = load_body(c.b);
        solve_contact_chain(k, &ba, &bb, params.use_bias);
        store_body(c.a, ba);
        store_body(c.b, bb);
    }
}

fn solve_revolute(
    jn: ptr<function, Joint>,
    ba: ptr<function, Body>,
    bb: ptr<function, Body>,
) {
    let base_frame_a = normalize(quat_mul((*ba).rot, (*jn).frame_a_rotation));
    var base_frame_b = normalize(quat_mul((*bb).rot, (*jn).frame_b_rotation));
    if (dot(base_frame_a, base_frame_b) < 0.0) {
        base_frame_b = -base_frame_b;
    }
    let base_rel = quat_mul(quat_inv(base_frame_a), base_frame_b);
    let base_axes = revolute_axes(base_frame_a, base_rel);
    let base_axis_x = revolute_axis_x(base_axes);
    let base_axis_y = revolute_axis_y(base_axes);
    let rotation_axis = quat_rotate(base_frame_a, vec3<f32>(0.0, 0.0, 1.0));

    let qa = normalize(quat_mul((*ba).dq, (*ba).rot));
    let qb = normalize(quat_mul((*bb).dq, (*bb).rot));
    let rA = joint_lever((*jn).a, qa, (*jn).anchor_a);
    let rB = joint_lever((*jn).b, qb, (*jn).anchor_b);

    if (params.use_bias == 2u) {
        let axial_impulse =
            (*jn).spring_impulse + (*jn).motor_impulse
            + (*jn).lower_impulse - (*jn).upper_impulse;
        let angular_impulse =
            (*jn).perp_impulse.x * base_axis_x
            + (*jn).perp_impulse.y * base_axis_y
            + axial_impulse * rotation_axis;
        apply_P(ba, rA, (*jn).angular_impulse, -1.0);
        apply_P(bb, rB, (*jn).angular_impulse, 1.0);
        (*ba).omega = (*ba).omega - world_inv_inertia(*ba, angular_impulse);
        (*bb).omega = (*bb).omega + world_inv_inertia(*bb, angular_impulse);
        return;
    }

    let i0 = world_inv_inertia(*ba, vec3<f32>(1.0, 0.0, 0.0))
        + world_inv_inertia(*bb, vec3<f32>(1.0, 0.0, 0.0));
    let i1 = world_inv_inertia(*ba, vec3<f32>(0.0, 1.0, 0.0))
        + world_inv_inertia(*bb, vec3<f32>(0.0, 1.0, 0.0));
    let i2 = world_inv_inertia(*ba, vec3<f32>(0.0, 0.0, 1.0))
        + world_inv_inertia(*bb, vec3<f32>(0.0, 0.0, 1.0));
    let fixed_rotation = abs(dot(i0, cross(i1, i2))) < 1e-30;
    let axial_k = dot(rotation_axis, world_inv_inertia(*ba, rotation_axis)
        + world_inv_inertia(*bb, rotation_axis));
    let axial_mass = select(0.0, 1.0 / axial_k, axial_k > 0.0);

    let frame_a = normalize(quat_mul(qa, (*jn).frame_a_rotation));
    var frame_b = normalize(quat_mul(qb, (*jn).frame_b_rotation));
    if (dot(frame_a, frame_b) < 0.0) {
        frame_b = -frame_b;
    }
    let rel = quat_mul(quat_inv(frame_a), frame_b);

    // Box3D order: spring, motor, lower/upper limits, collinearity, point-to-point.
    if (((*jn).flags & REVOLUTE_ENABLE_SPRING) != 0u && !fixed_rotation) {
        let soft = spring_softness((*jn).spring_hertz, (*jn).spring_damping);
        let c = twist_angle(rel) - (*jn).target_translation;
        let cdot = dot((*bb).omega - (*ba).omega, rotation_axis);
        let delta_impulse =
            -soft.y * axial_mass * (cdot + soft.x * c)
            - soft.z * (*jn).spring_impulse;
        (*jn).spring_impulse = (*jn).spring_impulse + delta_impulse;
        (*ba).omega =
            (*ba).omega - delta_impulse * world_inv_inertia(*ba, rotation_axis);
        (*bb).omega =
            (*bb).omega + delta_impulse * world_inv_inertia(*bb, rotation_axis);
    }

    if (((*jn).flags & REVOLUTE_ENABLE_MOTOR) != 0u && !fixed_rotation) {
        let cdot =
            dot((*bb).omega - (*ba).omega, rotation_axis) - (*jn).motor_speed;
        let old_impulse = (*jn).motor_impulse;
        let max_impulse = (*jn).max_motor_force * params.dt;
        (*jn).motor_impulse = clamp(
            old_impulse - axial_mass * cdot,
            -max_impulse,
            max_impulse,
        );
        let delta_impulse = (*jn).motor_impulse - old_impulse;
        (*ba).omega =
            (*ba).omega - delta_impulse * world_inv_inertia(*ba, rotation_axis);
        (*bb).omega =
            (*bb).omega + delta_impulse * world_inv_inertia(*bb, rotation_axis);
    }

    if (((*jn).flags & REVOLUTE_ENABLE_LIMIT) != 0u && !fixed_rotation) {
        let angle = twist_angle(rel);
        let soft = joint_softness((*jn).hertz, (*jn).damping);
        let inv_h = 1.0 / max(params.dt, 1e-8);

        let lower_c = angle - (*jn).lower_translation;
        var lower_bias = 0.0;
        var lower_mass_scale = 1.0;
        var lower_impulse_scale = 0.0;
        if (lower_c > 0.0) {
            lower_bias = lower_c * inv_h;
        } else if (params.use_bias == 1u) {
            lower_bias = soft.x * lower_c;
            lower_mass_scale = soft.y;
            lower_impulse_scale = soft.z;
        }
        let lower_cdot = dot((*bb).omega - (*ba).omega, rotation_axis);
        let old_lower = (*jn).lower_impulse;
        let lower_delta =
            -lower_mass_scale * axial_mass * (lower_cdot + lower_bias)
            - lower_impulse_scale * old_lower;
        (*jn).lower_impulse = max(old_lower + lower_delta, 0.0);
        let applied_lower = (*jn).lower_impulse - old_lower;
        (*ba).omega =
            (*ba).omega - applied_lower * world_inv_inertia(*ba, rotation_axis);
        (*bb).omega =
            (*bb).omega + applied_lower * world_inv_inertia(*bb, rotation_axis);

        let upper_c = (*jn).upper_translation - angle;
        var upper_bias = 0.0;
        var upper_mass_scale = 1.0;
        var upper_impulse_scale = 0.0;
        if (upper_c > 0.0) {
            upper_bias = upper_c * inv_h;
        } else if (params.use_bias == 1u) {
            upper_bias = soft.x * upper_c;
            upper_mass_scale = soft.y;
            upper_impulse_scale = soft.z;
        }
        let upper_cdot = dot((*ba).omega - (*bb).omega, rotation_axis);
        let old_upper = (*jn).upper_impulse;
        let upper_delta =
            -upper_mass_scale * axial_mass * (upper_cdot + upper_bias)
            - upper_impulse_scale * old_upper;
        (*jn).upper_impulse = max(old_upper + upper_delta, 0.0);
        let applied_upper = (*jn).upper_impulse - old_upper;
        (*ba).omega =
            (*ba).omega + applied_upper * world_inv_inertia(*ba, rotation_axis);
        (*bb).omega =
            (*bb).omega - applied_upper * world_inv_inertia(*bb, rotation_axis);
    }

    if (!fixed_rotation) {
        let axes = revolute_axes(frame_a, rel);
        let axis_x = revolute_axis_x(axes);
        let axis_y = revolute_axis_y(axes);
        let kxx = dot(axis_x, world_inv_inertia(*ba, axis_x)
            + world_inv_inertia(*bb, axis_x));
        let kyy = dot(axis_y, world_inv_inertia(*ba, axis_y)
            + world_inv_inertia(*bb, axis_y));
        let kxy = dot(axis_x, world_inv_inertia(*ba, axis_y)
            + world_inv_inertia(*bb, axis_y));
        let det = kxx * kyy - kxy * kxy;
        var solution = vec2<f32>(0.0);
        var bias = vec2<f32>(0.0);
        var mass_scale = 1.0;
        var impulse_scale = 0.0;
        if (params.use_bias == 1u) {
            let soft = joint_softness((*jn).hertz, (*jn).damping);
            bias = soft.x * rel.xy;
            mass_scale = soft.y;
            impulse_scale = soft.z;
        }
        let wrel = (*bb).omega - (*ba).omega;
        let rhs = vec2<f32>(
            dot(wrel, axis_x) + bias.x,
            dot(wrel, axis_y) + bias.y,
        );
        if (det > 1e-30) {
            solution = vec2<f32>(
                (kyy * rhs.x - kxy * rhs.y) / det,
                (kxx * rhs.y - kxy * rhs.x) / det,
            );
        }
        let delta_impulse =
            -mass_scale * solution - impulse_scale * (*jn).perp_impulse;
        (*jn).perp_impulse = (*jn).perp_impulse + delta_impulse;
        let angular_impulse = delta_impulse.x * axis_x + delta_impulse.y * axis_y;
        (*ba).omega = (*ba).omega - world_inv_inertia(*ba, angular_impulse);
        (*bb).omega = (*bb).omega + world_inv_inertia(*bb, angular_impulse);
    }

    let cdot =
        ((*bb).vel + cross((*bb).omega, rB))
        - ((*ba).vel + cross((*ba).omega, rA));
    var bias = vec3<f32>(0.0);
    var mass_scale = 1.0;
    var impulse_scale = 0.0;
    if (params.use_bias == 1u) {
        let soft = joint_softness((*jn).hertz, (*jn).damping);
        let separation =
            ((*bb).pos + (*bb).dp + rB) - ((*ba).pos + (*ba).dp + rA);
        bias = soft.x * separation;
        mass_scale = soft.y;
        impulse_scale = soft.z;
    }
    let solution = motor_linear_mass_mul(*ba, *bb, rA, rB, cdot + bias);
    let impulse =
        -mass_scale * solution - impulse_scale * (*jn).angular_impulse;
    (*jn).angular_impulse = (*jn).angular_impulse + impulse;
    apply_P(ba, rA, impulse, -1.0);
    apply_P(bb, rB, impulse, 1.0);
}

fn solve_prismatic(
    jn: ptr<function, Joint>,
    ba: ptr<function, Body>,
    bb: ptr<function, Body>,
) {
    let qa = normalize(quat_mul((*ba).dq, (*ba).rot));
    let qb = normalize(quat_mul((*bb).dq, (*bb).rot));
    let rA = joint_lever((*jn).a, qa, (*jn).anchor_a);
    let rB = joint_lever((*jn).b, qb, (*jn).anchor_b);
    var frame_a = normalize(quat_mul(qa, (*jn).frame_a_rotation));
    var frame_b = normalize(quat_mul(qb, (*jn).frame_b_rotation));
    if (dot(frame_a, frame_b) < 0.0) {
        frame_b = -frame_b;
    }
    // The Rust API also accepts an explicit local axis; native frames agree
    // with it. Project frame Y to retain the native basis in that usual case.
    let ax = normalize(quat_rotate(qa, (*jn).axis));
    let frame_y = quat_rotate(frame_a, vec3<f32>(0.0, 1.0, 0.0));
    var ay = frame_y - dot(frame_y, ax) * ax;
    if (dot(ay, ay) < 1e-12) {
        let frame_z = quat_rotate(frame_a, vec3<f32>(0.0, 0.0, 1.0));
        ay = frame_z - dot(frame_z, ax) * ax;
    }
    ay = normalize(ay);
    let az = cross(ax, ay);
    let d = ((*bb).pos + (*bb).dp + rB) - ((*ba).pos + (*ba).dp + rA);
    let sAx = cross(d + rA, ax);
    let sBx = cross(rB, ax);
    let sAy = cross(d + rA, ay);
    let sBy = cross(rB, ay);
    let sAz = cross(d + rA, az);
    let sBz = cross(rB, az);
    let translation = dot(ax, d);

    let i0 = world_inv_inertia(*ba, vec3<f32>(1.0, 0.0, 0.0))
        + world_inv_inertia(*bb, vec3<f32>(1.0, 0.0, 0.0));
    let i1 = world_inv_inertia(*ba, vec3<f32>(0.0, 1.0, 0.0))
        + world_inv_inertia(*bb, vec3<f32>(0.0, 1.0, 0.0));
    let i2 = world_inv_inertia(*ba, vec3<f32>(0.0, 0.0, 1.0))
        + world_inv_inertia(*bb, vec3<f32>(0.0, 0.0, 1.0));
    let fixed_rotation = abs(dot(i0, cross(i1, i2))) < 1.1754944e-35;
    if (params.use_bias == 2u) {
        let axial = (*jn).spring_impulse + (*jn).motor_impulse
            + (*jn).lower_impulse - (*jn).upper_impulse;
        let p = axial * ax + (*jn).perp_impulse.x * ay + (*jn).perp_impulse.y * az;
        let la = axial * sAx + (*jn).perp_impulse.x * sAy
            + (*jn).perp_impulse.y * sAz + (*jn).angular_impulse;
        let lb = axial * sBx + (*jn).perp_impulse.x * sBy
            + (*jn).perp_impulse.y * sBz + (*jn).angular_impulse;
        (*ba).vel -= (*ba).inv_mass * p;
        (*ba).omega -= world_inv_inertia(*ba, la);
        (*bb).vel += (*bb).inv_mass * p;
        (*bb).omega += world_inv_inertia(*bb, lb);
        return;
    }
    let k = (*ba).inv_mass + (*bb).inv_mass
        + dot(sAx, world_inv_inertia(*ba, sAx)) + dot(sBx, world_inv_inertia(*bb, sBx));
    let axial_mass = select(0.0, 1.0 / k, k > 0.0);
    // Match b3SolvePrismaticJoint: spring, motor, limits, rotation, point-to-line.
    if (((*jn).flags & PRISMATIC_ENABLE_SPRING) != 0u && !fixed_rotation) {
        let soft = spring_softness((*jn).spring_hertz, (*jn).spring_damping);
        let cdot = dot(ax, (*bb).vel - (*ba).vel)
            + dot(sBx, (*bb).omega) - dot(sAx, (*ba).omega);
        let delta = -soft.y * axial_mass
            * (cdot + soft.x * (translation - (*jn).target_translation))
            - soft.z * (*jn).spring_impulse;
        (*jn).spring_impulse += delta;
        apply_P(ba, rA + d, delta * ax, -1.0);
        apply_P(bb, rB, delta * ax, 1.0);
    }
    if (((*jn).flags & PRISMATIC_ENABLE_MOTOR) != 0u && !fixed_rotation) {
        let cdot = dot(ax, (*bb).vel - (*ba).vel)
            + dot(sBx, (*bb).omega) - dot(sAx, (*ba).omega) - (*jn).motor_speed;
        let old = (*jn).motor_impulse;
        let bound = params.dt * (*jn).max_motor_force;
        (*jn).motor_impulse = clamp(old - axial_mass * cdot, -bound, bound);
        let p = ((*jn).motor_impulse - old) * ax;
        apply_P(ba, rA + d, p, -1.0);
        apply_P(bb, rB, p, 1.0);
    }
    if (((*jn).flags & PRISMATIC_ENABLE_LIMIT) != 0u && !fixed_rotation) {
        let spec = 0.25 * ((*jn).upper_translation - (*jn).lower_translation);
        let soft = joint_softness((*jn).hertz, (*jn).damping);
        for (var side = 0u; side < 2u; side++) {
            let c = select(translation - (*jn).lower_translation,
                (*jn).upper_translation - translation, side == 1u);
            let old = select((*jn).lower_impulse, (*jn).upper_impulse, side == 1u);
            var accumulated = 0.0;
            if (c < spec) {
                var bias = 0.0;
                var ms = 1.0;
                var ips = 0.0;
                if (c > 0.0) { bias = c / params.dt; }
                else if (params.use_bias == 1u) { bias = soft.x * c; ms = soft.y; ips = soft.z; }
                let sign = select(1.0, -1.0, side == 1u);
                let cdot = sign * (dot(ax, (*bb).vel - (*ba).vel)
                    + dot(sBx, (*bb).omega) - dot(sAx, (*ba).omega));
                accumulated = max(0.0, old - ms * axial_mass * (cdot + bias) - ips * old);
                let p = sign * (accumulated - old) * ax;
                apply_P(ba, rA + d, p, -1.0);
                apply_P(bb, rB, p, 1.0);
            }
            if (side == 0u) { (*jn).lower_impulse = accumulated; }
            else { (*jn).upper_impulse = accumulated; }
        }
    }
    if (!fixed_rotation) {
        let rel = quat_mul(quat_inv(frame_a), frame_b);
        let soft = joint_softness((*jn).hertz, (*jn).damping);
        let c = -quat_rotate(frame_a,
            delta_quat_to_rotation(rel, vec4<f32>(0.0, 0.0, 0.0, 1.0)));
        let bias = select(vec3<f32>(0.0), soft.x * c, params.use_bias == 1u);
        let ms = select(1.0, soft.y, params.use_bias == 1u);
        let ips = select(0.0, soft.z, params.use_bias == 1u);
        let delta = -ms * solve3(i0, i1, i2, (*bb).omega - (*ba).omega + bias)
            - ips * (*jn).angular_impulse;
        (*jn).angular_impulse += delta;
        (*ba).omega -= world_inv_inertia(*ba, delta);
        (*bb).omega += world_inv_inertia(*bb, delta);
    }

    var bias = vec2<f32>(0.0);
    var mass_scale = 1.0;
    var impulse_scale = 0.0;
    if (params.use_bias == 1u) {
        let soft = joint_softness((*jn).hertz, (*jn).damping);
        bias = soft.x * vec2<f32>(dot(ay, d), dot(az, d));
        mass_scale = soft.y;
        impulse_scale = soft.z;
    }
    let vrel = ((*bb).vel + cross((*bb).omega, rB))
        - (*ba).vel - cross((*ba).omega, rA + d);
    let rhs = vec2<f32>(dot(ay, vrel), dot(az, vrel)) + bias;
    let kyy = (*ba).inv_mass + (*bb).inv_mass
        + dot(sAy, world_inv_inertia(*ba, sAy)) + dot(sBy, world_inv_inertia(*bb, sBy));
    let kyz = dot(sAy, world_inv_inertia(*ba, sAz)) + dot(sBy, world_inv_inertia(*bb, sBz));
    let kzz = (*ba).inv_mass + (*bb).inv_mass
        + dot(sAz, world_inv_inertia(*ba, sAz)) + dot(sBz, world_inv_inertia(*bb, sBz));
    let det = kyy * kzz - kyz * kyz;
    var solution = vec2<f32>(0.0);
    if (det > 1e-30) {
        solution = vec2<f32>(
            (kzz * rhs.x - kyz * rhs.y) / det,
            (kyy * rhs.y - kyz * rhs.x) / det,
        );
    }
    let delta = -mass_scale * solution - impulse_scale * (*jn).perp_impulse;
    (*jn).perp_impulse = (*jn).perp_impulse + delta;
    let p = delta.x * ay + delta.y * az;
    (*ba).vel = (*ba).vel - (*ba).inv_mass * p;
    (*ba).omega = (*ba).omega - world_inv_inertia(*ba, delta.x * sAy + delta.y * sAz);
    (*bb).vel = (*bb).vel + (*bb).inv_mass * p;
    (*bb).omega = (*bb).omega + world_inv_inertia(*bb, delta.x * sBy + delta.y * sBz);
}

fn solve_wheel(
    jn: ptr<function, Joint>,
    ba: ptr<function, Body>,
    bb: ptr<function, Body>,
) {
    let qa = normalize(quat_mul((*ba).dq, (*ba).rot));
    let qb = normalize(quat_mul((*bb).dq, (*bb).rot));
    let rA = joint_lever((*jn).a, qa, (*jn).anchor_a);
    let rB = joint_lever((*jn).b, qb, (*jn).anchor_b);
    var frame_a = normalize(quat_mul(qa, (*jn).frame_a_rotation));
    var frame_b = normalize(quat_mul(qb, (*jn).frame_b_rotation));
    if (dot(frame_a, frame_b) < 0.0) {
        frame_b = -frame_b;
    }
    let ax = quat_rotate(frame_a, vec3<f32>(1.0, 0.0, 0.0));
    let ay = quat_rotate(frame_a, vec3<f32>(0.0, 1.0, 0.0));
    let az = quat_rotate(frame_a, vec3<f32>(0.0, 0.0, 1.0));
    let bz = quat_rotate(frame_b, vec3<f32>(0.0, 0.0, 1.0));
    let d = ((*bb).pos + (*bb).dp + rB) - ((*ba).pos + (*ba).dp + rA);
    let sAx = cross(d + rA, ax);
    let sBx = cross(rB, ax);
    let sAy = cross(d + rA, ay);
    let sBy = cross(rB, ay);
    let sAz = cross(d + rA, az);
    let sBz = cross(rB, az);
    let translation = dot(ax, d);

    let cs = dot(bz, az);
    let ss = -dot(bz, ay);
    let steering_den = cs * cs + ss * ss;
    let steering_axis = select(
        vec3<f32>(0.0),
        cross(bz, -cs * ay - ss * az) / steering_den,
        steering_den > 0.0,
    );
    let inv_i_x = world_inv_inertia(*ba, vec3<f32>(1.0, 0.0, 0.0))
        + world_inv_inertia(*bb, vec3<f32>(1.0, 0.0, 0.0));
    let inv_i_y = world_inv_inertia(*ba, vec3<f32>(0.0, 1.0, 0.0))
        + world_inv_inertia(*bb, vec3<f32>(0.0, 1.0, 0.0));
    let inv_i_z = world_inv_inertia(*ba, vec3<f32>(0.0, 0.0, 1.0))
        + world_inv_inertia(*bb, vec3<f32>(0.0, 0.0, 1.0));
    let fixed_rotation =
        abs(dot(inv_i_x, cross(inv_i_y, inv_i_z))) < 1.1754944e-35;
    // Prepared once-per-step quantities use the base transforms (before dq),
    // matching b3PrepareWheelJoint while current Jacobians above follow TGS.
    let prepared_frame_a = normalize(quat_mul((*ba).rot, (*jn).frame_a_rotation));
    let prepared_frame_b = normalize(quat_mul((*bb).rot, (*jn).frame_b_rotation));
    let prepared_ax = quat_rotate(prepared_frame_a, vec3<f32>(1.0, 0.0, 0.0));
    let prepared_ay = quat_rotate(prepared_frame_a, vec3<f32>(0.0, 1.0, 0.0));
    let prepared_az = quat_rotate(prepared_frame_a, vec3<f32>(0.0, 0.0, 1.0));
    let prepared_bz = quat_rotate(prepared_frame_b, vec3<f32>(0.0, 0.0, 1.0));
    let prepared_rA = joint_lever((*jn).a, (*ba).rot, (*jn).anchor_a);
    let prepared_rB = joint_lever((*jn).b, (*bb).rot, (*jn).anchor_b);
    let prepared_sAx = cross(prepared_rA, prepared_ax);
    let prepared_sBx = cross(prepared_rB, prepared_ax);
    let suspension_k = (*ba).inv_mass + (*bb).inv_mass
        + dot(prepared_sAx, world_inv_inertia(*ba, prepared_sAx))
        + dot(prepared_sBx, world_inv_inertia(*bb, prepared_sBx));
    let suspension_mass = select(0.0, 1.0 / suspension_k, suspension_k > 0.0);
    let spin_k = dot(
        prepared_bz,
        world_inv_inertia(*ba, prepared_bz) + world_inv_inertia(*bb, prepared_bz),
    );
    let spin_mass = select(0.0, 1.0 / spin_k, spin_k > 0.0);
    let prepared_cs = dot(prepared_bz, prepared_az);
    let prepared_ss = -dot(prepared_bz, prepared_ay);
    let prepared_den = prepared_cs * prepared_cs + prepared_ss * prepared_ss;
    let prepared_steering_axis = select(
        vec3<f32>(0.0),
        cross(
            prepared_bz,
            -prepared_cs * prepared_ay - prepared_ss * prepared_az,
        ) / prepared_den,
        prepared_den > 0.0,
    );
    let steering_k = dot(
        prepared_steering_axis,
        world_inv_inertia(*ba, prepared_steering_axis)
            + world_inv_inertia(*bb, prepared_steering_axis),
    );
    let steering_mass = select(0.0, 1.0 / steering_k, steering_k > 0.0);

    if (params.use_bias == 2u) {
        let suspension_impulse = (*jn).spring_impulse + (*jn).lower_impulse - (*jn).upper_impulse;
        let linear_impulse =
            suspension_impulse * ax + (*jn).perp_impulse.x * ay + (*jn).perp_impulse.y * az;
        let angular_a =
            suspension_impulse * sAx + (*jn).perp_impulse.x * sAy + (*jn).perp_impulse.y * sAz;
        let angular_b =
            suspension_impulse * sBx + (*jn).perp_impulse.x * sBy + (*jn).perp_impulse.y * sBz;
        var angular_impulse = vec3<f32>(0.0);
        if (((*jn).flags & WHEEL_ENABLE_STEERING) != 0u) {
            let perpendicular_axis = cross(bz, ax);
            let steering_impulse =
                (*jn).weld_angular_impulse.x + (*jn).weld_angular_impulse.y - (*jn).weld_angular_impulse.z;
            angular_impulse =
                (*jn).angular_impulse.x * perpendicular_axis
                + (*jn).motor_impulse * bz
                + steering_impulse * steering_axis;
        } else {
            let rel = quat_mul(quat_inv(frame_a), frame_b);
            let perp_x = 0.5 * quat_rotate(
                frame_a,
                rel.w * vec3<f32>(1.0, 0.0, 0.0) + cross(rel.xyz, vec3<f32>(1.0, 0.0, 0.0)),
            );
            let perp_y = 0.5 * quat_rotate(
                frame_a,
                rel.w * vec3<f32>(0.0, 1.0, 0.0) + cross(rel.xyz, vec3<f32>(0.0, 1.0, 0.0)),
            );
            angular_impulse =
                (*jn).angular_impulse.x * perp_x
                + (*jn).angular_impulse.y * perp_y
                + (*jn).motor_impulse * bz;
        }
        (*ba).vel = (*ba).vel - (*ba).inv_mass * linear_impulse;
        (*ba).omega = (*ba).omega - world_inv_inertia(*ba, angular_a + angular_impulse);
        (*bb).vel = (*bb).vel + (*bb).inv_mass * linear_impulse;
        (*bb).omega = (*bb).omega + world_inv_inertia(*bb, angular_b + angular_impulse);
        return;
    }

    // Upstream order: spin, suspension spring, steering spring/limits,
    // suspension limits, collinearity, point-to-line.
    if (((*jn).flags & WHEEL_ENABLE_SPIN_MOTOR) != 0u && !fixed_rotation) {
        let old = (*jn).motor_impulse;
        let max_impulse = params.dt * (*jn).max_motor_force;
        let cdot = dot((*bb).omega - (*ba).omega, bz) - (*jn).motor_speed;
        (*jn).motor_impulse = clamp(old - spin_mass * cdot, -max_impulse, max_impulse);
        let impulse = (*jn).motor_impulse - old;
        (*ba).omega = (*ba).omega - impulse * world_inv_inertia(*ba, bz);
        (*bb).omega = (*bb).omega + impulse * world_inv_inertia(*bb, bz);
    }

    if (((*jn).flags & WHEEL_ENABLE_SUSPENSION_SPRING) != 0u) {
        let soft = spring_softness((*jn).spring_hertz, (*jn).spring_damping);
        let cdot = dot(ax, (*bb).vel - (*ba).vel) + dot(sBx, (*bb).omega) - dot(sAx, (*ba).omega);
        let impulse =
            -soft.y * suspension_mass * (cdot + soft.x * translation)
            - soft.z * (*jn).spring_impulse;
        (*jn).spring_impulse = (*jn).spring_impulse + impulse;
        let p = impulse * ax;
        (*ba).vel = (*ba).vel - (*ba).inv_mass * p;
        (*ba).omega = (*ba).omega - impulse * world_inv_inertia(*ba, sAx);
        (*bb).vel = (*bb).vel + (*bb).inv_mass * p;
        (*bb).omega = (*bb).omega + impulse * world_inv_inertia(*bb, sBx);
    }

    if (((*jn).flags & WHEEL_ENABLE_STEERING) != 0u && !fixed_rotation) {
        let steering_angle = atan2(ss, cs);
        let steering_soft = spring_softness((*jn).weld_linear_hertz, (*jn).weld_linear_damping);
        let old = (*jn).weld_angular_impulse.x;
        let cdot = dot(steering_axis, (*bb).omega - (*ba).omega);
        let max_impulse = params.dt * (*jn).target_rotation.y;
        (*jn).weld_angular_impulse.x = clamp(
            old - steering_soft.y * steering_mass
                * (cdot + steering_soft.x * (steering_angle - (*jn).target_rotation.x))
                - steering_soft.z * old,
            -max_impulse,
            max_impulse,
        );
        let steering_delta = (*jn).weld_angular_impulse.x - old;
        (*ba).omega = (*ba).omega - steering_delta * world_inv_inertia(*ba, steering_axis);
        (*bb).omega = (*bb).omega + steering_delta * world_inv_inertia(*bb, steering_axis);

        if (((*jn).flags & WHEEL_ENABLE_STEERING_LIMIT) != 0u) {
            let soft = joint_softness((*jn).hertz, (*jn).damping);
            let inv_h = 1.0 / max(params.dt, 1e-8);
            let lower_c = steering_angle - (*jn).target_rotation.z;
            var bias = select(0.0, lower_c * inv_h, lower_c > 0.0);
            var mass_scale = 1.0;
            var impulse_scale = 0.0;
            if (lower_c <= 0.0 && params.use_bias == 1u) {
                bias = soft.x * lower_c;
                mass_scale = soft.y;
                impulse_scale = soft.z;
            }
            let lower_old = (*jn).weld_angular_impulse.y;
            let lower_delta = -mass_scale * steering_mass
                * (dot(steering_axis, (*bb).omega - (*ba).omega) + bias)
                - impulse_scale * lower_old;
            (*jn).weld_angular_impulse.y = max(lower_old + lower_delta, 0.0);
            let lower_applied = (*jn).weld_angular_impulse.y - lower_old;
            (*ba).omega = (*ba).omega - lower_applied * world_inv_inertia(*ba, steering_axis);
            (*bb).omega = (*bb).omega + lower_applied * world_inv_inertia(*bb, steering_axis);

            let upper_c = (*jn).target_rotation.w - steering_angle;
            bias = select(0.0, upper_c * inv_h, upper_c > 0.0);
            mass_scale = 1.0;
            impulse_scale = 0.0;
            if (upper_c <= 0.0 && params.use_bias == 1u) {
                bias = soft.x * upper_c;
                mass_scale = soft.y;
                impulse_scale = soft.z;
            }
            let upper_old = (*jn).weld_angular_impulse.z;
            let upper_delta = -mass_scale * steering_mass
                * (dot(steering_axis, (*ba).omega - (*bb).omega) + bias)
                - impulse_scale * upper_old;
            (*jn).weld_angular_impulse.z = max(upper_old + upper_delta, 0.0);
            let upper_applied = (*jn).weld_angular_impulse.z - upper_old;
            (*ba).omega = (*ba).omega + upper_applied * world_inv_inertia(*ba, steering_axis);
            (*bb).omega = (*bb).omega - upper_applied * world_inv_inertia(*bb, steering_axis);
        }
    }

    if (((*jn).flags & WHEEL_ENABLE_SUSPENSION_LIMIT) != 0u) {
        let soft = joint_softness((*jn).hertz, (*jn).damping);
        let inv_h = 1.0 / max(params.dt, 1e-8);
        let lower_c = translation - (*jn).lower_translation;
        var bias = select(0.0, lower_c * inv_h, lower_c > 0.0);
        var mass_scale = 1.0;
        var impulse_scale = 0.0;
        if (lower_c <= 0.0 && params.use_bias == 1u) {
            bias = soft.x * lower_c;
            mass_scale = soft.y;
            impulse_scale = soft.z;
        }
        let lower_old = (*jn).lower_impulse;
        let lower_cdot =
            dot(ax, (*bb).vel - (*ba).vel) + dot(sBx, (*bb).omega) - dot(sAx, (*ba).omega);
        let lower_delta =
            -mass_scale * suspension_mass * (lower_cdot + bias) - impulse_scale * lower_old;
        (*jn).lower_impulse = max(lower_old + lower_delta, 0.0);
        let lower_applied = (*jn).lower_impulse - lower_old;
        (*ba).vel = (*ba).vel - (*ba).inv_mass * lower_applied * ax;
        (*ba).omega = (*ba).omega - lower_applied * world_inv_inertia(*ba, sAx);
        (*bb).vel = (*bb).vel + (*bb).inv_mass * lower_applied * ax;
        (*bb).omega = (*bb).omega + lower_applied * world_inv_inertia(*bb, sBx);

        let upper_c = (*jn).upper_translation - translation;
        bias = select(0.0, upper_c * inv_h, upper_c > 0.0);
        mass_scale = 1.0;
        impulse_scale = 0.0;
        if (upper_c <= 0.0 && params.use_bias == 1u) {
            bias = soft.x * upper_c;
            mass_scale = soft.y;
            impulse_scale = soft.z;
        }
        let upper_old = (*jn).upper_impulse;
        let upper_cdot =
            dot(ax, (*ba).vel - (*bb).vel) + dot(sAx, (*ba).omega) - dot(sBx, (*bb).omega);
        let upper_delta =
            -mass_scale * suspension_mass * (upper_cdot + bias) - impulse_scale * upper_old;
        (*jn).upper_impulse = max(upper_old + upper_delta, 0.0);
        let upper_applied = (*jn).upper_impulse - upper_old;
        (*ba).vel = (*ba).vel + (*ba).inv_mass * upper_applied * ax;
        (*ba).omega = (*ba).omega + upper_applied * world_inv_inertia(*ba, sAx);
        (*bb).vel = (*bb).vel - (*bb).inv_mass * upper_applied * ax;
        (*bb).omega = (*bb).omega - upper_applied * world_inv_inertia(*bb, sBx);
    }

    if (!fixed_rotation) {
        let soft = joint_softness((*jn).hertz, (*jn).damping);
        if (((*jn).flags & WHEEL_ENABLE_STEERING) != 0u) {
            let u = cross(bz, ax);
            let k = dot(u, world_inv_inertia(*ba, u) + world_inv_inertia(*bb, u));
            let mass = select(0.0, 1.0 / k, k > 0.0);
            let c = dot(ax, bz);
            let bias = select(0.0, soft.x * c, params.use_bias == 1u);
            let mass_scale = select(1.0, soft.y, params.use_bias == 1u);
            let impulse_scale = select(0.0, soft.z, params.use_bias == 1u);
            let delta = -mass_scale * mass * (dot((*bb).omega - (*ba).omega, u) + bias)
                - impulse_scale * (*jn).angular_impulse.x;
            (*jn).angular_impulse.x = (*jn).angular_impulse.x + delta;
            (*ba).omega = (*ba).omega - delta * world_inv_inertia(*ba, u);
            (*bb).omega = (*bb).omega + delta * world_inv_inertia(*bb, u);
        } else {
            let rel = quat_mul(quat_inv(frame_a), frame_b);
            let px = 0.5 * quat_rotate(
                frame_a,
                rel.w * vec3<f32>(1.0, 0.0, 0.0) + cross(rel.xyz, vec3<f32>(1.0, 0.0, 0.0)),
            );
            let py = 0.5 * quat_rotate(
                frame_a,
                rel.w * vec3<f32>(0.0, 1.0, 0.0) + cross(rel.xyz, vec3<f32>(0.0, 1.0, 0.0)),
            );
            let kxx = dot(px, world_inv_inertia(*ba, px) + world_inv_inertia(*bb, px));
            let kyy = dot(py, world_inv_inertia(*ba, py) + world_inv_inertia(*bb, py));
            let kxy = dot(px, world_inv_inertia(*ba, py) + world_inv_inertia(*bb, py));
            let det = kxx * kyy - kxy * kxy;
            var bias = vec2<f32>(0.0);
            var mass_scale = 1.0;
            var impulse_scale = 0.0;
            if (params.use_bias == 1u) {
                bias = soft.x * rel.xy;
                mass_scale = soft.y;
                impulse_scale = soft.z;
            }
            let wrel = (*bb).omega - (*ba).omega;
            let rhs = vec2<f32>(dot(wrel, px), dot(wrel, py)) + bias;
            var solution = vec2<f32>(0.0);
            if (det > 1e-30) {
                solution = vec2<f32>(
                    (kyy * rhs.x - kxy * rhs.y) / det,
                    (kxx * rhs.y - kxy * rhs.x) / det,
                );
            }
            let old = (*jn).angular_impulse.xy;
            let delta = -mass_scale * solution - impulse_scale * old;
            (*jn).angular_impulse.x = old.x + delta.x;
            (*jn).angular_impulse.y = old.y + delta.y;
            let impulse = delta.x * px + delta.y * py;
            (*ba).omega = (*ba).omega - world_inv_inertia(*ba, impulse);
            (*bb).omega = (*bb).omega + world_inv_inertia(*bb, impulse);
        }
    }

    var bias = vec2<f32>(0.0);
    var mass_scale = 1.0;
    var impulse_scale = 0.0;
    if (params.use_bias == 1u) {
        let soft = joint_softness((*jn).hertz, (*jn).damping);
        bias = soft.x * vec2<f32>(dot(ay, d), dot(az, d));
        mass_scale = soft.y;
        impulse_scale = soft.z;
    }
    let vrel = ((*bb).vel + cross((*bb).omega, rB))
        - (*ba).vel - cross((*ba).omega, rA + d);
    let rhs = vec2<f32>(dot(ay, vrel), dot(az, vrel)) + bias;
    let kyy = (*ba).inv_mass + (*bb).inv_mass
        + dot(sAy, world_inv_inertia(*ba, sAy)) + dot(sBy, world_inv_inertia(*bb, sBy));
    let kyz = dot(sAy, world_inv_inertia(*ba, sAz)) + dot(sBy, world_inv_inertia(*bb, sBz));
    let kzz = (*ba).inv_mass + (*bb).inv_mass
        + dot(sAz, world_inv_inertia(*ba, sAz)) + dot(sBz, world_inv_inertia(*bb, sBz));
    let det = kyy * kzz - kyz * kyz;
    var solution = vec2<f32>(0.0);
    if (det > 1e-30) {
        solution = vec2<f32>(
            (kzz * rhs.x - kyz * rhs.y) / det,
            (kyy * rhs.y - kyz * rhs.x) / det,
        );
    }
    let delta = -mass_scale * solution - impulse_scale * (*jn).perp_impulse;
    (*jn).perp_impulse = (*jn).perp_impulse + delta;
    let p = delta.x * ay + delta.y * az;
    (*ba).vel = (*ba).vel - (*ba).inv_mass * p;
    (*ba).omega = (*ba).omega - world_inv_inertia(*ba, delta.x * sAy + delta.y * sAz);
    (*bb).vel = (*bb).vel + (*bb).inv_mass * p;
    (*bb).omega = (*bb).omega + world_inv_inertia(*bb, delta.x * sBy + delta.y * sBz);
}

fn solve_spherical(
    jn: ptr<function, Joint>,
    ba: ptr<function, Body>,
    bb: ptr<function, Body>,
) {
    let base_frame_a = normalize(quat_mul((*ba).rot, (*jn).frame_a_rotation));
    let base_frame_b = normalize(quat_mul((*bb).rot, (*jn).frame_b_rotation));
    let cone_axis = quat_rotate(base_frame_a, vec3<f32>(0.0, 0.0, 1.0));
    let twist_axis = quat_rotate(base_frame_b, vec3<f32>(0.0, 0.0, 1.0));
    let swing_axis = normalize_or_zero(cross(cone_axis, twist_axis));
    let base_rel = quat_mul(quat_inv(base_frame_a), base_frame_b);
    let tangent_denominator = base_rel.z * base_rel.z + base_rel.w * base_rel.w;
    let tan_half_swing = select(
        0.0,
        sqrt((base_rel.x * base_rel.x + base_rel.y * base_rel.y) / tangent_denominator),
        tangent_denominator > 0.0,
    );
    let twist_jacobian =
        tan_half_swing * cone_axis + cross(swing_axis, cone_axis);

    let swing_k = dot(swing_axis, world_inv_inertia(*ba, swing_axis)
        + world_inv_inertia(*bb, swing_axis));
    let swing_mass = select(0.0, 1.0 / swing_k, swing_k > 0.0);
    let twist_k = dot(twist_jacobian, world_inv_inertia(*ba, twist_jacobian)
        + world_inv_inertia(*bb, twist_jacobian));
    let twist_mass = select(0.0, 1.0 / twist_k, twist_k > 0.0);

    let i0 = world_inv_inertia(*ba, vec3<f32>(1.0, 0.0, 0.0))
        + world_inv_inertia(*bb, vec3<f32>(1.0, 0.0, 0.0));
    let i1 = world_inv_inertia(*ba, vec3<f32>(0.0, 1.0, 0.0))
        + world_inv_inertia(*bb, vec3<f32>(0.0, 1.0, 0.0));
    let i2 = world_inv_inertia(*ba, vec3<f32>(0.0, 0.0, 1.0))
        + world_inv_inertia(*bb, vec3<f32>(0.0, 0.0, 1.0));
    let fixed_rotation = abs(dot(i0, cross(i1, i2))) < 1e-30;

    let qa = normalize(quat_mul((*ba).dq, (*ba).rot));
    let qb = normalize(quat_mul((*bb).dq, (*bb).rot));
    let rA = joint_lever((*jn).a, qa, (*jn).anchor_a);
    let rB = joint_lever((*jn).b, qb, (*jn).anchor_b);

    if (params.use_bias == 2u) {
        var angular_impulse =
            (*jn).spring_angular_impulse + (*jn).motor_angular_impulse;
        angular_impulse = angular_impulse
            - (*jn).swing_impulse * swing_axis
            + ((*jn).lower_impulse - (*jn).upper_impulse) * twist_jacobian;
        apply_P(ba, rA, (*jn).angular_impulse, -1.0);
        apply_P(bb, rB, (*jn).angular_impulse, 1.0);
        (*ba).omega = (*ba).omega - world_inv_inertia(*ba, angular_impulse);
        (*bb).omega = (*bb).omega + world_inv_inertia(*bb, angular_impulse);
        return;
    }

    let frame_a = normalize(quat_mul(qa, (*jn).frame_a_rotation));
    let frame_b = normalize(quat_mul(qb, (*jn).frame_b_rotation));
    let rel = quat_mul(quat_inv(frame_a), frame_b);

    // Box3D order: spring, motor, lower/upper twist, cone, point-to-point.
    if (((*jn).flags & SPHERICAL_ENABLE_SPRING) != 0u && !fixed_rotation) {
        let soft = spring_softness((*jn).spring_hertz, (*jn).spring_damping);
        let local_error = delta_quat_to_rotation(rel, (*jn).target_rotation);
        let c = -quat_rotate(frame_a, local_error);
        let cdot = (*bb).omega - (*ba).omega;
        let delta_impulse =
            -soft.y * rolling_mass_mul(*ba, *bb, cdot + soft.x * c)
            - soft.z * (*jn).spring_angular_impulse;
        (*jn).spring_angular_impulse =
            (*jn).spring_angular_impulse + delta_impulse;
        (*ba).omega = (*ba).omega - world_inv_inertia(*ba, delta_impulse);
        (*bb).omega = (*bb).omega + world_inv_inertia(*bb, delta_impulse);
    }

    if (((*jn).flags & SPHERICAL_ENABLE_MOTOR) != 0u && !fixed_rotation) {
        let cdot =
            ((*bb).omega - (*ba).omega) - (*jn).motor_angular_velocity;
        let delta = -rolling_mass_mul(*ba, *bb, cdot);
        let old_impulse = (*jn).motor_angular_impulse;
        (*jn).motor_angular_impulse = clamp_vector_length(
            old_impulse + delta,
            (*jn).max_motor_force * params.dt,
        );
        let applied = (*jn).motor_angular_impulse - old_impulse;
        (*ba).omega = (*ba).omega - world_inv_inertia(*ba, applied);
        (*bb).omega = (*bb).omega + world_inv_inertia(*bb, applied);
    }

    if (((*jn).flags & SPHERICAL_ENABLE_TWIST_LIMIT) != 0u && !fixed_rotation) {
        let angle = twist_angle(rel);
        let soft = joint_softness((*jn).hertz, (*jn).damping);
        let inv_h = 1.0 / max(params.dt, 1e-8);

        let lower_c = angle - (*jn).lower_translation;
        var lower_bias = 0.0;
        var lower_mass_scale = 1.0;
        var lower_impulse_scale = 0.0;
        if (lower_c > 0.0) {
            lower_bias = lower_c * inv_h;
        } else if (params.use_bias == 1u) {
            lower_bias = soft.x * lower_c;
            lower_mass_scale = soft.y;
            lower_impulse_scale = soft.z;
        }
        let lower_cdot =
            dot((*bb).omega - (*ba).omega, twist_jacobian);
        let old_lower = (*jn).lower_impulse;
        let lower_delta =
            -lower_mass_scale * twist_mass * (lower_cdot + lower_bias)
            - lower_impulse_scale * old_lower;
        (*jn).lower_impulse = max(old_lower + lower_delta, 0.0);
        let applied_lower = (*jn).lower_impulse - old_lower;
        (*ba).omega = (*ba).omega
            - applied_lower * world_inv_inertia(*ba, twist_jacobian);
        (*bb).omega = (*bb).omega
            + applied_lower * world_inv_inertia(*bb, twist_jacobian);

        let upper_c = (*jn).upper_translation - angle;
        var upper_bias = 0.0;
        var upper_mass_scale = 1.0;
        var upper_impulse_scale = 0.0;
        if (upper_c > 0.0) {
            upper_bias = upper_c * inv_h;
        } else if (params.use_bias == 1u) {
            upper_bias = soft.x * upper_c;
            upper_mass_scale = soft.y;
            upper_impulse_scale = soft.z;
        }
        let upper_cdot =
            dot((*ba).omega - (*bb).omega, twist_jacobian);
        let old_upper = (*jn).upper_impulse;
        let upper_delta =
            -upper_mass_scale * twist_mass * (upper_cdot + upper_bias)
            - upper_impulse_scale * old_upper;
        (*jn).upper_impulse = max(old_upper + upper_delta, 0.0);
        let applied_upper = (*jn).upper_impulse - old_upper;
        (*ba).omega = (*ba).omega
            + applied_upper * world_inv_inertia(*ba, twist_jacobian);
        (*bb).omega = (*bb).omega
            - applied_upper * world_inv_inertia(*bb, twist_jacobian);
    }

    if (((*jn).flags & SPHERICAL_ENABLE_CONE_LIMIT) != 0u && !fixed_rotation) {
        let c = (*jn).target_translation - swing_angle(rel);
        let soft = joint_softness((*jn).hertz, (*jn).damping);
        var bias = 0.0;
        var mass_scale = 1.0;
        var impulse_scale = 0.0;
        if (c > 0.0) {
            bias = c / max(params.dt, 1e-8);
        } else if (params.use_bias == 1u) {
            bias = soft.x * c;
            mass_scale = soft.y;
            impulse_scale = soft.z;
        }
        let cdot = dot((*ba).omega - (*bb).omega, swing_axis);
        let old_impulse = (*jn).swing_impulse;
        let delta =
            -mass_scale * swing_mass * (cdot + bias)
            - impulse_scale * old_impulse;
        (*jn).swing_impulse = max(old_impulse + delta, 0.0);
        let applied = (*jn).swing_impulse - old_impulse;
        (*ba).omega =
            (*ba).omega + applied * world_inv_inertia(*ba, swing_axis);
        (*bb).omega =
            (*bb).omega - applied * world_inv_inertia(*bb, swing_axis);
    }

    let cdot =
        ((*bb).vel + cross((*bb).omega, rB))
        - ((*ba).vel + cross((*ba).omega, rA));
    var bias = vec3<f32>(0.0);
    var mass_scale = 1.0;
    var impulse_scale = 0.0;
    if (params.use_bias == 1u) {
        let soft = joint_softness((*jn).hertz, (*jn).damping);
        let separation =
            ((*bb).pos + (*bb).dp + rB) - ((*ba).pos + (*ba).dp + rA);
        bias = soft.x * separation;
        mass_scale = soft.y;
        impulse_scale = soft.z;
    }
    let solution = motor_linear_mass_mul(*ba, *bb, rA, rB, cdot + bias);
    let impulse =
        -mass_scale * solution - impulse_scale * (*jn).angular_impulse;
    (*jn).angular_impulse = (*jn).angular_impulse + impulse;
    apply_P(ba, rA, impulse, -1.0);
    apply_P(bb, rB, impulse, 1.0);
}

fn solve_weld(
    jn: ptr<function, Joint>,
    ba: ptr<function, Body>,
    bb: ptr<function, Body>,
) {
    let qa = normalize(quat_mul((*ba).dq, (*ba).rot));
    let qb = normalize(quat_mul((*bb).dq, (*bb).rot));
    let rA = joint_lever((*jn).a, qa, (*jn).anchor_a);
    let rB = joint_lever((*jn).b, qb, (*jn).anchor_b);

    if (params.use_bias == 2u) {
        apply_P(ba, rA, (*jn).weld_linear_impulse, -1.0);
        apply_P(bb, rB, (*jn).weld_linear_impulse, 1.0);
        (*ba).omega = (*ba).omega
            - world_inv_inertia(*ba, (*jn).weld_angular_impulse);
        (*bb).omega = (*bb).omega
            + world_inv_inertia(*bb, (*jn).weld_angular_impulse);
        return;
    }

    let i0 = world_inv_inertia(*ba, vec3<f32>(1.0, 0.0, 0.0))
        + world_inv_inertia(*bb, vec3<f32>(1.0, 0.0, 0.0));
    let i1 = world_inv_inertia(*ba, vec3<f32>(0.0, 1.0, 0.0))
        + world_inv_inertia(*bb, vec3<f32>(0.0, 1.0, 0.0));
    let i2 = world_inv_inertia(*ba, vec3<f32>(0.0, 0.0, 1.0))
        + world_inv_inertia(*bb, vec3<f32>(0.0, 0.0, 1.0));
    let fixed_rotation = abs(dot(i0, cross(i1, i2))) < 1e-30;

    // Box3D weld order: angular constraint, then point-to-point constraint.
    if (!fixed_rotation) {
        let frame_a = normalize(quat_mul(qa, (*jn).frame_a_rotation));
        var frame_b = normalize(quat_mul(qb, (*jn).frame_b_rotation));
        if (dot(frame_a, frame_b) < 0.0) {
            frame_b = -frame_b;
        }
        let rel = quat_mul(quat_inv(frame_a), frame_b);
        let soft = select(
            joint_softness((*jn).hertz, (*jn).damping),
            spring_softness(
                (*jn).weld_angular_hertz,
                (*jn).weld_angular_damping,
            ),
            (*jn).weld_angular_hertz > 0.0,
        );
        var bias = vec3<f32>(0.0);
        var mass_scale = 1.0;
        var impulse_scale = 0.0;
        if (params.use_bias == 1u || (*jn).weld_angular_hertz > 0.0) {
            let local_error =
                delta_quat_to_rotation(rel, vec4<f32>(0.0, 0.0, 0.0, 1.0));
            bias = soft.x * -quat_rotate(frame_a, local_error);
            mass_scale = soft.y;
            impulse_scale = soft.z;
        }
        let cdot = (*bb).omega - (*ba).omega;
        let impulse =
            -mass_scale * solve3(i0, i1, i2, cdot + bias)
            - impulse_scale * (*jn).weld_angular_impulse;
        (*jn).weld_angular_impulse =
            (*jn).weld_angular_impulse + impulse;
        (*ba).omega = (*ba).omega - world_inv_inertia(*ba, impulse);
        (*bb).omega = (*bb).omega + world_inv_inertia(*bb, impulse);
    }

    let cdot =
        ((*bb).vel + cross((*bb).omega, rB))
        - ((*ba).vel + cross((*ba).omega, rA));
    let soft = select(
        joint_softness((*jn).hertz, (*jn).damping),
        spring_softness((*jn).weld_linear_hertz, (*jn).weld_linear_damping),
        (*jn).weld_linear_hertz > 0.0,
    );
    var bias = vec3<f32>(0.0);
    var mass_scale = 1.0;
    var impulse_scale = 0.0;
    if (params.use_bias == 1u || (*jn).weld_linear_hertz > 0.0) {
        let separation =
            ((*bb).pos + (*bb).dp + rB) - ((*ba).pos + (*ba).dp + rA);
        bias = soft.x * separation;
        mass_scale = soft.y;
        impulse_scale = soft.z;
    }
    let solution = motor_linear_mass_mul(*ba, *bb, rA, rB, cdot + bias);
    let impulse =
        -mass_scale * solution - impulse_scale * (*jn).weld_linear_impulse;
    (*jn).weld_linear_impulse = (*jn).weld_linear_impulse + impulse;
    apply_P(ba, rA, impulse, -1.0);
    apply_P(bb, rB, impulse, 1.0);
}

fn joint_writable_root(jn: Joint) -> u32 {
    var ra = EMPTY;
    var rb = EMPTY;
    let ba = load_body(jn.a);
    let bb = load_body(jn.b);
    if (joint_endpoint_writable(ba)) {
        ra = island_root(jn.a);
        if (ra == EMPTY) {
            ra = jn.a;
        }
    }
    if (joint_endpoint_writable(bb)) {
        rb = island_root(jn.b);
        if (rb == EMPTY) {
            rb = jn.b;
        }
    }
    if (ra == EMPTY) {
        return rb;
    }
    if (rb == EMPTY || ra == rb) {
        return ra;
    }
    return min(ra, rb);
}

fn joint_solver_serial() -> bool {
    return (params.diagnostic_flags & DIAG_SERIAL_JOINTS) != 0u;
}

@compute @workgroup_size(64)
fn solve_joints(
    @builtin(global_invocation_id) gid: vec3<u32>,
    @builtin(local_invocation_index) lid: u32,
) {
    let serial = joint_solver_serial();
    let list_ok = scratch[SCR_JOINT_LIST_OK] == 1u;
    if (serial && lid != 0u) {
        return;
    }
    let ncomp = scratch[SCR_JOINT_COMP_N];
    var use_list = !serial && list_ok;
    var n = params.joint_count;
    var list_off = 0u;
    if (use_list) {
        if (gid.x >= ncomp) {
            return;
        }
        list_off = scratch[joint_list_off_base() + gid.x];
        n = scratch[joint_list_count_base() + gid.x];
    } else if (!serial) {
        // Capacity/list failure must not drop joints: one lane solves all.
        if (lid != 0u) {
            return;
        }
    }
    let h = params.dt;
    for (var k = 0u; k < n; k++) {
        var i = k;
        if (use_list) {
            i = scratch[joint_list_base() + list_off + k];
        }
        var jn = joints[i];
        if (jn.kind == JOINT_NONE || jn.kind == JOINT_FILTER) {
            continue;
        }
        if (params.use_bias == 2u
            && jn.kind != JOINT_REVOLUTE
            && jn.kind != JOINT_SPHERICAL
            && jn.kind != JOINT_WELD
            && jn.kind != JOINT_WHEEL
            && jn.kind != JOINT_PRISMATIC
            && jn.kind != JOINT_DISTANCE
            && jn.kind != JOINT_PARALLEL
            && jn.kind != JOINT_MOTOR) {
            continue;
        }
        var ba = load_body(jn.a);
        var bb = load_body(jn.b);
        let qa = normalize(quat_mul(ba.dq, ba.rot));
        let qb = normalize(quat_mul(bb.dq, bb.rot));
        let rA = joint_lever(jn.a, qa, jn.anchor_a);
        let rB = joint_lever(jn.b, qb, jn.anchor_b);
        let pa = ba.pos + ba.dp + rA;
        let pb = bb.pos + bb.dp + rB;
        var err = pb - pa;
        if (jn.kind == JOINT_REVOLUTE) {
            solve_revolute(&jn, &ba, &bb);
            err = vec3<f32>(0.0);
        } else if (jn.kind == JOINT_WHEEL) {
            solve_wheel(&jn, &ba, &bb);
            err = vec3<f32>(0.0);
        } else if (jn.kind == JOINT_WELD) {
            solve_weld(&jn, &ba, &bb);
            err = vec3<f32>(0.0);
        } else if (jn.kind == JOINT_SPHERICAL) {
            solve_spherical(&jn, &ba, &bb);
            err = vec3<f32>(0.0);
        } else if (jn.kind == JOINT_MOTOR) {
            // The warm-start wave reapplies the cached impulse to this substep's
            // velocities. Solving a new delta here loses sustained support force.
            if (params.use_bias == 2u) {
                let linear = vec3<f32>(jn.impulse, jn.perp_impulse)
                    + vec3<f32>(jn.spring_impulse, jn.lower_impulse, jn.upper_impulse);
                let angular = jn.angular_impulse
                    + vec3<f32>(jn.motor_impulse, jn._pad2);
                apply_P(&ba, rA, linear, -1.0);
                apply_P(&bb, rB, linear, 1.0);
                apply_joint_torque(&ba, -angular);
                apply_joint_torque(&bb, angular);
                if (joint_endpoint_writable(ba)) { store_body(jn.a, ba); }
                if (joint_endpoint_writable(bb)) { store_body(jn.b, bb); }
                continue;
            }
            let frame_a = normalize(quat_mul(qa, jn.frame_a_rotation));
            var frame_b = normalize(quat_mul(qb, jn.frame_b_rotation));
            if (dot(frame_a, frame_b) < 0.0) {
                frame_b = -frame_b;
            }
            let rel = quat_mul(quat_inv(frame_a), frame_b);
            var angular_spring_impulse =
                vec3<f32>(jn.motor_impulse, jn._pad2.x, jn._pad2.y);
            if (jn.max_motor_force > 0.0 && jn.spring_hertz > 0.0) {
                let omega = 6.2831853 * jn.spring_hertz;
                let a1 = 2.0 * jn.spring_damping + h * omega;
                let a2 = h * omega * a1;
                let a3 = 1.0 / (1.0 + a2);
                let bias_rate = omega / max(a1, 1e-8);
                let mass_scale = a2 * a3;
                let impulse_scale = a3;
                let rotation_error = 2.0 * quat_rotate(frame_a, rel.xyz);
                let cdot = bb.omega - ba.omega;
                let old = angular_spring_impulse;
                let delta = -mass_scale
                    * rolling_mass_mul(ba, bb, cdot + bias_rate * rotation_error)
                    - impulse_scale * old;
                angular_spring_impulse =
                    clamp_vector_length(old + delta, h * jn.max_motor_force);
                let impulse = angular_spring_impulse - old;
                apply_joint_torque(&ba, -impulse);
                apply_joint_torque(&bb, impulse);
            }

            if (jn.lower_translation > 0.0) {
                let cdot = bb.omega - ba.omega - jn.motor_angular_velocity;
                let old = jn.angular_impulse;
                jn.angular_impulse = clamp_vector_length(
                    old - rolling_mass_mul(ba, bb, cdot),
                    h * jn.lower_translation,
                );
                let impulse = jn.angular_impulse - old;
                apply_joint_torque(&ba, -impulse);
                apply_joint_torque(&bb, impulse);
            }

            var linear_spring_impulse =
                vec3<f32>(jn.spring_impulse, jn.lower_impulse, jn.upper_impulse);
            if (jn.upper_translation > 0.0 && jn.hertz > 0.0) {
                let omega = 6.2831853 * jn.hertz;
                let a1 = 2.0 * jn.damping + h * omega;
                let a2 = h * omega * a1;
                let a3 = 1.0 / (1.0 + a2);
                let bias_rate = omega / max(a1, 1e-8);
                let mass_scale = a2 * a3;
                let impulse_scale = a3;
                let cdot =
                    (bb.vel + cross(bb.omega, rB)) - (ba.vel + cross(ba.omega, rA));
                let old = linear_spring_impulse;
                let delta = -mass_scale
                    * motor_linear_mass_mul(ba, bb, rA, rB, cdot + bias_rate * err)
                    - impulse_scale * old;
                linear_spring_impulse =
                    clamp_vector_length(old + delta, h * jn.upper_translation);
                let impulse = linear_spring_impulse - old;
                apply_P(&ba, rA, impulse, -1.0);
                apply_P(&bb, rB, impulse, 1.0);
            }

            var linear_velocity_impulse =
                vec3<f32>(jn.impulse, jn.perp_impulse.x, jn.perp_impulse.y);
            if (jn.target_translation > 0.0) {
                let cdot =
                    (bb.vel + cross(bb.omega, rB)) - (ba.vel + cross(ba.omega, rA))
                    - jn.axis;
                let old = linear_velocity_impulse;
                linear_velocity_impulse = clamp_vector_length(
                    old - motor_linear_mass_mul(ba, bb, rA, rB, cdot),
                    h * jn.target_translation,
                );
                let impulse = linear_velocity_impulse - old;
                apply_P(&ba, rA, impulse, -1.0);
                apply_P(&bb, rB, impulse, 1.0);
            }

            jn.impulse = linear_velocity_impulse.x;
            jn.perp_impulse = linear_velocity_impulse.yz;
            jn.spring_impulse = linear_spring_impulse.x;
            jn.lower_impulse = linear_spring_impulse.y;
            jn.upper_impulse = linear_spring_impulse.z;
            jn.motor_impulse = angular_spring_impulse.x;
            jn._pad2 = angular_spring_impulse.yz;
            err = vec3<f32>(0.0);
        } else if (jn.kind == JOINT_PARALLEL) {
            let frame_a = normalize(quat_mul(qa, jn.frame_a_rotation));
            var frame_b = normalize(quat_mul(qb, jn.frame_b_rotation));
            if (dot(frame_a, frame_b) < 0.0) {
                frame_b = -frame_b;
            }
            let rel = quat_mul(quat_inv(frame_a), frame_b);
            let axis_x = 0.5 * quat_rotate(
                frame_a,
                rel.w * vec3<f32>(1.0, 0.0, 0.0)
                    + cross(rel.xyz, vec3<f32>(1.0, 0.0, 0.0)),
            );
            let axis_y = 0.5 * quat_rotate(
                frame_a,
                rel.w * vec3<f32>(0.0, 1.0, 0.0)
                    + cross(rel.xyz, vec3<f32>(0.0, 1.0, 0.0)),
            );
            if (jn.max_motor_force > 0.0 || jn.spring_hertz > 0.0) {
                let kxx =
                    dot(axis_x, world_inv_inertia(ba, axis_x) + world_inv_inertia(bb, axis_x));
                let kyy =
                    dot(axis_y, world_inv_inertia(ba, axis_y) + world_inv_inertia(bb, axis_y));
                let kxy =
                    dot(axis_x, world_inv_inertia(ba, axis_y) + world_inv_inertia(bb, axis_y));
                let det = kxx * kyy - kxy * kxy;
                if (abs(det) > 1e-12) {
                    let omega = 6.2831853 * jn.spring_hertz;
                    let a1 = 2.0 * jn.spring_damping + h * omega;
                    let a2 = h * omega * a1;
                    let a3 = 1.0 / (1.0 + a2);
                    let bias_rate = select(0.0, omega / max(a1, 1e-8), jn.spring_hertz > 0.0);
                    let mass_scale = select(1.0, a2 * a3, params.use_bias == 1u);
                    let impulse_scale = select(0.0, a3, params.use_bias == 1u);
                    let wrel = bb.omega - ba.omega;
                    let rhs = vec2<f32>(
                        dot(wrel, axis_x) + bias_rate * rel.x,
                        dot(wrel, axis_y) + bias_rate * rel.y,
                    );
                    let solution = vec2<f32>(
                        (kyy * rhs.x - kxy * rhs.y) / det,
                        (kxx * rhs.y - kxy * rhs.x) / det,
                    );
                    let old = jn.perp_impulse;
                    jn.perp_impulse =
                        old - mass_scale * solution - impulse_scale * old;
                    let max_impulse = h * jn.max_motor_force;
                    let impulse_length = length(jn.perp_impulse);
                    if (impulse_length > max_impulse) {
                        jn.perp_impulse = jn.perp_impulse * (max_impulse / impulse_length);
                    }
                    let delta = jn.perp_impulse - old;
                    let angular_impulse = delta.x * axis_x + delta.y * axis_y;
                    apply_joint_torque(&ba, -angular_impulse);
                    apply_joint_torque(&bb, angular_impulse);
                }
            }
            if (params.use_bias == 0u) {
                jn.perp_impulse = vec2<f32>(0.0);
            }
            err = vec3<f32>(0.0);
        } else if (jn.kind == JOINT_DISTANCE) {
            let distance = length(err);
            var axis = vec3<f32>(1.0, 0.0, 0.0);
            if (distance > 1e-8) {
                axis = err / distance;
            }
            let axial_mass = normal_mass(ba, bb, rA, rB, axis);
            let enable_spring = (jn.flags & DISTANCE_ENABLE_SPRING) != 0u;
            let enable_limit = (jn.flags & DISTANCE_ENABLE_LIMIT) != 0u;
            let soft_distance =
                enable_spring && (jn.lower_translation < jn.upper_translation || !enable_limit);
            if (soft_distance) {
                if (jn.spring_hertz > 0.0) {
                    let omega = 6.2831853 * jn.spring_hertz;
                    let a1 = 2.0 * max(jn.spring_damping, 0.0) + h * omega;
                    let a2 = h * omega * a1;
                    let a3 = 1.0 / (1.0 + a2);
                    let bias_rate = omega / max(a1, 1e-8);
                    let mass_scale = select(1.0, a2 * a3, params.use_bias == 1u);
                    let impulse_scale = select(0.0, a3, params.use_bias == 1u);
                    let vrel =
                        (bb.vel + cross(bb.omega, rB)) - (ba.vel + cross(ba.omega, rA));
                    let cdot = dot(vrel, axis);
                    let bias = select(
                        0.0,
                        bias_rate * (distance - jn.target_translation),
                        params.use_bias == 1u,
                    );
                    let dj =
                        -mass_scale * axial_mass * (cdot + bias)
                        - impulse_scale * jn.spring_impulse;
                    let old = jn.spring_impulse;
                    jn.spring_impulse = clamp(
                        old + dj,
                        h * jn.perp_impulse.x,
                        h * jn.perp_impulse.y,
                    );
                    let P = axis * (jn.spring_impulse - old);
                    apply_P(&ba, rA, P, -1.0);
                    apply_P(&bb, rB, P, 1.0);
                }
                if (enable_limit) {
                    let inv_h = 1.0 / max(h, 1e-8);
                    let softness = contact_softness(is_immovable(ba) || is_immovable(bb));
                    let vrel =
                        (bb.vel + cross(bb.omega, rB)) - (ba.vel + cross(ba.omega, rA));

                    let lower_c = distance - jn.lower_translation;
                    let lower_bias = select(
                        lower_c * inv_h,
                        softness.x * lower_c,
                        lower_c <= 0.0 && params.use_bias == 1u,
                    );
                    let old_lower = jn.lower_impulse;
                    let lower_dj = -axial_mass * (dot(vrel, axis) + lower_bias);
                    jn.lower_impulse = max(old_lower + lower_dj, 0.0);
                    let lower_p = axis * (jn.lower_impulse - old_lower);
                    apply_P(&ba, rA, lower_p, -1.0);
                    apply_P(&bb, rB, lower_p, 1.0);

                    let upper_c = jn.upper_translation - distance;
                    let upper_bias = select(
                        upper_c * inv_h,
                        softness.x * upper_c,
                        upper_c <= 0.0 && params.use_bias == 1u,
                    );
                    let old_upper = jn.upper_impulse;
                    let upper_dj = -axial_mass * (-dot(vrel, axis) + upper_bias);
                    jn.upper_impulse = max(old_upper + upper_dj, 0.0);
                    let upper_p = -axis * (jn.upper_impulse - old_upper);
                    apply_P(&ba, rA, upper_p, -1.0);
                    apply_P(&bb, rB, upper_p, 1.0);
                }
                if ((jn.flags & DISTANCE_ENABLE_MOTOR) != 0u && jn.max_motor_force > 0.0) {
                    let vrel =
                        (bb.vel + cross(bb.omega, rB)) - (ba.vel + cross(ba.omega, rA));
                    let dj = axial_mass * (jn.motor_speed - dot(vrel, axis));
                    let old = jn.motor_impulse;
                    let max_impulse = h * jn.max_motor_force;
                    jn.motor_impulse = clamp(old + dj, -max_impulse, max_impulse);
                    let P = axis * (jn.motor_impulse - old);
                    apply_P(&ba, rA, P, -1.0);
                    apply_P(&bb, rB, P, 1.0);
                }
            } else {
                let omega = 6.2831853 * max(jn.hertz, 1.0);
                let a1 = 2.0 * max(jn.damping, 0.25) + h * omega;
                let a2 = h * omega * a1;
                let a3 = 1.0 / (1.0 + a2);
                let mass_scale = select(1.0, a2 * a3, params.use_bias == 1u);
                let impulse_scale = select(0.0, a3, params.use_bias == 1u);
                let bias = select(
                    0.0,
                    omega / max(a1, 1e-8) * (distance - jn.target_translation),
                    params.use_bias == 1u,
                );
                let vrel =
                    (bb.vel + cross(bb.omega, rB)) - (ba.vel + cross(ba.omega, rA));
                let dj =
                    -mass_scale * axial_mass * (dot(vrel, axis) + bias)
                    - impulse_scale * jn.impulse;
                jn.impulse = jn.impulse + dj;
                let P = axis * dj;
                apply_P(&ba, rA, P, -1.0);
                apply_P(&bb, rB, P, 1.0);
            }
            if (params.use_bias == 0u) {
                jn.impulse = 0.0;
                jn.spring_impulse = 0.0;
                jn.lower_impulse = 0.0;
                jn.upper_impulse = 0.0;
                jn.motor_impulse = 0.0;
            }
            err = vec3<f32>(0.0);
        } else if (jn.kind == JOINT_PRISMATIC) {
            solve_prismatic(&jn, &ba, &bb);
            err = vec3<f32>(0.0);
        }
        let nlen = length(err);
        if (nlen > 1e-8) {
            let n = err / nlen;
            let kmass = max(body_inv_mass(ba) + body_inv_mass(bb), 1e-8);
            let soft = contact_softness(is_immovable(ba) || is_immovable(bb));
            let hertz = max(jn.hertz, 1.0);
            let omega = 6.2831853 * hertz;
            let zeta = max(jn.damping, 0.25);
            let a1 = 2.0 * zeta + h * omega;
            let a2 = h * omega * a1;
            let a3 = 1.0 / (1.0 + a2);
            let bias_rate = omega / a1;
            var mass_scale = a2 * a3;
            var impulse_scale = a3;
            if (params.use_bias == 0u) {
                mass_scale = 1.0;
                impulse_scale = 0.0;
            }
            let vn = dot((bb.vel + cross(bb.omega, rB)) - (ba.vel + cross(ba.omega, rA)), n);
            let c = nlen;
            let bias = select(0.0, bias_rate * c, params.use_bias == 1u);
            let nm = 1.0 / kmass;
            let dj = -mass_scale * nm * (vn + bias) - impulse_scale * jn.impulse;
            jn.impulse = jn.impulse + dj;
            let P = n * dj;
            apply_P(&ba, rA, P, -1.0);
            apply_P(&bb, rB, P, 1.0);
            if (params.use_bias == 1u) {
                let corr = n * c * 0.2;
                if (!is_immovable(ba)) {
                    ba.dp = ba.dp + corr * (body_inv_mass(ba) / kmass);
                }
                if (!is_immovable(bb)) {
                    bb.dp = bb.dp - corr * (body_inv_mass(bb) / kmass);
                }
            }
        }
        if (joint_endpoint_writable(ba)) {
            store_body(jn.a, ba);
        }
        if (joint_endpoint_writable(bb)) {
            store_body(jn.b, bb);
        }
        joints[i] = jn;
    }
}

@compute @workgroup_size(64)
fn compact_joint_heads(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    if (i < params.body_count) {
        scratch[joint_head_base() + i] = EMPTY;
    }
    if (gid.x == 0u) {
        scratch[SCR_JOINT_COMP_N] = 0u;
        scratch[SCR_JOINT_LIST_OK] = 0u;
    }
}

@compute @workgroup_size(64)
fn compact_joint_components(@builtin(local_invocation_index) lid: u32) {
    if (lid != 0u) {
        return;
    }
    var ncomp = 0u;
    let head = joint_head_base();
    let unique = joint_comp_base();
    let offb = joint_list_off_base();
    let cntb = joint_list_count_base();
    let listb = joint_list_base();
    for (var i = 0u; i < params.joint_count; i++) {
        let jn = joints[i];
        if (jn.kind == JOINT_NONE || jn.kind == JOINT_FILTER) {
            continue;
        }
        let root = joint_writable_root(jn);
        if (root == EMPTY || root >= params.body_count) {
            continue;
        }
        if (scratch[head + root] == EMPTY) {
            scratch[unique + ncomp] = root;
            scratch[head + root] = ncomp;
            ncomp = ncomp + 1u;
        }
    }
    scratch[SCR_JOINT_COMP_N] = ncomp;
    for (var c = 0u; c < ncomp; c++) {
        scratch[cntb + c] = 0u;
    }
    for (var i = 0u; i < params.joint_count; i++) {
        let jn = joints[i];
        if (jn.kind == JOINT_NONE || jn.kind == JOINT_FILTER) {
            continue;
        }
        let root = joint_writable_root(jn);
        if (root == EMPTY || root >= params.body_count) {
            continue;
        }
        let c = scratch[head + root];
        scratch[cntb + c] = scratch[cntb + c] + 1u;
    }
    var acc = 0u;
    for (var c = 0u; c < ncomp; c++) {
        scratch[offb + c] = acc;
        acc = acc + scratch[cntb + c];
        scratch[cntb + c] = 0u;
    }
    for (var i = 0u; i < params.joint_count; i++) {
        let jn = joints[i];
        if (jn.kind == JOINT_NONE || jn.kind == JOINT_FILTER) {
            continue;
        }
        let root = joint_writable_root(jn);
        if (root == EMPTY || root >= params.body_count) {
            continue;
        }
        let c = scratch[head + root];
        let w = scratch[offb + c] + scratch[cntb + c];
        scratch[listb + w] = i;
        scratch[cntb + c] = scratch[cntb + c] + 1u;
    }
    scratch[SCR_JOINT_LIST_OK] = 1u;
}
