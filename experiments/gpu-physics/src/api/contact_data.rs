//! Native manifold representation. Pair lifetime and getter storage are separate.
use super::{ContactId, ShapeId};
use crate::types::ContactGpu;

#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub struct ManifoldPoint {
    pub anchor_a: [f32; 3],
    pub anchor_b: [f32; 3],
    pub separation: f32,
    pub base_separation: f32,
    pub normal_impulse: f32,
    pub total_normal_impulse: f32,
    pub normal_velocity: f32,
    pub feature_id: u32,
    pub triangle_index: i32,
    pub persisted: bool,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub struct Manifold {
    pub points: [ManifoldPoint; 4],
    pub normal: [f32; 3],
    pub twist_impulse: f32,
    pub friction_impulse: [f32; 3],
    pub rolling_impulse: [f32; 3],
    pub point_count: i32,
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct ContactData {
    pub contact_id: ContactId,
    pub shape_id_a: ShapeId,
    pub shape_id_b: ShapeId,
    pub manifolds: *const Manifold,
    pub manifold_count: i32,
}

impl Default for ContactData {
    fn default() -> Self {
        Self {
            contact_id: ContactId::default(),
            shape_id_a: ShapeId::default(),
            shape_id_b: ShapeId::default(),
            manifolds: std::ptr::null(),
            manifold_count: 0,
        }
    }
}

/// Decode actual solver data, without consulting current body transforms.
/// Box3D returns collision-time COM anchors, including on recycled contacts.
pub(crate) fn decode_manifold(c: &ContactGpu) -> Result<Manifold, &'static str> {
    if c.count > 4 {
        return Err("contact manifold point count exceeds four");
    }
    let n = [c.nx, c.ny, c.nz];
    let t1 = if n[0].abs() > 0.5 {
        let d = (n[0] * n[0] + n[1] * n[1]).sqrt().max(1e-8);
        [n[1] / d, -n[0] / d, 0.0]
    } else {
        let d = (n[1] * n[1] + n[2] * n[2]).sqrt().max(1e-8);
        [0.0, n[2] / d, -n[1] / d]
    };
    let t2 = [
        t1[1] * n[2] - t1[2] * n[1],
        t1[2] * n[0] - t1[0] * n[2],
        t1[0] * n[1] - t1[1] * n[0],
    ];
    let mut out = Manifold {
        normal: n,
        twist_impulse: c.twist_impulse,
        friction_impulse: std::array::from_fn(|i| {
            t1[i] * c.friction_impulse[0] + t2[i] * c.friction_impulse[1]
        }),
        rolling_impulse: c.rolling_impulse,
        point_count: c.count as i32,
        ..Manifold::default()
    };
    let ra = [c.ra0, c.ra1, c.ra2, c.ra3];
    let rb = [c.rb0, c.rb1, c.rb2, c.rb3];
    let cached_ra = [
        c.persistent_ra0,
        c.persistent_ra1,
        c.persistent_ra2,
        c.persistent_ra3,
    ];
    let cached_rb = [
        c.persistent_rb0,
        c.persistent_rb1,
        c.persistent_rb2,
        c.persistent_rb3,
    ];
    let features = [
        c._tail[6],
        c._tail[7],
        c._pad_ca.to_bits(),
        c._pad_cb.to_bits(),
    ];
    let separation = |a: [f32; 4], b: [f32; 4]| {
        a[3] + (b[0] - a[0]) * n[0] + (b[1] - a[1]) * n[1] + (b[2] - a[2]) * n[2]
    };
    for i in 0..c.count as usize {
        out.points[i] = ManifoldPoint {
            anchor_a: [ra[i][0], ra[i][1], ra[i][2]],
            anchor_b: [rb[i][0], rb[i][1], rb[i][2]],
            separation: separation(ra[i], rb[i]),
            base_separation: separation(cached_ra[i], cached_rb[i]),
            normal_impulse: rb[i][3],
            total_normal_impulse: c.total_normal_impulse[i],
            normal_velocity: f32::from_bits(c._tail[i]),
            feature_id: features[i],
            triangle_index: match c.point_triangles[i].checked_sub(1) {
                None => -1,
                Some(index) => {
                    i32::try_from(index).map_err(|_| "mesh triangle index exceeds native i32")?
                }
            },
            persisted: c.point_persisted(i),
        };
    }
    Ok(out)
}

