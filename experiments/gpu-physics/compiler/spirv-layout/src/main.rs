fn main() -> Result<(), Box<dyn std::error::Error>> {
    let args: Vec<_> = std::env::args().collect();
    if args.len() != 3 {
        return Err("usage: gpu-spirv-layout input.spv output.spv".into());
    }
    let bytes = std::fs::read(&args[1])?;
    if bytes.len() % 4 != 0 {
        return Err("unaligned SPIR-V input".into());
    }
    let input: Vec<_> = bytes
        .chunks_exact(4)
        .map(|b| u32::from_le_bytes(b.try_into().unwrap()))
        .collect();
    let output = gpu_spirv_layout::repair(&input)?;
    let twice = gpu_spirv_layout::repair(&output)?;
    if output != twice {
        return Err("repair is not idempotent".into());
    }
    let bytes: Vec<_> = output.into_iter().flat_map(u32::to_le_bytes).collect();
    std::fs::write(&args[2], bytes)?;
    Ok(())
}
