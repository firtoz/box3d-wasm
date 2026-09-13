//! Immutable render inputs. Poses are intentionally absent: renderers fetch
//! current COM/rotation/local-center from GPU buffers using `body_slot`.
use std::sync::Arc;
use crate::api::{BodyId, ShapeId, SurfaceMaterial, WorldId};

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct RenderSceneKey {
    pub world: WorldId,
    pub topology: u64,
}

#[derive(Clone, Debug)]
pub enum RenderGeometry {
    Sphere { center: [f32; 3], radius: f32 },
    Capsule { center: [f32; 3], half_axis: [f32; 3], radius: f32 },
    Box { center: [f32; 3], half: [f32; 3] },
    Hull { points: Arc<Vec<[f32; 3]>>, planes: Arc<Vec<[f32; 4]>>, topology: Arc<Vec<[u32; 4]>> },
    Mesh { vertices: Arc<Vec<[f32; 3]>>, triangles: Arc<Vec<[u32; 4]>>,
           position: [f32; 3], rotation: [f32; 4], scale: [f32; 3] },
}

#[derive(Clone, Debug)]
pub struct RenderShape {
    /// GPU state-buffer slot, not index within this render-shape vector.
    pub body_slot: u32,
    pub body: BodyId,
    pub collider: ShapeId,
    pub public_shape: ShapeId,
    pub child_index: i32,
    pub geometry: RenderGeometry,
    pub custom_color: u32,
    pub is_sensor: bool,
    pub materials: Vec<SurfaceMaterial>,
}

pub struct RenderScene {
    pub key: RenderSceneKey,
    pub shapes: Vec<RenderShape>,
}
