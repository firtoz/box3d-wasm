//! Checked reservations for GPU buffers. Static-heavy fallback is available only
//! when the original reservation cannot be bound on the selected device.
use crate::types::*;
use std::mem;
#[cfg(test)]
use bytemuck::Zeroable;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub(crate) struct AllocationPlan {
    pub contacts: u32,
    pub pairs: u32,
    pub fat_bounds_base: u32,
    pub order_base: u32,
    pub order_nodes: u32,
    pub insert_base: u32,
    pub insert_capacity: u32,
    pub scratch_bytes: u64,
    pub atom_words: u32,
    pub fallback: bool,
}

fn pow2(value: u64, label: &str) -> Result<u32, String> {
    let value = value.max(1).checked_next_power_of_two()
        .ok_or_else(|| format!("GPU {label} reservation overflow"))?;
    u32::try_from(value).map_err(|_| format!("GPU {label} reservation exceeds u32"))
}

/// Includes kinematic/disabled bodies and all spare body slots. Their types can
/// change without a scene-buffer allocation; never infer demand from mass.
pub(crate) fn moving_reservation(capacity: u32, bodies: &[BodyGpu]) -> u64 {
    bodies.iter().filter(|body| body.flags & FLAG_STATIC == 0).count() as u64
        + u64::from(capacity.saturating_sub(bodies.len() as u32))
}

pub(crate) fn fallback_contacts(
    caps: GpuSceneCaps, moving: u64, floor: u32,
) -> Result<u32, String> {
    // Wide static colliders live in pair-sized workspace. Retaining every shape
    // also supports one moving collider contacting the entire static population.
    // This is a finite reservation, not a maximum density guarantee.
    pow2(u64::from(caps.shapes).max(moving.checked_mul(16)
        .ok_or("GPU moving contact reservation overflow")?)
        .max(u64::from(floor)).max(256), "static-heavy contacts")
}

impl AllocationPlan {
    fn layout(caps: GpuSceneCaps, contacts: u32, order: bool, trace: u32) -> Result<Self, String> {
        let n = u64::from(caps.bodies.max(64));
        let s = u64::from(caps.shapes);
        let pairs = pair_capacity(contacts);
        let p = u64::from(pairs);
        let groups = p / u64::from(RADIX_GROUP_SIZE);
        // WGSL scr_graph() / phase arrays / joint_head_base(), with selected
        // contact capacity instead of recomputing it from all allocated bodies.
        let graph = u64::from(SCR_PAIRS) + 10*p + groups*u64::from(RADIX_BUCKETS)
            + groups.max(u64::from(RADIX_BUCKETS)) + 2 + 8*p;
        let fat = graph + 16*n + 24*8 + 24*8*n + u64::from(MAX_COLORS)*u64::from(contacts)
            + u64::from(JOINT_FILTER_CAP)*3 + 4*n + u64::from(caps.joints).max(n);
        let order_base = fat + 8*s;
        let nodes = if order {2*s} else {0};
        let order_words = if order {3*(8+19*nodes)+4*s} else {0};
        let insert_base = order_base + order_words;
        let inserts = u64::from(pow2(16*s, "spatial insertion")?).max(1024)
            .max(2*p/u64::from(RADIX_GROUP_SIZE));
        let to_word = |value: u64| u32::try_from(value)
            .map_err(|_| "GPU scratch address exceeds u32".to_string());
        Ok(Self {
            contacts, pairs, fat_bounds_base: to_word(fat)?,
            order_base: to_word(order_base)?, order_nodes: to_word(nodes)?,
            insert_base: to_word(insert_base)?, insert_capacity: to_word(inserts)?,
            scratch_bytes: (insert_base+3*inserts)*4,
            atom_words: to_word(16+u64::from(HASH_BUCKETS)+5*p+9*u64::from(caps.bodies)+25+u64::from(trace))?,
            fallback: false,
        })
    }

    fn validate(&self, caps: GpuSceneCaps, limit: u64) -> Result<(), String> {
        let sizes = [
            ("scratch", self.scratch_bytes),
            ("hot contacts", u64::from(self.contacts)*mem::size_of::<ContactHotGpu>() as u64),
            ("persistent contacts", u64::from(self.contacts)*mem::size_of::<ContactPersistentGpu>() as u64),
            ("prepared contacts", u64::from(self.contacts)*mem::size_of::<ContactPreparedGpu>() as u64),
            ("atom", u64::from(self.atom_words)*4),
            ("body state", u64::from(caps.bodies)*mem::size_of::<BodyStateGpu>() as u64),
            ("joints", u64::from(caps.joints.max(1))*mem::size_of::<JointGpu>() as u64),
            ("scene heap", caps.scene_heap_bytes().ok_or("GPU scene heap size overflow")?),
            ("query header", 256*4),
            ("indirect", (6+u64::from(MAX_COLORS))*16),
        ];
        for (label, bytes) in sizes {
            if bytes>limit {return Err(format!("GPU {label} needs {bytes} bytes, device permits {limit}"));}
        }
        Ok(())
    }

