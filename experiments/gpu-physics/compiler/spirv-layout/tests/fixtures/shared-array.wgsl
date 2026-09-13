
struct Block { values: array<u32, 256> }
@group(0) @binding(0) var<storage, read_write> output: Block;
var<workgroup> local: array<u32, 256>;
@compute @workgroup_size(256) fn main(@builtin(local_invocation_index) lane: u32) {
    local[lane] = lane + 1u;
    workgroupBarrier();
    output.values[lane] = local[255u - lane];
}
