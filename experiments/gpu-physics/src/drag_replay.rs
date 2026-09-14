//! Diagnostic-only state transfer for the unit-cube drag fixture. Not a viewer path.
use serde::Deserialize;

// Box3D reserves 1 for a backside axis; WGSL's face axes start at 1.
pub(crate) fn native_sat_tag(axis: f32) -> Result<u32, String> {
    match axis {
        0.0 => Ok(0),
        2.0 => Ok(1),
        3.0 => Ok(2),
        4.0 => Ok(3),
        _ => Err("unsupported native SAT axis".into()),
    }
}

#[cfg(test)]
mod tests {
    use super::native_sat_tag;

    #[test]
    fn translates_native_axis_enumeration_without_accepting_backside() {
        for (native, gpu) in [(0.0, 0), (2.0, 1), (3.0, 2), (4.0, 3)] {
            assert_eq!(native_sat_tag(native).unwrap(), gpu);
        }
        for invalid in [1.0, 5.0, -1.0, 2.5, f32::NAN] {
            assert!(native_sat_tag(invalid).is_err());
        }
    }
}
#[derive(Deserialize)]
#[serde(deny_unknown_fields)]
pub(crate) struct Snapshot {
    pub version: u32,
    pub bodies: Vec<Body>,
    pub contacts: Vec<Contact>,
    pub joint: Option<Joint>,
}
#[derive(Deserialize)]
#[serde(deny_unknown_fields)]
pub(crate) struct Body {
    pub id: usize,
    pub p: [f32; 3],
    pub q: [f32; 4],
    pub v: [f32; 3],
    pub w: [f32; 3],
    pub inv_mass: f32,
    pub inv_inertia: [f32; 3],
    pub awake: u32,
}
#[derive(Deserialize)]
#[serde(deny_unknown_fields)]
pub(crate) struct Contact {
    pub a: u32,
    pub b: u32,
    pub color: i32,
    pub local: i32,
    pub normal: [f32; 3],
    pub friction_impulse: [f32; 2],
    pub twist: f32,
    pub rolling_impulse: [f32; 3],
    pub material: [f32; 3],
    pub tangent_velocity: [f32; 3],
    pub qa: [f32; 4],
    pub qb: [f32; 4],
    pub relative: [f32; 3],
    pub sat: [f32; 4],
    pub cache_valid: u32,
    pub points: Vec<Point>,
}
#[derive(Deserialize)]
#[serde(deny_unknown_fields)]
pub(crate) struct Point {
    pub ra: [f32; 3],
    pub rb: [f32; 3],
    pub separation: f32,
    pub base: f32,
    pub impulse: f32,
    pub feature: u32,
    pub persisted: u32,
}
#[derive(Deserialize)]
#[serde(deny_unknown_fields)]
pub(crate) struct Joint {
    pub slot: usize,
    pub a: u32,
    pub b: u32,
    pub anchor_a: [f32; 3],
    pub anchor_b: [f32; 3],
    pub frame_a: [f32; 4],
    pub frame_b: [f32; 4],
    pub linear_velocity: [f32; 3],
    pub angular_velocity: [f32; 3],
    pub tuning: [f32; 8],
    pub lv: [f32; 3],
    pub ls: [f32; 3],
    pub av: [f32; 3],
    #[serde(rename = "as")]
    pub angular_spring: [f32; 3],
}

#[no_mangle]
pub unsafe extern "C" fn gpu_b3_world_seed_drag_snapshot(
    id: crate::api::WorldId,
    path: *const std::ffi::c_char,
) -> bool {
    if path.is_null() {
        return false;
    }
    let result = (|| -> Result<(), String> {
        let path = std::ffi::CStr::from_ptr(path)
            .to_str()
            .map_err(|e| e.to_string())?;
        let bytes = std::fs::read(path).map_err(|e| e.to_string())?;
        let snapshot: Snapshot = serde_json::from_slice(&bytes).map_err(|e| e.to_string())?;
        if snapshot.version != 1 || snapshot.bodies.len() > 16 || snapshot.contacts.len() > 16 {
            return Err("unsupported snapshot".into());
        }
        crate::api::seed_drag_snapshot(id, &snapshot)
    })();
    match result {
        Ok(()) => true,
        Err(e) => {
            eprintln!("drag-snapshot rejected: {e}");
            false
        }
    }
}
