use super::*;
use crate::api::{ContactData, Manifold};
use crate::types::ContactGpu;
use std::collections::HashSet;
use std::sync::atomic::{AtomicU64, Ordering};

// Contact handles must not alias when a world slot is destroyed and recreated.
static NEXT_CONTACT_TOKEN: AtomicU64 = AtomicU64::new(0);
pub(super) fn fresh_id(world0: u16) -> ContactId {
    let token = NEXT_CONTACT_TOKEN.fetch_add(1, Ordering::Relaxed);
    let span = i32::MAX as u64;
    ContactId {
        index1: (token % span) as i32 + 1,
        world0,
        padding: 0,
        generation: u32::try_from(token / span + 1).expect("contact handle space exhausted"),
    }
}

pub(super) struct ContactEntry {
    pub live: LiveContact,
    owners: HashSet<(usize, u32)>,
    manifolds: Box<[Manifold]>,
    bodies: [i32; 2],
}
impl ContactEntry {
    fn data(&self) -> ContactData {
        ContactData {
            contact_id: self.live.contact_id,
            shape_id_a: self.live.shape_id_a,
            shape_id_b: self.live.shape_id_b,
            manifolds: if self.manifolds.is_empty() {
                std::ptr::null()
            } else {
                self.manifolds.as_ptr()
            },
            manifold_count: self.manifolds.len() as i32,
        }
    }
}

pub(super) fn snapshot_key(w: &WorldInner) -> (u64, u64, u64) {
    (w.physics_step, w.query_state, w.query_topology)
}

