//! Handle epochs survive destruction of their world slot. Child epochs advance
//! past every issued body generation, including bodies already destroyed. Shapes
//! and joints append within a world and share its child epoch. Exhausted slots
//! are retired rather than wrapping any public u16 identity.
use super::*;
use std::sync::atomic::{AtomicU16, Ordering};

static CURRENT_WORLDS: [AtomicU16; u16::MAX as usize] =
    [const { AtomicU16::new(0) }; u16::MAX as usize];
static CURRENT_CHILDREN: [AtomicU16; u16::MAX as usize] =
    [const { AtomicU16::new(0) }; u16::MAX as usize];

#[derive(Default)]
struct Epoch {
    world: u16,
    child_ceiling: u16,
}
impl Epoch {
    fn next(&self) -> Option<(u16, u16)> {
        Some((self.world.checked_add(1)?, self.child_ceiling.checked_add(1)?))
    }
}

pub(super) struct WorldRegistry {
    slots: Vec<Option<WorldInner>>,
    epochs: Vec<Epoch>,
}
impl std::ops::Deref for WorldRegistry {
    type Target = [Option<WorldInner>];
    fn deref(&self) -> &Self::Target { &self.slots }
}
impl std::ops::DerefMut for WorldRegistry {
    fn deref_mut(&mut self) -> &mut Self::Target { &mut self.slots }
}
impl WorldRegistry {
    pub(super) const fn new() -> Self {
        Self { slots: Vec::new(), epochs: Vec::new() }
    }
    pub(super) fn reserve_slot(&mut self) -> Option<(usize, u16, u16)> {
        for (index, slot) in self.slots.iter().enumerate() {
            if slot.is_none() {
                if let Some((world, child)) = self.epochs[index].next() {
                    return Some((index, world, child));
                }
            }
        }
        if self.slots.len() == u16::MAX as usize { return None; }
        let index = self.slots.len();
        self.slots.push(None);
        self.epochs.push(Epoch::default());
        Some((index, 1, 1))
    }
    pub(super) fn publish(&mut self, index: usize, world: WorldInner) -> WorldId {
        let id = WorldId { index1: (index + 1) as u16, generation: world.generation };
        self.epochs[index].world = world.generation;
        CURRENT_CHILDREN[index].store(world.child_generation, Ordering::Release);
        self.slots[index] = Some(world);
        CURRENT_WORLDS[index].store(id.generation, Ordering::Release);
        id
    }
    pub(super) fn retire(&mut self, id: WorldId) {
        let index = id.index1 as usize - 1;
        // Caller validated the generation under WORLDS before polling/retirement.
        let world = self.slots[index].take().expect("validated world");
        self.epochs[index].child_ceiling = world.body_generations.iter().copied()
            .max().unwrap_or(world.child_generation).max(world.child_generation);
        CURRENT_WORLDS[index].store(0, Ordering::Release);
        drop(world);
    }
}

// These helpers never acquire WORLDS, including when called from a locked API.
// The normal world accessor rechecks this captured epoch under the registry lock.
// A concurrent recreation therefore rejects the old epoch; child operations must
// still validate the child's generation/liveness under that same lock.
pub(super) fn current_world_id(index1: u16) -> WorldId {
    let Some(index) = index1.checked_sub(1) else { return b3_null_world_id(); };
    WorldId { index1, generation: CURRENT_WORLDS[index as usize].load(Ordering::Acquire) }
}
fn fixed_child_world(index1: u16, generation: u16) -> WorldId {
    let id = current_world_id(index1);
    if id.generation == 0 || generation == 0 ||
        CURRENT_CHILDREN[index1 as usize - 1].load(Ordering::Acquire) != generation {
        return b3_null_world_id();
    }
    id
}
pub(super) fn world_id_from_shape(id: ShapeId) -> WorldId {
    if id.index1 <= 0 { return b3_null_world_id(); }
    fixed_child_world(id.world0, id.generation)
}
pub(super) fn world_id_from_joint(id: JointId) -> WorldId {
    if id.index1 <= 0 { return b3_null_world_id(); }
    fixed_child_world(id.world0, id.generation)
}

