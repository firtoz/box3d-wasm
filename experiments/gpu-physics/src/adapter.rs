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

/// Device limits requested by the solver, shared by selection and creation.
pub(crate) fn required_limits(supported: &wgpu::Limits) -> wgpu::Limits {
    wgpu::Limits {
        max_storage_buffer_binding_size: supported.max_storage_buffer_binding_size,
        max_buffer_size: supported.max_buffer_size,
        max_storage_buffers_per_shader_stage: supported
            .max_storage_buffers_per_shader_stage
            .max(10),
        max_compute_workgroup_storage_size: supported.max_compute_workgroup_storage_size.min(32768),
        max_compute_workgroup_size_x: supported.max_compute_workgroup_size_x.max(256),
        max_compute_invocations_per_workgroup: supported
            .max_compute_invocations_per_workgroup
            .max(256),
        ..wgpu::Limits::default()
    }
}

#[cfg(not(target_arch = "wasm32"))]
fn device_rank(kind: wgpu::DeviceType, low_power: bool) -> u8 {
    use wgpu::DeviceType::*;
    match kind {
        IntegratedGpu if low_power => 0,
        DiscreteGpu => {
            if low_power {
                1
            } else {
                0
            }
        }
        IntegratedGpu => 1,
        VirtualGpu => 2,
        Other => 3,
        Cpu => 4,
    }
}

#[cfg(not(target_arch = "wasm32"))]
fn matches_adapter(info: &wgpu::AdapterInfo, query: &str) -> bool {
    let vendor = match info.vendor {
        0x10de => "nvidia",
        0x1002 => "amd",
        0x8086 => "intel",
        0x106b => "apple",
        _ => "",
    };
    info.name.to_lowercase().contains(query) || vendor == query
}

/// Prefer a capable hardware adapter, with explicit benchmark overrides.
/// The selected device is always logged; no vendor or model is required.
pub async fn pick_adapters(
    instance: &Instance,
    surface: Option<&Surface<'_>>,
) -> Result<Vec<(Adapter, AdapterReport)>, String> {
    #[cfg(not(target_arch = "wasm32"))]
    {
        let split_render =
            surface.is_some() && std::env::var("GPU_PHYSICS_SPLIT_ADAPTER").as_deref() == Ok("1");
        let query = if split_render {
            String::from("amd")
        } else {
            std::env::var("GPU_PHYSICS_ADAPTER")
                .unwrap_or_default()
                .to_lowercase()
        };
        let backend = std::env::var("GPU_PHYSICS_BACKEND").unwrap_or_else(|_| "auto".into());
        let backend_filter = match backend.as_str() {
            "auto" => Backends::VULKAN | Backends::METAL | Backends::DX12,
            "vulkan" => Backends::VULKAN,
            "metal" => Backends::METAL,
            "dx12" => Backends::DX12,
            _ => return Err("GPU_PHYSICS_BACKEND must be auto, vulkan, metal or dx12".into()),
        };
        let power = std::env::var("GPU_PHYSICS_POWER_PREFERENCE").unwrap_or_else(|_| "high".into());
        if power != "high" && power != "low" {
            return Err("GPU_PHYSICS_POWER_PREFERENCE must be high or low".into());
        }
        let allow_software = std::env::var("GPU_PHYSICS_ALLOW_SOFTWARE").as_deref() == Ok("1");
        let mut rejected = Vec::new();
        let mut candidates = Vec::new();
        for adapter in instance.enumerate_adapters(backend_filter) {
            let info = adapter.get_info();
            let reason = if !query.is_empty() && !matches_adapter(&info, &query) {
                Some("does not match GPU_PHYSICS_ADAPTER")
            } else if split_render && info.vendor != 0x1002 {
                Some("split-adapter rendering explicitly requires AMD")
            } else if info.device_type == wgpu::DeviceType::Cpu && !allow_software {
                Some("software adapter (set GPU_PHYSICS_ALLOW_SOFTWARE=1 to allow)")
            } else if surface.is_some_and(|s| !adapter.is_surface_supported(s)) {
                Some("cannot present to this window")
            } else if !required_limits(&adapter.limits()).check_limits(&adapter.limits()) {
                Some("insufficient compute/buffer limits for this solver")
            } else {
                None
            };
            if let Some(reason) = reason {
                rejected.push(format!("  {} ({:?}): {reason}", info.name, info.backend));
            } else {
                let native_backend = if cfg!(target_os = "macos") {
                    wgpu::Backend::Metal
                } else if cfg!(target_os = "windows") {
                    wgpu::Backend::Dx12
                } else {
                    wgpu::Backend::Vulkan
                };
                candidates.push((
                    (
                        device_rank(info.device_type, power == "low"),
                        u8::from(info.backend != native_backend),
                        info.name.clone(),
                    ),
                    adapter,
                ));
            }
        }
        candidates.sort_by(|a, b| a.0.cmp(&b.0));
        if candidates.is_empty() {
            return Err(format!(
                "no compatible GPU adapter (backend={backend}, adapter={query:?}).\n{}\nInstall a compatible GPU driver or use samples:cpu. Cached builds require Vulkan; use GPU_PHYSICS_SAMPLES_NATIVE_CACHE=0 for the ordinary backend.",
                rejected.join("\n")));
        }
        Ok(candidates
            .into_iter()
            .map(|(_, adapter)| {
                let report = AdapterReport::from_adapter(&adapter);
                (adapter, report)
            })
            .collect())
    }
    #[cfg(target_arch = "wasm32")]
    {
        let adapter = instance
            .request_adapter(&wgpu::RequestAdapterOptions {
                power_preference: wgpu::PowerPreference::HighPerformance,
                compatible_surface: surface,
                force_fallback_adapter: false,
            })
            .await
            .map_err(|e| format!("request_adapter failed: {e:?}"))?;
        let report = AdapterReport::from_adapter(&adapter);
        Ok(vec![(adapter, report)])
    }
}

#[cfg(all(test, not(target_arch = "wasm32")))]
mod tests {
    use super::*;
    #[test]
    fn hardware_ranking_is_vendor_independent_and_honors_power() {
        use wgpu::DeviceType::*;
        assert!(device_rank(DiscreteGpu, false) < device_rank(IntegratedGpu, false));
        assert!(device_rank(IntegratedGpu, true) < device_rank(DiscreteGpu, true));
        assert!(device_rank(IntegratedGpu, false) < device_rank(Cpu, false));
    }
    #[test]
    fn vendor_and_name_overrides_do_not_require_nvidia() {
        let info = wgpu::AdapterInfo {
            name: "Radeon 780M".into(),
            vendor: 0x1002,
            device: 0,
            device_type: wgpu::DeviceType::IntegratedGpu,
            driver: String::new(),
            driver_info: String::new(),
            backend: wgpu::Backend::Vulkan,
        };
        assert!(matches_adapter(&info, "amd"));
        assert!(matches_adapter(&info, "780m"));
        assert!(!matches_adapter(&info, "nvidia"));
    }
    #[test]
    fn insufficient_storage_bindings_are_rejected_before_device_creation() {
        let mut limits = wgpu::Limits::default();
        limits.max_storage_buffers_per_shader_stage = 8;
        assert!(!required_limits(&limits).check_limits(&limits));
        limits.max_storage_buffers_per_shader_stage = 10;
        assert!(required_limits(&limits).check_limits(&limits));
    }
}
