// Keep this convention in sync with src/dispatch.rs. Existing 1D grids stay
// unchanged. Only grids exceeding 65,535 groups use rows of 256 workgroups.
const LINEAR_DISPATCH_LIMIT: u32 = 65535u;
const LINEAR_DISPATCH_TILE: u32 = 256u;

fn linear_dispatch_groups(groups: u32) -> vec3<u32> {
    if (groups <= LINEAR_DISPATCH_LIMIT) {
        return vec3<u32>(groups, 1u, 1u);
    }
    return vec3<u32>(LINEAR_DISPATCH_TILE, (groups + LINEAR_DISPATCH_TILE - 1u) / LINEAR_DISPATCH_TILE, 1u);
}

fn linear_workgroup_id(group: vec3<u32>) -> u32 {
    return group.x + group.y * LINEAR_DISPATCH_TILE;
}

fn linear_invocation_id(id: vec3<u32>, workgroup_size: u32) -> u32 {
    return id.x + id.y * LINEAR_DISPATCH_TILE * workgroup_size;
}