/// Keep native compound children distinct; mesh manifold pieces share one ID.
/// GPU slot generations detect retirement/reuse between explicit reads.
pub(super) fn update_registry(w: &mut WorldInner, world0: u16, contacts: &[ContactGpu]) {
    // With observations on the same or adjacent completed step, an alive public
    // pair remains the same pair even if compound child compaction moves its
    // physical root slots. Across unread steps, require a surviving GPU owner.
    let adjacent = w.contact_snapshot_key.is_some_and(|(step, _, _)| {
        step == w.physics_step || step.checked_add(1) == Some(w.physics_step)
    });
    // Queries can rebuild GPU geometry before a physics step advances the
    // submitted event maps. Decode getters against the actual GPU slot order;
    // keep the old submitted maps untouched for pending previous-step events.
    let (shape_ids, child_ordinals) = if w.contact_shape_revision == w.query_topology {
        (w.contact_shape_ids.clone(), w.contact_child_ordinals.clone())
    } else {
        let mut ids = Vec::new();
        let mut children = Vec::new();
        if let Some(sim) = w.sim.as_ref() {
            for &(source, generation) in &sim.shape_identities {
                let shape = w.shapes.get(source as usize).and_then(Option::as_ref)
                    .filter(|s| s.generation == generation);
                children.push(shape.map_or(-1, |s| s.compound_child_index));
                let public = shape.and_then(|s| {
                    if s.compound_parent == 0 { Some((source, s)) }
                    else {
                        let parent = (s.compound_parent - 1) as usize;
                        w.shapes.get(parent).and_then(Option::as_ref).map(|s| (parent as u32, s))
                    }
                });
                ids.push(public.map_or(ShapeId::default(), |(index, s)| ShapeId {
                    index1: index as i32 + 1, world0, generation: s.generation,
                }));
            }
        }
        (Arc::new(ids), Arc::new(children))
    };
    let mut next: HashMap<ContactKey, ContactEntry> = HashMap::new();
    let mut pieces: HashMap<ContactKey, Vec<Manifold>> = HashMap::new();
    for (slot, c) in contacts.iter().enumerate() {
        if c.a == u32::MAX || c.b == u32::MAX || c.a == c.b {
            continue;
        }
        let key = c.pair_key();
        let (Some(&id_a), Some(&id_b)) = (
            shape_ids.get((key & 0xffff_ffff) as usize),
            shape_ids.get((key >> 32) as usize),
        ) else {
            continue;
        };
        let (Some(sa), Some(sb)) = (
            w.shapes
                .get(id_a.index1.saturating_sub(1) as usize)
                .and_then(Option::as_ref),
            w.shapes
                .get(id_b.index1.saturating_sub(1) as usize)
                .and_then(Option::as_ref),
        ) else {
            continue;
        };
        if sa.generation != id_a.generation
            || sb.generation != id_b.generation
            || (sa.event_flags | sb.event_flags) & SHAPE_IS_SENSOR != 0
        {
            continue;
        }
        let public_key = keyed_contact(((id_a.index1.max(id_b.index1) as u64 - 1) << 32)
            | (id_a.index1.min(id_b.index1) as u64 - 1), key, &child_ordinals);
        // Box3D contact.c registers hull/capsule/sphere and mesh/convex
        // primary ordering independently of the solver's manifold endpoint order.
        let rank = |shape: &CpuShape| match shape.public_kind {
            KIND_SPHERE => 0,
            KIND_CAPSULE => 1,
            KIND_BOX | KIND_CONVEX_HULL => 2,
            _ => 3,
        };
        let actual_shape_a = if sa.body_index.saturating_sub(1) as u32 == c.a {
            id_a
        } else {
            id_b
        };
        let (shape_a, shape_b, bodies) = if rank(sa) >= rank(sb) {
            (id_a, id_b, [sa.body_index, sb.body_index])
        } else {
            (id_b, id_a, [sb.body_index, sa.body_index])
        };
        let entry = next.entry(public_key).or_insert_with(|| ContactEntry {
            live: LiveContact {
                shape_id_a: shape_a,
                shape_id_b: shape_b,
                contact_id: ContactId::default(),
                events_enabled: (sa.event_flags | sb.event_flags) & SHAPE_ENABLE_CONTACT_EVENTS
                    != 0,
            },
            owners: HashSet::new(),
            manifolds: Box::new([]),
            bodies,
        });
        if c.manifold_link[1] == 0 {
            entry.owners.insert((slot, c.lifecycle[0]));
        }
        if c.count != 0 && c.lifecycle[1] & CONTACT_TOUCHING != 0 {
            match crate::api::contact_data::decode_manifold(c) {
                Ok(mut manifold) => {
                    if actual_shape_a != entry.live.shape_id_a {
                        manifold.reverse();
                    }
                    pieces.entry(public_key).or_default().push(manifold);
                }
                Err(error) => {
                    w.physics_invalid = true;
                    w.gpu_fail = std::ffi::CString::new(error).ok();
                    w.contact_registry.clear();
                    return;
                }
            }
        }
    }
    for (key, entry) in &mut next {
        if let Some(old) = w.contact_registry.get(key).filter(|old| {
            (adjacent || !old.owners.is_disjoint(&entry.owners))
                && ((old.live.shape_id_a == entry.live.shape_id_a
                    && old.live.shape_id_b == entry.live.shape_id_b)
                    || (old.live.shape_id_b == entry.live.shape_id_a
                        && old.live.shape_id_a == entry.live.shape_id_b))
        }) {
            if old.live.shape_id_a != entry.live.shape_id_a {
                if let Some(manifolds) = pieces.get_mut(key) {
                    for m in manifolds {
                        m.reverse();
                    }
                }
                entry.bodies.swap(0, 1);
            }
            entry.live.shape_id_a = old.live.shape_id_a;
            entry.live.shape_id_b = old.live.shape_id_b;
            entry.live.contact_id = old.live.contact_id;
        } else {
            entry.live.contact_id = fresh_id(world0);
        }
        entry.manifolds = pieces.remove(key).unwrap_or_default().into_boxed_slice();
    }
    w.contact_by_id.clear();
    w.contact_by_body.clear();
    w.contact_by_shape.clear();
    for (&key, entry) in &next {
        let id = entry.live.contact_id;
        w.contact_by_id.insert((id.index1, id.generation), key);
        for body in entry.bodies {
            w.contact_by_body.entry(body).or_default().push(key);
        }
        for shape in [entry.live.shape_id_a.index1, entry.live.shape_id_b.index1] {
            w.contact_by_shape.entry(shape).or_default().push(key);
        }
    }
    for keys in w
        .contact_by_body
        .values_mut()
        .chain(w.contact_by_shape.values_mut())
    {
        keys.sort_by_key(|key| {
            let id = next[key].live.contact_id;
            (id.index1, id.generation)
        });
    }
    w.contact_registry = next;
    w.contact_snapshot_key = Some(snapshot_key(w));
    w.contact_snapshot_copies += 1;
}

