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
    pub(super) owners: HashSet<(usize, u32)>,
    pub(super) manifolds: Box<[Manifold]>,
    pub(super) bodies: [i32; 2],
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

// Atomic child allocation may change physical slots between identical runs.
// Query order follows stable shape pairs and each root's logical patch chain.
pub(super) fn query_contact_order(contacts: &[ContactGpu]) -> Result<Vec<usize>, &'static str> {
    let live = |c: &ContactGpu| c.a != u32::MAX && c.b != u32::MAX && c.a != c.b;
    let mut roots: Vec<_> = contacts.iter().enumerate()
        .filter(|(_, c)| live(c) && c.manifold_link[1] == 0)
        .map(|(slot, _)| slot).collect();
    roots.sort_unstable_by_key(|&slot| contacts[slot].pair_key());
    let mut seen = vec![false; contacts.len()];
    let mut order = Vec::new();
    for root in roots {
        let first = &contacts[root];
        let count = first.manifold_link[2].max(1) as usize;
        if count > contacts.len() { return Err("invalid query contact chain length"); }
        let mut slot = root;
        for ordinal in 0..count {
            let c = contacts.get(slot).ok_or("query contact chain exceeds storage")?;
            let parent = if ordinal == 0 { 0 } else { root as u32 + 1 };
            if seen[slot] || !live(c) || c.a != first.a || c.b != first.b
                || c.pair_key() != first.pair_key() || c.manifold_link[1] != parent {
                return Err("invalid query contact chain ownership");
            }
            seen[slot] = true;
            order.push(slot);
            let next = c.manifold_link[0];
            if ordinal + 1 == count {
                if next != 0 { return Err("query contact chain has extra members"); }
            } else {
                slot = next.checked_sub(1).ok_or("truncated query contact chain")? as usize;
            }
        }
    }
    if contacts.iter().enumerate().any(|(slot, c)| live(c) && !seen[slot]) {
        return Err("orphan query contact child");
    }
    Ok(order)
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
    let order = match query_contact_order(contacts) {
        Ok(order) => order,
        Err(error) => {
            w.physics_invalid = true;
            w.gpu_fail = std::ffi::CString::new(error).ok();
            w.contact_registry.clear();
            return;
        }
    };
    for slot in order {
        let c = &contacts[slot];
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
    // Contact queries preserve handle-creation order. HashMap iteration is
    // randomized, so allocate new handles in public pair/child order instead.
    // Existing handles retain their lifetime and enumeration position.
    let mut ordered_keys:Vec<_>=next.keys().copied().collect();
    ordered_keys.sort_unstable();
    for key in &ordered_keys {
        let entry=next.get_mut(key).expect("collected contact key");
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

#[cfg(all(test,not(target_arch="wasm32")))]
mod determinism_tests {
    use super::*;
    #[test]
    fn previous_touching_storage_permutation_preserves_end_events() {
        let gpu=pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let world=b3_create_world(gpu,&crate::api::b3_default_world_def());
        let mut sd=crate::api::b3_default_shape_def(); sd.enable_contact_events=true;
        let ground=b3_create_body(world,&crate::api::b3_default_body_def());
        b3_create_hull_shape(ground,&sd,&crate::api::b3_make_box_hull(8.0,0.5,2.0));
        let mut bd=crate::api::b3_default_body_def(); bd.body_type=BodyType::Dynamic;
        for x in [-3.0,0.0,3.0] {
            bd.position=[x,0.99,0.0];
            b3_create_hull_shape(b3_create_body(world,&bd),&sd,&crate::api::b3_make_box_hull(0.5,0.5,0.5));
        }
        b3_world_step_gpu(world,1.0/60.0,4);b3_world_gpu_wait_with_mirror(world);
        let snapshot=pollster::block_on(b3_world_sync_contacts(world));
        let pairs:Vec<_>=snapshot.iter().filter(|c|c.a!=u32::MAX && c.count>0 && c.manifold_link[1]==0)
            .map(|c|c.pair_key()).collect();
        assert_eq!(pairs.len(),3);
        let reduce=|history:&[u64]| with_world_mut_no_sync(world,|w| {
            w.previous_contact_shape_ids=w.contact_shape_ids.clone();
            w.previous_contact_child_ordinals=w.contact_child_ordinals.clone();
            w.contact_begin_events.clear(); w.contact_hit_events.clear();
            w.contact_end_events.clear(); w.contact_end_keys.clear();
            update_contact_events(w,world.index1,&[],history);
            assert!(w.contact_begin_events.is_empty() && w.contact_hit_events.is_empty());
            w.contact_end_events.iter().map(|e|(e.shape_id_a,e.shape_id_b,e.contact_id)).collect::<Vec<_>>()
        }).unwrap();
        let expected=reduce(&pairs);assert_eq!(expected.len(),3);
        for order in [[0,1,2],[0,2,1],[1,0,2],[1,2,0],[2,0,1],[2,1,0]] {
            let storage=[u64::MAX,pairs[order[0]],pairs[order[1]],u64::MAX,
                pairs[order[2]],pairs[order[0]],pairs[order[2]],u64::MAX];
            assert_eq!(reduce(&storage),expected,"slot order, duplicate patches and empty slots must not change end events");
        }
        assert_eq!(reduce(&pairs[..2]).len(),2,"removing a real pair must remain observable");
        b3_destroy_world(world);
    }

    #[test]
    fn unread_steps_preserve_live_handles_and_retire_reused_roots() {
        unread_root_reuse(false);
    }

    #[test]
    #[cfg(feature="replay-diagnostics")]
    fn retired_generation_offset_preserves_unread_handle_lifetimes() {
        unread_root_reuse(true);
    }

    fn unread_root_reuse(perturb_generation: bool) {
        let gpu = pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let mut wd = crate::api::b3_default_world_def();
        wd.gravity=[0.0;3]; wd.enable_sleep=false; wd.enable_continuous=false;
        let world=b3_create_world(gpu,&wd);
        let sd=crate::api::b3_default_shape_def();
        let ground=b3_create_body(world,&crate::api::b3_default_body_def());
        b3_create_hull_shape(ground,&sd,&crate::api::b3_make_box_hull(4.0,0.5,4.0));
        let mut bd=crate::api::b3_default_body_def(); bd.body_type=BodyType::Dynamic;
        bd.position=[0.0,1.0,0.0];
        let body=b3_create_body(world,&bd);
        b3_create_hull_shape(body,&sd,&crate::api::b3_make_box_hull(0.5,0.5,0.5));
        let step=|| {b3_world_step_gpu(world,1.0/60.0,4);b3_world_gpu_wait(world);};
        let read=|| {
            let mut data=[ContactData::default();2];
            assert_eq!(b3_body_get_contact_data(body,&mut data),1);
            data[0].contact_id
        };
        let owner=|| with_world_no_sync(world,|w| {
            let entry=w.contact_registry.values().next().unwrap();
            assert_eq!(entry.owners.len(),1);
            *entry.owners.iter().next().unwrap()
        }).unwrap();
        step(); let mut live=read(); let mut retired=Vec::new();
        for _ in 0..4 {
            let previous_owner=owner();
            let copies=contact_snapshot_copies(world);
            step(); step();
            assert_eq!(contact_snapshot_copies(world),copies,"fixture accidentally harvested unread contacts");
            assert_eq!(read(),live,"surviving GPU owner must retain the public handle across unread steps");
            let copies=contact_snapshot_copies(world);
            b3_body_set_transform(body,[0.0,10.0,0.0],[0.0,0.0,0.0,1.0]);
            step(); step(); step();
            if perturb_generation {
                #[cfg(feature="replay-diagnostics")]
                with_world_mut_no_sync(world, |w| {
                    w.sim.as_mut().unwrap().perturb_retired_contact_generation_test(previous_owner.0, 17);
                }).unwrap();
            }
            b3_body_set_transform(body,bd.position,[0.0,0.0,0.0,1.0]);
            step();
            assert_eq!(contact_snapshot_copies(world),copies,"retirement/re-entry must remain unread");
            let replacement=read();
            let replacement_owner=owner();
            assert_eq!(replacement_owner.0,previous_owner.0,"fixture must reuse the same root slot");
            assert_eq!(replacement_owner.1,previous_owner.1+1+if perturb_generation {17} else {0},"root reuse must advance the actual stored generation");
            assert_ne!(replacement,live,"same shape pair after retirement needs a new handle");
            retired.push(live);
            for old in &retired {assert!(!b3_contact_is_valid(*old),"retired handle aliased a reused root");}
            assert!(b3_contact_is_valid(replacement));
            live=replacement;
        }
        b3_destroy_world(world);
    }

    #[test]
    fn equal_speed_hit_event_survives_child_slot_relocation() {
        let gpu = pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let world = b3_create_world(gpu,&crate::api::b3_default_world_def());
        let mut sd = crate::api::b3_default_shape_def(); sd.enable_hit_events = true;
        let ground = b3_create_body(world,&crate::api::b3_default_body_def());
        b3_create_hull_shape(ground,&sd,&crate::api::b3_make_box_hull(2.0,0.5,2.0));
        let mut bd = crate::api::b3_default_body_def(); bd.body_type=BodyType::Dynamic;
        bd.position=[0.0,0.99,0.0];
        b3_create_hull_shape(b3_create_body(world,&bd),&sd,&crate::api::b3_make_box_hull(0.5,0.5,0.5));
        b3_world_step_gpu(world,1.0/60.0,4); b3_world_gpu_wait_with_mirror(world);
        let snapshot = pollster::block_on(b3_world_sync_contacts(world));
        let root = *snapshot.iter().find(|c|c.a!=u32::MAX && c.count>0).unwrap();
        // Feed the host event reducer three valid patches. Equal-speed child
        // hits have different points, so storage-order tie breaking is visible.
        let mut contacts = vec![ContactGpu::empty();4];
        for slot in [0,2,3] {
            contacts[slot]=root;
            contacts[slot].count=1;
            contacts[slot].lifecycle[1]=CONTACT_TOUCHING;
            contacts[slot].total_normal_impulse[0]=1.0;
            contacts[slot]._tail[0]=(-10.0f32).to_bits();
            contacts[slot].ra0=[slot as f32,0.0,0.0,0.0];
            contacts[slot].rb0=[0.0;4];
        }
        contacts[0]._tail[0]=0;
        contacts[0].manifold_link=[3,0,3,0];
        contacts[2].manifold_link=[4,1,0,0];
        contacts[3].manifold_link=[0,1,0,0];
        let reduce = |cs:&[ContactGpu]| with_world_mut_no_sync(world,|w| {
            w.contact_hit_events.clear();
            update_contact_events(w,world.index1,cs,&[]);
            assert_eq!(w.contact_hit_events.len(),1);
            let hit=w.contact_hit_events[0];
            (hit.point,hit.normal,hit.approach_speed)
        }).unwrap();
        let before=reduce(&contacts);
        contacts.swap(1,3); contacts[2].manifold_link[0]=2;
        let after=reduce(&contacts);
        b3_destroy_world(world);
        assert_eq!(before,after,"hit tie must follow logical patch order, not allocation slots");
    }

    #[test]
    fn contact_end_event_order_repeats_across_identical_worlds() {
        let gpu=pollster::block_on(GpuDevice::new(None)).expect("GPU");
        for mode in 0..3 {
            let mut orders=Vec::new();
            for _ in 0..8 {
                let world=b3_create_world(gpu.clone(),&crate::api::b3_default_world_def());
                let mut sd=crate::api::b3_default_shape_def();sd.enable_contact_events=true;
                let mut bd=crate::api::b3_default_body_def();
                for x in [-1.5,0.0,1.5] {
                    bd.position=[x,-0.5,0.0];
                    b3_create_hull_shape(b3_create_body(world,&bd),&sd,&crate::api::b3_make_box_hull(0.4,0.5,0.5));
                }
                bd.body_type=BodyType::Dynamic;bd.position=[0.0,0.49,0.0];
                let body=b3_create_body(world,&bd);
                let shape=b3_create_hull_shape(body,&sd,&crate::api::b3_make_box_hull(2.0,0.5,0.5));
                b3_world_step_gpu(world,1.0/60.0,4);b3_world_gpu_wait_with_mirror(world);
                assert_eq!(b3_world_contact_event_ptrs(world).1,3);
                match mode {
                    0=>b3_body_set_transform(body,[0.0,10.0,0.0],[0.0,0.0,0.0,1.0]),
                    1=>b3_destroy_body(body),
                    _=>{let mut filter=sd.filter;filter.mask_bits=0;b3_shape_set_filter(shape,filter,true);},
                }
                b3_world_step_gpu(world,1.0/60.0,4);b3_world_gpu_wait_with_mirror(world);
                let (_,_,ends,count,_,_)=b3_world_contact_event_ptrs(world);assert_eq!(count,3);
                let ends=unsafe {std::slice::from_raw_parts(ends,count as usize)};
                orders.push(ends.iter().map(|e|[e.shape_id_a.index1.min(e.shape_id_b.index1),e.shape_id_a.index1.max(e.shape_id_b.index1)]).collect::<Vec<_>>());
                if mode==2 {
                    b3_shape_set_filter(shape,sd.filter,true);
                    b3_world_step_gpu(world,1.0/60.0,4);b3_world_gpu_wait_with_mirror(world);
                    let (_,begins,_,ends,_,_)=b3_world_contact_event_ptrs(world);
                    assert_eq!((begins,ends),(3,0),"restoring collision must emit new begins without stale ends");
                }
                b3_destroy_world(world);
            }
            assert!(orders.iter().all(|order|order==&orders[0]),"mode {mode}: identical worlds changed contact end order: {orders:?}");
        }
    }
    #[test]
    fn contact_query_order_repeats_across_identical_worlds() {
        let gpu=pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let mut orders=Vec::new();
        for _ in 0..8 {
            let world=b3_create_world(gpu.clone(),&crate::api::b3_default_world_def());
            let sd=crate::api::b3_default_shape_def();
            let mut bd=crate::api::b3_default_body_def();
            for x in [-1.5,0.0,1.5] {
                bd.position=[x,-0.5,0.0];
                b3_create_hull_shape(b3_create_body(world,&bd),&sd,&crate::api::b3_make_box_hull(0.4,0.5,0.5));
            }
            bd.body_type=BodyType::Dynamic;bd.position=[0.0,0.49,0.0];
            let body=b3_create_body(world,&bd);
            b3_create_hull_shape(body,&sd,&crate::api::b3_make_box_hull(2.0,0.5,0.5));
            b3_world_step_gpu(world,1.0/60.0,4);b3_world_gpu_wait_with_mirror(world);
            let mut contacts=[crate::api::ContactData::default();8];
            let count=b3_body_get_contact_data(body,&mut contacts);
            assert_eq!(count,3,"fixture must have three support contacts");
            orders.push(contacts[..count as usize].iter().map(|c|
                [c.shape_id_a.index1.min(c.shape_id_b.index1),c.shape_id_a.index1.max(c.shape_id_b.index1)]).collect::<Vec<_>>());
            b3_destroy_world(world);
        }
        assert!(orders.iter().all(|order|order==&orders[0]),"identical worlds changed contact query order: {orders:?}");
    }
}

#[cfg(test)]
mod query_order_tests {
    use super::*;

    #[test]
    fn query_patch_order_survives_child_relocation() {
        let mut contacts = vec![ContactGpu::empty(); 4];
        for slot in [0, 2, 3] {
            contacts[slot].a = 1;
            contacts[slot].b = 2;
            contacts[slot].count = 1;
        }
        contacts[0].manifold_link = [3, 0, 3, 0];
        contacts[2].manifold_link = [4, 1, 0, 0];
        contacts[3].manifold_link = [0, 1, 0, 0];
        contacts[0].nx = 1.0;
        contacts[2].ny = 1.0;
        contacts[3].nz = 1.0;
        let query = |cs: &[ContactGpu]| query_contact_order(cs).unwrap().into_iter()
            .map(|slot| crate::api::contact_data::decode_manifold(&cs[slot]).unwrap())
            .collect::<Vec<_>>();
        let before = query(&contacts);
        contacts.swap(1, 3);
        contacts[2].manifold_link[0] = 2;
        assert_eq!(query_contact_order(&contacts).unwrap(), vec![0, 2, 1]);
        assert_eq!(query(&contacts), before);
        // A malformed chain must fail instead of returning a partial query.
        contacts[1].manifold_link[0] = 3;
        assert!(query_contact_order(&contacts).is_err());
        contacts[1].manifold_link[0] = 0;
        contacts[1].manifold_link[1] = 3;
        assert!(query_contact_order(&contacts).is_err());
    }
}
