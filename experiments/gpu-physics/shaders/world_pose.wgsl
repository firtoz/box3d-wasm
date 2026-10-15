// Existing 28-word pose ABI. Translate position and valid cached origin only.
@group(0) @binding(0) var<storage, read> local_poses: array<u32>;
@group(0) @binding(1) var<storage, read_write> world_poses: array<u32>;
@group(0) @binding(2) var<uniform> coordinate_origin: vec4<f32>;

@compute @workgroup_size(64)
fn export_poses(@builtin(global_invocation_id) id: vec3<u32>) {
    let base = id.x * 28u;
    if base + 28u > arrayLength(&local_poses) { return; }
    for (var i = 0u; i < 28u; i++) { world_poses[base + i] = local_poses[base + i]; }
    for (var i = 0u; i < 3u; i++) {
        if coordinate_origin[i] != 0.0 {
            world_poses[base + i] = bitcast<u32>(bitcast<f32>(local_poses[base + i]) + coordinate_origin[i]);
            if local_poses[base + 27u] != 0u {
                world_poses[base + 24u + i] = bitcast<u32>(bitcast<f32>(local_poses[base + 24u + i]) + coordinate_origin[i]);
            }
        }
    }
}
