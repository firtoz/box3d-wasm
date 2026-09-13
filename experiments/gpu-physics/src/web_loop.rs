//! Browser entry is a later workstream (native confirmation first).

pub async fn run() -> Result<(), String> {
    Err(
        "gpu-physics WebGPU present is not in this loop; run the native window / --self-test"
            .into(),
    )
}
