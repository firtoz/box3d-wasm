// One invocation owns one ordinary dynamic body. Eligible target bodies are
// static/kinematic and never written by this pass, so no cross-body write race.
struct CcdState { p:vec4<f32>, v:vec4<f32>, q:vec4<f32>, w:vec4<f32>, dp:vec4<f32>, dq:vec4<f32>, origin:vec3<f32>, origin_valid:u32 }
struct CcdBody { first:u32, count:u32, min_extent:f32, max_extent:f32, center:vec4<f32> }
struct CcdShape { proxy:CcdProxy, body:u32, group:i32, category:vec2<u32>, mask:vec2<u32>, sweep_radius:f32, unused:u32 }
struct CcdConfig { bodies:u32, targets_first:u32, targets_count:u32, joints_first:u32, joints_count:u32, unused:vec3<u32> }
@group(0) @binding(0) var<storage,read> ccd_start:array<CcdState>;
@group(0) @binding(1) var<storage,read_write> ccd_finish:array<CcdState>;
@group(0) @binding(2) var<storage,read> ccd_points:array<vec4<f32>>;
@group(0) @binding(3) var<storage,read> ccd_shapes:array<CcdShape>;
@group(0) @binding(4) var<storage,read> ccd_bodies:array<CcdBody>;
@group(0) @binding(5) var<storage,read> ccd_indices:array<u32>;
@group(0) @binding(6) var<uniform> ccd_config:CcdConfig;
fn ccd_origin(state:CcdState,center:vec3<f32>) -> CcdTransform {
    if (state.origin_valid != 0u) { return CcdTransform(vec4<f32>(state.origin,0.0),state.q); }
    return CcdTransform(vec4<f32>(state.p.xyz-ccd_rotate(state.q,center),0.0),state.q);
}
fn ccd_filter(a:CcdShape,b:CcdShape) -> bool {
    if (a.group!=0 && a.group==b.group) { return a.group>0; }
    return any((a.category & b.mask)!=vec2<u32>(0u)) && any((b.category & a.mask)!=vec2<u32>(0u));
}
fn ccd_joint_allows(a:u32,b:u32) -> bool {
    for (var i=0u;i<ccd_config.joints_count;i++) {
        let first=ccd_config.joints_first+2u*i;
        let x=ccd_indices[first]; let y=ccd_indices[first+1u];
        if ((a==x && b==y) || (a==y && b==x)) { return false; }
    }
    return true;
}
fn ccd_bounds_overlap(a0:CcdTransform,a1:CcdTransform,ra:f32,b0:CcdTransform,b1:CcdTransform,rb:f32) -> bool {
    let alo=min(a0.p.xyz,a1.p.xyz)-vec3<f32>(ra); let ahi=max(a0.p.xyz,a1.p.xyz)+vec3<f32>(ra);
    let blo=min(b0.p.xyz,b1.p.xyz)-vec3<f32>(rb); let bhi=max(b0.p.xyz,b1.p.xyz)+vec3<f32>(rb);
    return all(alo<=bhi) && all(blo<=ahi);
}
@compute @workgroup_size(64)
fn ccd_correct(@builtin(global_invocation_id) id:vec3<u32>) {
    let body=id.x; if (body>=ccd_config.bodies) { return; }
    let end=ccd_finish[body]; let flags=bitcast<u32>(end.v.w);
    if ((flags & (1u|4u|16u|32u|128u))!=0u) { return; }
    let begin=ccd_start[body]; let body_info=ccd_bodies[body];
    let motion=distance(begin.p.xyz,end.p.xyz)+2.0*acos(clamp(abs(dot(begin.q,end.q)),0.0,1.0))*body_info.max_extent;
    // Motion beyond the speculative shell needs a sweep even for large bodies.
    if (motion<=min(0.5*body_info.min_extent,0.02)) { return; }
    let b0=ccd_origin(begin,body_info.center.xyz); let b1=ccd_origin(end,body_info.center.xyz);
    var fraction=1.0;
    for (var own=0u;own<body_info.count;own++) {
        let b=ccd_shapes[ccd_indices[body_info.first+own]];
        for (var t=0u;t<ccd_config.targets_count;t++) {
            let a=ccd_shapes[ccd_indices[ccd_config.targets_first+t]];
            if (a.body==body || !ccd_filter(a,b) || !ccd_joint_allows(a.body,body)) { continue; }
            let a0=ccd_origin(ccd_start[a.body],ccd_bodies[a.body].center.xyz);
            let a1=ccd_origin(ccd_finish[a.body],ccd_bodies[a.body].center.xyz);
            if (!ccd_bounds_overlap(a0,a1,a.sweep_radius,b0,b1,b.sweep_radius)) { continue; }
            let hit=ccd_sweep(a.proxy,a0,a1,b.proxy,b0,b1,fraction,512u);
            if (hit.status!=0u) { fraction=hit.fraction; }
        }
    }
    if (fraction<1.0) {
        // Interpolate COM as the CPU correction does, not the origin path.
        let corrected=ccd_interpolate(CcdTransform(begin.p,begin.q),CcdTransform(end.p,end.q),fraction);
        ccd_finish[body].p=vec4<f32>(corrected.p.xyz,end.p.w);
        ccd_finish[body].q=corrected.q;
        ccd_finish[body].origin_valid=0u;
        ccd_finish[body].v.w=bitcast<f32>(flags & ~(4u | 65536u));
        ccd_finish[body].dp.w=0.0;
    }
}