#[cfg(test)]
mod world_lifetime_tests {
    use super::*;
    #[test]
    fn epoch_exhaustion_retires_instead_of_wrapping() {
        assert_eq!(Epoch::default().next(), Some((1, 1)));
        assert_eq!(Epoch { world: 5, child_ceiling: 12 }.next(), Some((6, 13)));
        assert_eq!(Epoch { world: u16::MAX, child_ceiling: 1 }.next(), None);
        assert_eq!(Epoch { world: 1, child_ceiling: u16::MAX }.next(), None);
        let mut registry = WorldRegistry::new();
        registry.slots = vec![None, None];
        registry.epochs = vec![Epoch { world: u16::MAX, child_ceiling: 1 },
            Epoch { world: 2, child_ceiling: 9 }];
        assert_eq!(registry.reserve_slot(), Some((1, 3, 10)));
        registry.epochs[1].child_ceiling = u16::MAX;
        assert_eq!(registry.reserve_slot(), Some((2, 1, 1)));
        let mut full = WorldRegistry::new();
        full.slots = (0..u16::MAX).map(|_| None).collect();
        full.epochs = (0..u16::MAX).map(|_| Epoch { world: u16::MAX, child_ceiling: 0 }).collect();
        assert_eq!(full.reserve_slot(), None);
    }
}

#[cfg(test)]
mod live_world_lifetime_tests {
    use super::*;
    use crate::api::*;

    fn joints(world: WorldId, a: BodyId, b: BodyId) -> Vec<JointId> {
        macro_rules! make {
            ($default:ident, $create:ident) => {{
                let mut def = $default(); def.body_a = a; def.body_b = b;
                let id = $create(world, &def);
                assert!(b3_joint_is_valid(id)); id
            }};
        }
        vec![make!(b3_default_revolute_joint_def, b3_create_revolute_joint),
            make!(b3_default_spherical_joint_def, b3_create_spherical_joint),
            make!(b3_default_prismatic_joint_def, b3_create_prismatic_joint),
            make!(b3_default_distance_joint_def, b3_create_distance_joint),
            make!(b3_default_parallel_joint_def, b3_create_parallel_joint),
            make!(b3_default_filter_joint_def, b3_create_filter_joint),
            make!(b3_default_motor_joint_def, b3_create_motor_joint),
            make!(b3_default_weld_joint_def, b3_create_weld_joint),
            make!(b3_default_wheel_joint_def, b3_create_wheel_joint)]
    }

