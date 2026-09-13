//! Opaque IDs matching `box3d/include/box3d/id.h` (null = all zeros).

#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub struct WorldId {
    pub index1: u16,
    pub generation: u16,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub struct BodyId {
    pub index1: i32,
    pub world0: u16,
    pub generation: u16,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub struct ShapeId {
    pub index1: i32,
    pub world0: u16,
    pub generation: u16,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub struct JointId {
    pub index1: i32,
    pub world0: u16,
    pub generation: u16,
}

pub fn b3_null_world_id() -> WorldId {
    WorldId::default()
}

pub fn b3_null_body_id() -> BodyId {
    BodyId::default()
}

pub fn b3_null_shape_id() -> ShapeId {
    ShapeId::default()
}

pub fn b3_null_joint_id() -> JointId {
    JointId::default()
}

pub fn b3_world_id_is_valid(id: WorldId) -> bool {
    id.index1 != 0
}

pub fn b3_body_id_is_valid(id: BodyId) -> bool {
    id.index1 != 0
}

pub fn b3_shape_id_is_valid(id: ShapeId) -> bool {
    id.index1 != 0
}

pub fn b3_joint_id_is_valid(id: JointId) -> bool {
    id.index1 != 0
}
