//! Diagnostic core-state capture. Not a restorable or complete engine snapshot.
//! Float values use exact IEEE-754 hex words; true padding is excluded.
use super::*;
use serde_json::{json, Map, Value};
use std::io::Write;

trait TraceValue { fn trace(&self) -> Result<Value, String>; }
impl TraceValue for f32 {
    fn trace(&self) -> Result<Value, String> {
        if !self.is_finite() { return Err("non-finite solver state".into()); }
        Ok(json!(format!("{:08x}", self.to_bits())))
    }
}
impl TraceValue for u32 { fn trace(&self) -> Result<Value, String> { Ok(json!(self)) } }
impl TraceValue for i32 { fn trace(&self) -> Result<Value, String> { Ok(json!(self)) } }
impl<T: TraceValue> TraceValue for Vec<T> {
    fn trace(&self) -> Result<Value, String> {
        Ok(Value::Array(self.iter().map(TraceValue::trace).collect::<Result<_, _>>()?))
    }
}
impl<T: TraceValue, const N: usize> TraceValue for [T; N] {
    fn trace(&self) -> Result<Value, String> {
        Ok(Value::Array(self.iter().map(TraceValue::trace).collect::<Result<_, _>>()?))
    }
}
macro_rules! fields {
    ($value:expr; $($name:ident),* $(,)?) => {{
        let mut record = Map::new();
        $(record.insert(stringify!($name).into(), $value.$name.trace()
            .map_err(|e| format!("{}: {}", stringify!($name), e))?);)*
        record
    }};
}

#[path = "host_events_trace.rs"]
mod host_events_trace;
#[path = "host_state_trace.rs"]
mod host_state_trace;
#[path = "geometry_trace.rs"]
mod geometry_trace;
#[path = "ccd_state_trace.rs"]
mod ccd_state_trace;

// Contact handles are opaque; their cross-event/registry alias relationships
// are a separate capture audit. Preserve endpoint validity, logical child keys,
// published order here. Deferred enqueue order is overwritten before publication.
fn contact_end_state(w: &WorldInner) -> Value {
    let shape = |id: ShapeId| {
        if id.index1 <= 0 { return Value::Null; }
        json!({"identity":id.index1,"live":w.shapes.get(id.index1 as usize-1)
            .and_then(Option::as_ref).is_some_and(|s|s.generation==id.generation)})
    };
    let key = |key: ContactKey| json!({"pair":[(key.0 & 0xffff_ffff)+1,(key.0 >> 32)+1],"child":key.1});
    let event = |e: &ContactEndTouchEvent| json!({"shapes":[shape(e.shape_id_a),shape(e.shape_id_b)]});
    let mut ended:Vec<_>=w.contact_end_keys.iter().copied().collect();
    ended.sort_unstable(); // Set membership is semantic; hash iteration is not.
    let mut deferred:Vec<_>=w.deferred_contact_end_events.iter().collect();
    deferred.sort_unstable_by_key(|(k,_)|*k);
    json!({"published":w.contact_end_events.iter().map(event).collect::<Vec<_>>(),
        "deferred":deferred.into_iter().map(|(k,e)|json!({"key":key(*k),"event":event(e)})).collect::<Vec<_>>(),
        "ended_keys":ended.into_iter().map(key).collect::<Vec<_>>()})
}

// Assign trace-local handle identities on first observation in semantic order.
// Keep the map across frames: a retired handle must not alias its replacement.
fn host_contact_state(w: &mut WorldInner, world0: u16) -> Result<Value,String> {
    let token = |id: ContactId| (id.index1,id.world0,id.generation);
    let mut registry:Vec<_>=w.contact_registry.keys().copied().collect();registry.sort_unstable();
    let mut live:Vec<_>=w.live_contacts.keys().copied().collect();live.sort_unstable();
    let mut deferred:Vec<_>=w.deferred_contact_end_events.iter().collect();deferred.sort_unstable_by_key(|(k,_)|*k);
    let mut lookups:Vec<_>=w.contact_by_id.iter().collect();lookups.sort_unstable_by_key(|(_,key)|**key);
    let ids=registry.iter().map(|k|w.contact_registry[k].live.contact_id)
        .chain(live.iter().map(|k|w.live_contacts[k].contact_id))
        .chain(w.contact_begin_events.iter().map(|e|e.contact_id))
        .chain(w.contact_end_events.iter().map(|e|e.contact_id))
        .chain(w.contact_hit_events.iter().map(|e|e.contact_id))
        .chain(deferred.iter().map(|(_,e)|e.contact_id))
        .chain(lookups.iter().map(|((index1,generation),_)|ContactId{index1:*index1,world0,padding:0,generation:*generation}))
        .collect::<Vec<_>>();
    for id in ids {
        if id.index1>0 {
            let next=w.diagnostic_contact_ids.len() as u64+1;
            w.diagnostic_contact_ids.entry(token(id)).or_insert(next);
        }
    }
    let handle = |id: ContactId| {
        if id.index1<=0 {return Value::Null;}
        json!({"identity":w.diagnostic_contact_ids[&token(id)],"world_matches":id.world0==world0})
    };
    let key = |k: ContactKey| json!({"pair":[(k.0 & 0xffff_ffff)+1,(k.0>>32)+1],"child":k.1});
    let shape = |id:ShapeId| json!({"identity":id.index1,"world_matches":id.world0==world0,
        "live":id.index1>0 && w.shapes.get(id.index1 as usize-1).and_then(Option::as_ref).is_some_and(|s|s.generation==id.generation)});
    let body = |index:i32| json!({"slot":index,"identity":index.checked_sub(1).and_then(|i|w.bodies.get(i as usize))
        .and_then(Option::as_ref).map(|b|b.creation_ordinal)});
    let event = |a,b,id|json!({"shapes":[shape(a),shape(b)],"handle":handle(id)});
    let live_record = |k:ContactKey,c:&LiveContact|json!({"key":key(k),"event":event(c.shape_id_a,c.shape_id_b,c.contact_id),"events_enabled":c.events_enabled});
    let entries=registry.iter().map(|k| {
        let e=&w.contact_registry[k];let mut owners:Vec<_>=e.owners.iter().copied().collect();owners.sort_unstable();
        let manifolds=e.manifolds.iter().map(|m| {
            let n=usize::try_from(m.point_count).map_err(|_|"negative host manifold point count")?;
            if n>4 {return Err("host manifold point count exceeds four".to_string());}
            let points=m.points[..n].iter().map(|p| {
                let mut r=fields!(p;anchor_a,anchor_b,separation,base_separation,normal_impulse,total_normal_impulse,normal_velocity,feature_id,triangle_index);
                r.insert("persisted".into(),json!(p.persisted));Ok(Value::Object(r))
            }).collect::<Result<Vec<_>,String>>()?;
            Ok(json!({"points":points,"normal":m.normal.trace()?,"twist_impulse":m.twist_impulse.trace()?,
                "friction_impulse":m.friction_impulse.trace()?,"rolling_impulse":m.rolling_impulse.trace()?}))
        }).collect::<Result<Vec<_>,String>>()?;
        Ok(json!({"live":live_record(*k,&e.live),"owners":owners,"bodies":[body(e.bodies[0]),body(e.bodies[1])],"manifolds":manifolds}))
    }).collect::<Result<Vec<_>,String>>()?;
    let mut by_body:Vec<_>=w.contact_by_body.iter().collect();by_body.sort_unstable_by_key(|(b,_)|**b);
    let mut by_shape:Vec<_>=w.contact_by_shape.iter().collect();by_shape.sort_unstable_by_key(|(s,_)|**s);
    let mut by_id=lookups.iter().map(|((index1,generation),k)|json!({"handle":handle(ContactId{index1:*index1,world0,padding:0,generation:*generation}),"key":key(**k)})).collect::<Vec<_>>();
    by_id.sort_by_key(|v|v["handle"]["identity"].as_u64());
    let hits=w.contact_hit_events.iter().map(|e|Ok(json!({"event":event(e.shape_id_a,e.shape_id_b,e.contact_id),
        "point":e.point.trace()?,"normal":e.normal.trace()?,"approach_speed":e.approach_speed.trace()?,
        "materials":[e.user_material_id_a,e.user_material_id_b]}))).collect::<Result<Vec<_>,String>>()?;
    Ok(json!({"registry":entries,"live":live.iter().map(|k|live_record(*k,&w.live_contacts[k])).collect::<Vec<_>>(),
        "by_body":by_body.into_iter().map(|(b,keys)|json!({"body":body(*b),"keys":keys.iter().copied().map(key).collect::<Vec<_>>()})).collect::<Vec<_>>(),
        "by_shape":by_shape.into_iter().map(|(s,keys)|json!({"shape":s,"keys":keys.iter().copied().map(key).collect::<Vec<_>>()})).collect::<Vec<_>>(),
        "by_id":by_id,"snapshot_key":w.contact_snapshot_key,"current_snapshot_key":contact_api::snapshot_key(w),
        "shape_revision":w.contact_shape_revision,"handle_count":w.diagnostic_contact_ids.len(),
        "begins":w.contact_begin_events.iter().map(|e|event(e.shape_id_a,e.shape_id_b,e.contact_id)).collect::<Vec<_>>(),
        "ends":w.contact_end_events.iter().map(|e|event(e.shape_id_a,e.shape_id_b,e.contact_id)).collect::<Vec<_>>(),"hits":hits,
        "deferred_ends":deferred.into_iter().map(|(k,e)|json!({"key":key(*k),"event":event(e.shape_id_a,e.shape_id_b,e.contact_id)})).collect::<Vec<_>>() }))
}

fn contact_feature_ids(c:&crate::types::ContactGpu) -> [u32;4] {
    // The first two persistent words hold SAT type/separation, not features.
    // The final two feature IDs occupy float-typed legacy padding lanes.
    [c._tail[6],c._tail[7],c._pad_ca.to_bits(),c._pad_cb.to_bits()]
}
fn motor_spring_angular_impulse(j:&JointGpu) -> Result<Value,String> {
    if j.kind!=JOINT_MOTOR {return Ok(Value::Null);}
    [j.motor_impulse,j._pad2[0],j._pad2[1]].trace()
}

pub fn b3_world_diagnostic_contact_impulses(id: WorldId, first: u64, count: u32, clear: bool) -> Result<(usize,usize),String> {
    diagnostic_impulses(id,first,count,clear,false)
}

pub fn b3_world_diagnostic_spherical_impulses(id: WorldId, first: u64, count: u32, clear: bool) -> Result<(usize,usize),String> {
    diagnostic_impulses(id,first,count,clear,true)
}

pub fn b3_joint_diagnostic_set_spherical_cache(id: JointId, values: [f32;12]) -> Result<(),String> {
    if id.index1<=0 || values.iter().any(|v|!v.is_finite()) {return Err("invalid joint cache input".into());}
    let world=with_joint_metadata(id,|_,_|WorldId{index1:id.world0,generation:1}).ok_or("joint missing or stale")?;
    b3_world_ensure_gpu(world);
    b3_world_gpu_wait_with_mirror(world);
    let mut worlds=lock_worlds();let w=slot_mut(&mut worlds,world).ok_or("world missing")?;
    let slot=id.index1 as usize-1;
    let meta=w.joint_meta.get(slot).and_then(Option::as_ref).ok_or("joint missing")?;
    if meta.generation!=id.generation {return Err("stale joint generation".into());}
    w.sim.as_mut().ok_or("simulation missing")?.diagnostic_set_spherical_cache(slot,values)
}

pub fn b3_joint_diagnostic_set_revolute_cache(id: JointId, values: [f32;9]) -> Result<(),String> {
    if id.index1<=0 || values.iter().any(|v|!v.is_finite()) {return Err("invalid joint cache input".into());}
    let world=with_joint_metadata(id,|_,_|WorldId{index1:id.world0,generation:1}).ok_or("joint missing or stale")?;
    b3_world_ensure_gpu(world);
    b3_world_gpu_wait_with_mirror(world);
    let mut worlds=lock_worlds();let w=slot_mut(&mut worlds,world).ok_or("world missing")?;
    let slot=id.index1 as usize-1;
    let meta=w.joint_meta.get(slot).and_then(Option::as_ref).ok_or("joint missing")?;
    if meta.generation!=id.generation {return Err("stale joint generation".into());}
    w.sim.as_mut().ok_or("simulation missing")?.diagnostic_set_revolute_cache(slot,values)
}

fn diagnostic_impulses(id: WorldId, first: u64, count: u32, clear: bool, spherical: bool) -> Result<(usize,usize),String> {
    if first==0 || count==0 {return Err("empty diagnostic body range".into());}
    let end=first.checked_add(u64::from(count)).ok_or("body range overflow")?;
    b3_world_gpu_wait_with_mirror(id);
    let mut worlds=lock_worlds();
    let w=slot_mut(&mut worlds,id).ok_or("world missing")?;
    let slots:Vec<u32>=w.bodies.iter().enumerate().filter_map(|(slot,b)|
        b.as_ref().filter(|b|(first..end).contains(&b.creation_ordinal)).map(|_|slot as u32)).collect();
    if slots.len()!=count as usize {return Err("diagnostic body range is not entirely live".into());}
    let sim=w.sim.as_mut().ok_or("simulation missing")?;
    sim.validate_diagnostic_boundary()?;
    Ok(if spherical {sim.diagnostic_spherical_impulse_control(&slots,clear)}
        else {sim.diagnostic_contact_impulse_control(&slots,clear)})
}

