use wgpu::{Adapter, Backends, Instance, Surface};

#[derive(Clone, Debug)]
pub struct AdapterReport {
    pub name: String,
    pub backend: String,
    pub driver: String,
    pub vendor: u32,
    pub device: u32,
}

impl AdapterReport {
    pub fn from_adapter(adapter: &Adapter) -> Self {
        let info = adapter.get_info();
        let name = if info.name.trim().is_empty() {
            format!(
                "WebGPU vendor=0x{:04x} device=0x{:04x} type={:?}",
                info.vendor, info.device, info.device_type
            )
        } else {
            info.name
        };
        Self {
            name,
            backend: format!("{:?}", info.backend),
            driver: if info.driver.is_empty() {
                info.driver_info.clone()
            } else {
                info.driver
            },
            vendor: info.vendor,
            device: info.device,
        }
    }

    pub fn summary_line(&self) -> String {
        format!(
            "{} | backend={} driver={}",
            self.name, self.backend, self.driver
        )
    }
}

fn is_nvidia(adapter: &Adapter) -> bool {
    let name = adapter.get_info().name.to_ascii_uppercase();
    name.contains("NVIDIA") || name.contains("4070")
}

/// Pick a wgpu adapter. Native Linux requires NVIDIA (this laptop is hybrid).
/// Wasm logs the chosen adapter and prefers high-performance, but cannot pin a vendor.
pub async fn pick_adapter(
    instance: &Instance,
    surface: Option<&Surface<'_>>,
) -> Result<(Adapter, AdapterReport), String> {
    #[cfg(not(target_arch = "wasm32"))]
    {
        let mut adapters: Vec<Adapter> =
            instance.enumerate_adapters(Backends::VULKAN | Backends::METAL | Backends::DX12);
        if adapters.is_empty() {
            return Err("wgpu found no GPU adapters".into());
        }

        if let Some(surface) = surface {
            adapters.retain(|a| a.is_surface_supported(surface));
            if adapters.is_empty() {
                return Err(
                    "no adapter supports the window surface (try VK_DRIVER_FILES=/usr/share/vulkan/icd.d/nvidia_icd.json and __NV_PRIME_RENDER_OFFLOAD=1)"
                        .into(),
                );
            }
        }

        let listed = adapters
            .iter()
            .map(|a| {
                let info = a.get_info();
                format!("  - {} ({:?}, {})", info.name, info.backend, info.driver)
            })
            .collect::<Vec<_>>()
            .join("\n");

        let chosen = adapters.into_iter().find(|a| if surface.is_some() && std::env::var("GPU_PHYSICS_SPLIT_ADAPTER").as_deref()==Ok("1") {a.get_info().vendor==0x1002} else {is_nvidia(a)}).ok_or_else(|| {
            format!(
                "no NVIDIA adapter (refusing Intel/other iGPU fallback)\navailable adapters:\n{listed}\n\nOn hybrid laptops try:\n  __NV_PRIME_RENDER_OFFLOAD=1 __GLX_VENDOR_LIBRARY_NAME=nvidia \\\n  VK_DRIVER_FILES=/usr/share/vulkan/icd.d/nvidia_icd.json \\\n  cargo run --release"
            )
        })?;
        let report = AdapterReport::from_adapter(&chosen);
        Ok((chosen, report))
    }

    #[cfg(target_arch = "wasm32")]
    {
        let _ = is_nvidia;
        let _ = Backends::all();
        let adapter = instance
            .request_adapter(&wgpu::RequestAdapterOptions {
                power_preference: wgpu::PowerPreference::HighPerformance,
                compatible_surface: surface,
                force_fallback_adapter: false,
            })
            .await
            .map_err(|e| format!("request_adapter failed: {e:?}"))?;
        let report = AdapterReport::from_adapter(&adapter);
        Ok((adapter, report))
    }
}