    pub fn select(caps: GpuSceneCaps, moving: u64, floor: u32, order: bool,
        trace: u32, limit: u64) -> Result<Self, String> {
        let legacy_contacts = pow2(u64::from(caps.bodies.max(1))*16, "contacts")?.max(256);
        let legacy = Self::layout(caps, legacy_contacts, order, trace);
        let original_error = match legacy {
            Ok(plan) => match plan.validate(caps, limit) {
                Ok(()) => return Ok(plan),
                Err(error) => error,
            },
            Err(error) => error,
        };
        let contacts = fallback_contacts(caps, moving, floor)?;
        if contacts>=legacy_contacts {return Err(original_error);}
        let mut plan = Self::layout(caps, contacts, order, trace)?;
        plan.validate(caps, limit).map_err(|error|
            format!("{original_error}; static-heavy reservation also rejected: {error}"))?;
        plan.fallback = true;
        Ok(plan)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    const LIMIT:u64=2147483647;
    fn caps(n:u32,s:u32)->GpuSceneCaps {
        GpuSceneCaps::allocate(GpuSceneCaps::live(n,s,0,0,0,0,0,0,0,s,0),0,0)
    }
    #[test] fn cap01_large_world_exact_rejection_and_full_static_reserve() {
        let c=caps(1000000,1000000);
        let old=AllocationPlan::layout(c,16777216,false,0).unwrap();
        assert_eq!(old.scratch_bytes,3971611912);
        assert!(old.validate(c,LIMIT).unwrap_err().contains("3971611912"));
        let p=AllocationPlan::select(c,356,1024,false,0,LIMIT).unwrap();
        assert!(p.fallback);assert_eq!(p.contacts,1048576);assert!(p.pairs>=1000000);
        assert_eq!(p.scratch_bytes,1266040072);assert_eq!(p.insert_capacity,16777216);
    }
    #[test] fn cap01_supported_layouts_remain_identical() {
        for n in [1,64,256,1000,10000,100000] {for order in [false,true] {
            let c=caps(n,n);let p=AllocationPlan::select(c,0,u32::MAX,order,0,LIMIT).unwrap();
            assert!(!p.fallback);assert_eq!(p.contacts,contact_capacity(c.bodies));
            assert_eq!(p.fat_bounds_base as usize,scratch_u32_count_with_joints(c.bodies,c.joints.max(c.bodies)));
            assert_eq!(p.order_base,p.fat_bounds_base+8*c.shapes);
        }}
    }
    #[test] fn cap01_one_visitor_all_statics_keeps_full_list_and_roots() {
        let c=caps(1000001,1000001);let p=AllocationPlan::select(c,257,1024,false,0,LIMIT).unwrap();
        assert!(p.contacts>=c.shapes);assert!(p.pairs>=c.shapes);
    }
    #[test] fn cap01_moving_reserve_includes_types_and_spare_slots() {
        let mut b=vec![BodyGpu::zeroed();4];
        b[0].flags=FLAG_STATIC;b[1].flags=FLAG_KINEMATIC;b[2].flags=FLAG_DISABLED;b[3].flags=0;
        assert_eq!(moving_reservation(256,&b),255);
    }
    #[test] fn cap01_fallback_honors_native_and_old_contact_floors() {
        let c=caps(1000000,1000000);
        assert_eq!(fallback_contacts(c,256,1500000).unwrap(),2097152);
        assert_eq!(fallback_contacts(c,256,2097152).unwrap(),2097152);
    }
    #[test] fn cap01_type_and_spawn_demand_grows_monotonically() {
        let c=caps(1000000,1000000);let old=fallback_contacts(c,356,1024).unwrap();
        let grown=fallback_contacts(c,100000,old).unwrap();assert_eq!(grown,2097152);
        assert_eq!(fallback_contacts(c,0,grown).unwrap(),grown);
    }
    #[test] fn cap01_oversized_requests_return_errors() {
        assert!(fallback_contacts(caps(1,1),u64::MAX,0).is_err());
        let c=GpuSceneCaps::live(u32::MAX,u32::MAX,0,0,0,0,0,0,0,u32::MAX,u32::MAX);
        assert!(AllocationPlan::select(c,0,0,true,0,LIMIT).is_err());
        assert!(AllocationPlan::select(caps(1,1),0,0,false,0,1023).is_err());
    }
    #[test] fn cap01_dense_dynamic_demand_is_not_clamped() {
        let c=caps(1000000,1000000);
        let error=AllocationPlan::select(c,u64::from(c.bodies),0,false,0,LIMIT).unwrap_err();
        assert!(error.contains("3971611912"));
    }
}
