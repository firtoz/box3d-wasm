//! Minimal regression for relaxed storage/workgroup atomic SPIR-V emission.
fn main() {
    let source = r#"
@group(0) @binding(0) var<storage,read_write> global:array<atomic<u32>>;
var<workgroup> local:atomic<u32>;
@compute @workgroup_size(64) fn main(@builtin(local_invocation_index) lane:u32) {
  if(lane==0u){atomicStore(&local,0u);}
  workgroupBarrier();
  atomicAdd(&local,1u);
  workgroupBarrier();
  if(lane==0u){
    atomicStore(&global[0],atomicLoad(&local));
    let old=atomicExchange(&global[1],atomicLoad(&global[0]));
    let exchanged=atomicCompareExchangeWeak(&global[2],old,64u);
    atomicOr(&global[3],select(0u,1u,exchanged.exchanged));
  }
}
"#;
    let module=naga::front::wgsl::parse_str(source).unwrap();
    let info=naga::valid::Validator::new(naga::valid::ValidationFlags::all(),naga::valid::Capabilities::all()).validate(&module).unwrap();
    let words=naga::back::spv::write_vec(&module,&info,&naga::back::spv::Options::default(),Some(&naga::back::spv::PipelineOptions{shader_stage:naga::ShaderStage::Compute,entry_point:"main".into()})).unwrap();
    std::fs::write(std::env::args().nth(1).expect("output .spv path"),bytemuck::cast_slice(&words)).unwrap();
}
