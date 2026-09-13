//! Vulkan opaque-FD pose buffer for GL_EXT_memory_object_fd import.

use std::os::raw::c_char;

use ash::vk;
use wgpu::hal::api::Vulkan;
use wgpu::{Buffer, BufferUsages, Device};

pub struct PoseExportBuf {
    pub buffer: Buffer,
    pub size: u64,
    pub fd: i32,
}

impl Drop for PoseExportBuf {
    fn drop(&mut self) {
        if self.fd >= 0 {
            let _ = unsafe { libc::close(self.fd) };
            self.fd = -1;
        }
    }
}

pub fn try_create_export_buffer(device: &Device, size: u64) -> Option<PoseExportBuf> {
    if size == 0 {
        return None;
    }
    let size = size.max(256).next_multiple_of(256);
    let created = unsafe { create_vk_export_buffer(device, size) };
    let (hal_buffer, fd) = created?;
    let buffer = unsafe {
        device.create_buffer_from_hal::<Vulkan>(
            hal_buffer,
            &wgpu::BufferDescriptor {
                label: Some("pose-export"),
                size,
                usage: BufferUsages::COPY_DST | BufferUsages::VERTEX | BufferUsages::STORAGE,
                mapped_at_creation: false,
            },
        )
    };
    eprintln!("GPU pose export: Vulkan opaque FD {fd} ({size} bytes)");
    Some(PoseExportBuf { buffer, size, fd })
}

unsafe fn create_vk_export_buffer(device: &Device, size: u64) -> Option<(wgpu::hal::vulkan::Buffer, i32)> {
    let hal = unsafe { device.as_hal::<Vulkan>() }?;
    let extensions = hal.enabled_device_extensions();
    if !extensions
        .iter()
        .any(|name| *name == ash::khr::external_memory_fd::NAME)
    {
        eprintln!("GPU pose export: VK_KHR_external_memory_fd missing");
        return None;
    }
    let raw = hal.raw_device();
    let phys = hal.raw_physical_device();
    let instance = hal.shared_instance().raw_instance();

    let mut ext_buf = vk::ExternalMemoryBufferCreateInfo::default()
        .handle_types(vk::ExternalMemoryHandleTypeFlags::OPAQUE_FD);
    let buffer_info = vk::BufferCreateInfo::default()
        .size(size)
        .usage(
            vk::BufferUsageFlags::TRANSFER_DST
                | vk::BufferUsageFlags::STORAGE_BUFFER
                | vk::BufferUsageFlags::VERTEX_BUFFER,
        )
        .sharing_mode(vk::SharingMode::EXCLUSIVE)
        .push_next(&mut ext_buf);
    let vk_buffer = unsafe { raw.create_buffer(&buffer_info, None).ok()? };
    let req = unsafe { raw.get_buffer_memory_requirements(vk_buffer) };
    let mem_props = unsafe { instance.get_physical_device_memory_properties(phys) };
    let mut memory_type = None;
    for i in 0..mem_props.memory_type_count {
        let ty = mem_props.memory_types[i as usize];
        if req.memory_type_bits & (1 << i) != 0
            && ty.property_flags.contains(vk::MemoryPropertyFlags::DEVICE_LOCAL)
        {
            memory_type = Some(i);
            break;
        }
    }
    let Some(memory_type_index) = memory_type else {
        unsafe { raw.destroy_buffer(vk_buffer, None) };
        return None;
    };
    let mut export = vk::ExportMemoryAllocateInfo::default()
        .handle_types(vk::ExternalMemoryHandleTypeFlags::OPAQUE_FD);
    let alloc = vk::MemoryAllocateInfo::default()
        .allocation_size(req.size)
        .memory_type_index(memory_type_index)
        .push_next(&mut export);
    let memory = match unsafe { raw.allocate_memory(&alloc, None) } {
        Ok(m) => m,
        Err(_) => {
            unsafe { raw.destroy_buffer(vk_buffer, None) };
            return None;
        }
    };
    if unsafe { raw.bind_buffer_memory(vk_buffer, memory, 0) }.is_err() {
        unsafe {
            raw.free_memory(memory, None);
            raw.destroy_buffer(vk_buffer, None);
        }
        return None;
    }
    let fd_loader = ash::khr::external_memory_fd::Device::new(instance, raw);
    let fd_info = vk::MemoryGetFdInfoKHR::default()
        .memory(memory)
        .handle_type(vk::ExternalMemoryHandleTypeFlags::OPAQUE_FD);
    let fd = match unsafe { fd_loader.get_memory_fd(&fd_info) } {
        Ok(fd) => fd,
        Err(err) => {
            eprintln!("GPU pose export: GetMemoryFdKHR failed: {err:?}");
            unsafe {
                raw.free_memory(memory, None);
                raw.destroy_buffer(vk_buffer, None);
            }
            return None;
        }
    };
    let hal_buffer = unsafe { wgpu::hal::vulkan::Buffer::from_raw_managed(vk_buffer, memory, 0, size) };
    Some((hal_buffer, fd))
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct PoseExportC {
    pub mode: u32,
    pub fd: i32,
    pub size: u64,
    pub stride: u32,
    pub count: u32,
    pub _pad: u32,
}

pub const POSE_EXPORT_NONE: u32 = 0;
pub const POSE_EXPORT_FD: u32 = 1;
pub const POSE_EXPORT_CPU: u32 = 2;

#[allow(dead_code)]
pub fn export_label() -> *const c_char {
    b"vk-gl\0".as_ptr().cast()
}