impl Manifold {
    /// Reverse public endpoints while preserving opaque matching IDs, as in
    /// Box3D b3UpdateConvexContact. Signed world impulses follow the endpoints.
    pub fn reverse(&mut self) {
        self.normal = self.normal.map(|x| -x);
        self.friction_impulse = self.friction_impulse.map(|x| -x);
        self.rolling_impulse = self.rolling_impulse.map(|x| -x);
        for point in self
            .points
            .iter_mut()
            .take(self.point_count.max(0) as usize)
        {
            std::mem::swap(&mut point.anchor_a, &mut point.anchor_b);
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use bytemuck::Zeroable;
    use std::mem::{align_of, offset_of, size_of};

    #[test]
    fn native_contact_layout_matches_box3d() {
        assert_eq!(
            (size_of::<ManifoldPoint>(), align_of::<ManifoldPoint>()),
            (56, 4)
        );
        assert_eq!(offset_of!(ManifoldPoint, anchor_b), 12);
        assert_eq!(offset_of!(ManifoldPoint, separation), 24);
        assert_eq!(offset_of!(ManifoldPoint, base_separation), 28);
        assert_eq!(offset_of!(ManifoldPoint, normal_impulse), 32);
        assert_eq!(offset_of!(ManifoldPoint, total_normal_impulse), 36);
        assert_eq!(offset_of!(ManifoldPoint, normal_velocity), 40);
        assert_eq!(offset_of!(ManifoldPoint, feature_id), 44);
        assert_eq!(offset_of!(ManifoldPoint, triangle_index), 48);
        assert_eq!(offset_of!(ManifoldPoint, persisted), 52);
        assert_eq!((size_of::<Manifold>(), align_of::<Manifold>()), (268, 4));
        assert_eq!(offset_of!(Manifold, normal), 224);
        assert_eq!(offset_of!(Manifold, twist_impulse), 236);
        assert_eq!(offset_of!(Manifold, friction_impulse), 240);
        assert_eq!(offset_of!(Manifold, rolling_impulse), 252);
        assert_eq!(offset_of!(Manifold, point_count), 264);
        assert_eq!(offset_of!(ContactData, shape_id_a), 12);
        assert_eq!(offset_of!(ContactData, shape_id_b), 20);
        if size_of::<usize>() == 8 {
            assert_eq!(
                (size_of::<ContactData>(), align_of::<ContactData>()),
                (48, 8)
            );
            assert_eq!(offset_of!(ContactData, manifolds), 32);
            assert_eq!(offset_of!(ContactData, manifold_count), 40);
        }
    }

    #[test]
    fn manifold_export_preserves_history_impulses_and_opaque_ids() {
        let mut c = ContactGpu::zeroed();
        c.count = 4;
        c.ny = 1.0;
        c.ra0 = [1.0, 2.0, 3.0, 5.0];
        c.rb0 = [4.0, -1.0, 6.0, 0.0];
        c.persistent_ra0 = [1.0, 2.0, 3.0, 4.0];
        c.persistent_rb0 = [4.0, -1.0, 6.0, 0.0];
        c.total_normal_impulse[0] = 7.0;
        c._tail[0] = (-8.0f32).to_bits();
        c._tail[6] = 0xdeadbeef;
        c._tail[7] = 0x12345678;
        c._pad_ca = f32::from_bits(0x01020304);
        c._pad_cb = f32::from_bits(0x87654321);
        c.point_triangles = [0, 1, 32769, 65537];
        c.lifecycle[1] = 1 << crate::types::CONTACT_PERSISTED_SHIFT;
        c.friction_impulse = [2.0, 3.0];
        c.twist_impulse = 4.0;
        c.rolling_impulse = [5.0, 6.0, 7.0];
        let m = decode_manifold(&c).unwrap();
        let p = m.points[0];
        assert_eq!((p.separation, p.base_separation), (2.0, 1.0));
        assert_eq!(
            (p.normal_impulse, p.total_normal_impulse, p.normal_velocity),
            (0.0, 7.0, -8.0)
        );
        assert!(p.persisted);
        assert_eq!(
            m.points.map(|p| p.feature_id),
            [0xdeadbeef, 0x12345678, 0x01020304, 0x87654321]
        );
        assert_eq!(m.points.map(|p| p.triangle_index), [-1, 0, 32768, 65536]);
        assert_eq!(m.friction_impulse, [3.0, 0.0, -2.0]);
        let mut reversed = m;
        reversed.reverse();
        assert_eq!(reversed.points[0].anchor_a, p.anchor_b);
        assert_eq!(reversed.points[0].anchor_b, p.anchor_a);
        assert_eq!(reversed.points[0].feature_id, p.feature_id);
        assert_eq!(reversed.friction_impulse, [-3.0, 0.0, 2.0]);
        assert_eq!(reversed.rolling_impulse, [-5.0, -6.0, -7.0]);
        assert_eq!(reversed.twist_impulse, 4.0);
        reversed.reverse();
        assert_eq!(reversed, m);
        c.count = 5;
        assert!(decode_manifold(&c).is_err());
        c.count = 4;
        c.point_triangles[0] = u32::MAX;
        assert!(decode_manifold(&c).is_err());
    }
}
