//! Owned metadata for read-only shape getters during locked contact callbacks.
//! The registry stays locked: callbacks cannot mutate it or observe partial steps.
//! Never expose a reference to the actively borrowed WorldInner/GpuSim.
use super::*;
use std::{cell::RefCell, marker::PhantomData, rc::Rc};

#[derive(Clone, Copy, Debug, PartialEq)]
pub(super) struct ShapeRead {
    generation: u16,
    pub owner: BodyId,
    pub kind: u32,
    pub event_flags: u32,
    pub user_data: usize,
}
struct WorldRead { generation: u16, bodies: Vec<Option<u16>>, shapes: Vec<Option<ShapeRead>> }
pub(super) struct Snapshot { worlds: Vec<Option<WorldRead>> }
impl Snapshot {
    pub fn capture(worlds: &WorldRegistry) -> Self {
        Self { worlds: worlds.iter().enumerate().map(|(index,slot)| slot.as_ref().map(|w| WorldRead {
            generation: w.generation,
            bodies: w.bodies.iter().map(|b| b.as_ref().map(|b| b.generation)).collect(),
            shapes: w.shapes.iter().map(|shape| shape.as_ref().map(|s| ShapeRead {
                generation: s.generation, owner: live_body_id(w, (index+1) as u16, s.body_index),
                kind: s.public_kind, event_flags: s.event_flags, user_data: s.user_data,
            })).collect(),
        })).collect() }
    }
    fn get(&self, id: ShapeId) -> Option<ShapeRead> {
        let w = self.worlds.get(id.world0.checked_sub(1)? as usize)?.as_ref()?;
        let shape = w.shapes.get(id.index1.checked_sub(1)? as usize)?.as_ref()?;
        (id.generation != 0 && shape.generation == id.generation).then_some(*shape)
    }
}
thread_local! { static ACTIVE: RefCell<Option<Snapshot>> = const { RefCell::new(None) }; }
/// Outer None means no callback scope; inner None means an invalid public ID.
pub(super) fn read(id: ShapeId) -> Option<Option<ShapeRead>> {
    ACTIVE.with(|slot| slot.borrow().as_ref().map(|snapshot| snapshot.get(id)))
}
pub(super) fn world_valid(id: WorldId) -> Option<bool> {
    ACTIVE.with(|slot| slot.borrow().as_ref().map(|snapshot|
        id.index1.checked_sub(1).and_then(|i| snapshot.worlds.get(i as usize)).and_then(Option::as_ref)
            .is_some_and(|w| id.generation != 0 && w.generation == id.generation)))
}
pub(super) fn body_valid(id: BodyId) -> Option<bool> {
    ACTIVE.with(|slot| slot.borrow().as_ref().map(|snapshot|
        id.world0.checked_sub(1).and_then(|i| snapshot.worlds.get(i as usize)).and_then(Option::as_ref)
            .and_then(|w| id.index1.checked_sub(1).filter(|i| *i >= 0).and_then(|i| w.bodies.get(i as usize)))
            .and_then(|g| *g).is_some_and(|g| g != 0 && g == id.generation)))
}
pub(super) struct Scope { previous: Option<Snapshot>, _thread: PhantomData<Rc<()>> }
pub(super) fn enter(snapshot: Snapshot) -> Scope {
    Scope { previous: ACTIVE.with(|slot| slot.replace(Some(snapshot))), _thread: PhantomData }
}
impl Drop for Scope {
    fn drop(&mut self) { ACTIVE.with(|slot| { slot.replace(self.previous.take()); }); }
}
pub(super) fn pending(worlds: &WorldRegistry, id: WorldId, stepping: bool) -> Option<Snapshot> {
    let w = worlds.get(id.index1.checked_sub(1)? as usize)?.as_ref()?;
    (w.generation == id.generation && (stepping || w.post_ccd_pending)
        && (w.custom_filter_callback.is_some() || w.pre_solve_callback.is_some()))
        .then(|| Snapshot::capture(worlds))
}

#[cfg(test)]
mod runtime02_callback_reads_tests {
    use super::*;
    fn snapshot(value: usize) -> Snapshot {
        Snapshot { worlds: vec![Some(WorldRead { generation: 3, bodies: vec![Some(7)], shapes: vec![Some(ShapeRead {
            owner: BodyId { index1: 1, world0: 1, generation: 7 }, kind: KIND_SPHERE,
            generation: 7, event_flags: SHAPE_IS_SENSOR | SHAPE_ENABLE_CUSTOM_FILTERING,
            user_data: value,
        }), None] }), None, Some(WorldRead { generation: 4, bodies: vec![Some(12)], shapes: vec![Some(ShapeRead {
            owner: BodyId { index1: 1, world0: 3, generation: 12 }, kind: KIND_BOX,
            generation: 12, event_flags: 0, user_data: 987,
        })] })] }
    }
    fn id() -> ShapeId { ShapeId { index1: 1, world0: 1, generation: 7 } }
    #[test]
    fn owned_metadata_validates_identity_and_other_worlds() {
        assert!(read(id()).is_none());
        let _guard = enter(snapshot(123));
        let data = read(id()).unwrap().unwrap();
        assert_eq!(data.user_data,123);assert_ne!(data.event_flags & SHAPE_IS_SENSOR,0);
        assert_eq!(body_valid(data.owner), Some(true));
        assert_eq!(body_valid(BodyId { generation: 8, ..data.owner }), Some(false));
        assert_eq!(world_valid(WorldId { index1: 1, generation: 3 }), Some(true));
        assert_eq!(world_valid(WorldId { index1: 1, generation: 4 }), Some(false));
        for bad in [ShapeId { index1: 0,..id() },ShapeId { index1: -1,..id() },
            ShapeId { index1: 2,..id() },ShapeId { index1: 3,..id() },
            ShapeId { generation: 0,..id() },ShapeId { generation: 8,..id() },
            ShapeId { world0: 0,..id() },ShapeId { world0: 2,..id() },
            ShapeId { world0: 4,..id() }] { assert_eq!(read(bad),Some(None)); }
        let other=ShapeId { index1:1,world0:3,generation:12 };
        assert_eq!(read(other).unwrap().unwrap().user_data,987);
        assert_eq!(read(other).unwrap().unwrap().event_flags,0);
        // A second thread cannot borrow the active callback's metadata.
        std::thread::spawn(|| assert!(read(id()).is_none())).join().unwrap();
    }
    #[test]
    fn nested_scope_and_unwind_restore_prior_metadata() {
        assert!(read(id()).is_none());
        { let _outer = enter(snapshot(123));
          { let _inner = enter(snapshot(456));assert_eq!(read(id()).unwrap().unwrap().user_data,456); }
          assert_eq!(read(id()).unwrap().unwrap().user_data,123);
          let result=std::panic::catch_unwind(|| { let _inner=enter(snapshot(789));panic!("scope cleanup control"); });
          assert!(result.is_err());assert_eq!(read(id()).unwrap().unwrap().user_data,123);
        }
        assert!(read(id()).is_none());
        let _next = enter(snapshot(321));assert_eq!(read(id()).unwrap().unwrap().user_data,321);
    }
}
