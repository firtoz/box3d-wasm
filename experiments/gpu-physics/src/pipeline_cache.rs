//! Optional startup experiment. Cache files contain wgpu output, never shader state.
//!
//! wgpu's adapter key does not include Vulkan's pipeline-cache UUID. NVIDIA's
//! GLX and EGL implementations can report different UUIDs for the same adapter
//! and driver; importing the other implementation's data silently starts cold.
#[cfg(not(target_arch = "wasm32"))]
#[derive(Clone, Copy)]
struct VulkanIdentity {
    vendor: u32,
    device: u32,
    uuid: [u8; 16],
}

#[cfg(not(target_arch = "wasm32"))]
impl VulkanIdentity {
    fn suffix(self) -> String {
        self.uuid.iter().map(|byte| format!("{byte:02x}")).collect()
    }

    fn validate(self, data: &[u8]) -> Result<(), &'static str> {
        // This project pins wgpu 26's v1 envelope. Reject unknown envelopes
        // rather than interpreting future opaque cache formats as this one.
        if data.len() < 96 { return Err("truncated cache header"); }
        if &data[..8] != b"WGPUPLCH"
            || u32::from_be_bytes(data[8..12].try_into().unwrap()) != 1
            || u32::from_be_bytes(data[12..16].try_into().unwrap()) != 8
            || data[16] != 1
            || u64::from_be_bytes(data[48..56].try_into().unwrap()) != (data.len() - 64) as u64
        { return Err("unsupported wgpu cache envelope"); }
        let body = &data[64..];
        let word = |offset| u32::from_le_bytes(body[offset..offset+4].try_into().unwrap());
        if word(0) != 32 || word(4) != 1 { return Err("unsupported Vulkan cache header"); }
        if word(8) != self.vendor || word(12) != self.device { return Err("Vulkan adapter mismatch"); }
        if body[16..32] != self.uuid { return Err("Vulkan pipeline-cache UUID mismatch"); }
        Ok(())
    }
}

#[cfg(not(target_arch = "wasm32"))]
fn read_cache(path: &std::path::Path) -> Option<Vec<u8>> {
    // A corrupt/oversized file must not cause a giant allocation.
    std::fs::metadata(path).ok().filter(|m| m.is_file() && m.len() <= 128*1024*1024)
        .and_then(|_| std::fs::read(path).ok())
}
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
            // SAFETY: the guard retains this adapter/instance while reading its
            // physical-device properties. No raw handles escape the guard.
            let identity = unsafe { gpu.adapter.as_hal::<wgpu::hal::api::Vulkan>() }
                .map(|hal| {
                    let properties = unsafe { hal.shared_instance().raw_instance()
                        .get_physical_device_properties(hal.raw_physical_device()) };
                    VulkanIdentity { vendor: properties.vendor_id, device: properties.device_id,
                        uuid: properties.pipeline_cache_uuid }
                });
            let legacy = dir.join(&key);
            let path = identity.map_or_else(|| legacy.clone(), |id| dir.join(format!("{key}_{}", id.suffix())));
            // Preserve a compatible legacy cache, then migrate on the next save.
            // Never overwrite another UUID's cache or rewrite its header.
            let mut data = None;
            let mut imported = None;
            for candidate in [&path, &legacy] {
                if let Some(bytes) = read_cache(candidate) {
                    if let Some(id) = identity {
                        if let Err(reason) = id.validate(&bytes) {
                            eprintln!("physics-pipeline-cache: rejected {}: {reason}", candidate.display());
                            continue;
                        }
                    }
                    imported = Some(candidate);
                    data = Some(bytes);
                    break;
                }
                if path == legacy { break; }
            }
            // SAFETY: this private experiment directory holds only get_data output.
            // wgpu validates adapter/driver/cache version and falls back on incompatibility.
            let cache = unsafe { gpu.device.create_pipeline_cache(&wgpu::PipelineCacheDescriptor {
                label: Some("physics-startup"), data: data.as_deref(), fallback: true,
            }) };
            if let Some(imported) = imported {
                // Compatible input is not proof that a particular pipeline hits.
                eprintln!("physics-pipeline-cache: submitted {} compatible bytes from {}",
                    data.as_ref().unwrap().len(), imported.display());
            } else {
                eprintln!("physics-pipeline-cache: cold start; no compatible data for {}", path.display());
            }
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

#[cfg(all(test, not(target_arch = "wasm32")))]
mod tests {
    use super::*;
    // Actual get_data output captured at the previously failing API boundary.
    const EMPTY_EGL_CACHE: [u8; 100] = [87, 71, 80, 85, 80, 76, 67, 72, 0, 0, 0, 1, 0, 0, 0, 8, 1, 255, 255, 255, 0, 0, 16, 222, 0, 0, 39, 131, 255, 255, 255, 255, 152, 142, 65, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 36, 254, 220, 186, 152, 118, 84, 50, 16, 32, 0, 0, 0, 1, 0, 0, 0, 222, 16, 0, 0, 131, 39, 0, 0, 134, 48, 193, 232, 101, 97, 3, 39, 45, 242, 1, 201, 162, 225, 79, 201, 0, 0, 0, 0];
    fn identity() -> VulkanIdentity {
        VulkanIdentity { vendor: 4318, device: 10115,
            uuid: EMPTY_EGL_CACHE[80..96].try_into().unwrap() }
    }
    #[test]
    fn actual_cache_header_and_uuid_compatibility() {
        let identity = identity();
        assert_eq!(identity.validate(&EMPTY_EGL_CACHE), Ok(()));
        let mut glx = identity;
        glx.uuid = [0x86,0x30,0xc1,0xe8,0x65,0x61,0x03,0x27,0x4c,0xe2,0xc5,0x94,0x06,0x4a,0x47,0x87];
        assert_eq!(glx.validate(&EMPTY_EGL_CACHE), Err("Vulkan pipeline-cache UUID mismatch"));
        for size in 0..96 { assert_eq!(identity.validate(&EMPTY_EGL_CACHE[..size]), Err("truncated cache header")); }
        let mut corrupt = EMPTY_EGL_CACHE;
        corrupt[55] += 1;
        assert_eq!(identity.validate(&corrupt), Err("unsupported wgpu cache envelope"));
        corrupt = EMPTY_EGL_CACHE;
        corrupt[68] = 2;
        assert_eq!(identity.validate(&corrupt), Err("unsupported Vulkan cache header"));
        let mut other_adapter = identity;
        other_adapter.device += 1;
        assert_eq!(other_adapter.validate(&EMPTY_EGL_CACHE), Err("Vulkan adapter mismatch"));
    }
}
