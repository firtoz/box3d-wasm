// Step endpoints, not instantaneous velocity: contacts can change motion during TGS.
struct State { p: vec4<f32>, v: vec4<f32>, q: vec4<f32>, w: vec4<f32>, dp: vec4<f32>, dq: vec4<f32> }
@group(0) @binding(0) var<storage, read> start: array<State>;
@group(0) @binding(1) var<storage, read> finish: array<State>;
// Per body slot: minimum and maximum extent, matching HostBody's CCD bounds.
@group(0) @binding(2) var<storage, read> extents: array<vec2<f32>>;
@group(0) @binding(3) var<storage, read_write> candidates: array<u32>;
@compute @workgroup_size(64)
fn classify(@builtin(global_invocation_id) id: vec3<u32>) {
    let i = id.x;
    if (i >= arrayLength(&extents)) { return; }
    candidates[i] = 0u;
    let a = start[i];
    let b = finish[i];
    let flags = bitcast<u32>(b.v.w);
    if ((flags & (1u | 4u | 16u | 128u)) != 0u) { return; }
    let translation = distance(a.p.xyz, b.p.xyz);
    let rotation = 2.0 * acos(clamp(abs(dot(a.q, b.q)), 0.0, 1.0)) * extents[i].y;
    if (translation + rotation > 0.5 * extents[i].x) {
        candidates[i] = select(1u, 2u, (flags & 32u) != 0u);
    }
}