fn with_snapshot<T>(world0: u16, f: impl FnOnce(&WorldInner) -> T) -> Option<T> {
    // Native contact queries return no data while a step/callback locks the
    // world. In particular, do not recursively lock the world mutex in pre-solve.
    let mut worlds = match WORLDS.try_lock() {
        Ok(worlds) => worlds,
        Err(std::sync::TryLockError::Poisoned(error)) => error.into_inner(),
        Err(std::sync::TryLockError::WouldBlock) => return None,
    };
    let w = worlds.get_mut(world0.checked_sub(1)? as usize)?.as_mut()?;
    if w.sim.is_none() {
        w.contact_snapshot_key = Some(snapshot_key(w));
    }
    #[cfg(not(target_arch = "wasm32"))]
    if w.contact_snapshot_key != Some(snapshot_key(w)) {
        let id = WorldId {
            index1: world0,
            generation: w.generation,
        };
        sync_world_mirror_parts(w, id, false, true, true);
    }
    if w.physics_invalid {
        return None;
    }
    Some(f(w))
}

pub fn b3_contact_is_valid(id: ContactId) -> bool {
    if id.index1 <= 0 {
        return false;
    }
    with_snapshot(id.world0, |w| {
        w.contact_by_id
            .get(&(id.index1, id.generation))
            .and_then(|key| w.contact_registry.get(key))
            .is_some_and(|e| e.live.contact_id == id)
    })
    .unwrap_or(false)
}
pub fn b3_contact_get_data(id: ContactId) -> ContactData {
    with_snapshot(id.world0, |w| {
        w.contact_by_id
            .get(&(id.index1, id.generation))
            .and_then(|key| w.contact_registry.get(key))
            .filter(|e| e.live.contact_id == id)
            .map(ContactEntry::data)
            .unwrap_or_default()
    })
    .unwrap_or_default()
}
pub fn b3_body_get_contact_capacity(id: BodyId) -> i32 {
    with_snapshot(id.world0, |w| {
        if body_ref(w, id).is_none() {
            return 0;
        }
        w.contact_by_body
            .get(&id.index1)
            .map_or(0, |keys| keys.len() as i32)
    })
    .unwrap_or(0)
}
pub fn b3_shape_get_contact_capacity(id: ShapeId) -> i32 {
    with_snapshot(id.world0, |w| {
        let Some(s) = w
            .shapes
            .get(id.index1.saturating_sub(1) as usize)
            .and_then(Option::as_ref)
            .filter(|s| s.generation == id.generation && s.event_flags & SHAPE_IS_SENSOR == 0)
        else {
            return 0;
        };
        // Box3D uses the body's conservative pair count for a shape capacity.
        w.contact_by_body
            .get(&s.body_index)
            .map_or(0, |keys| keys.len() as i32)
    })
    .unwrap_or(0)
}
pub fn b3_body_get_contact_data(id: BodyId, out: &mut [ContactData]) -> i32 {
    if out.is_empty() {
        return 0;
    }
    with_snapshot(id.world0, |w| {
        if body_ref(w, id).is_none() {
            return 0;
        }
        copy_data(
            w,
            out,
            w.contact_by_body
                .get(&id.index1)
                .map(Vec::as_slice)
                .unwrap_or_default(),
            |_| true,
        )
    })
    .unwrap_or(0)
}
pub fn b3_shape_get_contact_data(id: ShapeId, out: &mut [ContactData]) -> i32 {
    if out.is_empty() {
        return 0;
    }
    with_snapshot(id.world0, |w| {
        copy_data(
            w,
            out,
            w.contact_by_shape
                .get(&id.index1)
                .map(Vec::as_slice)
                .unwrap_or_default(),
            |e| e.live.shape_id_a == id || e.live.shape_id_b == id,
        )
    })
    .unwrap_or(0)
}
fn copy_data(
    w: &WorldInner,
    out: &mut [ContactData],
    keys: &[ContactKey],
    matches: impl Fn(&ContactEntry) -> bool,
) -> i32 {
    let mut count = 0;
    for key in keys {
        if count == out.len() {
            break;
        }
        let entry = &w.contact_registry[key];
        if !entry.manifolds.is_empty() && matches(entry) {
            out[count] = entry.data();
            count += 1;
        }
    }
    count as i32
}
#[cfg(test)]
pub(crate) fn contact_snapshot_copies(id: WorldId) -> u64 {
    with_world_no_sync(id, |w| w.contact_snapshot_copies).unwrap_or(0)
}

/// Prevent an adjacent-step snapshot from retaining a destroyed pair's public ID.
pub(super) fn retire_body_pair(w: &mut WorldInner, a: i32, b: i32) {
    w.contact_registry.retain(|_, entry| {
        entry.bodies != [a, b] && entry.bodies != [b, a]
    });
    w.contact_snapshot_key = None;
}
