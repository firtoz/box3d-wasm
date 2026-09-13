use wgpu::util::DeviceExt;

#[repr(C)]
#[derive(Clone, Copy, bytemuck::Pod, bytemuck::Zeroable)]
pub struct MeshVertex {
    pub position: [f32; 3],
    pub normal: [f32; 3],
}

pub struct CpuMesh {
    pub vertices: Vec<MeshVertex>,
    pub indices: Vec<u32>,
}

pub fn unit_uv_sphere(slices: u32, stacks: u32) -> CpuMesh {
    let mut vertices = Vec::new();
    let mut indices = Vec::new();
    for y in 0..=stacks {
        let v = y as f32 / stacks as f32;
        let phi = v * std::f32::consts::PI;
        let sy = phi.cos();
        let r = phi.sin();
        for x in 0..=slices {
            let u = x as f32 / slices as f32;
            let theta = u * std::f32::consts::TAU;
            let px = r * theta.cos();
            let pz = r * theta.sin();
            vertices.push(MeshVertex {
                position: [px, sy, pz],
                normal: [px, sy, pz],
            });
        }
    }
    let stride = slices + 1;
    for y in 0..stacks {
        for x in 0..slices {
            let i0 = y * stride + x;
            let i1 = i0 + 1;
            let i2 = i0 + stride;
            let i3 = i2 + 1;
            indices.extend_from_slice(&[i0, i2, i1, i1, i2, i3]);
        }
    }
    CpuMesh { vertices, indices }
}

pub fn ground_quad(half: f32, y: f32) -> CpuMesh {
    CpuMesh {
        vertices: vec![
            MeshVertex {
                position: [-half, y, -half],
                normal: [0.0, 1.0, 0.0],
            },
            MeshVertex {
                position: [half, y, -half],
                normal: [0.0, 1.0, 0.0],
            },
            MeshVertex {
                position: [half, y, half],
                normal: [0.0, 1.0, 0.0],
            },
            MeshVertex {
                position: [-half, y, half],
                normal: [0.0, 1.0, 0.0],
            },
        ],
        indices: vec![0, 1, 2, 0, 2, 3],
    }
}

pub fn upload_mesh(
    device: &wgpu::Device,
    mesh: &CpuMesh,
    label: &str,
) -> (wgpu::Buffer, wgpu::Buffer, u32) {
    let vb = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
        label: Some(label),
        contents: bytemuck::cast_slice(&mesh.vertices),
        usage: wgpu::BufferUsages::VERTEX,
    });
    let ib = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
        label: Some(&format!("{label}-idx")),
        contents: bytemuck::cast_slice(&mesh.indices),
        usage: wgpu::BufferUsages::INDEX,
    });
    (vb, ib, mesh.indices.len() as u32)
}

pub fn vertex_layout() -> wgpu::VertexBufferLayout<'static> {
    wgpu::VertexBufferLayout {
        array_stride: std::mem::size_of::<MeshVertex>() as u64,
        step_mode: wgpu::VertexStepMode::Vertex,
        attributes: &[
            wgpu::VertexAttribute {
                offset: 0,
                shader_location: 0,
                format: wgpu::VertexFormat::Float32x3,
            },
            wgpu::VertexAttribute {
                offset: 12,
                shader_location: 1,
                format: wgpu::VertexFormat::Float32x3,
            },
        ],
    }
}

/// Capsule along +X: cylinder `x ∈ [-1, 1]` radius 1, hemispheres at `x = ±1` (tips at `±2`).
/// Instance `half = (radius, half_length, radius)` is applied in `vs_capsule`.
pub fn unit_capsule_x(slices: u32, hemi_stacks: u32) -> CpuMesh {
    let slices = slices.max(3);
    let hemi_stacks = hemi_stacks.max(1);
    let mut vertices = Vec::new();
    let mut indices = Vec::new();
    let ring = slices + 1;

    let push_ring = |vertices: &mut Vec<MeshVertex>, x: f32| {
        for i in 0..=slices {
            let theta = (i as f32 / slices as f32) * std::f32::consts::TAU;
            let y = theta.cos();
            let z = theta.sin();
            vertices.push(MeshVertex {
                position: [x, y, z],
                normal: [0.0, y, z],
            });
        }
    };
    push_ring(&mut vertices, -1.0);
    push_ring(&mut vertices, 1.0);
    for i in 0..slices {
        let a = i;
        let b = i + 1;
        let c = ring + i;
        let d = ring + i + 1;
        indices.extend_from_slice(&[a, c, b, b, c, d]);
    }

    let hemi = |vertices: &mut Vec<MeshVertex>, indices: &mut Vec<u32>, sign: f32, equator: u32| {
        let pole = vertices.len() as u32;
        vertices.push(MeshVertex {
            position: [sign * 2.0, 0.0, 0.0],
            normal: [sign, 0.0, 0.0],
        });
        let mut rings = vec![equator];
        for s in 1..hemi_stacks {
            let phi = (s as f32 / hemi_stacks as f32) * (std::f32::consts::PI * 0.5);
            let cx = sign * (1.0 + phi.sin());
            let rr = phi.cos();
            let start = vertices.len() as u32;
            for i in 0..=slices {
                let theta = (i as f32 / slices as f32) * std::f32::consts::TAU;
                let y = rr * theta.cos();
                let z = rr * theta.sin();
                let px = cx;
                vertices.push(MeshVertex {
                    position: [px, y, z],
                    normal: [sign * phi.sin(), y, z],
                });
            }
            rings.push(start);
        }
        rings.push(pole);
        for r in 0..rings.len() - 1 {
            let a0 = rings[r];
            let b0 = rings[r + 1];
            let b_is_pole = r + 1 == rings.len() - 1;
            for i in 0..slices {
                let a1 = a0 + i;
                let a2 = a0 + i + 1;
                if b_is_pole {
                    if sign > 0.0 {
                        indices.extend_from_slice(&[a1, pole, a2]);
                    } else {
                        indices.extend_from_slice(&[a1, a2, pole]);
                    }
                } else {
                    let b1 = b0 + i;
                    let b2 = b0 + i + 1;
                    if sign > 0.0 {
                        indices.extend_from_slice(&[a1, b1, a2, a2, b1, b2]);
                    } else {
                        indices.extend_from_slice(&[a1, a2, b1, a2, b2, b1]);
                    }
                }
            }
        }
    };
    hemi(&mut vertices, &mut indices, 1.0, ring);
    hemi(&mut vertices, &mut indices, -1.0, 0);
    CpuMesh { vertices, indices }
}

