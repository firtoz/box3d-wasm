struct Camera { view_proj: mat4x4<f32> }
struct State { p: vec4<f32>, v: vec4<f32>, q: vec4<f32>, w: vec4<f32>, dp: vec4<f32>, dq: vec4<f32>, origin: vec3<f32>, origin_valid: u32 }
struct Cold { a: vec4<f32>, b: vec4<f32>, c: vec4<f32>, d: vec4<f32> }
@group(0) @binding(0) var<uniform> camera: Camera;
@group(1) @binding(0) var<storage, read> states: array<State>;
@group(1) @binding(1) var<storage, read> cold: array<Cold>;
fn rotate(q: vec4<f32>, p: vec3<f32>) -> vec3<f32> {
    return p + 2.0 * cross(q.xyz, cross(q.xyz, p) + q.w * p);
}
struct Out { @builtin(position) clip: vec4<f32>, @location(0) normal: vec3<f32>, @location(1) color: vec3<f32> }
@vertex fn vs_main(@location(0) p: vec3<f32>, @location(1) n: vec3<f32>,
    @location(2) packed_body: u32, @location(3) color: u32, @location(4) local_p: vec3<f32>,
    @location(5) local_q: vec4<f32>, @location(6) scale: vec3<f32>) -> Out {
    var out: Out;
    let body = packed_body & 0x7fffffffu;
    let state = states[body];
    let point = local_p + rotate(local_q, p * scale);
    let origin_relative = point - cold[body].d.yzw;
    var world_point = state.p.xyz + rotate(state.q, origin_relative);
    if (state.origin_valid != 0u) { world_point = state.origin + rotate(state.q, point); }
    out.clip = camera.view_proj * vec4<f32>(world_point, 1.0);
    out.normal = normalize(rotate(state.q, rotate(local_q, n / scale)));
    // Box3D physics_world.c debug colour precedence, using live GPU state.
    let flags = bitcast<u32>(state.v.w);
    let asleep = (flags & 4u) != 0u;
    var rgb = color;
    if (color == 0u) {
        if ((flags & 17u) == 0u && state.p.w == 0.0) { rgb = 0xff0000u; }
        else if ((flags & 128u) != 0u) { rgb = 0x708090u; }
        else if ((packed_body & 0x80000000u) != 0u) { rgb = 0xf5deb3u; }
        else if ((flags & 32u) != 0u && !asleep) { rgb = 0x40e0d0u; }
        else if ((flags & 32768u) != 0u) { rgb = 0xffa500u; }
        else if ((flags & 1u) != 0u) { rgb = 0xa9a9a9u; }
        else if ((flags & 16u) != 0u) { rgb = select(0x4682b4u, 0xb0c4deu, asleep); }
        else { rgb = select(0xd2b48cu, 0x778899u, asleep); }
    }
    out.color = vec3<f32>(f32((rgb >> 16u) & 255u), f32((rgb >> 8u) & 255u), f32(rgb & 255u)) / 255.0;
    if ((bitcast<u32>(state.v.w) & 130u) != 0u) { out.clip = vec4<f32>(0.0, 0.0, 2.0, 1.0); }
    return out;
}
@fragment fn fs_main(v: Out) -> @location(0) vec4<f32> {
    let light = normalize(vec3<f32>(0.4, 0.8, 0.3));
    return vec4<f32>(v.color * (0.14 + 0.55 * max(dot(normalize(v.normal), light), 0.0)), 1.0);
}