    #[test]
    fn world_lifetime_tests_recreation_rejects_stale_roots_and_children() {
        let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
        let def = b3_default_world_def();
        let old = b3_create_world(gpu.clone(), &def);
        let a = b3_create_body(old, &b3_default_body_def());
        let initial = b3_create_body(old, &b3_default_body_def());
        b3_destroy_body(initial);
        let b = b3_create_body(old, &b3_default_body_def());
        assert_eq!(b.index1, initial.index1);
        assert!(b.generation > initial.generation);
        let shape = b3_create_hull_shape(b, &b3_default_shape_def(), &b3_make_box_hull(0.5, 0.5, 0.5));
        let old_joints = joints(old, a, b);
        let other = b3_create_world(gpu.clone(), &def);
        b3_world_set_user_data(other, 33);
        b3_destroy_world(old);
        let fresh = b3_create_world(gpu.clone(), &def);
        assert_eq!(fresh.index1, old.index1);
        assert_ne!(fresh, old);
        assert!(!b3_world_is_valid(old));
        b3_world_set_user_data(fresh, 22);
        b3_world_enable_warm_starting(old, false);
        b3_world_enable_speculative(old, false);
        b3_world_set_user_data(old, 44);
        b3_destroy_world(old);
        b3_destroy_world(WorldId { generation: fresh.generation + 1, ..fresh });
        assert!(b3_world_is_valid(fresh));
        assert!(b3_world_is_warm_starting_enabled(fresh));
        assert!(b3_world_is_speculative_enabled(fresh));
        assert_eq!(b3_world_get_user_data(old), 0);
        assert_eq!(b3_world_get_user_data(fresh), 22);
        assert_eq!(b3_world_get_user_data(other), 33);
        assert_eq!(b3_create_body(old, &b3_default_body_def()), BodyId::default());

        let next_a = b3_create_body(fresh, &b3_default_body_def());
        let next_b = b3_create_body(fresh, &b3_default_body_def());
        assert_eq!(next_b.index1, b.index1);
        assert!(next_b.generation > b.generation);
        let next_shape = b3_create_hull_shape(next_b, &b3_default_shape_def(), &b3_make_box_hull(0.5, 0.5, 0.5));
        assert_eq!(next_shape.index1, shape.index1);
        assert_ne!(next_shape.generation, shape.generation);
        let next_joints = joints(fresh, next_a, next_b);
        assert!(!b3_body_is_valid(b));
        assert!(!b3_shape_is_valid(shape));
        assert_eq!(b3_body_get_world(b), WorldId::default());
        assert_eq!(b3_shape_get_world(shape), WorldId::default());
        assert_eq!(b3_body_get_world(next_b), fresh);
        assert_eq!(b3_shape_get_world(next_shape), fresh);
        assert_eq!(b3_shape_body(next_shape), next_b);
        b3_body_set_user_data(next_b, 55);
        b3_body_set_user_data(b, 66);
        b3_destroy_body(b);
        assert!(b3_body_is_valid(next_b));
        assert_eq!(b3_body_get_user_data(next_b), 55);
        b3_shape_set_user_data(next_shape, 77);
        b3_shape_set_user_data(shape, 88);
        b3_shape_set_density(shape, 99.0, true);
        b3_destroy_shape(shape, true);
        assert!(b3_shape_is_valid(next_shape));
        assert_eq!(b3_shape_get_user_data(next_shape), 77);
        assert_eq!(b3_shape_get_density(next_shape), b3_default_shape_def().density);
        assert_eq!(b3_create_hull_shape(b, &b3_default_shape_def(), &b3_make_box_hull(0.5, 0.5, 0.5)), ShapeId::default());
        for (old, next) in old_joints.iter().zip(&next_joints) {
            assert_eq!(old.index1, next.index1);
            assert_ne!(old.generation, next.generation);
            assert!(!b3_joint_is_valid(*old));
            assert_eq!(b3_joint_get_world(*old), WorldId::default());
            assert_eq!(b3_joint_get_world(*next), fresh);
            assert_eq!(b3_joint_get_body(*next, true), next_b);
            b3_joint_set_user_data(*next, 99);
            b3_joint_set_user_data(*old, 111);
            b3_joint_set_collide_connected(*old, true);
            b3_joint_set_constraint_tuning(*old, 13.0, 1.0);
            b3_destroy_joint(*old, true);
            assert!(b3_joint_is_valid(*next));
            assert_eq!(b3_joint_get_user_data(*next), 99);
            assert!(!b3_joint_get_collide_connected(*next));
            b3_destroy_joint(*next, true);
        }
        let mut bad = b3_default_revolute_joint_def(); bad.body_a = a; bad.body_b = next_b;
        assert_eq!(b3_create_revolute_joint(fresh, &bad), JointId::default());
        bad.body_a = b3_create_body(other, &b3_default_body_def());
        assert_eq!(b3_create_revolute_joint(fresh, &bad), JointId::default());
        let mut bd = b3_default_body_def(); bd.body_type = BodyType::Dynamic; bd.position = [0.0, 4.0, 0.0];
        let falling = b3_create_body(fresh, &bd);
        b3_create_hull_shape(falling, &b3_default_shape_def(), &b3_make_box_hull(0.5, 0.5, 0.5));
        b3_world_step_gpu(fresh, 1.0 / 60.0, 4);
        b3_world_gpu_wait_with_mirror(fresh);
        assert!(b3_body_get_position(falling)[1] < 4.0);
        assert!(!b3_world_physics_invalid(fresh));
        b3_destroy_world(fresh); b3_destroy_world(other);
    }

    #[test]
    fn world_lifetime_tests_body_generation_exhaustion_does_not_revive_handles() {
        let gpu = pollster::block_on(GpuDevice::new(None)).unwrap();
        let world = b3_create_world(gpu.clone(), &b3_default_world_def());
        let body = b3_create_body(world, &b3_default_body_def());
        // Exercise the actual allocator's boundary without 65535 GPU world builds.
        with_world_mut_no_sync(world, |w| {
            w.body_generations[0] = u16::MAX;
            w.bodies[0].as_mut().unwrap().generation = u16::MAX;
        });
        let last = BodyId { generation: u16::MAX, ..body };
        b3_destroy_body(last);
        let next = b3_create_body(world, &b3_default_body_def());
        assert_ne!(next.index1, last.index1);
        assert!(b3_body_is_valid(next));
        assert!(!b3_body_is_valid(body)); assert!(!b3_body_is_valid(last));
        b3_destroy_world(world);
        let fresh = b3_create_world(gpu, &b3_default_world_def());
        assert_ne!(fresh.index1, world.index1);
        assert!(!b3_world_is_valid(world));
        let replacement = b3_create_body(fresh, &b3_default_body_def());
        b3_destroy_body(last); assert!(b3_body_is_valid(replacement));
        b3_destroy_world(fresh);
    }
}