pub fn unit_cube() -> CpuMesh {
    let faces: [([f32; 3], [f32; 3], [f32; 3], [f32; 3], [f32; 3]); 6] = [
        (
            [-1.0, -1.0, 1.0],
            [1.0, -1.0, 1.0],
            [1.0, 1.0, 1.0],
            [-1.0, 1.0, 1.0],
            [0.0, 0.0, 1.0],
        ),
        (
            [1.0, -1.0, -1.0],
            [-1.0, -1.0, -1.0],
            [-1.0, 1.0, -1.0],
            [1.0, 1.0, -1.0],
            [0.0, 0.0, -1.0],
        ),
        (
            [-1.0, 1.0, 1.0],
            [1.0, 1.0, 1.0],
            [1.0, 1.0, -1.0],
            [-1.0, 1.0, -1.0],
            [0.0, 1.0, 0.0],
        ),
        (
            [-1.0, -1.0, -1.0],
            [1.0, -1.0, -1.0],
            [1.0, -1.0, 1.0],
            [-1.0, -1.0, 1.0],
            [0.0, -1.0, 0.0],
        ),
        (
            [1.0, -1.0, 1.0],
            [1.0, -1.0, -1.0],
            [1.0, 1.0, -1.0],
            [1.0, 1.0, 1.0],
            [1.0, 0.0, 0.0],
        ),
        (
            [-1.0, -1.0, -1.0],
            [-1.0, -1.0, 1.0],
            [-1.0, 1.0, 1.0],
            [-1.0, 1.0, -1.0],
            [-1.0, 0.0, 0.0],
        ),
    ];
    let mut vertices = Vec::new();
    let mut indices = Vec::new();
    for (a, b, c, d, n) in faces {
        let base = vertices.len() as u32;
        for p in [a, b, c, d] {
            vertices.push(MeshVertex {
                position: p,
                normal: n,
            });
        }
        indices.extend_from_slice(&[base, base + 1, base + 2, base, base + 2, base + 3]);
    }
    CpuMesh { vertices, indices }
}

pub fn instance_state_layout() -> wgpu::VertexBufferLayout<'static> {
    wgpu::VertexBufferLayout {
        array_stride: std::mem::size_of::<crate::types::BodyStateGpu>() as u64,
        step_mode: wgpu::VertexStepMode::Instance,
        attributes: &[
            wgpu::VertexAttribute {
                offset: 0,
                shader_location: 2,
                format: wgpu::VertexFormat::Float32x4,
            },
            wgpu::VertexAttribute {
                offset: 16,
                shader_location: 3,
                format: wgpu::VertexFormat::Float32x3,
            },
            wgpu::VertexAttribute {
                offset: 28,
                shader_location: 6,
                format: wgpu::VertexFormat::Uint32,
            },
            wgpu::VertexAttribute {
                offset: 32,
                shader_location: 7,
                format: wgpu::VertexFormat::Float32x4,
            },
        ],
    }
}

pub fn instance_cold_layout() -> wgpu::VertexBufferLayout<'static> {
    wgpu::VertexBufferLayout {
        array_stride: std::mem::size_of::<crate::types::BodyColdGpu>() as u64,
        step_mode: wgpu::VertexStepMode::Instance,
        attributes: &[
            wgpu::VertexAttribute {
                offset: 0,
                shader_location: 5,
                format: wgpu::VertexFormat::Float32x3,
            },
            wgpu::VertexAttribute {
                offset: 12,
                shader_location: 4,
                format: wgpu::VertexFormat::Uint32,
            },
        ],
    }
}

pub fn camera_matrix(aspect: f32) -> glam::Mat4 {
    let proj = glam::Mat4::perspective_rh(45_f32.to_radians(), aspect.max(0.01), 0.1, 160.0);
    let view = glam::Mat4::look_at_rh(
        glam::Vec3::new(18.0, 22.0, 42.0),
        glam::Vec3::new(0.0, 8.0, 0.0),
        glam::Vec3::Y,
    );
    proj * view
}
