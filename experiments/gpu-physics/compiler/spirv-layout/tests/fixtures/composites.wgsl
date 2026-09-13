struct Inner { m: mat3x3<f32>, v: array<vec3<f32>, 2> }
struct Outer { a: array<Inner, 2>, tag: u32 }
@group(0) @binding(0) var<storage,read_write> data: array<Outer>;
var<workgroup> group_values: array<Outer, 4>;
var<private> private_value: Outer;
fn adjust(x: Outer, k: u32) -> Outer {
    var y=x;
    y.a[k%2u].m[1][2] += 0.5;
    y.tag+=1u;
    return y;
}
@compute @workgroup_size(4) fn main(@builtin(local_invocation_index) lane:u32) {
    var value=data[lane];
    if (lane%2u==0u) {value=adjust(value,lane);} else {value.tag+=2u;}
    private_value=value;
    group_values[lane]=private_value;
    workgroupBarrier();
    data[lane]=group_values[3u-lane];
}
