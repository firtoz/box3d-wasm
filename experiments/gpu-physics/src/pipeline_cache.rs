//! Optional startup experiment. Cache bytes are opaque wgpu output, never shader state.
pub(crate) struct StartupCache {
    pub cache: Option<wgpu::PipelineCache>,
    #[cfg(not(target_arch = "wasm32"))]
    path: Option<std::path::PathBuf>,
}
impl StartupCache {
    pub fn new(gpu: &crate::sim::GpuDevice) -> Self {
        #[cfg(target_arch = "wasm32")]
        { let _ = gpu; Self { cache: None } }
        #[cfg(not(target_arch = "wasm32"))]
        {
            let none = || Self { cache: None, path: None };
            if !gpu.device.features().contains(wgpu::Features::PIPELINE_CACHE) { return none(); }
            let Some(dir) = std::env::var_os("GPU_PHYSICS_PIPELINE_CACHE_DIR") else { return none(); };
            let Some(key) = wgpu::util::pipeline_cache_key(&gpu.adapter.get_info()) else { return none(); };
            let dir = std::path::PathBuf::from(dir);
            if std::fs::create_dir_all(&dir).is_err() { return none(); }
            let path = dir.join(key);
            // Bound reads; a corrupt/oversized cache must not cause a giant allocation.
            let data = std::fs::metadata(&path).ok().filter(|m| m.is_file() && m.len() <= 128*1024*1024)
                .and_then(|_| std::fs::read(&path).ok());
            // SAFETY: this private experiment directory holds only get_data output.
            // wgpu validates adapter/driver/cache version and falls back on incompatibility.
            let cache = unsafe { gpu.device.create_pipeline_cache(&wgpu::PipelineCacheDescriptor {
                label: Some("physics-startup"), data: data.as_deref(), fallback: true,
            }) };
            eprintln!("physics-pipeline-cache: loaded {} bytes", data.as_ref().map_or(0, Vec::len));
            Self { cache: Some(cache), path: Some(path) }
        }
    }
    pub fn save(&self) {
        #[cfg(not(target_arch = "wasm32"))]
        if let (Some(cache), Some(path)) = (&self.cache, &self.path) {
            if let Some(data) = cache.get_data() {
                use std::io::Write;
                use std::sync::atomic::{AtomicU64, Ordering};
                static NEXT: AtomicU64 = AtomicU64::new(0);
                let temp = path.with_extension(format!("{}-{}.tmp", std::process::id(), NEXT.fetch_add(1, Ordering::Relaxed)));
                let result = (|| -> std::io::Result<()> {
                    let mut file = std::fs::OpenOptions::new().write(true).create_new(true).open(&temp)?;
                    file.write_all(&data)?;
                    std::fs::rename(&temp, path)
                })();
                if result.is_err() { let _ = std::fs::remove_file(&temp); }
                eprintln!("physics-pipeline-cache: save {} bytes, success={}", data.len(), result.is_ok());
            }
        }
    }
}