pub fn b3_world_write_core_state(id: WorldId, path: &std::path::Path, frame: u32) -> Result<(), String> {
    b3_world_gpu_wait_with_mirror(id);
    let value = {
        let mut worlds = lock_worlds();
        let w = slot_mut(&mut worlds, id).ok_or("world missing")?;
        let sim = w.sim.as_mut().ok_or("simulation missing; capture after a step")?;
        sim.validate_diagnostic_boundary()?;
        if !sim.diagnostic_pose_policy_matches(w.automatic_pose_snapshots) {
            return Err("GPU pose snapshot policy differs from captured world policy".into());
        }
        let bodies = pollster::block_on(sim.read_bodies());
        let (body_extras, device_centers, force_slots) = sim.read_diagnostic_body_extras();
        // Membership is emitted on live body records below. Reject any entry
        // that would otherwise disappear from the diagnostic representation.
        if force_slots.iter().any(|&slot| w.bodies.get(slot as usize).is_none_or(Option::is_none)) {
            return Err("force-clear membership references an uncaptured body".into());
        }
        let joints = pollster::block_on(sim.read_joints());
        let contacts = pollster::block_on(sim.read_contacts());
        let shape_ids = sim.shape_identities.clone();
        let (params, scratch) = pollster::block_on(sim.read_diagnostic_scratch());
        // This persistent lookup is consumed by later broadphase steps. Its
        // exact contents are derivable from captured joints, but stale device
        // contents must not be silently replaced with that expected value.
        let (joint_filter, overflow) = crate::types::build_joint_filter_table(&joints);
        let filter_end = crate::types::joint_head_live(params.body_count, params.contact_capacity) as usize;
        let filter_start = filter_end.checked_sub(joint_filter.len()).ok_or("invalid joint filter span")?;
        let filter_consumed = params.joint_count != 0 && params.diagnostic_flags
            & (crate::types::DIAG_JOINT_FILTER_SCAN | crate::types::DIAG_JOINT_FILTER_OVERFLOW) == 0;
        if filter_consumed && scratch.get(filter_start..filter_end) != Some(joint_filter.as_slice()) {
            let artifact=path.with_extension("filter-failure.json");
            let joint_records=|items:&[JointGpu]|items.iter().enumerate().map(|(slot,j)|
                json!({"slot":slot,"a":j.a,"b":j.b,"kind":j.kind,"flags":j.flags})).collect::<Vec<_>>();
            let failure=json!({"frame":frame,"body_count":params.body_count,"joint_count":params.joint_count,
                "contact_capacity":params.contact_capacity,"filter_start":filter_start,"filter_end":filter_end,
                "expected":joint_filter,"actual":scratch.get(filter_start..filter_end),
                "device_joints":joint_records(&joints),"host_joints":joint_records(&w.joints)});
            std::fs::write(&artifact,serde_json::to_vec(&failure).map_err(|e|e.to_string())?).map_err(|e|e.to_string())?;
            return Err(format!("GPU joint collision filter differs from captured joints; diagnostic {}",artifact.display()));
        }
        if params.joint_count != 0 && (params.diagnostic_flags & crate::types::DIAG_JOINT_FILTER_OVERFLOW != 0) != overflow {
            return Err("GPU joint collision filter overflow flag mismatch".into());
        }
        let (gpu_shapes, gpu_materials) = sim.read_diagnostic_colliders();
        let geometry_words = sim.read_diagnostic_geometry_words();
        let idle_state = sim.diagnostic_idle_state();
        let gpu_policy = sim.diagnostic_policy_state()?;
        let convex_ccd = ccd_state_trace::capture(sim.read_diagnostic_ccd_scene())?;
        let adapter=json!({"name":sim.report.name,"backend":sim.report.backend,"driver":sim.report.driver,
            "driver_info":sim.report.driver_info,"vendor":sim.report.vendor,"device":sim.report.device});
        let graph_cache = sim.read_diagnostic_graph_cache()?;
        let contact_high_water = sim.read_diagnostic_contact_high_water();
        let contact_hash_words = sim.read_diagnostic_contact_hash();
        let words = |base: u32, count: u32, stride: usize| -> Result<&[u32], String> {
            let start = base.checked_sub(params.hull_base_u32).ok_or("invalid geometry base")? as usize;
            let end = start.checked_add(count as usize * stride).ok_or("geometry span overflow")?;
            geometry_words.get(start..end).ok_or("GPU geometry span out of bounds".into())
        };
        // Only live records and consumed components: point/edge/vertex w is
        // padding, but plane w and triangle flags are physical data.
        let floats = |base, count, width| -> Result<Value, String> {
            Ok(Value::Array(words(base,count,4)?.chunks_exact(4).map(|r| {
                r[..width].iter().map(|&v|f32::from_bits(v).trace()).collect::<Result<Vec<_>,_>>().map(Value::Array)
            }).collect::<Result<Vec<_>,_>>()?))
        };
        let mesh_nodes = words(params.mesh_node_base_u32,params.mesh_node_count,8)?.chunks_exact(8).map(|r| {
            // BVH metadata occupies float-typed storage but contains integer
            // bits, including possible NaN encodings; do not interpret as f32.
            Ok(json!({"lower":([f32::from_bits(r[0]),f32::from_bits(r[1]),f32::from_bits(r[2])].trace()?),
                "upper":([f32::from_bits(r[4]),f32::from_bits(r[5]),f32::from_bits(r[6])].trace()?),
                "data":r[3],"triangle_offset":r[7]}))
        }).collect::<Result<Vec<_>,String>>()?;
        let mix_table = words(params.mix_pair_base_u32,crate::types::MIX_PAIR_CAP,8)?.chunks_exact(8).map(|r| {
            // Probe order and empty slots affect the bounded hash lookup.
            if r[4]==0 { return Ok(Value::Null); }
            Ok(json!({"key":[r[0],r[1]],"friction":f32::from_bits(r[2]).trace()?,
                "restitution":f32::from_bits(r[3]).trace()?,"occupied":r[4]}))
        }).collect::<Result<Vec<_>,String>>()?;
        let gpu_geometry = json!({
            "hull_points":floats(params.hull_base_u32,params.hull_point_count,3)?,
            "hull_planes":floats(params.hull_plane_base_u32,params.hull_plane_count,4)?,
            "hull_edges":floats(params.hull_edge_base_u32,params.hull_edge_count,3)?,
            "hull_topology":words(params.hull_topology_base_u32,params.hull_topology_count,4)?.chunks_exact(4).collect::<Vec<_>>(),
            "mesh_vertices":floats(params.mesh_vertex_base_u32,params.mesh_vertex_count,3)?,
            "mesh_triangles":words(params.mesh_triangle_base_u32,params.mesh_triangle_count,4)?.chunks_exact(4).collect::<Vec<_>>(),
            "mesh_nodes":mesh_nodes,"mix_pair_count":params.mix_pair_count,"mix_table":mix_table});
        let body_key = |slot: u32| -> Result<Value, String> {
            let body = w.bodies.get(slot as usize).and_then(Option::as_ref).ok_or("invalid body reference")?;
            Ok(json!(body.creation_ordinal))
        };
        let shape_key = |slot: u32| -> Result<Value, String> {
            let &(index, generation) = shape_ids.get(slot as usize).ok_or("invalid shape reference")?;
            let shape = w.shapes.get(index as usize).and_then(Option::as_ref)
                .ok_or("shape identity is not live")?;
            if shape.generation != generation { return Err("stale shape identity".into()); }
            // Public shape slots are append-only, so index is creation ordinal.
            Ok(json!(index + 1))
        };
        let mut contact_keys = vec![Value::Null; contacts.len()];
        for (root, c) in contacts.iter().enumerate() {
            if c.a == u32::MAX || c.manifold_link[1] != 0 { continue; }
            let pair = json!([shape_key(c.pair[0])?, shape_key(c.pair[1])?]);
            let mut slot = root;
            let mut ordinal = 0;
            loop {
                let child = contacts.get(slot).ok_or("contact chain out of range")?;
                if !contact_keys[slot].is_null() { return Err("cyclic or shared contact chain".into()); }
                if child.a == u32::MAX || (ordinal != 0 && child.manifold_link[1] != root as u32 + 1) {
                    return Err("invalid contact chain member".into());
                }
                contact_keys[slot] = json!([pair, ordinal]);
                ordinal += 1;
                if child.manifold_link[0] == 0 { break; }
                slot = child.manifold_link[0] as usize - 1;
            }
        }
        if contacts.iter().enumerate().any(|(slot,c)| c.a != u32::MAX && contact_keys[slot].is_null()) {
            return Err("orphan contact chain member".into());
        }
        let contact_key = |slot: usize| -> Result<Value,String> {
            contact_keys.get(slot).filter(|v| !v.is_null()).cloned().ok_or("invalid contact reference".into())
        };
        let layout = crate::types::pair_layout(params.pair_capacity);
        // Retirement can consume this list between steps, before broadphase
        // resets it. Keep physical slots (including EMPTY) and both bounds.
        let candidate_unique_count=scratch[crate::types::SCR_UNIQUE_N as usize];
        let candidate_contact_count=scratch[3]; // WGSL SCR_NCONTACTS
        let candidate_start=(crate::types::SCR_PAIRS+3*params.pair_capacity) as usize;
        let candidate_count=candidate_unique_count.max(candidate_contact_count).min(params.pair_capacity) as usize;
        let candidate_slots=&scratch[candidate_start..candidate_start+candidate_count];
        if contact_high_water>params.contact_capacity || contact_high_water as usize>contacts.len() {
            return Err("invalid contact allocation high-water mark".into());
        }
        if contact_keys[contact_high_water as usize..].iter().any(|key|!key.is_null()) {
            return Err("live contact above allocation high-water mark".into());
        }
        // The allocator chooses the lowest available physical slot. Retain
        // holes and slot-to-semantic-contact relationships, not just a sorted
        // set of current manifolds. Generation counters survive retirement.
        let contact_slots=(0..contact_high_water as usize).map(|slot|json!({
            "contact":contact_keys[slot],"generation":contacts[slot].lifecycle[0]})).collect::<Vec<_>>();
        let occupied_count=scratch[crate::types::SCR_OCCUPIED_N as usize] as usize;
        if occupied_count>params.contact_capacity as usize {return Err("invalid occupied contact count".into());}
        let mut occupied_order=Vec::new();let mut occupied_seen=std::collections::HashSet::new();
        for &slot in &scratch[layout.occupied as usize..layout.occupied as usize+occupied_count] {
            if !occupied_seen.insert(slot) {return Err("duplicate occupied contact slot".into());}
            let contact=contacts.get(slot as usize).ok_or("occupied contact outside storage")?;
            if contact.manifold_link[1]!=0 {return Err("occupied list contains child patch".into());}
            occupied_order.push(contact_key(slot as usize)?);
        }
        let hash_capacity=2*params.pair_capacity as usize;
        let mut hash_buckets=Vec::new();let mut hash_seen=std::collections::HashSet::new();
        for (bucket,&slot) in contact_hash_words[..hash_capacity].iter().enumerate() {
            if slot==u32::MAX {continue;}
            if slot==u32::MAX-1 {hash_buckets.push(json!({"bucket":bucket,"tombstone":true}));continue;}
            let c=contacts.get(slot as usize).ok_or("contact hash slot out of range")?;
            if c.manifold_link[1]!=0 || !hash_seen.insert(slot) {return Err("invalid/duplicate contact hash root".into());}
            let key=contact_key(slot as usize)?;
            let identity=&contact_hash_words[hash_capacity+2*slot as usize..hash_capacity+2*slot as usize+2];
            if identity!=&c.pair[..2] {return Err("contact hash identity mismatch".into());}
            hash_buckets.push(json!({"bucket":bucket,"contact":key}));
        }
        // Empty buckets are implicit; tombstones retain probe-chain history.
        // Unreferenced identity words are overwritten before publication.
        let contact_hash=json!({"capacity":hash_capacity,"buckets":hash_buckets});
        let event_shape = |id:&ShapeId| -> Value {
            if id.index1<=0 {return Value::Null;}
            json!({"identity":id.index1,"live":w.shapes.get(id.index1 as usize-1).and_then(Option::as_ref)
                .is_some_and(|s|s.generation==id.generation)})
        };
        let mut previous_touching=Vec::new();
        let previous_base=layout.previous_touching as usize;
        for slot in 0..contact_high_water as usize {
            let pair=&scratch[previous_base+2*slot..previous_base+2*slot+2];
            if pair==[u32::MAX,u32::MAX] {previous_touching.push(Value::Null);continue;}
            let decode=|index:u32| -> Result<Value,String> {
                let id=w.previous_contact_shape_ids.get(index as usize).ok_or("previous contact shape reference missing")?;
                let child=w.previous_contact_child_ordinals.get(index as usize).ok_or("previous contact child ordinal missing")?;
                Ok(json!({"shape":event_shape(id),"child":child}))
            };
            previous_touching.push(json!([decode(pair[0])?,decode(pair[1])?]));
        }
        let event_history=json!({"previous_touching":previous_touching,
            "current_shapes":w.contact_shape_ids.iter().map(event_shape).collect::<Vec<_>>(),
            "previous_shapes":w.previous_contact_shape_ids.iter().map(event_shape).collect::<Vec<_>>(),
            "current_children":w.contact_child_ordinals.as_ref(),"previous_children":w.previous_contact_child_ordinals.as_ref()});
        let h = layout.history as usize;
        let high = scratch[h] as usize;
        let free_count = scratch[h+1] as usize;
        if high > params.pair_capacity as usize || free_count > high { return Err("invalid contact history header".into()); }
        let mut history = Vec::new();
        for rank in 0..high {
            let r = &scratch[h+2+6*rank..h+2+6*(rank+1)];
            if r[0] == 0 { history.push(Value::Null); continue; }
            let slot = r[0] as usize - 1;
            let c = contacts.get(slot).ok_or("history contact outside storage")?;
            history.push(json!({"contact":contact_key(slot)?,
                "physical_slot":slot,"saved_generation":r[1],
                "generation_matches":r[1]==c.lifecycle[0], "a":body_key(r[2])?, "b":body_key(r[3])?,
                "color_plus_one":r[4], "last_seen_step":r[5]}));
        }
        // The reverse lookup is independently persistent: a stale/missing map
        // changes future ID allocation even if every logical record matches.
        // Retain physical slots and saved generations too, including mismatches.
        let lookup_start = h+2+6*params.pair_capacity as usize;
        let slot_to_rank = &scratch[lookup_start..lookup_start+params.contact_capacity as usize];
        // Logical record ranks and free-stack order influence subsequent coloring.
        let free_start = h+2+7*params.pair_capacity as usize;
        let free = &scratch[free_start..free_start+free_count];
        let mut seen_free = std::collections::HashSet::new();
        for &rank in free {
            if rank as usize >= high || !history[rank as usize].is_null() || !seen_free.insert(rank) {
                return Err("invalid contact history free stack".into());
            }
        }
        let n = params.body_count as usize;
        let color_base = layout.graph as usize + 16*n + 24*8 + 24*8*n;
        let mut color_order = Vec::new();
        for color in 0..24usize {
            let count = scratch[crate::types::SCR_COLOR as usize + color] as usize;
            if count > params.contact_capacity as usize { return Err("invalid color contact count".into()); }
            let start = color_base + color * params.contact_capacity as usize;
            color_order.push((0..count).map(|i|contact_key(scratch[start+i] as usize)).collect::<Result<Vec<_>,_>>()?);
        }
        let mut joint_order = Vec::new();
        let list_ok = scratch[62] == 1;
        if list_ok && params.joint_count != 0 {
            let head = crate::types::joint_head_live(params.body_count, params.contact_capacity) as usize;
            let unique = head + n;
            let offb = unique + n;
            let cntb = offb + n;
            let listb = cntb + n;
            let components = scratch[63] as usize;
            if components > n { return Err("invalid joint component count".into()); }
            for component in 0..components {
                let offset = scratch[offb+component] as usize;
                let count = scratch[cntb+component] as usize;
                if offset + count > n.max(params.joint_count as usize) {
                    return Err("invalid joint component range".into());
                }
                let mut order = Vec::new();
                for &slot in &scratch[listb+offset..listb+offset+count] {
                    let meta = w.joint_meta.get(slot as usize).and_then(Option::as_ref)
                        .ok_or("invalid joint component member")?;
                    let _ = meta; // Validate lifetime; joint slots are append-only.
                    order.push(json!(slot+1));
                }
                joint_order.push(json!({"root":body_key(scratch[unique+component])?,"joints":order}));
            }
        }
        let mut bounds = Vec::new();
        for shape in 0..params.shape_count as usize {
            let start = params.fat_bounds_base as usize + 8*shape;
            bounds.push(json!({"shape":shape_key(shape as u32)?,
                "lower_bits":&scratch[start..start+3],"upper_bits":&scratch[start+4..start+7],
                "initialized":scratch[start+3]==params.fat_bounds_epoch,
                "commands_applied":scratch[start+7]==params.fat_commands_epoch}));
        }
        let mut body_records = Vec::new();
        for (slot, host) in w.bodies.iter().enumerate() {
            let Some(host) = host else { continue; };
            let b = bodies.get(slot).ok_or("missing GPU body")?;
            // The normal BodyGpu export drops this device field. Verify the
            // actual bytes before representing it with the captured host value.
            let center = device_centers.get(slot).ok_or("missing GPU body local center")?;
            if center.map(f32::to_bits) != host.local_center.map(f32::to_bits) {
                return Err(format!("GPU body local center differs from captured host at slot {slot}"));
            }
            let mut r = fields!(b; pos, inv_mass, vel, kind, half, flags, rot, omega, restitution, inv_inertia, friction, gravity_scale, linear_damping, angular_damping, rolling, dp, sleep_time, dq, sleep_velocity, sleep_threshold, origin, origin_valid);
            r.insert("identity".into(), body_key(slot as u32)?);
            r.insert("local_center".into(), host.local_center.trace()?);
            r.insert("local_inertia".into(), host.local_inertia.trace()?);
            r.insert("mass".into(), host.mass.trace()?);
            r.insert("shape_order".into(), json!(host.shape_indices.iter().map(|i|i+1).collect::<Vec<_>>()));
            r.insert("host_geometry".into(), fields!(host; axis, min_extent, max_extent, sleep_threshold).into());
            let extra = body_extras.get(slot).ok_or("missing GPU body extras")?;
            let vec3 = |i: usize| [extra[i],extra[i+1],extra[i+2]].trace();
            r.insert("gpu_motion".into(), json!({
                "inv_inertia_offdiag":vec3(0)?, "minimum_extent":extra[3].trace()?,
                "maximum_extent":vec3(4)?, "sleep_threshold":extra[7].trace()?,
                "step_force":vec3(8)?, "step_torque":vec3(12)?,
                "inv_inertia_upper":vec3(16)?, "clear_force_next_step":force_slots.contains(&(slot as u32))
            }));
            r.insert("pending_force".into(), host.force.trace()?);
            r.insert("pending_torque".into(), host.torque.trace()?);
            // Island labels are allocation-dependent: retain membership instead.
            let mut members: Vec<_> = w.bodies.iter().enumerate().filter_map(|(other, live)| {
                live.as_ref().filter(|_| bodies.get(other).is_some_and(|x| x.island_id == b.island_id))
                    .map(|body| body.creation_ordinal)
            }).collect();
            members.sort_unstable();
            r.insert("island_members".into(), if b.island_id == u32::MAX { Value::Null } else { json!(members) });
            body_records.push(Value::Object(r));
        }
        let mut joint_records = Vec::new();
        for (slot, j) in joints.iter().enumerate() {
            if j.kind == JOINT_NONE { continue; }
            let meta = w.joint_meta.get(slot).and_then(Option::as_ref).ok_or("missing joint metadata")?;
            let mut r = fields!(j; kind, solver_color, anchor_a, hertz, anchor_b, damping, axis, impulse, frame_a_rotation, frame_b_rotation, perp_impulse, flags, revolute_axes_step, angular_impulse, spring_impulse, motor_impulse, lower_impulse, upper_impulse, spring_hertz, spring_damping, target_translation, lower_translation, upper_translation, max_motor_force, motor_speed, motor_angular_velocity, target_rotation, spring_angular_impulse, swing_impulse, motor_angular_impulse, weld_linear_hertz, weld_linear_damping, weld_angular_hertz, weld_angular_damping, weld_linear_impulse, weld_angular_impulse);
            r.insert("motor_spring_angular_impulse".into(),motor_spring_angular_impulse(j)?);
            r.insert("reaction_frames".into(), meta.reaction_frames.trace()?);
            r.insert("force_threshold".into(), meta.force_threshold.trace()?);
            r.insert("torque_threshold".into(), meta.torque_threshold.trace()?);
            r.insert("pending_reaction_frames".into(), match meta.pending_reaction_frames {
                Some((frames, armed)) => json!({"frames":frames.trace()?,"armed":armed}),
                None => Value::Null,
            });
            r.insert("identity".into(), json!(slot + 1));
            r.insert("a".into(), body_key(j.a)?); r.insert("b".into(), body_key(j.b)?);
            joint_records.push(Value::Object(r));
        }
        let mut contact_records = Vec::new();
        for (slot, c) in contacts.iter().enumerate() {
            if c.a == u32::MAX { continue; }
            let mut r = fields!(c; color, count, nx, ny, nz, friction, friction_impulse, twist_impulse, rolling, center_a, center_b, rolling_impulse, tangent_velocity, material_index, cached_relative, cached_rotation_a, cached_rotation_b, prepared_tangent_inv, prepared_softness);
            r.insert("identity".into(), contact_key(slot)?);
            r.insert("solver_local_rank".into(), json!(c.lifecycle[2]));
            // The reverse slot map decides whether the next step reuses this
            // logical history record. A stale/absent entry takes the same
            // allocation path; preserve only a validated logical rank.
            if c.manifold_link[1] == 0 {
                let mapped = scratch[h+2+6*params.pair_capacity as usize+slot];
                let rank = mapped.wrapping_sub(1) as usize;
                let valid = mapped != 0 && rank < high
                    && scratch[h+2+6*rank] == slot as u32+1
                    && scratch[h+2+6*rank+1] == c.lifecycle[0];
                r.insert("history_rank".into(), if valid { json!(rank) } else { Value::Null });
            }
            r.insert("a".into(), body_key(c.a)?); r.insert("b".into(), body_key(c.b)?);
            r.insert("shapes".into(), json!([shape_key(c.pair[0])?, shape_key(c.pair[1])?]));
            // Despite its historical name, _pad_end holds restitution.
            r.insert("restitution".into(), c._pad_end.trace()?);
            if c.count > 4 { return Err("invalid contact point count".into()); }
            let ra = [c.ra0,c.ra1,c.ra2,c.ra3]; let rb = [c.rb0,c.rb1,c.rb2,c.rb3];
            let pra = [c.persistent_ra0,c.persistent_ra1,c.persistent_ra2,c.persistent_ra3];
            let prb = [c.persistent_rb0,c.persistent_rb1,c.persistent_rb2,c.persistent_rb3];
            let feature_ids=contact_feature_ids(c);
            // Matching inspects all lanes to choose feature/mesh identity mode,
            // even when the corresponding manifold point is currently inactive.
            r.insert("feature_id_words".into(),json!(feature_ids));
            r.insert("point_triangle_words".into(),json!(c.point_triangles));
            // Raw SAT type/index and float-separation bits are persistent even
            // when fewer than two manifold points are active.
            r.insert("sat_cache_words".into(),json!([c._tail[4],c._tail[5]]));
            let points = (0..c.count as usize).map(|i| -> Result<Value,String> {
                Ok(json!({"ra":ra[i].trace()?, "rb":rb[i].trace()?,
                    "persistent_ra":pra[i].trace()?, "persistent_rb":prb[i].trace()?,
                    "normal_mass":c.prepared_normal_mass[i].trace()?,
                    "lever_arm":c.prepared_lever_arm[i].trace()?,
                    "total_normal_impulse":c.total_normal_impulse[i].trace()?,
                    "relative_velocity_bits":c._tail[i], "feature_id":feature_ids[i],
                    "triangle":c.point_triangles[i]}))
            }).collect::<Result<Vec<_>,_>>()?;
            r.insert("points".into(), json!(points));
            r.insert("lifecycle_flags".into(), json!(c.lifecycle[1]));
            r.insert("previous_color".into(), json!(c.lifecycle[3]));
            r.insert("chain_length".into(), json!(c.manifold_link[2]));
            r.insert("representative_triangle".into(), json!(c.manifold_link[3]));
            contact_records.push(Value::Object(r));
        }
        // Ignore contact slot allocation, keeping all contact records as a multiset.
        contact_records.sort_by_cached_key(|r| r.to_string());
        let mut pending_transforms = w.pending_fat_transforms.iter().map(|(id,commands)| {
            let body = w.bodies.get(id.0.saturating_sub(1) as usize).and_then(Option::as_ref)
                .ok_or("pending transform references missing body")?;
            if body.generation != id.1 { return Err("pending transform references stale body".into()); }
            Ok(json!({"body":body.creation_ordinal,"commands":commands.iter().map(TraceValue::trace).collect::<Result<Vec<_>,_>>()?}))
        }).collect::<Result<Vec<Value>,String>>()?;
        pending_transforms.sort_by_cached_key(Value::to_string);
        let body_storage_order: Vec<_> = body_records.iter().map(|r|r["identity"].clone()).collect();
        body_records.sort_by_key(|r|r["identity"].as_u64().unwrap());
        let mut geometries = Vec::new();
        let mut geometry_indices = std::collections::HashMap::new();
        let mut shapes = Vec::new();
        for (slot, shape) in w.shapes.iter().enumerate() {
            let Some(shape) = shape else { continue; };
            let nodes = shape.mesh_nodes.iter().map(|n| Ok(Value::Object(fields!(n; lower, upper, data, triangle_offset))))
                .collect::<Result<Vec<_>,String>>()?;
            let geometry = json!({"hull_points":shape.hull_points.trace()?,
                "hull_planes":shape.hull_planes.trace()?,"hull_edges":shape.hull_edges.trace()?,
                "hull_topology":shape.hull_topology.trace()?,"mesh_vertices":shape.mesh_vertices.as_ref().trace()?,
                "mesh_triangles":shape.mesh_triangles.as_ref().trace()?,
                "mesh_triangle_ids":shape.mesh_triangle_ids.as_ref().trace()?,"mesh_nodes":nodes});
            // Exact-content interning, not a lossy fingerprint. Stable shape
            // traversal determines table order; allocation addresses are absent.
            let key = geometry.to_string();
            let geometry_index = *geometry_indices.entry(key).or_insert_with(|| {
                let index = geometries.len(); geometries.push(geometry); index
            });
            let mut record = fields!(shape; kind, public_kind, compound_parent, compound_child_index,
                next_compound_child_index, compound_material_indices, half, axis, density, friction,
                restitution, rolling, tangent_velocity, explosion_scale, event_flags, inner_radius,
                mesh_scale, mass, unit_mass, unit_inertia, geometry_center, local_center, local_inertia);
            record.insert("identity".into(),json!(slot+1));
            record.insert("body".into(),body_key(u32::try_from(shape.body_index-1).map_err(|_|"invalid shape owner")?)?);
            record.insert("geometry".into(),json!(geometry_index));
            record.insert("filter".into(),json!({"category_bits":shape.filter.category_bits,
                "mask_bits":shape.filter.mask_bits,"group_index":shape.filter.group_index}));
            record.insert("user_material_id".into(),json!(shape.user_material_id));
            record.insert("mesh_instance".into(),match shape.mesh_instance {
                Some(instance) => Value::Object(fields!(instance; position, rotation, scale)), None => Value::Null });
            record.insert("materials".into(),Value::Array(shape.mesh_materials.iter().map(|m| {
                let mut r=fields!(m; friction, restitution, rolling_resistance, tangent_velocity);
                r.insert("user_material_id".into(),json!(m.user_material_id)); Ok(Value::Object(r))
            }).collect::<Result<Vec<_>,String>>()?));
            shapes.push(Value::Object(record));
        }
        let body_slots: Vec<_> = w.bodies.iter().map(|b|b.as_ref().map(|b|b.creation_ordinal)).collect();
        for &slot in &w.free_bodies {
            if slot>=body_slots.len() || body_slots[slot].is_some() { return Err("invalid free body slot".into()); }
        }
        let shape_storage_order = (0..shape_ids.len()).map(|i|shape_key(i as u32)).collect::<Result<Vec<_>,_>>()?;
        if gpu_shapes.len() != shape_ids.len() { return Err("GPU shape identity count mismatch".into()); }
        let gpu_colliders = gpu_shapes.iter().enumerate().map(|(slot, shape)| {
            let span = |start:u32,count:u32,total:u32,label:&str| -> Result<(),String> {
                if count!=0 && start.checked_add(count).is_none_or(|end|end>total) {
                    return Err(format!("GPU shape {slot}: invalid {label} span"));
                }
                Ok(())
            };
            if shape.kind==KIND_MESH {
                span(shape.plane_slot,shape.topology_slot,params.mesh_triangle_count,"mesh triangles")?;
                span(shape.edge_slot,shape.topology_counts,params.mesh_node_count,"mesh nodes")?;
                if shape.topology_counts>0 {
                    let nodes=words(params.mesh_node_base_u32,params.mesh_node_count,8)?;
                    let begin=shape.edge_slot as usize*8;
                    geometry_trace::mesh_tree(&nodes[begin..begin+shape.topology_counts as usize*8],shape.topology_slot as usize)
                        .map_err(|e|format!("GPU shape {slot}: {e}"))?;
                }
                for tri in words(params.mesh_triangle_base_u32,params.mesh_triangle_count,4)?
                    .chunks_exact(4).skip(shape.plane_slot as usize).take(shape.topology_slot as usize) {
                    for &index in &tri[..3] { span(shape.hull_slot,index.checked_add(1).ok_or("mesh vertex index overflow")?,params.mesh_vertex_count,"mesh vertex")?; }
                }
            } else {
                span(shape.hull_slot,shape.topology_counts&255,params.hull_point_count,"hull points")?;
                span(shape.plane_slot,(shape.topology_counts>>8)&255,params.hull_plane_count,"hull planes")?;
                span(shape.edge_slot,(shape.topology_counts>>16)&255,params.hull_edge_count,"hull edges")?;
                span(shape.topology_slot,shape.topology_counts>>24,params.hull_topology_count,"hull topology")?;
                let edges=words(params.hull_topology_base_u32,params.hull_topology_count,4)?;
                let count=(shape.topology_counts>>24) as usize;
                if count>0 {
                    let begin=shape.topology_slot as usize*4;
                    geometry_trace::hull_topology(&edges[begin..begin+count*4],(shape.topology_counts&255) as usize,((shape.topology_counts>>8)&255) as usize)
                        .map_err(|e|format!("GPU shape {slot}: {e}"))?;
                }
            }
            // Resolve material slots to values; omit padding and visual color.
            let mut record = fields!(shape; kind, topology_counts, local_center, half, rolling,
                axis, friction, restitution, inner_radius, category_bits_lo, category_bits_hi,
                mask_bits_lo, mask_bits_hi, group_index, event_flags, tangent_velocity,
                user_material_id_lo, user_material_id_hi, instance_position, instance_flags,
                instance_rotation);
            record.insert("identity".into(),shape_key(slot as u32)?);
            record.insert("body".into(),body_key(shape.body_index)?);
            // These legacy "pad" fields are consumed by broadphase and query
            // shaders. Capture their semantic references, not raw slot IDs.
            record.insert("bounds_shape".into(),shape_key(shape._pad_filter[0])?);
            w.shapes.get(shape._pad_filter[1] as usize).and_then(Option::as_ref)
                .ok_or("GPU query shape reference is not live")?;
            record.insert("query_shape".into(),json!(u64::from(shape._pad_filter[1])+1));
            record.insert("initial_order_rank".into(),json!(shape.initial_order[0]));
            // Relative indices into captured live geometry arrays preserve
            // reference relationships without recording GPU buffer addresses.
            record.insert("geometry_slots".into(),json!({"hull":shape.hull_slot,
                "plane":shape.plane_slot,"edge":shape.edge_slot,"topology":shape.topology_slot}));
            let start = shape.material_slot as usize;
            let end = start.checked_add(shape.material_count as usize).ok_or("material span overflow")?;
            let materials = gpu_materials.get(start..end).ok_or("GPU material span out of bounds")?;
            record.insert("materials".into(),Value::Array(materials.iter().map(|m| {
                Ok(Value::Object(fields!(m; friction, restitution, rolling_resistance,
                    tangent_velocity, user_material_id_lo, user_material_id_hi)))
            }).collect::<Result<Vec<_>,String>>()?));
            Ok(Value::Object(record))
        }).collect::<Result<Vec<_>,String>>()?;
        let settings = fields!(params; dt, gravity_x, gravity_y, gravity_z, enable_contacts,
            cell_size, bias_rate, mass_scale, impulse_scale, contact_speed, sleep_threshold,
            solver_mode, enable_sleep, step_dt, contact_hertz, contact_damping, sub_step_count,
            physics_step, enable_continuous, contact_recycle_distance, maximum_linear_speed,
            restitution_threshold, order_enabled, disable_warm_starting);
        let host_contacts=host_contact_state(w,id.index1)?;
        json!({"schema":"gpu-core-state-v22", "frame":frame,"idle_state":idle_state,"graph_cache":graph_cache,
            "adapter":adapter,"convex_ccd":convex_ccd,"gpu_policy":gpu_policy,"host_state":host_state_trace::capture(w,id.index1)?,"host_events":host_events_trace::capture(w,id.index1)?,"host_contacts":host_contacts,"contact_hash":contact_hash,"event_history":event_history,"contact_end_state":contact_end_state(w),
            "contact_allocation":{"high_water":contact_high_water,"slots":contact_slots,"occupied_order":occupied_order,
                "candidate_unique_count":candidate_unique_count,"candidate_contact_count":candidate_contact_count,
                "candidate_slots":candidate_slots},
            "gpu_geometry":gpu_geometry,
            "gpu_colliders":gpu_colliders,
            "shapes":shapes,"geometries":geometries,"shape_storage_order":shape_storage_order,
            "body_allocation":{"slots":body_slots,"free_stack":w.free_bodies,"generations":w.body_generations},
            "body_storage_order":body_storage_order,"settings":settings,"joint_order":{"valid":list_ok,"components":joint_order},
            "scope":"core plus contact history/order, GPU body motion data and pending host loads; remaining engine state requires audit",
            "contact_history":{"records":history,"free_stack":free,"slot_to_rank":slot_to_rank}, "contact_color_order":color_order,
            "fat_bounds":bounds,"pending_transforms":pending_transforms,
            "host_flags":{"scene_dirty":w.scene_dirty,"bodies_dirty":w.bodies_dirty,"topology_dirty":w.topology_dirty,
                "post_ccd_pending":w.post_ccd_pending,"events_pending":w.events_pending},
            "bodies":body_records,"joints":joint_records,"contacts":contact_records})
    };
    let file = std::fs::OpenOptions::new().create(true).append(true).open(path).map_err(|e|e.to_string())?;
    // JSON emits many tiny writes. Buffer them without changing the capture
    // schema or dropping any state, then expose the complete frame to readers.
    let mut file = std::io::BufWriter::with_capacity(1024 * 1024, file);
    serde_json::to_writer(&mut file, &value).map_err(|e|e.to_string())?;
    file.write_all(b"\n").map_err(|e|e.to_string())?;
    // Drop cannot report flush errors; a failed write must fail the capture.
    file.flush().map_err(|e|e.to_string())
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn trace_captures_between_step_retirement_candidates() {
        let gpu=pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let world=b3_create_world(gpu,&crate::api::b3_default_world_def());
        let mut bd=crate::api::b3_default_body_def();
        let a=b3_create_body(world,&bd);
        bd.body_type=BodyType::Dynamic;bd.position=[0.9,0.0,0.0];
        let b=b3_create_body(world,&bd);
        for body in [a,b] {
            b3_create_sphere_shape(body,&crate::api::b3_default_shape_def(),
                &crate::api::Sphere{center:[0.0;3],radius:0.5});
        }
        b3_world_step_gpu(world,1.0/60.0,4);
        let path=std::env::temp_dir().join(format!("gpu-retirement-candidates-{}.jsonl",std::process::id()));
        let _=std::fs::remove_file(&path);
        b3_world_write_core_state(world,&path,1).unwrap();
        let before:Value=serde_json::from_str(std::fs::read_to_string(&path).unwrap().trim()).unwrap();
        let (word,slot,body_a,body_b)=with_world_mut_no_sync(world,|w| {
            let sim=w.sim.as_mut().unwrap();
            let (params,scratch)=pollster::block_on(sim.read_diagnostic_scratch());
            assert_eq!(scratch[crate::types::SCR_UNIQUE_N as usize],1);
            let word=(crate::types::SCR_PAIRS+3*params.pair_capacity) as usize;
            let slot=scratch[word];
            let contacts=pollster::block_on(sim.read_contacts());let c=&contacts[slot as usize];
            assert_ne!(c.a,u32::MAX);
            sim.overwrite_diagnostic_scratch_word_test(word,u32::MAX);
            (word,slot,c.a,c.b)
        }).unwrap();
        b3_world_write_core_state(world,&path,2).unwrap();
        let text=std::fs::read_to_string(&path).unwrap();
        let changed:Value=serde_json::from_str(text.lines().last().unwrap()).unwrap();
        assert_eq!(before["contacts"],changed["contacts"],"mutation only changes candidate workspace");
        with_world_mut_no_sync(world,|w| {
            let sim=w.sim.as_mut().unwrap();
            sim.retire_body_pair_contacts(body_a,body_b);
            assert_ne!(pollster::block_on(sim.read_contacts())[slot as usize].a,u32::MAX,
                "missing candidate prevents selected retirement");
            sim.overwrite_diagnostic_scratch_word_test(word,slot);
            sim.retire_body_pair_contacts(body_a,body_b);
            assert_eq!(pollster::block_on(sim.read_contacts())[slot as usize].a,u32::MAX,
                "restored candidate enables selected retirement");
        }).unwrap();
        b3_destroy_world(world);std::fs::remove_file(path).unwrap();
        assert_ne!(before["contact_allocation"],changed["contact_allocation"],
            "capture must distinguish retirement candidate mutation");
    }
    #[test]
    fn trace_distinguishes_history_lookup_and_saved_generations() {
        let gpu=pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let world=b3_create_world(gpu,&crate::api::b3_default_world_def());
        let ground=b3_create_body(world,&crate::api::b3_default_body_def());
        let sd=crate::api::b3_default_shape_def();
        b3_create_hull_shape(ground,&sd,&crate::api::b3_make_box_hull(5.0,0.5,5.0));
        let mut bd=crate::api::b3_default_body_def();
        bd.body_type=BodyType::Dynamic;bd.position=[0.0,0.99,0.0];
        let body=b3_create_body(world,&bd);
        b3_create_hull_shape(body,&sd,&crate::api::b3_make_box_hull(0.5,0.5,0.5));
        let mut jd=crate::api::b3_default_revolute_joint_def();
        jd.body_a=ground;jd.body_b=body;jd.collide_connected=true;
        b3_create_revolute_joint(world,&jd);
        b3_world_step_gpu(world,1.0/60.0,4);
        let path=std::env::temp_dir().join(format!("gpu-history-lookup-trace-{}.jsonl",std::process::id()));
        let _=std::fs::remove_file(&path);
        let capture=|frame| {
            b3_world_write_core_state(world,&path,frame).expect("history capture");
            let text=std::fs::read_to_string(&path).unwrap();
            serde_json::from_str::<Value>(text.lines().last().unwrap()).unwrap()["contact_history"].clone()
        };
        let baseline=capture(1);
        let (lookup_word,lookup,generation_word,generation)=with_world_mut_no_sync(world,|w| {
            let sim=w.sim.as_mut().unwrap();
            let (params,scratch)=pollster::block_on(sim.read_diagnostic_scratch());
            let h=crate::types::pair_layout(params.pair_capacity).history as usize;
            let rank=(0..scratch[h] as usize).find(|&i|scratch[h+2+6*i]!=0).expect("live history record");
            let record=h+2+6*rank;let slot=scratch[record] as usize-1;
            let word=h+2+6*params.pair_capacity as usize+slot;
            assert_eq!(scratch[word],rank as u32+1);
            (word,scratch[word],record+1,scratch[record+1])
        }).unwrap();
        let write=|word,value|with_world_mut_no_sync(world,|w|
            w.sim.as_ref().unwrap().overwrite_diagnostic_scratch_word_test(word,value)).unwrap();
        write(lookup_word,0);
        let lookup_changed=capture(2);
        write(lookup_word,lookup);
        assert_eq!(capture(3),baseline,"lookup restoration");
        write(generation_word,generation^0x4000_0000);
        let stale_a=capture(4);
        write(generation_word,generation^0x8000_0000);
        let stale_b=capture(5);
        write(generation_word,generation);
        assert_eq!(capture(6),baseline,"generation restoration");
        b3_destroy_world(world);std::fs::remove_file(path).unwrap();
        assert!(baseline!=lookup_changed && stale_a!=stale_b,
            "capture omitted history state: lookup_distinct={}, stale_generations_distinct={}",
            baseline!=lookup_changed,stale_a!=stale_b);
    }
    #[test]
    fn trace_mass_mutations_preserve_analytical_impulse_response() {
        use crate::api::*;
        let gpu=pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let mut wd=b3_default_world_def();wd.gravity=[0.0;3];wd.enable_sleep=false;
        let world=b3_create_world(gpu,&wd);
        let mut bd=b3_default_body_def();bd.body_type=BodyType::Dynamic;
        let body=b3_create_body(world,&bd);
        let mut sd=b3_default_shape_def();sd.density=1000.0;
        let shape=b3_create_hull_shape(body,&sd,&b3_make_box_hull(0.5,0.5,0.5));
        let path=std::env::temp_dir().join(format!("gpu-mass-mutations-{}.jsonl",std::process::id()));
        let _=std::fs::remove_file(&path);
        let mut frame=0;
        for stage in 0..6 {
            match stage {
                0=>b3_body_apply_mass_from_shapes(body),
                1=>b3_shape_set_density(shape,2000.0,true),
                2=>b3_body_set_mass_data(body,MassData{mass:1000.0,center:[0.25,-0.5,0.125],
                    inertia:[[2.0,0.0,0.0],[0.0,3.0,0.0],[0.0,0.0,4.0]]}),
                3=>b3_body_set_type(body,BodyType::Kinematic),
                4=>b3_body_set_type(body,BodyType::Dynamic),
                5=>b3_destroy_shape(shape,true),
                _=>unreachable!(),
            }
            let expected_mass: f32=[1000.0,2000.0,1000.0,0.0,2000.0,0.0][stage];
            assert_eq!(b3_body_get_mass(body).to_bits(),expected_mass.to_bits(),"stage {stage}");
            let mass=b3_body_get_mass_data(body);
            assert_eq!(mass.mass.to_bits(),expected_mass.to_bits());
            if stage==2 {
                assert_eq!(mass.center,[0.25,-0.5,0.125]);
                assert_eq!(mass.inertia,[[2.0,0.0,0.0],[0.0,3.0,0.0],[0.0,0.0,4.0]]);
            }
            // Identity orientation isolates a principal-axis impulse. A unit
            // cube has Izz=m/6; explicit mass uses its independently supplied4.
            let inertia=if stage==2 {4.0} else {expected_mass/6.0};
            b3_body_set_transform(body,[0.0;3],[0.0,0.0,0.0,1.0]);
            b3_body_set_linear_velocity(body,[0.0;3]);
            b3_body_set_angular_velocity(body,[0.0;3]);
            // Nonzero input on zero-mass stages catches stale inverse mass
            // and inertia after type changes or removal of the last shape.
            let linear_impulse=if expected_mass>0.0 {expected_mass} else {1000.0};
            let angular_impulse=if expected_mass>0.0 {inertia} else {4.0};
            b3_body_apply_linear_impulse_to_center(body,[linear_impulse,0.0,0.0],true);
            b3_body_apply_angular_impulse(body,[0.0,0.0,angular_impulse],true);
            let moving=if expected_mass>0.0 {1.0} else {0.0};
            for _ in 0..3 {
                frame+=1;
                b3_world_step_gpu(world,1.0/60.0,4);
                b3_world_write_core_state(world,&path,frame).expect("mass mutation capture");
                let v=b3_body_get_linear_velocity(body);
                let w=b3_body_get_angular_velocity(body);
                for axis in 0..3 {
                    assert!((v[axis]-if axis==0{moving}else{0.0}).abs()<1e-5,"stage {stage} v={v:?}");
                    assert!((w[axis]-if axis==2{moving}else{0.0}).abs()<1e-4,"stage {stage} w={w:?}");
                }
            }
        }
        assert!(!b3_shape_is_valid(shape));
        let text=std::fs::read_to_string(&path).unwrap();
        assert_eq!(text.lines().count(),18);
        if let Ok(export)=std::env::var("GPU_PHYSICS_TEST_TRACE") {
            std::fs::copy(&path,export).unwrap();
        }
        b3_destroy_world(world);
        std::fs::remove_file(path).unwrap();
    }
    #[test]
    fn trace_captures_contact_metrics_refresh_state() {
        let gpu=pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let world=b3_create_world(gpu,&crate::api::b3_default_world_def());
        let mut bd=crate::api::b3_default_body_def();bd.body_type=BodyType::Dynamic;
        let body=b3_create_body(world,&bd);
        b3_create_hull_shape(body,&crate::api::b3_default_shape_def(),&crate::api::b3_make_box_hull(0.5,0.5,0.5));
        b3_world_step_gpu(world,1.0/60.0,4);
        let path=std::env::temp_dir().join(format!("gpu-contact-metrics-policy-{}.jsonl",std::process::id()));
        let _=std::fs::remove_file(&path);
        b3_world_write_core_state(world,&path,1).unwrap();
        let public=b3_world_contact_metrics(world,false);
        assert_eq!(public.known,1);
        with_world_mut_no_sync(world,|w|w.sim.as_mut().unwrap().toggle_diagnostic_contact_metric_test()).unwrap();
        assert_eq!(b3_world_contact_metrics(world,false).candidate_pairs,public.candidate_pairs^1);
        b3_world_write_core_state(world,&path,2).unwrap();
        let frames:Vec<Value>=std::fs::read_to_string(&path).unwrap().lines()
            .map(|line|serde_json::from_str(line).unwrap()).collect();
        let metrics=&frames[0]["gpu_policy"]["contact_metrics"];
        assert_eq!(*metrics,json!({"step":public.snapshot_step,
            "topology_revision":public.snapshot_topology,"state_revision":public.snapshot_state,
            "capacity_loss":public.capacity_loss!=0,"candidate_pairs":public.candidate_pairs,
            "allocated_roots":public.allocated_roots,"allocated_manifold_slots":public.allocated_manifold_slots,
            "touching_roots":public.touching_roots,"non_sensor_roots":public.non_sensor_roots}));
        let mut changed=frames[1]["gpu_policy"].clone();
        assert_eq!(changed["contact_metrics"]["candidate_pairs"],public.candidate_pairs^1);
        changed["contact_metrics"]["candidate_pairs"]=json!(public.candidate_pairs);
        assert_eq!(changed,frames[0]["gpu_policy"],"mutation must change only the selected metric");
        with_world_mut_no_sync(world,|w|w.sim.as_mut().unwrap().toggle_diagnostic_contact_metric_test()).unwrap();
        b3_destroy_world(world);std::fs::remove_file(path).unwrap();
    }
    #[test]
    fn trace_captures_timing_window_replay_policy() {
        let gpu=pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let world=b3_create_world(gpu,&crate::api::b3_default_world_def());
        let mut bd=crate::api::b3_default_body_def();bd.body_type=BodyType::Dynamic;
        let body=b3_create_body(world,&bd);
        b3_create_hull_shape(body,&crate::api::b3_default_shape_def(),&crate::api::b3_make_box_hull(0.5,0.5,0.5));
        b3_world_step_gpu(world,1.0/60.0,4);
        let policy=||with_world_no_sync(world,|w|w.sim.as_ref().unwrap().diagnostic_policy_state().unwrap()).unwrap();
        let before=policy();assert!(before.get("timing_window").is_none());
        with_world_mut_no_sync(world,|w|w.sim.as_mut().unwrap().begin_timing_window(3));
        let mut active=policy();
        assert_eq!(active["timing_window"],json!({"steps":3,"submitted":0}));
        active.as_object_mut().unwrap().remove("timing_window");
        assert_eq!(before,active,"opening metrics must not change unrelated policy fields");
        b3_world_step_gpu(world,1.0/60.0,4);
        let path=std::env::temp_dir().join(format!("gpu-timing-policy-{}.jsonl",std::process::id()));
        let _=std::fs::remove_file(&path);b3_world_write_core_state(world,&path,2).unwrap();
        let frame:Value=serde_json::from_str(std::fs::read_to_string(&path).unwrap().trim()).unwrap();
        assert_eq!(frame["gpu_policy"]["timing_window"],json!({"steps":3,"submitted":1}));
        with_world_mut_no_sync(world,|w| {
            assert!(pollster::block_on(w.sim.as_mut().unwrap().finish_timing_window()).is_some());
        });
        assert!(policy().get("timing_window").is_none(),"finished window must stop suppressing replay");
        b3_destroy_world(world);std::fs::remove_file(path).unwrap();
    }
    #[test]
    fn trace_joint_island_wake_propagation() {
        trace_island_wake_propagation(false);
    }
    #[test]
    fn trace_contact_island_wake_propagation() {
        trace_island_wake_propagation(true);
    }
    fn trace_island_wake_propagation(contact_island: bool) {
        let gpu=pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let mut wd=crate::api::b3_default_world_def();wd.gravity=[0.0;3];
        let world=b3_create_world(gpu,&wd);
        let mut bd=crate::api::b3_default_body_def();bd.body_type=BodyType::Dynamic;
        let mut sd=crate::api::b3_default_shape_def();sd.density=1.0;
        let mut bodies=Vec::new();
        let spacing=if contact_island {1.0}else{2.0};
        for x in [0.0,spacing,2.0*spacing,10.0] {
            bd.position=[x,0.0,0.0];
            let body=b3_create_body(world,&bd);
            b3_create_hull_shape(body,&sd,&crate::api::b3_make_box_hull(0.5,0.5,0.5));
            bodies.push(body);
        }
        if !contact_island {for pair in bodies[..3].windows(2) {
            let mut jd=crate::api::b3_default_distance_joint_def();
            jd.body_a=pair[0];jd.body_b=pair[1];jd.length=2.0;
            b3_create_distance_joint(world,&jd);
        }}
        let path=std::env::temp_dir().join(format!("gpu-island-wake-{contact_island}-{}.jsonl",std::process::id()));
        let _=std::fs::remove_file(&path);
        let mut peak_spin=0.0f32;
        let mut peak_transverse=0.0f32;
        for step in 1..=73 {
            if step==65 {
                assert!(bodies.iter().all(|&b|!b3_body_is_awake(b)),"all bodies must sleep before wake probes");
                b3_body_apply_linear_impulse_to_center(bodies[0],[1.0,0.0,0.0],false);
            }
            if step==66 {b3_body_apply_linear_impulse_to_center(bodies[0],[1.0,0.0,0.0],true);}
            b3_world_step_gpu(world,1.0/60.0,4);
            b3_world_write_core_state(world,&path,step).expect("island wake capture");
            if step==64 && contact_island {
                let contacts=pollster::block_on(b3_world_sync_contacts(world));
                for (a,b) in [(0,1),(1,2)] {
                    assert!(contacts.iter().any(|c|c.count>0 && ((c.a==a && c.b==b)||(c.a==b && c.b==a))),
                        "fixture must establish both touching links before wake probes");
                }
            }
            if step==65 {
                assert!(bodies.iter().all(|&b|!b3_body_is_awake(b)),"wake=false must not wake a sleeping island");
                for &body in &bodies {assert_eq!(b3_body_get_linear_velocity(body),[0.0;3]);}
            }
            if step>=66 {
                assert!(bodies[..3].iter().all(|&b|b3_body_is_awake(b)),"island wake did not propagate at step {step}");
                assert!(!b3_body_is_awake(bodies[3]),"unconnected island woke at step {step}");
                assert_eq!(b3_body_get_position(bodies[3]),[10.0,0.0,0.0]);
                let mut momentum=glam::Vec3::ZERO;let mut energy=0.0;
                for &body in &bodies[..3] {
                    let velocity=glam::Vec3::from_array(b3_body_get_linear_velocity(body));
                    let omega=glam::Vec3::from_array(b3_body_get_angular_velocity(body));
                    assert!(velocity.is_finite() && omega.is_finite());
                    peak_spin=peak_spin.max(omega.length());
                    // Unit-volume cubes at unit density: mass=1, inertia=1/6.
                    momentum+=velocity;energy+=0.5*velocity.length_squared()+omega.length_squared()/12.0;
                }
                assert!((momentum-glam::Vec3::X).length()<1e-4,"island momentum changed: {momentum:?}");
                assert!(energy<=0.5001,"unexplained energy gain: {energy}");
                for pair in bodies[..3].windows(2) {
                    let a=glam::Vec3::from_array(b3_body_get_position(pair[0]));
                    let b=glam::Vec3::from_array(b3_body_get_position(pair[1]));
                    if contact_island {
                        assert!(b.x-a.x>0.99,"contact penetration exceeds 1cm after wake");
                        peak_transverse=peak_transverse.max((b.y-a.y).abs()).max((b.z-a.z).abs());
                    } else {
                        assert!(((b-a).length()-2.0).abs()<0.01,"joint length error after wake");
                    }
                }
            }
        }
        if let Ok(export)=std::env::var("GPU_PHYSICS_TEST_TRACE") {std::fs::copy(&path,export).unwrap();}
        b3_destroy_world(world);
        let spin_limit=if contact_island {1e-3}else{1e-6};
        // Preserve the original failed screen, but retain the whole window for
        // conservation analysis instead of hiding later behavior on early exit.
        assert!(peak_spin<spin_limit && (!contact_island || peak_transverse<0.001),
            "island symmetry screens failed: spin={peak_spin}, spin_limit={spin_limit}, transverse={peak_transverse}, transverse_limit=0.001; trace={}",path.display());
        std::fs::remove_file(path).unwrap();
    }
    #[test]
    fn trace_restitution_matches_analytical_rebound() {
        let gpu=pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let world=b3_create_world(gpu,&crate::api::b3_default_world_def());
        let mut sd=crate::api::b3_default_shape_def();sd.density=1.0;sd.friction=0.0;sd.restitution=0.0;
        let mut bd=crate::api::b3_default_body_def();bd.position=[0.0,-0.5,0.0];
        b3_create_hull_shape(b3_create_body(world,&bd),&sd,&crate::api::b3_make_box_hull(5.0,0.5,5.0));
        bd.body_type=BodyType::Dynamic;
        let mut bodies=Vec::new();
        for (x,e) in [(-2.0,0.0),(0.0,0.5),(2.0,1.0)] {
            bd.position=[x,2.5,0.0];sd.restitution=e;
            let body=b3_create_body(world,&bd);
            b3_create_sphere_shape(body,&sd,&crate::api::Sphere{center:[0.0;3],radius:0.5});
            bodies.push(body);
        }
        let path=std::env::temp_dir().join(format!("gpu-restitution-{}.jsonl",std::process::id()));
        let _=std::fs::remove_file(&path);
        let mut descended=[false;3];let mut rebounding=[false;3];let mut apex_done=[false;3];let mut apex=[0.0f32;3];
        let mut peak_energy=0.0f32;let mut peak_penetration=0.0f32;
        for step in 1..=240 {
            b3_world_step_gpu(world,1.0/60.0,4);
            b3_world_write_core_state(world,&path,step).expect("restitution capture");
            for (i,&body) in bodies.iter().enumerate() {
                let p=glam::Vec3::from_array(b3_body_get_position(body));
                let v=glam::Vec3::from_array(b3_body_get_linear_velocity(body));
                let omega=glam::Vec3::from_array(b3_body_get_angular_velocity(body));
                assert!(p.is_finite() && v.is_finite() && omega.is_finite());
                assert!((p.x-(-2.0+2.0*i as f32)).abs()<0.01 && p.z.abs()<0.01);
                let penetration=0.5-p.y;peak_penetration=peak_penetration.max(penetration);
                // Evaluate the penetration gate after capturing the complete run.
                // A failed screen must not hide later rebound or energy behavior.
                let energy=10.0*p.y+0.5*v.length_squared()+0.05*omega.length_squared();
                peak_energy=peak_energy.max(energy);assert!(energy<25.2,"energy gain step{step} sphere{i}: {energy}");
                if v.y < -1.0 {descended[i]=true;}
                if descended[i] && v.y>0.1 && !apex_done[i] {rebounding[i]=true;}
                if rebounding[i] && !apex_done[i] {
                    apex[i]=apex[i].max(p.y);
                    if v.y<=0.0 {apex_done[i]=true;}
                }
                if i==0 && step>90 {assert!((p.y-0.5).abs()<0.01 && v.length()<0.03,"inelastic settling step{step}: {p:?} {v:?}");}
            }
        }
        assert!(descended.iter().all(|x|*x));
        assert!(apex_done[1] && apex_done[2],"must observe both first rebound apices");
        assert!((apex[1]-1.0).abs()<0.05 && (apex[2]-2.5).abs()<0.05,"incorrect rebound heights: {apex:?}");
        eprintln!("RESTITUTION first_apices={apex:?} peak_energy_j_per_kg={peak_energy} peak_penetration={peak_penetration}");
        if peak_penetration >= 0.03 {
            eprintln!("RESTITUTION failed trace retained at {}", path.display());
            assert!(peak_penetration < 0.03, "penetration screen failed: {peak_penetration}");
        }
        if let Ok(export)=std::env::var("GPU_PHYSICS_TEST_TRACE") {std::fs::copy(&path,export).unwrap();}
        b3_destroy_world(world);std::fs::remove_file(path).unwrap();
    }
    #[test]
    fn joint_filter_survives_body_growth_within_capacity() {
        let gpu=pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let world=b3_create_world(gpu,&crate::api::b3_default_world_def());
        let mut bd=crate::api::b3_default_body_def();bd.body_type=BodyType::Dynamic;
        let a=b3_create_body(world,&bd);bd.position=[0.2,0.0,0.0];let b=b3_create_body(world,&bd);
        for body in [a,b] {b3_create_sphere_shape(body,&crate::api::b3_default_shape_def(),&crate::api::Sphere{center:[0.0;3],radius:0.5});}
        let mut jd=crate::api::b3_default_spherical_joint_def();jd.body_a=a;jd.body_b=b;jd.local_anchor_a=[0.2,0.0,0.0];
        b3_create_spherical_joint(world,&jd);
        b3_world_step_gpu(world,1.0/60.0,4);
        let path=std::env::temp_dir().join(format!("gpu-filter-growth-{}.jsonl",std::process::id()));
        let _=std::fs::remove_file(&path);
        b3_world_write_core_state(world,&path,1).unwrap();
        let caps=with_world_no_sync(world,|w|w.sim.as_ref().unwrap().caps).unwrap();
        bd.position=[100.0,0.0,0.0];let extra=b3_create_body(world,&bd);
        b3_create_sphere_shape(extra,&crate::api::b3_default_shape_def(),&crate::api::Sphere{center:[0.0;3],radius:0.5});
        b3_world_step_gpu(world,1.0/60.0,4);
        let after=with_world_no_sync(world,|w|w.sim.as_ref().unwrap().caps).unwrap();
        assert_eq!(caps.bodies,after.bodies);assert_eq!(caps.joints,after.joints);
        b3_world_write_core_state(world,&path,2).expect("filter table must follow live body count within existing capacity");
        b3_destroy_world(world);std::fs::remove_file(path).unwrap();
    }

    #[test]
    fn revolute_cache_transplant_preserves_nonimpulse_state() {
        let gpu=pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let world=b3_create_world(gpu,&crate::api::b3_default_world_def());
        let anchor=b3_create_body(world,&crate::api::b3_default_body_def());
        let mut ids=Vec::new();
        for x in [0.0,2.0] {
            let mut bd=crate::api::b3_default_body_def();bd.body_type=BodyType::Dynamic;bd.position=[x,2.0,0.0];
            let body=b3_create_body(world,&bd);
            b3_create_hull_shape(body,&crate::api::b3_default_shape_def(),&crate::api::b3_make_box_hull(0.5,0.5,0.5));
            let mut jd=crate::api::b3_default_revolute_joint_def();jd.body_a=anchor;jd.body_b=body;jd.local_anchor_a=bd.position;
            ids.push(b3_create_revolute_joint(world,&jd));
        }
        for _ in 0..4 {b3_world_step_gpu(world,1.0/60.0,4);}
        b3_world_gpu_wait_with_mirror(world);
        let read=||with_world_mut_no_sync(world,|w|pollster::block_on(w.sim.as_mut().unwrap().read_joints())).unwrap();
        let mut expected=read();let values=[1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0];
        b3_joint_diagnostic_set_revolute_cache(ids[0],values).unwrap();
        let j=&mut expected[0];j.angular_impulse=[1.0,2.0,3.0];j.perp_impulse=[4.0,5.0];
        j.spring_impulse=6.0;j.motor_impulse=7.0;j.lower_impulse=8.0;j.upper_impulse=9.0;
        assert_eq!(bytemuck::cast_slice::<_,u8>(&read()),bytemuck::cast_slice::<_,u8>(&expected));
        let mut stale=ids[0];stale.generation+=1;
        assert!(b3_joint_diagnostic_set_revolute_cache(stale,values).is_err());
        assert!(b3_joint_diagnostic_set_revolute_cache(ids[0],[f32::NAN;9]).is_err());
        assert!(b3_joint_diagnostic_set_spherical_cache(ids[0],[0.0;12]).is_err());
        assert_eq!(bytemuck::cast_slice::<_,u8>(&read()),bytemuck::cast_slice::<_,u8>(&expected));
        b3_destroy_world(world);
    }

    #[test]
    fn spherical_impulse_control_preserves_nonimpulse_state() {
        let gpu=pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let world=b3_create_world(gpu,&crate::api::b3_default_world_def());
        let anchor=b3_create_body(world,&crate::api::b3_default_body_def());
        let mut ids=Vec::new();
        for x in [0.0,2.0] {
            let mut bd=crate::api::b3_default_body_def();bd.body_type=BodyType::Dynamic;bd.position=[x,2.0,0.0];
            bd.rotation=glam::Quat::from_euler(glam::EulerRot::XYZ,0.3,0.2,0.4).to_array();
            let body=b3_create_body(world,&bd);
            b3_create_hull_shape(body,&crate::api::b3_default_shape_def(),&crate::api::b3_make_box_hull(0.5,0.5,0.5));
            let mut jd=crate::api::b3_default_spherical_joint_def();jd.body_a=anchor;jd.body_b=body;jd.local_anchor_a=bd.position;
            jd.enable_cone_limit=true;jd.cone_angle=0.01;jd.enable_twist_limit=true;jd.lower_twist_angle=-0.01;jd.upper_twist_angle=0.01;
            jd.enable_motor=true;jd.max_motor_torque=10.0;jd.motor_velocity=[0.2,0.3,0.4];
            jd.enable_spring=true;jd.spring_hertz=1.0;jd.spring_damping=0.7;
            ids.push(b3_create_spherical_joint(world,&jd));
        }
        for _ in 0..8 {b3_world_step_gpu(world,1.0/60.0,4);}
        b3_world_gpu_wait_with_mirror(world);
        let read=||with_world_mut_no_sync(world,|w|pollster::block_on(w.sim.as_mut().unwrap().read_joints())).unwrap();
        let before=read();
        assert_eq!(b3_world_diagnostic_spherical_impulses(world,2,1,false).unwrap(),(1,1));
        assert_eq!(bytemuck::cast_slice::<_,u8>(&read()),bytemuck::cast_slice::<_,u8>(&before));
        b3_world_diagnostic_spherical_impulses(world,2,1,true).unwrap();
        let mut expected=before;
        let j=expected.iter_mut().find(|j|j.b==1).unwrap();
        j.angular_impulse=[0.0;3];j.spring_angular_impulse=[0.0;3];j.motor_angular_impulse=[0.0;3];
        j.swing_impulse=0.0;j.lower_impulse=0.0;j.upper_impulse=0.0;
        assert_eq!(bytemuck::cast_slice::<_,u8>(&read()),bytemuck::cast_slice::<_,u8>(&expected),"only selected spherical impulses may change");
        assert_eq!(b3_world_diagnostic_spherical_impulses(world,2,1,false).unwrap(),(1,0));
        let values=[1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0,10.0,11.0,12.0];
        b3_joint_diagnostic_set_spherical_cache(ids[0],values).unwrap();
        let j=expected.iter_mut().find(|j|j.b==1).unwrap();
        j.angular_impulse=[1.0,2.0,3.0];j.spring_angular_impulse=[4.0,5.0,6.0];j.motor_angular_impulse=[7.0,8.0,9.0];
        j.lower_impulse=10.0;j.upper_impulse=11.0;j.swing_impulse=12.0;
        assert_eq!(bytemuck::cast_slice::<_,u8>(&read()),bytemuck::cast_slice::<_,u8>(&expected));
        let mut stale=ids[0];stale.generation+=1;
        assert!(b3_joint_diagnostic_set_spherical_cache(stale,values).is_err());
        assert!(b3_joint_diagnostic_set_spherical_cache(ids[0],[f32::NAN;12]).is_err());
        assert_eq!(bytemuck::cast_slice::<_,u8>(&read()),bytemuck::cast_slice::<_,u8>(&expected));
        b3_destroy_world(world);
    }
    #[test]
    fn trace_sliding_friction_matches_analytical_motion() {
        let gpu=pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let world=b3_create_world(gpu,&crate::api::b3_default_world_def());
        let mut sd=crate::api::b3_default_shape_def();sd.density=1.0;sd.friction=0.5;sd.restitution=0.0;
        let mut bd=crate::api::b3_default_body_def();bd.position=[0.0,-0.5,0.0];
        b3_create_hull_shape(b3_create_body(world,&bd),&sd,&crate::api::b3_make_box_hull(15.0,0.5,5.0));
        bd.body_type=BodyType::Dynamic;bd.linear_velocity=[2.0,0.0,0.0];
        let mut bodies=Vec::new();
        for (z,friction) in [(-2.0,0.5),(2.0,0.0)] {
            bd.position=[0.0,0.5,z];sd.friction=friction;
            let body=b3_create_body(world,&bd);
            b3_create_hull_shape(body,&sd,&crate::api::b3_make_box_hull(0.5,0.5,0.5));
            bodies.push(body);
        }
        let path=std::env::temp_dir().join(format!("gpu-slide-friction-{}.jsonl",std::process::id()));
        let _=std::fs::remove_file(&path);
        let mut final_stop=0.0;let mut peak_control_error=0.0f32;
        for step in 1..=240 {
            b3_world_step_gpu(world,1.0/60.0,4);
            b3_world_write_core_state(world,&path,step).expect("sliding capture");
            for (i,&body) in bodies.iter().enumerate() {
                let p=glam::Vec3::from_array(b3_body_get_position(body));
                let v=glam::Vec3::from_array(b3_body_get_linear_velocity(body));
                let omega=glam::Vec3::from_array(b3_body_get_angular_velocity(body));
                assert!(p.is_finite() && v.is_finite() && omega.is_finite());
                assert!((p.y-0.5).abs()<0.01 && (p.z-if i==0 {-2.0}else{2.0}).abs()<0.01,"support step{step}: {p:?}");
                if step>60 {assert!(omega.length()<0.1,"rotation step{step}: {omega:?}");}
                if i==0 {
                    if step>36 {assert!(v.length()<0.05,"sliding did not stop step{step}: {v:?}");}
                    if step>60 {assert!((p.x-0.4).abs()<0.04,"stopping distance step{step}: {}",p.x);}
                    final_stop=p.x;
                } else {
                    let error=(p.x-2.0*step as f32/60.0).abs();peak_control_error=peak_control_error.max(error);
                    assert!(error<0.005 && (v.x-2.0).abs()<0.005,"frictionless control step{step}: {p:?} {v:?}");
                }
            }
        }
        eprintln!("SLIDING_FRICTION stopping_distance={final_stop} peak_control_position_error={peak_control_error}");
        if let Ok(export)=std::env::var("GPU_PHYSICS_TEST_TRACE") {std::fs::copy(&path,export).unwrap();}
        b3_destroy_world(world);std::fs::remove_file(path).unwrap();
    }
    #[test]
    fn contact_impulse_control_preserves_nonimpulse_state() {
        let gpu=pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let world=b3_create_world(gpu,&crate::api::b3_default_world_def());
        let sd=crate::api::b3_default_shape_def();
        let mut bd=crate::api::b3_default_body_def();bd.position=[0.0,-0.5,0.0];
        b3_create_hull_shape(b3_create_body(world,&bd),&sd,&crate::api::b3_make_box_hull(5.0,0.5,5.0));
        bd.body_type=BodyType::Dynamic;
        for x in [0.0,2.0] {
            bd.position=[x,0.49,0.0];
            b3_create_hull_shape(b3_create_body(world,&bd),&sd,&crate::api::b3_make_box_hull(0.5,0.5,0.5));
        }
        for _ in 0..8 {b3_world_step_gpu(world,1.0/60.0,4);}
        b3_world_gpu_wait_with_mirror(world);
        let read=||with_world_mut_no_sync(world,|w|pollster::block_on(w.sim.as_mut().unwrap().read_contacts())).unwrap();
        let before=read();
        let (selected,nonzero)=b3_world_diagnostic_contact_impulses(world,2,1,false).unwrap();
        assert!(selected>0 && nonzero>0);
        assert_eq!(bytemuck::cast_slice::<_,u8>(&read()),bytemuck::cast_slice::<_,u8>(&before),"sham must preserve every contact field");
        assert!(b3_world_diagnostic_contact_impulses(world,100,1,true).is_err());
        b3_world_diagnostic_contact_impulses(world,2,1,true).unwrap();
        let mut expected=before;
        for c in &mut expected {
            if c.a==u32::MAX || c.count==0 || !(c.a==1 || c.b==1) {continue;}
            c.rb0[3]=0.0;c.rb1[3]=0.0;c.rb2[3]=0.0;c.rb3[3]=0.0;
            c.friction_impulse=[0.0;2];c.twist_impulse=0.0;c.rolling_impulse=[0.0;3];
        }
        assert_eq!(bytemuck::cast_slice::<_,u8>(&read()),bytemuck::cast_slice::<_,u8>(&expected),"only selected cached impulses may change");
        assert_eq!(b3_world_diagnostic_contact_impulses(world,2,1,false).unwrap().1,0);
        b3_destroy_world(world);
    }
    #[test]
    fn trace_stack_support_and_energy() {
        let gpu=pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let world=b3_create_world(gpu,&crate::api::b3_default_world_def());
        let mut sd=crate::api::b3_default_shape_def();sd.density=1.0;sd.restitution=0.0;
        let mut bd=crate::api::b3_default_body_def();bd.position=[0.0,-0.5,0.0];
        b3_create_hull_shape(b3_create_body(world,&bd),&sd,&crate::api::b3_make_box_hull(5.0,0.5,5.0));
        bd.body_type=BodyType::Dynamic;
        let mut bodies=Vec::new();
        for i in 0..3 {
            bd.position=[0.0,0.51+1.01*i as f32,0.0];
            let body=b3_create_body(world,&bd);
            b3_create_hull_shape(body,&sd,&crate::api::b3_make_box_hull(0.5,0.5,0.5));
            bodies.push(body);
        }
        let masses:Vec<f64>=bodies.iter().map(|&b|b3_body_get_mass(b) as f64).collect();
        assert!(masses.iter().all(|m|(m-1.0).abs()<1e-6));
        let total_mass:f64=masses.iter().sum();
        let initial_energy:f64=bodies.iter().zip(&masses)
            .map(|(&b,&m)|10.0*m*b3_body_get_position(b)[1] as f64).sum::<f64>()/total_mass;
        let path=std::env::temp_dir().join(format!("gpu-stack-support-{}.jsonl",std::process::id()));
        let _=std::fs::remove_file(&path);
        let mut peak_gain=0.0f64;let mut peak_overlap=0.0f32;let mut tail_speed=0.0f32;
        for step in 1..=600 {
            b3_world_step_gpu(world,1.0/60.0,4);
            b3_world_write_core_state(world,&path,step).expect("stack state capture");
            let mut energy=0.0f64;let mut previous_top=0.0f32;
            for (i,(&body,&mass)) in bodies.iter().zip(&masses).enumerate() {
                let p=glam::Vec3::from_array(b3_body_get_position(body));
                let q=glam::Quat::from_array(b3_body_get_rotation(body));
                let v=glam::Vec3::from_array(b3_body_get_linear_velocity(body));
                let omega=glam::Vec3::from_array(b3_body_get_angular_velocity(body));
                assert!(p.is_finite() && q.is_finite() && v.is_finite() && omega.is_finite());
                assert!(p.x.abs()<0.025 && p.z.abs()<0.025,"lateral drift step{step} body{i}: {p:?}");
                assert!(glam::Vec3::new(q.x,q.y,q.z).length()<0.01,"stack tilt step{step}: {q:?}");
                let half_height=0.5*((q*glam::Vec3::X).y.abs()+(q*glam::Vec3::Y).y.abs()+(q*glam::Vec3::Z).y.abs());
                let overlap=previous_top-(p.y-half_height);peak_overlap=peak_overlap.max(overlap);
                assert!(overlap<0.025,"overlap step{step} body{i}: {overlap}");
                previous_top=p.y+half_height;
                energy+=mass*(10.0*p.y as f64+0.5*v.length_squared() as f64+omega.length_squared() as f64/12.0);
                if step>480 {
                    assert!((p.y-(0.5+i as f32)).abs()<0.025,"support height step{step}: {p:?}");
                    assert!(v.length()<0.05 && omega.length()<0.1,"unsettled step{step}: {v:?} {omega:?}");
                    tail_speed=tail_speed.max(v.length());
                }
            }
            let gain=energy/total_mass-initial_energy;peak_gain=peak_gain.max(gain);
            assert!(gain<0.1,"mechanical energy increase step{step}: {gain} J/kg");
        }
        eprintln!("STACK_SUPPORT peak_energy_gain_j_per_kg={peak_gain} peak_vertical_overlap={peak_overlap} tail_speed={tail_speed}");
        if let Ok(export)=std::env::var("GPU_PHYSICS_TEST_TRACE") {std::fs::copy(&path,export).unwrap();}
        b3_destroy_world(world);std::fs::remove_file(path).unwrap();
    }
    #[test]
    fn trace_rejects_uncaptured_host_policy_and_force_membership() {
        let gpu=pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let world=b3_create_world(gpu,&crate::api::b3_default_world_def());
        let mut bd=crate::api::b3_default_body_def();bd.body_type=BodyType::Dynamic;
        let body=b3_create_body(world,&bd);
        b3_create_hull_shape(body,&crate::api::b3_default_shape_def(),&crate::api::b3_make_box_hull(0.5,0.5,0.5));
        b3_world_step_gpu(world,1.0/60.0,4);
        let path=std::env::temp_dir().join(format!("gpu-policy-membership-{}.jsonl",std::process::id()));
        let _=std::fs::remove_file(&path);
        b3_world_write_core_state(world,&path,1).unwrap();
        let before=std::fs::read(&path).unwrap();
        with_world_mut_no_sync(world,|w| {
            w.sim.as_mut().unwrap().set_automatic_pose_snapshots(!w.automatic_pose_snapshots);
        }).unwrap();
        assert!(b3_world_write_core_state(world,&path,2).unwrap_err().contains("pose snapshot policy"));
        assert_eq!(std::fs::read(&path).unwrap(),before);
        with_world_mut_no_sync(world,|w| {
            let sim=w.sim.as_mut().unwrap();
            sim.set_automatic_pose_snapshots(w.automatic_pose_snapshots);
            sim.shape_identities[0].1+=1;
        }).unwrap();
        assert!(b3_world_write_core_state(world,&path,2).unwrap_err().contains("stale shape identity"));
        assert_eq!(std::fs::read(&path).unwrap(),before);
        with_world_mut_no_sync(world,|w| {
            let sim=w.sim.as_mut().unwrap();
            sim.shape_identities[0].1-=1;
            sim.corrupt_diagnostic_force_membership_test();
        }).unwrap();
        assert!(b3_world_write_core_state(world,&path,2).unwrap_err().contains("uncaptured body"));
        assert_eq!(std::fs::read(&path).unwrap(),before);
        b3_destroy_world(world);std::fs::remove_file(path).unwrap();
    }
    #[test]
    fn trace_boundary_rejects_unconsumed_pose_readback() {
        let gpu=pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let world=b3_create_world(gpu,&crate::api::b3_default_world_def());
        let mut bd=crate::api::b3_default_body_def();bd.body_type=BodyType::Dynamic;
        let body=b3_create_body(world,&bd);
        b3_create_hull_shape(body,&crate::api::b3_default_shape_def(),&crate::api::b3_make_box_hull(0.5,0.5,0.5));
        b3_world_step_gpu(world,1.0/60.0,4);
        let path=std::env::temp_dir().join(format!("gpu-pose-boundary-{}.jsonl",std::process::id()));
        let _=std::fs::remove_file(&path);
        b3_world_write_core_state(world,&path,1).expect("completed mirror supersedes staged poses");
        with_world_mut_no_sync(world,|w| {
            let sim=w.sim.as_mut().unwrap();
            sim.validate_diagnostic_boundary().unwrap();
            sim.reopen_diagnostic_pose_readback_test();
            assert!(sim.validate_diagnostic_boundary().unwrap_err().contains("superseded pose readback"));
            sim.accept_body_mirror_as_pose_snapshot();
            sim.validate_diagnostic_boundary().unwrap();
        }).unwrap();
        let before=b3_body_get_position(body);
        b3_world_prepare_pose_snapshot(world);
        assert_eq!(b3_body_get_position(body),before);
        b3_destroy_world(world);std::fs::remove_file(path).unwrap();
    }
    #[test]
    fn trace_rejects_stale_device_body_center() {
        let gpu=pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let world=b3_create_world(gpu,&crate::api::b3_default_world_def());
        let mut bd=crate::api::b3_default_body_def();bd.body_type=BodyType::Dynamic;
        let body=b3_create_body(world,&bd);
        b3_create_hull_shape(body,&crate::api::b3_default_shape_def(),&crate::api::b3_make_box_hull(0.5,0.5,0.5));
        b3_world_step_gpu(world,1.0/60.0,4);
        let path=std::env::temp_dir().join(format!("gpu-body-center-trace-{}.jsonl",std::process::id()));
        let _=std::fs::remove_file(&path);
        b3_world_write_core_state(world,&path,1).unwrap();
        let before=std::fs::read(&path).unwrap();
        for center in [[0.125,0.0,0.0],[f32::NAN,0.0,0.0]] {
            with_world_mut_no_sync(world,|w| {
                w.sim.as_ref().unwrap().overwrite_diagnostic_body_center_test(0,center);
            }).unwrap();
            let error=b3_world_write_core_state(world,&path,2).expect_err("device center must be checked");
            assert!(error.contains("GPU body local center differs"),"{error}");
            assert_eq!(std::fs::read(&path).unwrap(),before,"invalid capture must not append");
        }
        with_world_mut_no_sync(world,|w| {
            w.sim.as_ref().unwrap().overwrite_diagnostic_body_center_test(0,[0.0;3]);
        }).unwrap();
        b3_world_write_core_state(world,&path,2).unwrap();
        b3_destroy_world(world);std::fs::remove_file(path).unwrap();
    }
    #[test]
    fn trace_rejects_stale_device_joint_filter() {
        let gpu=pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let world=b3_create_world(gpu,&crate::api::b3_default_world_def());
        let mut bd=crate::api::b3_default_body_def();bd.body_type=BodyType::Dynamic;
        let body=b3_create_body(world,&bd);
        b3_create_hull_shape(body,&crate::api::b3_default_shape_def(),&crate::api::b3_make_box_hull(0.5,0.5,0.5));
        let ground=b3_create_body(world,&crate::api::b3_default_body_def());
        let mut jd=crate::api::b3_default_motor_joint_def();jd.body_a=ground;jd.body_b=body;
        b3_create_motor_joint(world,&jd);
        b3_world_step_gpu(world,1.0/60.0,4);
        let path=std::env::temp_dir().join(format!("gpu-filter-trace-{}.jsonl",std::process::id()));
        let _=std::fs::remove_file(&path);
        b3_world_write_core_state(world,&path,1).expect("valid derived table");
        let before=std::fs::read(&path).unwrap();
        with_world_mut_no_sync(world,|w|w.sim.as_ref().unwrap().corrupt_joint_filter_test()).unwrap();
        let error=b3_world_write_core_state(world,&path,2).unwrap_err();
        assert!(error.contains("joint collision filter differs"),"{error}");
        assert_eq!(std::fs::read(&path).unwrap(),before,"invalid capture must not append a frame");
        b3_destroy_world(world);std::fs::remove_file(path).unwrap();
    }
    #[test]
    fn trace_rejects_undrained_contact_status() {
        let gpu=pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let world=b3_create_world(gpu,&crate::api::b3_default_world_def());
        let mut bd=crate::api::b3_default_body_def();bd.body_type=BodyType::Dynamic;
        let body=b3_create_body(world,&bd);
        b3_create_hull_shape(body,&crate::api::b3_default_shape_def(),&crate::api::b3_make_box_hull(0.5,0.5,0.5));
        b3_world_step_gpu(world,1.0/60.0,4);
        with_world_mut_no_sync(world,|w|w.sim.as_mut().unwrap().hold_idle_status_test(true)).unwrap();
        b3_world_step_gpu(world,1.0/60.0,4);
        let path=std::env::temp_dir().join(format!("undrained-status-{}.jsonl",std::process::id()));
        let _=std::fs::remove_file(&path);
        let error=b3_world_write_core_state(world,&path,1).unwrap_err();
        assert!(error.contains("drained contact status"),"{error}");
        assert!(!path.exists(),"failed capture must not publish a partial frame");
        with_world_mut_no_sync(world,|w|w.sim.as_mut().unwrap().hold_idle_status_test(false)).unwrap();
        b3_world_write_core_state(world,&path,1).unwrap();
        let frame:Value=serde_json::from_str(&std::fs::read_to_string(&path).unwrap()).unwrap();
        assert_eq!(frame["gpu_policy"]["completed_step"],frame["idle_state"]["physics_step"]);
        b3_destroy_world(world);std::fs::remove_file(path).unwrap();
    }

    #[test]
    fn trace_captures_inactive_contact_identity_lanes() {
        let gpu=pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let mut wd=crate::api::b3_default_world_def();wd.gravity=[0.0;3];
        let world=b3_create_world(gpu,&wd);
        let mut bd=crate::api::b3_default_body_def();
        let a=b3_create_body(world,&bd);
        bd.body_type=BodyType::Dynamic;bd.position=[0.9,0.0,0.0];
        let b=b3_create_body(world,&bd);
        for body in [a,b] {
            b3_create_sphere_shape(body,&crate::api::b3_default_shape_def(),&crate::api::Sphere{center:[0.0;3],radius:0.5});
        }
        b3_world_step_gpu(world,1.0/60.0,4);
        let path=std::env::temp_dir().join(format!("gpu-inactive-contact-ids-{}.jsonl",std::process::id()));
        let _=std::fs::remove_file(&path);
        b3_world_write_core_state(world,&path,1).unwrap();
        with_world_mut_no_sync(world,|w| {
            let sim=w.sim.as_mut().unwrap();
            let contacts=pollster::block_on(sim.read_contacts());
            let (slot,c)=contacts.iter().enumerate().find(|(_,c)|c.a!=u32::MAX).unwrap();
            assert_eq!(c.count,1);
            sim.overwrite_diagnostic_inactive_contact_ids_test(slot as u32);
        }).unwrap();
        b3_world_write_core_state(world,&path,2).unwrap();
        let frames:Vec<Value>=std::fs::read_to_string(&path).unwrap().lines().map(|s|serde_json::from_str(s).unwrap()).collect();
        assert_eq!(frames[0]["schema"],"gpu-core-state-v22");
        let first=&frames[0]["contacts"][0];let second=&frames[1]["contacts"][0];
        assert_eq!(first["count"],1);
        assert_eq!(first["points"],second["points"],"active point unchanged");
        assert_eq!(first["feature_id_words"][2],0);
        assert_eq!(second["feature_id_words"][2],37);
        assert_eq!(first["point_triangle_words"][3],0);
        assert_eq!(second["point_triangle_words"][3],53);
        if let Ok(export)=std::env::var("GPU_PHYSICS_TEST_TRACE") {std::fs::copy(&path,export).unwrap();}
        b3_destroy_world(world);std::fs::remove_file(path).unwrap();
    }
    #[test]
    fn trace_decodes_semantic_legacy_padding_lanes() {
        let mut c=crate::types::ContactGpu::zeroed();c.count=4;c.ny=1.0;
        c._tail[4]=0x12345678;c._tail[5]=(-0.25f32).to_bits();
        c._tail[6]=11;c._tail[7]=22;c._pad_ca=f32::from_bits(33);c._pad_cb=f32::from_bits(0x7fc01234);
        assert_eq!(contact_feature_ids(&c),[11,22,33,0x7fc01234]);
        let decoded=crate::api::contact_data::decode_manifold(&c).unwrap();
        assert_eq!(decoded.points.map(|p|p.feature_id),contact_feature_ids(&c));
        let mut j=JointGpu{kind:JOINT_MOTOR,motor_impulse:1.0,_pad2:[2.0,3.0],..JointGpu::default()};
        assert_eq!(motor_spring_angular_impulse(&j).unwrap(),json!(["3f800000","40000000","40400000"]));
        j._pad2[1]=f32::NAN;assert!(motor_spring_angular_impulse(&j).is_err());
        j.kind=JOINT_DISTANCE;assert!(motor_spring_angular_impulse(&j).unwrap().is_null());
    }
    #[test]
    fn trace_captures_active_motor_spring_angular_impulses() {
        let gpu=pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let mut wd=crate::api::b3_default_world_def();wd.gravity=[0.0;3];
        let world=b3_create_world(gpu,&wd);
        let anchor=b3_create_body(world,&crate::api::b3_default_body_def());
        let mut bd=crate::api::b3_default_body_def();bd.body_type=BodyType::Dynamic;
        bd.rotation=glam::Quat::from_euler(glam::EulerRot::XYZ,0.1,0.2,0.3).to_array();
        let body=b3_create_body(world,&bd);
        b3_create_hull_shape(body,&crate::api::b3_default_shape_def(),&crate::api::b3_make_box_hull(0.5,0.5,0.5));
        let mut jd=crate::api::b3_default_motor_joint_def();jd.body_a=anchor;jd.body_b=body;
        jd.angular_hertz=5.0;jd.angular_damping=0.7;jd.max_spring_torque=100.0;
        b3_create_motor_joint(world,&jd);
        b3_world_step_gpu(world,1.0/60.0,4);
        let path=std::env::temp_dir().join(format!("gpu-motor-spring-trace-{}.jsonl",std::process::id()));
        let _=std::fs::remove_file(&path);b3_world_write_core_state(world,&path,1).unwrap();
        let frame:Value=serde_json::from_str(std::fs::read_to_string(&path).unwrap().trim()).unwrap();
        let mut worlds=lock_worlds();let w=slot_mut(&mut worlds,world).unwrap();
        let joints=pollster::block_on(w.sim.as_mut().unwrap().read_joints());
        assert!(joints[0]._pad2.iter().all(|x|x.abs()>1.0e-8),"fixture must exercise both legacy padding lanes");
        assert_eq!(frame["joints"][0]["motor_spring_angular_impulse"],motor_spring_angular_impulse(&joints[0]).unwrap());
        drop(worlds);b3_destroy_world(world);std::fs::remove_file(path).unwrap();
    }
    #[cfg(all(feature="native-command-cache",not(target_arch="wasm32")))]
    #[test]
    fn trace_covers_native_full_replay_and_reentry() {
        let gpu=pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let world=b3_create_world(gpu,&crate::api::b3_default_world_def());
        with_world_mut_no_sync(world,|w| {
            w.component_tgs_requested=true;
            w.gpu_idle_requested=false;
        });
        b3_world_enable_sleeping(world,false);
        b3_world_set_diagnostic_flags(world,crate::types::DIAG_BOUNDED_STATIC_SORT);
        let sd=crate::api::b3_default_shape_def();
        let mut bd=crate::api::b3_default_body_def();bd.position=[0.0,-0.5,0.0];
        b3_create_hull_shape(b3_create_body(world,&bd),&sd,&crate::api::b3_make_box_hull(5.0,0.5,5.0));
        bd.body_type=BodyType::Dynamic;
        let mut bodies=Vec::new();
        for i in 0..3 {
            bd.position=[0.0,0.49+i as f32,0.0];
            let body=b3_create_body(world,&bd);
            b3_create_hull_shape(body,&sd,&crate::api::b3_make_box_hull(0.5,0.5,0.5));
            bodies.push(body);
        }
        b3_world_ensure_gpu(world);
        with_world_mut_no_sync(world,|w|w.sim.as_mut().unwrap().set_full_replay_test(true));
        let path=std::env::temp_dir().join(format!("gpu-native-replay-trace-{}.jsonl",std::process::id()));
        let _=std::fs::remove_file(&path);
        let mut hits=Vec::new();
        for step in 1..=48 {
            if step==32 {
                b3_body_set_transform(bodies[2],[0.1,3.0,0.0],[0.0,0.0,0.0,1.0]);
            }
            b3_world_step_gpu(world,1.0/60.0,if (20..24).contains(&step){2}else{4});
            b3_world_write_core_state(world,&path,step).expect("full replay capture");
            hits.push(with_world_no_sync(world,|w|w.sim.as_ref().unwrap().physics_replay_hits()).unwrap());
        }
        assert!(hits[18]>=16,"must reuse before substep changes: {hits:?}");
        assert!(hits[30]>=hits[23]+5,"must resume after substep changes: {hits:?}");
        assert!(hits[47]>=hits[32]+12,"must resume after transform upload: {hits:?}");
        let text=std::fs::read_to_string(&path).unwrap();
        let frames:Vec<Value>=text.lines().map(|s|serde_json::from_str(s).unwrap()).collect();
        assert_eq!(frames.len(),48);
        assert!(frames.iter().any(|f|!f["contacts"].as_array().unwrap().is_empty()),"must exercise collision state");
        eprintln!("FULL_STATE_REPLAY_HITS {hits:?}");
        if let Ok(export)=std::env::var("GPU_PHYSICS_TEST_TRACE") {
            std::fs::copy(&path,export).unwrap();
        }
        b3_destroy_world(world);
        std::fs::remove_file(path).unwrap();
    }
    #[test]
    fn trace_prismatic_motor_reversal_respects_both_limits() {
        let gpu=pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let mut wd=crate::api::b3_default_world_def();wd.gravity=[0.0;3];
        let world=b3_create_world(gpu,&wd);
        let anchor=b3_create_body(world,&crate::api::b3_default_body_def());
        let mut bd=crate::api::b3_default_body_def();bd.body_type=BodyType::Dynamic;
        let body=b3_create_body(world,&bd);
        b3_create_hull_shape(body,&crate::api::b3_default_shape_def(),&crate::api::b3_make_box_hull(0.5,0.5,0.5));
        let mut jd=crate::api::b3_default_prismatic_joint_def();jd.body_a=anchor;jd.body_b=body;
        jd.enable_limit=true;jd.lower_translation=-0.5;jd.upper_translation=0.5;
        // Default-density unit cube is 1000 kg. Choose force from measured mass
        // for 100 m/s^2 authority, reaching 2 m/s in 0.02 s before either stop.
        let mass=b3_body_get_mass(body);assert!((mass-1000.0).abs()<0.01);
        jd.enable_motor=true;jd.motor_speed=2.0;jd.max_motor_force=100.0*mass;
        let joint=b3_create_prismatic_joint(world,&jd);
        let path=std::env::temp_dir().join(format!("gpu-prismatic-reversal-{}.jsonl",std::process::id()));
        let _=std::fs::remove_file(&path);
        let mut peak_limit_error=0.0f32;
        for step in 1..=120 {
            if step==46 {b3_prismatic_joint_set_motor_speed(joint,-2.0);}
            b3_world_step_gpu(world,1.0/60.0,4);
            b3_world_write_core_state(world,&path,step).expect("motor reversal capture");
            let p=b3_body_get_position(body);let v=b3_body_get_linear_velocity(body);
            peak_limit_error=peak_limit_error.max((p[0].abs()-0.5).max(0.0));
            assert!(p[0].abs()<=0.51,"step {step}: limit exceeded {p:?}");
            assert!(p[1].abs()<1.0e-5 && p[2].abs()<1.0e-5,"step {step}: transverse drift {p:?}");
            if (30..=45).contains(&step) || step>=90 {
                let stop=if step<=45 {0.5}else{-0.5};
                assert!((p[0]-stop).abs()<0.01,"step {step}: did not reach stop {p:?}");
                assert!(v.iter().all(|x|x.abs()<0.01),"step {step}: stop did not settle {v:?}");
            }
        }
        eprintln!("PRISMATIC_REVERSAL steps=120 peak_limit_error={peak_limit_error}");
        if let Ok(export)=std::env::var("GPU_PHYSICS_TEST_TRACE") {std::fs::copy(&path,export).unwrap();}
        b3_destroy_world(world);std::fs::remove_file(path).unwrap();
    }
    #[test]
    fn occupied_membership_permutation_preserves_future_state() {
        let gpu=pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let run=|reverse:bool| {
            let world=b3_create_world(gpu.clone(),&crate::api::b3_default_world_def());
            let sd=crate::api::b3_default_shape_def();
            let mut bd=crate::api::b3_default_body_def();bd.position=[0.0,-0.5,0.0];
            b3_create_hull_shape(b3_create_body(world,&bd),&sd,&crate::api::b3_make_box_hull(20.0,0.5,5.0));
            bd.body_type=BodyType::Dynamic;
            let mut bodies=Vec::new();
            for i in 0..8 {
                bd.position=[-7.0+2.0*i as f32,0.49,0.0];
                let body=b3_create_body(world,&bd);
                b3_create_hull_shape(body,&sd,&crate::api::b3_make_box_hull(0.5,0.5,0.5));bodies.push(body);
            }
            let path=std::env::temp_dir().join(format!("occupied-permutation-{}-{reverse}.jsonl",std::process::id()));
            let _=std::fs::remove_file(&path);
            for step in 1..=6 {
                if step==3 {b3_body_set_transform(bodies[0],[-7.0,10.0,0.0],[0.0,0.0,0.0,1.0]);}
                if step==5 {b3_body_set_transform(bodies[0],[-7.0,0.49,0.0],[0.0,0.0,0.0,1.0]);}
                b3_world_step_gpu(world,1.0/60.0,4);
                b3_world_write_core_state(world,&path,step).unwrap();
                if reverse {
                    let count=with_world_mut_no_sync(world,|w|w.sim.as_mut().unwrap().reverse_diagnostic_occupied_test()).unwrap();
                    assert!(count>=7,"must reverse a populated contact list");
                }
            }
            let text=std::fs::read_to_string(&path).unwrap();
            let mut frames:Vec<Value>=text.lines().map(|line|serde_json::from_str(line).unwrap()).collect();
            for frame in &mut frames {
                // Only normalize the deliberately permuted membership list.
                // Slot allocation, history, hash layout and all solver state stay strict.
                frame["contact_allocation"]["occupied_order"].as_array_mut().unwrap().sort_by_key(|v|v.to_string());
            }
            b3_destroy_world(world);std::fs::remove_file(path).unwrap();frames
        };
        if let Ok(case)=std::env::var("GPU_PHYSICS_OCCUPIED_TEST_CASE") {
            assert!(case=="control" || case=="reverse");
            let frames=run(case=="reverse");
            std::fs::write(std::env::var("GPU_PHYSICS_OCCUPIED_TEST_OUTPUT").unwrap(),serde_json::to_vec(&frames).unwrap()).unwrap();
            return;
        }
        // Global contact epochs differ between successive worlds in one process.
        // Compare fresh processes, matching the public repeatability contract.
        let mut outputs=Vec::new();
        for case in ["control","reverse"] {
            let output=std::env::temp_dir().join(format!("occupied-child-{}-{case}.json",std::process::id()));
            let status=std::process::Command::new(std::env::current_exe().unwrap())
                .args(["--exact","api::world::state_trace::tests::occupied_membership_permutation_preserves_future_state","--test-threads=1"])
                .env("GPU_PHYSICS_OCCUPIED_TEST_CASE",case)
                .env("GPU_PHYSICS_OCCUPIED_TEST_OUTPUT",&output).status().unwrap();
            assert!(status.success(),"{case} child failed");
            outputs.push(serde_json::from_slice::<Vec<Value>>(&std::fs::read(&output).unwrap()).unwrap());
            std::fs::remove_file(output).unwrap();
        }
        let actual=outputs.pop().unwrap();let expected=outputs.pop().unwrap();
        if expected!=actual {
            let prefix=std::env::temp_dir().join(format!("occupied-difference-{}",std::process::id()));
            let left=prefix.with_extension("expected.json");let right=prefix.with_extension("actual.json");
            std::fs::write(&left,serde_json::to_vec(&expected).unwrap()).unwrap();
            std::fs::write(&right,serde_json::to_vec(&actual).unwrap()).unwrap();
            panic!("occupied permutation changed state; expected={} actual={}",left.display(),right.display());
        }
    }
    #[test]
    fn trace_order_sensitive_callbacks_with_body_reuse() {
        struct Context { step: u32, calls: Vec<Value>, filtered: usize, vetoed: usize }
        unsafe extern "C" fn filter(a: ShapeId, b: ShapeId, raw: *mut std::ffi::c_void) -> bool {
            let c = unsafe { &mut *(raw as *mut Context) };
            let accept = c.step != 3 || c.calls.len() % 2 == 0;
            if !accept { c.filtered += 1; }
            c.calls.push(json!(["filter", a.index1, a.generation, b.index1, b.generation, accept]));
            accept
        }
        unsafe extern "C" fn pre_solve(a: ShapeId, b: ShapeId, point: crate::api::Vec3,
            normal: crate::api::Vec3, raw: *mut std::ffi::c_void) -> bool {
            let c = unsafe { &mut *(raw as *mut Context) };
            let accept = c.step != 2 || c.calls.len() % 2 == 0;
            if !accept { c.vetoed += 1; }
            c.calls.push(json!(["pre_solve", a.index1, a.generation, b.index1, b.generation,
                [point.x.to_bits(),point.y.to_bits(),point.z.to_bits()],
                [normal.x.to_bits(),normal.y.to_bits(),normal.z.to_bits()], accept]));
            accept
        }
        let gpu = pollster::block_on(GpuDevice::new(None)).expect("GPU");
        // Isolate discrete callback order from sleeping and CCD callback policy.
        let mut wd = crate::api::b3_default_world_def();
        wd.gravity = [0.0;3]; wd.enable_sleep = false; wd.enable_continuous = false;
        let world = b3_create_world(gpu, &wd);
        let mut sd = crate::api::b3_default_shape_def();
        sd.enable_custom_filtering = true; sd.enable_pre_solve_events = true;
        sd.enable_contact_events = true;
        let ground = b3_create_body(world, &crate::api::b3_default_body_def());
        b3_create_hull_shape(ground, &sd, &crate::api::b3_make_box_hull(20.0,0.5,2.0));
        let mut bd = crate::api::b3_default_body_def(); bd.body_type = BodyType::Dynamic;
        let mut bodies = Vec::new();
        for i in 0..8 {
            bd.position = [i as f32 * 2.0,0.99,0.0];
            let body = b3_create_body(world,&bd);
            b3_create_hull_shape(body,&sd,&crate::api::b3_make_box_hull(0.5,0.5,0.5));
            bodies.push(body);
        }
        let mut context = Context { step:0, calls:Vec::new(), filtered:0, vetoed:0 };
        let raw = &mut context as *mut _ as *mut std::ffi::c_void;
        b3_world_set_custom_filter_callback(world,Some(filter),raw);
        b3_world_set_pre_solve_callback(world,Some(pre_solve),raw);
        let path = std::env::temp_dir().join(format!("callback-state-{}.jsonl",std::process::id()));
        let _ = std::fs::remove_file(&path);
        let mut calls = Vec::new();
        for step in 1..=12 {
            context.step = step; context.calls.clear();
            if step == 5 { b3_destroy_body(bodies[0]); }
            if step == 6 {
                bd.position = [0.0,0.99,0.0];
                let replacement = b3_create_body(world,&bd);
                assert_eq!(replacement.index1,bodies[0].index1);
                assert_ne!(replacement.generation,bodies[0].generation);
                b3_create_hull_shape(replacement,&sd,&crate::api::b3_make_box_hull(0.5,0.5,0.5));
            }
            b3_world_step_gpu(world,1.0/60.0,4);
            b3_world_write_core_state(world,&path,step).unwrap();
            assert!(!context.calls.is_empty(),"must exercise callbacks each step");
            calls.push(json!({"calls":context.calls,"filtered":context.filtered,"vetoed":context.vetoed}));
        }
        assert!(context.filtered > 0 && context.vetoed > 0,"both order-sensitive rejection paths must run");
        // Record external callback inputs, decisions and persistent state alongside
        // every simulation frame; pointer values themselves are not state.
        let frames: Vec<Value> = std::fs::read_to_string(&path).unwrap().lines().zip(calls)
            .map(|(line,calls)| { let mut frame:Value=serde_json::from_str(line).unwrap();
                frame["callback_fixture"]=calls; frame }).collect();
        if let Ok(output) = std::env::var("GPU_PHYSICS_TEST_TRACE") {
            std::fs::write(output,frames.iter().map(|f|serde_json::to_string(f).unwrap()+"\n").collect::<String>()).unwrap();
        }
        b3_destroy_world(world); std::fs::remove_file(path).unwrap();
    }

    #[test]
    fn child_manifold_relocation_fixture() { run_child_manifold_relocation(false); }

    #[test]
    fn child_relocation_joint_history_fixture() { run_child_manifold_relocation(true); }

    fn run_child_manifold_relocation(with_joint_history: bool) {
        let gpu=pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let world=b3_create_world(gpu,&crate::api::b3_default_world_def());
        let mut sd=crate::api::b3_default_shape_def();sd.enable_contact_events=true;
        let mut original_handle=None;
        let mut original_root_slot=None;
        let ground=b3_create_body(world,&crate::api::b3_default_body_def());
        let vertices=vec![[-2.0,0.0,-2.0],[-2.0,0.0,2.0],[2.0,0.0,-2.0],[2.0,0.0,2.0],
            [0.0,0.0,-2.0],[0.0,2.0,-2.0],[0.0,0.0,2.0],[0.0,2.0,2.0],
            [-2.0,0.0,0.0],[2.0,0.0,0.0],[-2.0,2.0,0.0],[2.0,2.0,0.0]];
        let triangles=vec![[0,1,2],[2,1,3],[4,5,6],[6,5,7],[8,9,10],[10,9,11]];
        b3_create_mesh_shape(ground,&sd,&vertices,&triangles,&[],&[],&[],[1.0;3]);
        let mut bd=crate::api::b3_default_body_def();bd.body_type=BodyType::Dynamic;bd.position=[0.49,0.49,0.49];
        let body=b3_create_body(world,&bd);
        b3_create_hull_shape(body,&sd,&crate::api::b3_make_box_hull(0.5,0.5,0.5));
        let path=std::env::temp_dir().join(format!("child-relocation-{}.jsonl",std::process::id()));
        let _=std::fs::remove_file(&path);
        let mut motor=None;
        for step in 1..=if with_joint_history {16} else {8} {
            if with_joint_history && (step==7 || step==12) {
                let mut jd=crate::api::b3_default_motor_joint_def();
                jd.body_a=ground; jd.body_b=body; jd.collide_connected=true;
                jd.local_anchor_a=b3_body_get_position(body);
                jd.linear_hertz=5.0; jd.linear_damping=0.7; jd.max_spring_force=10000.0;
                motor=Some(b3_create_motor_joint(world,&jd));
            }
            if with_joint_history && step==10 { b3_destroy_joint(motor.take().unwrap(),true); }
            if step==4 {b3_body_set_transform(body,[3.0,5.0,0.0],[0.0,0.0,0.0,1.0]);}
            if step==5 {
                let mut added=crate::api::b3_default_body_def();added.body_type=BodyType::Dynamic;added.position=[1.25,0.24,0.0];
                let added=b3_create_body(world,&added);
                b3_create_hull_shape(added,&sd,&crate::api::b3_make_box_hull(0.25,0.25,0.25));
            }
            if step==6 {b3_body_set_transform(body,[0.49,0.49,0.49],[0.0,0.0,0.0,1.0]);}
            b3_world_step_gpu(world,1.0/60.0,4);
            b3_world_gpu_wait_with_mirror(world);
            let mut contacts=[crate::api::ContactData::default();8];
            let count=contact_api::b3_body_get_contact_data(body,&mut contacts);
            if step==1 {
                assert_eq!(count,1,"mesh patches must share one public contact");
                assert!(contacts[0].manifold_count>=3,"fixture requires at least three patches");
                original_handle=Some(contacts[0].contact_id);
                original_root_slot=Some(with_world_mut_no_sync(world,|w| {
                    let entry=w.contact_registry.values().next().unwrap();
                    assert_eq!(entry.owners.len(),1);
                    entry.owners.iter().next().unwrap().0
                }).unwrap());
            }
            if step<=3 {
                assert!(contact_api::b3_contact_is_valid(original_handle.unwrap()),"live root handle must survive child relocation");
                assert_eq!(contacts[0].contact_id,original_handle.unwrap());
            } else {
                assert!(!contact_api::b3_contact_is_valid(original_handle.unwrap()),"retired handle must never alias replacement");
            }
            if step>=6 {assert!(count>0,"return must create live contacts");}
            for c in &contacts[..count as usize] {assert!(contact_api::b3_contact_is_valid(c.contact_id));}
            b3_world_write_core_state(world,&path,step).unwrap();
            if step==4 && std::env::var("GPU_PHYSICS_PERTURB_RETIRED_GENERATION").as_deref()==Ok("1") {
                let slot=original_root_slot.unwrap();
                with_world_mut_no_sync(world,|w| {
                    w.sim.as_mut().unwrap().perturb_retired_contact_generation_test(slot,17);
                }).unwrap();
                eprintln!("PERTURBED_RETIRED_ROOT {slot}");
            }
            if step==1 && std::env::var("GPU_PHYSICS_RELOCATE_CHILD").as_deref()==Ok("1") {
                let slots=with_world_mut_no_sync(world,|w|w.sim.as_mut().unwrap().relocate_diagnostic_child_test()).unwrap();
                eprintln!("RELOCATED_CHILD {slots:?}");
            }
        }
        if with_joint_history {
            let frames:Vec<Value>=std::fs::read_to_string(&path).unwrap().lines()
                .map(|line|serde_json::from_str(line).unwrap()).collect();
            for step in [7usize,8,9,12,13,14,15,16] {
                assert!(!frames[step-1]["joints"].as_array().unwrap().is_empty());
                assert!(frames[step-1]["contact_history"]["records"].as_array().unwrap()
                    .iter().any(|r|!r.is_null()),"must exercise persistent contact color history at step {step}");
            }
            for step in [10usize,11] { assert!(frames[step-1]["joints"].as_array().unwrap().is_empty()); }
        }
        if let Ok(output)=std::env::var("GPU_PHYSICS_TEST_TRACE") {std::fs::copy(&path,output).unwrap();}
        b3_destroy_world(world);std::fs::remove_file(path).unwrap();
    }
    #[test]
    fn trace_tracks_contact_retirement_and_slot_reuse() {
        let gpu=pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let world=b3_create_world(gpu,&crate::api::b3_default_world_def());
        let mut sd=crate::api::b3_default_shape_def();sd.enable_contact_events=true;
        let mut bd=crate::api::b3_default_body_def();bd.position=[0.0,-0.5,0.0];
        b3_create_hull_shape(b3_create_body(world,&bd),&sd,&crate::api::b3_make_box_hull(5.0,0.5,5.0));
        bd.body_type=BodyType::Dynamic;bd.position=[0.0,0.49,0.0];
        let create=|| {
            let body=b3_create_body(world,&bd);
            b3_create_hull_shape(body,&sd,&crate::api::b3_make_box_hull(0.5,0.5,0.5));body
        };
        let body=create();
        let path=std::env::temp_dir().join(format!("gpu-contact-reuse-trace-{}.jsonl",std::process::id()));
        let _=std::fs::remove_file(&path);
        b3_world_step_gpu(world,1.0/60.0,4);b3_world_write_core_state(world,&path,1).unwrap();
        b3_destroy_body(body);
        {
            let mut worlds=lock_worlds();let w=slot_mut(&mut worlds,world).unwrap();
            let pending=contact_end_state(w);
            assert_eq!(pending["deferred"].as_array().unwrap().len(),1);
            assert_eq!(pending["deferred"][0]["key"]["pair"],json!([1,2]));
            assert!(pending["deferred"][0]["event"]["shapes"].as_array().unwrap().iter()
                .any(|s|s["identity"]==json!(2) && s["live"]==json!(false)));
        }
        b3_world_step_gpu(world,1.0/60.0,4);b3_world_write_core_state(world,&path,2).unwrap();
        let replacement=create();assert_eq!(replacement.index1,body.index1);
        b3_world_step_gpu(world,1.0/60.0,4);b3_world_write_core_state(world,&path,3).unwrap();
        let frames:Vec<Value>=std::fs::read_to_string(&path).unwrap().lines().map(|line|serde_json::from_str(line).unwrap()).collect();
        assert_eq!(frames[1]["contact_end_state"]["published"].as_array().unwrap().len(),1);
        assert_eq!(frames[1]["contact_end_state"]["ended_keys"].as_array().unwrap().len(),1);
        assert!(frames[1]["contact_end_state"]["deferred"].as_array().unwrap().is_empty());
        assert!(frames[2]["contact_end_state"]["published"].as_array().unwrap().is_empty());
        assert!(frames[2]["contact_end_state"]["ended_keys"].as_array().unwrap().is_empty());
        assert_eq!(frames[0]["body_allocation"]["generations"][1],frames[1]["body_allocation"]["generations"][1]);
        assert_eq!(frames[2]["body_allocation"]["generations"][1].as_u64().unwrap(),frames[1]["body_allocation"]["generations"][1].as_u64().unwrap()+1);
        let before=&frames[0]["contact_allocation"];let retired=&frames[1]["contact_allocation"];let after=&frames[2]["contact_allocation"];
        assert_eq!(before["occupied_order"].as_array().unwrap().len(),1);
        assert!(retired["occupied_order"].as_array().unwrap().is_empty());
        assert_eq!(before["high_water"],retired["high_water"],"retirement must retain allocation history");
        assert!(retired["slots"].as_array().unwrap().iter().all(|s|s["contact"].is_null()));
        assert!(frames[1]["event_history"]["previous_shapes"].as_array().unwrap().iter()
            .any(|s|s["identity"]==json!(2) && s["live"]==json!(false)),"old map must retain the deleted shape identity");
        assert!(frames[1]["contact_hash"]["buckets"].as_array().unwrap().iter()
            .all(|entry|entry.get("contact").is_none()),"no lookup may retain the retired contact");
        assert_eq!(after["occupied_order"].as_array().unwrap().len(),1);
        assert_ne!(before["slots"][0]["contact"],after["slots"][0]["contact"],"replacement shape needs a new semantic identity");
        assert!(after["slots"][0]["generation"].as_u64().unwrap()>before["slots"][0]["generation"].as_u64().unwrap(),"reused contact slot must advance generation");
        let first=&frames[0]["host_contacts"];let ended=&frames[1]["host_contacts"];let replacement=&frames[2]["host_contacts"];
        assert_eq!(first["registry"].as_array().unwrap().len(),1);
        let old_handle=&first["registry"][0]["live"]["event"]["handle"];
        assert_eq!(&first["begins"][0]["handle"],old_handle,"begin must reference the query contact");
        assert_eq!(&ended["ends"][0]["handle"],old_handle,"retirement must retain the old handle");
        assert_ne!(&replacement["begins"][0]["handle"],old_handle,"replacement must not alias retired contact");
        assert!(!first["registry"][0]["manifolds"][0]["points"].as_array().unwrap().is_empty());
        assert_eq!(first["by_body"].as_array().unwrap().len(),2);
        assert_eq!(first["by_shape"].as_array().unwrap().len(),2);
        {
            let mut worlds=lock_worlds();let w=slot_mut(&mut worlds,world).unwrap();
            let baseline=host_contact_state(w,world.index1).unwrap();
            let saved=w.contact_begin_events[0].contact_id;
            w.contact_begin_events[0].contact_id=contact_api::fresh_id(world.index1);
            let corrupted=host_contact_state(w,world.index1).unwrap();
            assert_eq!(baseline["begins"][0]["shapes"],corrupted["begins"][0]["shapes"]);
            assert_ne!(baseline["begins"][0]["handle"],corrupted["begins"][0]["handle"],"same endpoint pair must not hide a broken handle relationship");
            w.contact_begin_events[0].contact_id=saved;
            let sim=w.sim.as_mut().unwrap();
            let policy=sim.diagnostic_policy_state().unwrap();
            sim.overwrite_diagnostic_policy_test(false);
            let changed=sim.diagnostic_policy_state().unwrap();
            for key in ["body_sleep_thresholds","fat_geometry","one_group_pair_ok"] {
                assert_ne!(policy[key],changed[key],"cached policy mutation must be captured: {key}");
            }
            sim.overwrite_diagnostic_policy_test(true);
            assert!(sim.diagnostic_policy_state().is_err(),"nonfinite cached threshold must fail capture");
        }
        // Optional artifact for fresh-process replay qualification of this fixture.
        if let Some(destination)=std::env::var_os("GPU_PHYSICS_TEST_TRACE") {
            std::fs::copy(&path,destination).expect("write lifecycle trace artifact");
        }
        b3_destroy_world(world);std::fs::remove_file(path).unwrap();
    }
    #[test]
    fn trace_validates_uploaded_mesh_tree_relationships() {
        let gpu=pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let world=b3_create_world(gpu,&crate::api::b3_default_world_def());
        let body=b3_create_body(world,&crate::api::b3_default_body_def());
        let vertices=(0..4).flat_map(|z|(0..4).map(move |x|[x as f32,0.0,z as f32])).collect::<Vec<_>>();
        let triangles=(0..3u32).flat_map(|z|(0..3u32).flat_map(move |x| {
            let a=4*z+x;[[a,a+4,a+1],[a+1,a+4,a+5]]
        })).collect::<Vec<_>>();
        let mesh=b3_create_mesh_shape(body,&crate::api::b3_default_shape_def(),&vertices,&triangles,&[],&[],&[],[1.0;3]);
        assert!(mesh.index1>0);
        b3_world_step_gpu(world,1.0/60.0,4);
        let path=std::env::temp_dir().join(format!("gpu-mesh-tree-trace-{}.jsonl",std::process::id()));
        let _=std::fs::remove_file(&path);
        b3_world_write_core_state(world,&path,1).expect("valid uploaded mesh tree");
        let frame:Value=serde_json::from_str(std::fs::read_to_string(&path).unwrap().trim()).unwrap();
        assert_eq!(frame["gpu_geometry"]["mesh_triangles"].as_array().unwrap().len(),18);
        assert!(frame["gpu_geometry"]["mesh_nodes"].as_array().unwrap().len()>1);
        {
            let mut worlds=lock_worlds();let w=slot_mut(&mut worlds,world).unwrap();
            w.sim.as_ref().unwrap().overwrite_diagnostic_mesh_child_test();
        }
        let error=b3_world_write_core_state(world,&path,2).expect_err("invalid device-only child must fail capture");
        assert!(error.contains("mesh tree child out of range"),"{error}");
        b3_destroy_world(world);std::fs::remove_file(path).unwrap();
    }
    #[test]
    fn trace_captures_nonempty_convex_geometry() {
        let gpu=pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let world=b3_create_world(gpu,&crate::api::b3_default_world_def());
        let body=b3_create_body(world,&crate::api::b3_default_body_def());
        let faces=[[0u32,2,1],[0,1,3],[0,3,2],[1,2,3]];
        let mut half_edges=Vec::new();
        for (face,vertices) in faces.iter().enumerate() {
            for edge in 0..3 {
                let a=vertices[edge]; let b=vertices[(edge+1)%3];
                let twin=faces.iter().enumerate().find_map(|(f,v)| (0..3).find(|&e|v[e]==b && v[(e+1)%3]==a).map(|e|3*f+e)).unwrap();
                half_edges.push([(3*face+(edge+1)%3) as u32,twin as u32,a,face as u32]);
            }
        }
        let n=1.0f32/3.0f32.sqrt();
        let k=1.0f32/2.0f32.sqrt();
        let hull=ConvexHull { points:vec![[0.0,0.0,0.0],[1.0,0.0,0.0],[0.0,1.0,0.0],[0.0,0.0,1.0]],
            planes:vec![[0.0,0.0,-1.0,0.0],[0.0,-1.0,0.0,0.0],[-1.0,0.0,0.0,0.0],[n,n,n,n]],
            edge_directions:vec![[1.0,0.0,0.0],[0.0,1.0,0.0],[0.0,0.0,1.0],[-k,k,0.0],[-k,0.0,k],[0.0,-k,k]],half_edges,
            half_extents:[0.5;3],aabb_center:[0.5;3],center:[0.25;3],inner_radius:0.1,
            volume:1.0/6.0,central_inertia:[1.0/80.0,1.0/80.0,1.0/80.0,1.0/480.0,1.0/480.0,1.0/480.0] };
        b3_create_convex_hull_shape(body,&crate::api::b3_default_shape_def(),&hull);
        b3_world_step_gpu(world,1.0/60.0,4);
        let path=std::env::temp_dir().join(format!("gpu-hull-trace-{}.jsonl",std::process::id()));
        let _=std::fs::remove_file(&path);
        b3_world_write_core_state(world,&path,1).expect("convex capture");
        let frame:Value=serde_json::from_str(std::fs::read_to_string(&path).unwrap().trim()).unwrap();
        let g=&frame["gpu_geometry"];
        assert_eq!(g["hull_points"],hull.points.trace().unwrap());
        assert_eq!(g["hull_planes"],hull.planes.trace().unwrap());
        assert_eq!(g["hull_edges"],hull.edge_directions.trace().unwrap());
        assert_eq!(g["hull_topology"],hull.half_edges.trace().unwrap());
        assert_eq!(frame["gpu_colliders"][0]["bounds_shape"],json!(1));
        assert_eq!(frame["gpu_colliders"][0]["query_shape"],json!(1));
        {
            let mut worlds=lock_worlds();
            slot_mut(&mut worlds,world).unwrap().sim.as_ref().unwrap().overwrite_diagnostic_hull_slot_test();
        }
        let error=b3_world_write_core_state(world,&path,2).expect_err("out-of-range geometry must fail capture");
        assert!(error.contains("invalid hull points span"),"{error}");
        b3_destroy_world(world);
        std::fs::remove_file(path).unwrap();
    }
    #[test]
    fn trace_observes_gpu_loads_and_their_next_step_clear() {
        let gpu = pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let world = b3_create_world(gpu, &crate::api::b3_default_world_def());
        let mut bd = crate::api::b3_default_body_def();
        bd.body_type = BodyType::Dynamic;
        let body = b3_create_body(world, &bd);
        b3_create_hull_shape(body, &crate::api::b3_default_shape_def(),
            &crate::api::b3_make_box_hull(0.5, 0.5, 0.5));
        let force = [2.0f32,3.0,4.0];
        let torque = [0.1f32,0.2,0.3];
        b3_body_apply_force_to_center(body, force, true);
        b3_body_apply_torque(body, torque, true);
        let stamp = std::time::SystemTime::now().duration_since(std::time::UNIX_EPOCH).unwrap().as_nanos();
        let path = std::env::temp_dir().join(format!("gpu-motion-trace-{}-{stamp}.jsonl",std::process::id()));
        for step in 1..=2 {
            b3_world_step_gpu(world, 1.0/60.0, 4);
            b3_world_write_core_state(world, &path, step).expect("capture");
        }
        let text = std::fs::read_to_string(&path).unwrap();
        let frames: Vec<Value> = text.lines().map(|line|serde_json::from_str(line).unwrap()).collect();
        assert_eq!(frames.len(),2);
        let first = &frames[0]["bodies"][0]["gpu_motion"];
        let second = &frames[1]["bodies"][0]["gpu_motion"];
        assert_eq!(first["step_force"],force.trace().unwrap());
        assert_eq!(first["step_torque"],torque.trace().unwrap());
        assert_eq!(first["clear_force_next_step"],json!(true));
        assert_eq!(second["step_force"],[0.0f32;3].trace().unwrap());
        assert_eq!(second["step_torque"],[0.0f32;3].trace().unwrap());
        assert_eq!(second["clear_force_next_step"],json!(false));
        b3_destroy_world(world);
        std::fs::remove_file(path).unwrap();
    }

    #[test]
    fn trace_creation_identity_survives_body_slot_reuse() {
        let gpu = pollster::block_on(GpuDevice::new(None)).expect("GPU");
        let world = b3_create_world(gpu, &crate::api::b3_default_world_def());
        unsafe extern "C" fn sum_friction(a:f32,_a_id:u64,b:f32,_b_id:u64)->f32 { a+b }
        b3_world_set_friction_callback(world,Some(sum_friction));
        let create = |x: f32, size: f32| {
            let mut bd = crate::api::b3_default_body_def();
            bd.body_type = BodyType::Dynamic; bd.position = [x,10.0,0.0];
            let body = b3_create_body(world,&bd);
            b3_create_capsule_shape(body,&crate::api::b3_default_shape_def(),
                &Capsule { center1:[-size,0.0,0.0], center2:[size,0.0,0.0], radius:0.25 });
            body
        };
        let first = create(0.0,0.5);
        let survivor = create(10.0,0.5);
        let stamp = std::time::SystemTime::now().duration_since(std::time::UNIX_EPOCH).unwrap().as_nanos();
        let path = std::env::temp_dir().join(format!("gpu-identity-trace-{}-{stamp}.jsonl",std::process::id()));
        b3_world_step_gpu(world,1.0/60.0,4);
        b3_world_write_core_state(world,&path,1).expect("first capture");
        b3_destroy_body(first);
        b3_world_step_gpu(world,1.0/60.0,4);
        b3_world_write_core_state(world,&path,2).expect("deleted capture");
        let mut ids=[crate::api::b3_null_shape_id();1];
        assert_eq!(b3_body_get_shapes(survivor,&mut ids),1);
        b3_shape_set_friction(ids[0],0.375);
        let replacement = create(20.0,0.75);
        assert_eq!(replacement.index1,first.index1,"fixture must reuse the physical slot");
        assert_ne!(replacement.generation,first.generation);
        b3_world_step_gpu(world,1.0/60.0,4);
        b3_world_write_core_state(world,&path,3).expect("replacement capture");
        let text = std::fs::read_to_string(&path).unwrap();
        let frames: Vec<Value> = text.lines().map(|line|serde_json::from_str(line).unwrap()).collect();
        assert_eq!(frames[0]["body_storage_order"],json!([1,2]));
        assert_eq!(frames[1]["body_allocation"],json!({"slots":[null,2],"free_stack":[0],
            "generations":[first.generation,survivor.generation]}));
        assert_eq!(frames[0]["body_allocation"]["generations"],frames[1]["body_allocation"]["generations"],
            "retirement must preserve stored generations");
        assert_eq!(frames[2]["body_allocation"]["generations"],json!([replacement.generation,survivor.generation]),
            "reuse must capture the replacement generation without changing survivors");
        assert_eq!(frames[2]["body_storage_order"],json!([3,2]));
        assert_eq!(frames[2]["bodies"][0]["identity"],json!(2));
        assert_eq!(frames[2]["bodies"][1]["identity"],json!(3));
        assert_eq!(frames[2]["fat_bounds"].as_array().unwrap().iter()
            .map(|r|r["shape"].clone()).collect::<Vec<_>>(),vec![json!(2),json!(3)]);
        assert_eq!(frames[0]["geometries"].as_array().unwrap().len(),1,"identical capsules share exact endpoint geometry");
        assert_eq!(frames[2]["geometries"].as_array().unwrap().len(),2,"replacement geometry must be observed");
        assert_eq!(frames[2]["shapes"][0]["friction"],0.375f32.trace().unwrap());
        assert_eq!(frames[2]["shapes"][0]["body"],json!(2));
        assert_eq!(frames[2]["shapes"][1]["body"],json!(3));
        let uploaded = frames[2]["gpu_colliders"].as_array().unwrap();
        assert_eq!(uploaded.len(),2);
        let survivor_gpu = uploaded.iter().find(|s|s["identity"]==json!(2)).unwrap();
        let replacement_gpu = uploaded.iter().find(|s|s["identity"]==json!(3)).unwrap();
        assert_eq!(survivor_gpu["body"],json!(2));
        assert_eq!(survivor_gpu["friction"],0.375f32.trace().unwrap());
        assert_eq!(survivor_gpu["materials"][0]["friction"],0.375f32.trace().unwrap());
        assert_eq!(replacement_gpu["body"],json!(3));
        assert_eq!(replacement_gpu["axis"],[0.75f32,0.0,0.0].trace().unwrap());
        let mix=frames[2]["gpu_geometry"]["mix_table"].as_array().unwrap();
        assert_eq!(mix.iter().filter(|r|!r.is_null()).count(),3,"two live shapes produce three unordered material pairs");
        let survivor_slot=uploaded.iter().position(|s|s["identity"]==json!(2)).unwrap();
        let survivor_mix=mix.iter().find(|r|r["key"]==json!([survivor_slot,survivor_slot])).unwrap();
        assert_eq!(survivor_mix["friction"],0.75f32.trace().unwrap());
        assert_eq!(frames[1]["gpu_geometry"]["mix_table"].as_array().unwrap().iter().filter(|r|!r.is_null()).count(),1,
            "deleted shape must disappear from uploaded material pairs");
        {
            let mut worlds=lock_worlds();
            slot_mut(&mut worlds,world).unwrap().sim.as_ref().unwrap().overwrite_diagnostic_hull_point_test();
        }
        b3_world_write_core_state(world,&path,4).expect("device-only change capture");
        let changed: Value=serde_json::from_str(std::fs::read_to_string(&path).unwrap().lines().last().unwrap()).unwrap();
        assert_eq!(changed["geometries"],frames[2]["geometries"],"host geometry must remain unchanged");
        assert_ne!(changed["gpu_geometry"]["hull_points"],frames[2]["gpu_geometry"]["hull_points"]);
        assert_eq!(changed["gpu_geometry"]["hull_points"][0],[0.125f32,0.0,0.0].trace().unwrap());
        b3_destroy_world(world);
        std::fs::remove_file(path).unwrap();
    }

    #[test]
    fn trace_preserves_signed_zero_and_rejects_non_finite_values() {
        assert_ne!(0.0f32.trace().unwrap(), (-0.0f32).trace().unwrap());
        assert_eq!(f32::from_bits(0x3f800001).trace().unwrap(), json!("3f800001"));
        for value in [f32::NAN, f32::INFINITY, f32::NEG_INFINITY] {
            assert!(value.trace().is_err());
            assert!([0.0, value].trace().is_err());
        }
    }
}
