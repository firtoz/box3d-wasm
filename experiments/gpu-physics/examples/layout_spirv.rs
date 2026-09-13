//! Offline regression: workgroup array layout, with/without SPIR-V debug names.
fn main() {
    let source = r#"
struct Block { values: array<u32, 256> }
@group(0) @binding(0) var<storage, read_write> output: Block;
var<workgroup> local: array<u32, 256>;
@compute @workgroup_size(256) fn main(@builtin(local_invocation_index) lane: u32) {
    local[lane] = lane + 1u;
    workgroupBarrier();
    output.values[lane] = local[255u - lane];
}
"#;
    let module = naga::front::wgsl::parse_str(source).unwrap();
    let info = naga::valid::Validator::new(naga::valid::ValidationFlags::all(), naga::valid::Capabilities::all()).validate(&module).unwrap();
    let dir = std::path::PathBuf::from(std::env::args().nth(1).expect("output directory"));
    std::fs::create_dir_all(&dir).unwrap();
    std::fs::write(dir.join("input.wgsl"), source).unwrap();
    for debug in [false, true] {
        for native in [false, true] {
            let mut options = naga::back::spv::Options::default();
            options.lang_version = (1, 6);
            options.flags.set(naga::back::spv::WriterFlags::DEBUG, debug);
            options.zero_initialize_workgroup_memory = if native {
                naga::back::spv::ZeroInitializeWorkgroupMemoryMode::Native
            } else { naga::back::spv::ZeroInitializeWorkgroupMemoryMode::Polyfill };
            let words = naga::back::spv::write_vec(&module, &info, &options, Some(&naga::back::spv::PipelineOptions { shader_stage: naga::ShaderStage::Compute, entry_point: "main".into() })).unwrap();
            std::fs::write(dir.join(format!("debug-{debug}-native-{native}.spv")), bytemuck::cast_slice(&words)).unwrap();
        }
    }
}
