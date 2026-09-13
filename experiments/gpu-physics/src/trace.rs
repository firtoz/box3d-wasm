//! Frame-by-frame CPU/GPU lock: bodies + manifolds after each world step.
//! File magic `B3TR`. Pair indices are creation-order body slots, `a < b`.

use crate::types::{BodyGpu, ContactGpu, SPECULATIVE_DISTANCE};

pub const TRACE_MAGIC: &[u8; 4] = b"B3TR";
pub const TRACE_VERSION: u32 = 4;
pub const PHASE_PROBE_MAGIC: &[u8; 4] = b"B3PR";
pub const PHASE_PROBE_VERSION: u32 = 1;
pub const PHASE_COUNT: u32 = 24;
pub const PHASE_SUMMARY_WORDS: u32 = 8;

pub fn begin_phase_probe(body_count: u32, step_count: u32) -> Vec<u8> {
    let mut bytes = Vec::new();
    bytes.extend_from_slice(PHASE_PROBE_MAGIC);
    bytes.extend_from_slice(&PHASE_PROBE_VERSION.to_le_bytes());
    bytes.extend_from_slice(&body_count.to_le_bytes());
    bytes.extend_from_slice(&PHASE_COUNT.to_le_bytes());
    bytes.extend_from_slice(&step_count.to_le_bytes());
    bytes
}

pub fn append_phase_probe(bytes: &mut Vec<u8>, step: u32, words: &[u32]) {
    bytes.extend_from_slice(&step.to_le_bytes());
    bytes.extend_from_slice(&(words.len() as u32).to_le_bytes());
    bytes.extend_from_slice(bytemuck::cast_slice(words));
}

#[repr(C)]
#[derive(Clone, Copy, Debug, bytemuck::Pod, bytemuck::Zeroable)]
pub struct TracePoint {
    /// World `rA` (xyz) + separation (w).
    pub r_a: [f32; 4],
    /// World `rB` (xyz) + normal impulse (w).
    pub r_b: [f32; 4],
}

#[repr(C)]
#[derive(Clone, Copy, Debug, bytemuck::Pod, bytemuck::Zeroable)]
pub struct TraceManifold {
    pub a: u32,
    pub b: u32,
    pub count: u32,
    pub _pad: u32,
    pub n: [f32; 3],
    pub _pn: f32,
    pub pts: [TracePoint; 4],
    pub point_ids: [u32; 4],
    /// World-space central friction impulse.
    pub friction: [f32; 3],
    pub twist: f32,
    /// World-space rolling impulse.
    pub rolling: [f32; 3],
    pub _pad_impulse: f32,
}

const _: () = assert!(core::mem::size_of::<TraceManifold>() == 208);

#[derive(Clone, Debug)]
pub struct TraceFrame {
    pub bodies: Vec<BodyGpu>,
    pub contacts: Vec<TraceManifold>,
}

#[derive(Clone, Debug)]
pub struct TraceFile {
    pub frames: u32,
    pub body_count: u32,
    pub data: Vec<TraceFrame>,
}

#[derive(Clone, Debug, PartialEq)]
pub struct GpuContactDiagnostic {
    pub slot: usize,
    pub manifold: Result<crate::api::Manifold, &'static str>,
    pub pair: (u32, u32),
    pub generation: u32,
    pub lifecycle_flags: u32,
    pub local_order: u32,
    pub color: u32,
    pub point_count: u32,
    pub sat_type: u32,
    pub sat_index_a: u32,
    pub sat_index_b: u32,
    pub cache_separation: f32,
    pub point_ids: [u32; 4],
    /// Triangle index + 1, or zero for a non-mesh witness.
    pub point_triangles: [u32; 4],
    pub point_persisted: [bool; 4],
    pub base_separations: [f32; 4],
    pub normal_impulses: [f32; 4],
    pub friction_impulse: [f32; 2],
    pub twist_impulse: f32,
    pub rolling_impulse: [f32; 3],
    pub prepared_normal_mass: [f32; 4],
    pub prepared_lever_arm: [f32; 4],
    pub prepared_tangent_inv: [f32; 4],
    pub prepared_softness: [f32; 4],
}

/// Decode the solver/contact-cache state without filtering speculative or
/// zero-point persistent pairs. This is intentionally separate from B3TR's
/// public manifold record so diagnostics can evolve without invalidating dumps.
pub fn gpu_contact_diagnostics(raw: &[ContactGpu]) -> Vec<GpuContactDiagnostic> {
    raw.iter()
        .enumerate()
        .filter(|(_, c)| c.a != u32::MAX && c.b != u32::MAX && c.a != c.b)
        .map(|(slot, c)| {
            let packed_sat = c._tail[4];
            GpuContactDiagnostic {
                slot,
                manifold: crate::api::contact_data::decode_manifold(c),
                pair: (c.a.min(c.b), c.a.max(c.b)),
                generation: c.lifecycle[0],
                lifecycle_flags: c.lifecycle[1],
                local_order: c.lifecycle[2],
                color: c.color,
                point_count: c.count.min(4),
                sat_type: packed_sat & 0xff,
                sat_index_a: (packed_sat >> 8) & 0xff,
                sat_index_b: (packed_sat >> 16) & 0xff,
                cache_separation: f32::from_bits(c._tail[5]),
                point_ids: [
                    c._tail[6],
                    c._tail[7],
                    c._pad_ca.to_bits(),
                    c._pad_cb.to_bits(),
                ],
                point_triangles: c.point_triangles,
                point_persisted: std::array::from_fn(|i| c.point_persisted(i)),
                base_separations: [c.ra0[3], c.ra1[3], c.ra2[3], c.ra3[3]],
                normal_impulses: [c.rb0[3], c.rb1[3], c.rb2[3], c.rb3[3]],
                friction_impulse: c.friction_impulse,
                twist_impulse: c.twist_impulse,
                rolling_impulse: c.rolling_impulse,
                prepared_normal_mass: c.prepared_normal_mass,
                prepared_lever_arm: c.prepared_lever_arm,
                prepared_tangent_inv: c.prepared_tangent_inv,
                prepared_softness: c.prepared_softness,
            }
        })
        .collect()
}

impl TraceManifold {
    pub fn empty() -> Self {
        Self {
            a: u32::MAX,
            b: u32::MAX,
            count: 0,
            _pad: 0,
            n: [0.0; 3],
            _pn: 0.0,
            pts: [TracePoint {
                r_a: [0.0; 4],
                r_b: [0.0; 4],
            }; 4],
            point_ids: [0; 4],
            friction: [0.0; 3],
            twist: 0.0,
            rolling: [0.0; 3],
            _pad_impulse: 0.0,
        }
    }

    /// Canonical pair with `a < b`. Flips `n` and swaps anchors if needed.
    pub fn normalize(mut self) -> Self {
        if self.a <= self.b {
            return self;
        }
        std::mem::swap(&mut self.a, &mut self.b);
        self.n = [-self.n[0], -self.n[1], -self.n[2]];
        self.friction = [-self.friction[0], -self.friction[1], -self.friction[2]];
        self.rolling = [-self.rolling[0], -self.rolling[1], -self.rolling[2]];
        for p in self.pts.iter_mut().take(self.count as usize) {
            let ra = p.r_a;
            p.r_a = [p.r_b[0], p.r_b[1], p.r_b[2], ra[3]];
            p.r_b = [ra[0], ra[1], ra[2], p.r_b[3]];
        }
        for id in self.point_ids.iter_mut().take(self.count as usize) {
            *id = flip_feature(*id);
        }
        self
    }
}

pub fn decode_gpu_contact(c: &ContactGpu) -> Option<(TraceManifold, f32)> {
    if c.a == u32::MAX || c.b == u32::MAX || c.a == c.b || c.count == 0 {
        return None;
    }
    let data = crate::api::contact_data::decode_manifold(c).ok()?;
    let mut m = TraceManifold::empty();
    m.a = c.a;
    m.b = c.b;
    m.count = data.point_count as u32;
    m.n = data.normal;
    m.friction = data.friction_impulse;
    m.twist = data.twist_impulse;
    m.rolling = data.rolling_impulse;
    for i in 0..m.count as usize {
        let point = data.points[i];
        m.point_ids[i] = point.feature_id;
        m.pts[i].r_a = [point.anchor_a[0], point.anchor_a[1], point.anchor_a[2], point.separation];
        m.pts[i].r_b = [point.anchor_b[0], point.anchor_b[1], point.anchor_b[2], point.normal_impulse];
    }
    let m = m.normalize();
    let mut smin = f32::MAX;
    for i in 0..m.count as usize {
        smin = smin.min(m.pts[i].r_a[3]);
    }
    Some((m, smin))
}

pub fn from_gpu_contact(c: &ContactGpu) -> Option<TraceManifold> {
    let (m, smin) = decode_gpu_contact(c)?;
    // Box3D touching flag = manifold exists (`minSeparation < B3_SPECULATIVE_DISTANCE`).
    if smin >= SPECULATIVE_DISTANCE {
        return None;
    }
    Some(m)
}

pub fn from_gpu_contact_unfiltered(c: &ContactGpu) -> Option<(TraceManifold, f32)> {
    decode_gpu_contact(c)
}

pub fn read_trace(path: &std::path::Path) -> Result<TraceFile, String> {
    let bytes = std::fs::read(path).map_err(|e| format!("read {}: {e}", path.display()))?;
    if bytes.len() < 16 || &bytes[0..4] != TRACE_MAGIC {
        return Err(format!("{} is not a B3TR dump", path.display()));
    }
    let version = u32::from_le_bytes(bytes[4..8].try_into().unwrap());
    if version != TRACE_VERSION {
        return Err(format!("unsupported B3TR version {version}"));
    }
    let frames = u32::from_le_bytes(bytes[8..12].try_into().unwrap());
    let body_count = u32::from_le_bytes(bytes[12..16].try_into().unwrap());
    let body_stride = std::mem::size_of::<BodyGpu>();
    let man_stride = std::mem::size_of::<TraceManifold>();
    let mut off = 16usize;
    let mut data = Vec::with_capacity(frames as usize);
    for f in 0..frames {
        let need = off + body_count as usize * body_stride + 4;
        if bytes.len() < need {
            return Err(format!("{} truncated at frame {f} bodies", path.display()));
        }
        let mut bodies = Vec::with_capacity(body_count as usize);
        for _ in 0..body_count {
            let slice = &bytes[off..off + body_stride];
            bodies.push(*bytemuck::from_bytes::<BodyGpu>(slice));
            off += body_stride;
        }
        let nc = u32::from_le_bytes(bytes[off..off + 4].try_into().unwrap());
        off += 4;
        let need_c = off + nc as usize * man_stride;
        if bytes.len() < need_c {
            return Err(format!(
                "{} truncated at frame {f} contacts",
                path.display()
            ));
        }
        let mut contacts = Vec::with_capacity(nc as usize);
        for _ in 0..nc {
            let slice = &bytes[off..off + man_stride];
            contacts.push(bytemuck::from_bytes::<TraceManifold>(slice).normalize());
            off += man_stride;
        }
        contacts.sort_by_key(|c| (c.a, c.b));
        data.push(TraceFrame { bodies, contacts });
    }
    Ok(TraceFile {
        frames,
        body_count,
        data,
    })
}

fn nlen(n: [f32; 3]) -> f32 {
    (n[0] * n[0] + n[1] * n[1] + n[2] * n[2]).sqrt()
}

fn ndot(a: [f32; 3], b: [f32; 3]) -> f32 {
    a[0] * b[0] + a[1] * b[1] + a[2] * b[2]
}

fn max_abs3(a: [f32; 3], b: [f32; 3]) -> f32 {
    (a[0] - b[0])
        .abs()
        .max((a[1] - b[1]).abs())
        .max((a[2] - b[2]).abs())
}

fn flip_feature(id: u32) -> u32 {
    let owner1 = (id >> 24) & 1;
    let index1 = (id >> 16) & 255;
    let owner2 = (id >> 8) & 1;
    let index2 = id & 255;
    ((1 - owner2) << 24) | (index2 << 16) | ((1 - owner1) << 8) | index1
}

fn vdist3(a: [f32; 4], b: [f32; 4]) -> f32 {
    let dx = a[0] - b[0];
    let dy = a[1] - b[1];
    let dz = a[2] - b[2];
    (dx * dx + dy * dy + dz * dz).sqrt()
}

/// Match GPU manifolds to CPU. Returns human-readable mismatch lines (empty = ok).
pub fn diff_contacts(cpu: &[TraceManifold], gpu: &[TraceManifold], eps: f32) -> Vec<String> {
    use std::collections::BTreeMap;
    let mut cpu_map: BTreeMap<(u32, u32), &TraceManifold> = BTreeMap::new();
    for c in cpu {
        cpu_map.insert((c.a, c.b), c);
    }
    let mut gpu_map: BTreeMap<(u32, u32), &TraceManifold> = BTreeMap::new();
    for c in gpu {
        gpu_map.insert((c.a, c.b), c);
    }
    let mut lines = Vec::new();
    for k in cpu_map.keys() {
        if !gpu_map.contains_key(k) {
            let c = cpu_map[k];
            lines.push(format!(
                "  missing GPU pair ({},{}) cpu_pts={} n=({:.4},{:.4},{:.4})",
                k.0, k.1, c.count, c.n[0], c.n[1], c.n[2]
            ));
        }
    }
    for k in gpu_map.keys() {
        if !cpu_map.contains_key(k) {
            let g = gpu_map[k];
            lines.push(format!(
                "  extra GPU pair ({},{}) gpu_pts={} n=({:.4},{:.4},{:.4})",
                k.0, k.1, g.count, g.n[0], g.n[1], g.n[2]
            ));
        }
    }
    for (k, c) in &cpu_map {
        let Some(g) = gpu_map.get(k) else {
            continue;
        };
        if c.count != g.count {
            lines.push(format!(
                "  pair ({},{}) point count CPU {} vs GPU {}",
                k.0, k.1, c.count, g.count
            ));
        }
        for i in 0..c.count.min(g.count) as usize {
            if c.point_ids[i] != g.point_ids[i] {
                lines.push(format!(
                    "  pair ({},{}) point {i} feature CPU {:#06x} GPU {:#06x}",
                    k.0, k.1, c.point_ids[i], g.point_ids[i]
                ));
            }
        }
        let cn = nlen(c.n).max(1e-8);
        let gn = nlen(g.n).max(1e-8);
        let cnu = [c.n[0] / cn, c.n[1] / cn, c.n[2] / cn];
        let gnu = [g.n[0] / gn, g.n[1] / gn, g.n[2] / gn];
        let align = ndot(cnu, gnu).abs();
        if align < 0.99 {
            lines.push(format!(
                "  pair ({},{}) normal CPU ({:.4},{:.4},{:.4}) GPU ({:.4},{:.4},{:.4}) |dot|={align:.4}",
                k.0, k.1, c.n[0], c.n[1], c.n[2], g.n[0], g.n[1], g.n[2]
            ));
        }
        let npts = c.count.min(g.count) as usize;
        let mut used = [false; 4];
        for i in 0..npts {
            let mut best = 1e9f32;
            let mut bj = 0usize;
            for j in 0..npts {
                if used[j] {
                    continue;
                }
                let d = vdist3(c.pts[i].r_a, g.pts[j].r_a) + vdist3(c.pts[i].r_b, g.pts[j].r_b);
                if d < best {
                    best = d;
                    bj = j;
                }
            }
            used[bj] = true;
            if best > eps.max(5e-4) {
                lines.push(format!(
                    "  pair ({},{}) point {i} anchor Δ={best:.3e} CPU rA ({:.5},{:.5},{:.5}) GPU ({:.5},{:.5},{:.5})",
                    k.0,
                    k.1,
                    c.pts[i].r_a[0],
                    c.pts[i].r_a[1],
                    c.pts[i].r_a[2],
                    g.pts[bj].r_a[0],
                    g.pts[bj].r_a[1],
                    g.pts[bj].r_a[2]
                ));
            }
            let ds = (c.pts[i].r_a[3] - g.pts[bj].r_a[3]).abs();
            if ds > eps.max(5e-4) {
                lines.push(format!(
                    "  pair ({},{}) point {i} sep CPU {:.4e} GPU {:.4e}",
                    k.0, k.1, c.pts[i].r_a[3], g.pts[bj].r_a[3]
                ));
            }
            let dj = (c.pts[i].r_b[3] - g.pts[bj].r_b[3]).abs();
            if dj > eps {
                lines.push(format!(
                    "  pair ({},{}) point {i} normal impulse CPU {:.4e} GPU {:.4e}",
                    k.0, k.1, c.pts[i].r_b[3], g.pts[bj].r_b[3]
                ));
            }
        }
        let df = max_abs3(c.friction, g.friction);
        if df > eps {
            lines.push(format!(
                "  pair ({},{}) friction impulse Δ={df:.3e} CPU ({:.4e},{:.4e},{:.4e}) GPU ({:.4e},{:.4e},{:.4e})",
                k.0, k.1,
                c.friction[0], c.friction[1], c.friction[2],
                g.friction[0], g.friction[1], g.friction[2]
            ));
        }
        let dt = (c.twist - g.twist).abs();
        if dt > eps {
            lines.push(format!(
                "  pair ({},{}) twist impulse CPU {:.4e} GPU {:.4e}",
                k.0, k.1, c.twist, g.twist
            ));
        }
        let dr = max_abs3(c.rolling, g.rolling);
        if dr > eps {
            lines.push(format!(
                "  pair ({},{}) rolling impulse Δ={dr:.3e} CPU ({:.4e},{:.4e},{:.4e}) GPU ({:.4e},{:.4e},{:.4e})",
                k.0, k.1,
                c.rolling[0], c.rolling[1], c.rolling[2],
                g.rolling[0], g.rolling[1], g.rolling[2]
            ));
        }
    }
    lines
}

/// Print CPU/GPU manifolds for pairs present on both sides (lock-step diagnosis).
pub fn dump_matched_contacts(cpu: &[TraceManifold], gpu: &[TraceManifold]) {
    use std::collections::BTreeMap;
    let mut cpu_map: BTreeMap<(u32, u32), &TraceManifold> = BTreeMap::new();
    for c in cpu {
        cpu_map.insert((c.a, c.b), c);
    }
    let mut gpu_map: BTreeMap<(u32, u32), &TraceManifold> = BTreeMap::new();
    for c in gpu {
        gpu_map.insert((c.a, c.b), c);
    }
    for (k, c) in &cpu_map {
        let Some(g) = gpu_map.get(k) else {
            continue;
        };
        println!(
            "    pair ({},{}) n CPU ({:.6},{:.6},{:.6}) GPU ({:.6},{:.6},{:.6}) pts {}/{}",
            k.0, k.1, c.n[0], c.n[1], c.n[2], g.n[0], g.n[1], g.n[2], c.count, g.count
        );
        let npts = c.count.min(g.count) as usize;
        for i in 0..npts {
            let mut best = 1e9f32;
            let mut bj = 0usize;
            for j in 0..npts {
                let d = vdist3(c.pts[i].r_a, g.pts[j].r_a) + vdist3(c.pts[i].r_b, g.pts[j].r_b);
                if d < best {
                    best = d;
                    bj = j;
                }
            }
            println!(
                "      p{i} Δ={best:.3e} sep CPU {:.5e} GPU {:.5e}",
                c.pts[i].r_a[3], g.pts[bj].r_a[3]
            );
            println!(
                "         CPU rA ({:.6},{:.6},{:.6}) rB ({:.6},{:.6},{:.6})",
                c.pts[i].r_a[0],
                c.pts[i].r_a[1],
                c.pts[i].r_a[2],
                c.pts[i].r_b[0],
                c.pts[i].r_b[1],
                c.pts[i].r_b[2]
            );
            println!(
                "         GPU rA ({:.6},{:.6},{:.6}) rB ({:.6},{:.6},{:.6})",
                g.pts[bj].r_a[0],
                g.pts[bj].r_a[1],
                g.pts[bj].r_a[2],
                g.pts[bj].r_b[0],
                g.pts[bj].r_b[1],
                g.pts[bj].r_b[2]
            );
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use bytemuck::Zeroable;

    #[test]
    fn normalize_swaps_pair_and_normal() {
        let mut m = TraceManifold::empty();
        m.a = 3;
        m.b = 1;
        m.count = 1;
        m.n = [0.0, 1.0, 0.0];
        m.pts[0].r_a = [0.0, 0.5, 0.0, -0.01];
        m.pts[0].r_b = [0.0, -0.5, 0.0, 0.0];
        let n = m.normalize();
        assert_eq!((n.a, n.b), (1, 3));
        assert!((n.n[1] + 1.0).abs() < 1e-6);
        assert!((n.pts[0].r_a[1] + 0.5).abs() < 1e-6);
    }

    #[test]
    fn contact_diagnostic_decodes_cache_and_features() {
        let mut c = ContactGpu::zeroed();
        c.a = 7;
        c.b = 2;
        c.color = 4;
        c.count = 1;
        c.ra0[3] = -0.01;
        c.rb0[3] = 2.5;
        c._tail[4] = 1 | (3 << 8) | (5 << 16);
        c._tail[5] = (-0.02f32).to_bits();
        c._tail[6] = 0x0102_0003;
        c.point_triangles[0] = 65537;
        c.lifecycle[1] = (1 << crate::types::CONTACT_PERSISTED_SHIFT) | (1 << (crate::types::CONTACT_PERSISTED_SHIFT + 2));
        let d = gpu_contact_diagnostics(&[c]);
        assert_eq!(d.len(), 1);
        assert_eq!(d[0].pair, (2, 7));
        assert_eq!(
            (d[0].sat_type, d[0].sat_index_a, d[0].sat_index_b),
            (1, 3, 5)
        );
        assert_eq!(d[0].point_ids[0], 0x0102_0003);
        assert_eq!(d[0].point_triangles[0], 65537);
        assert_eq!(d[0].point_persisted, [true,false,false,false]);
        assert_eq!(d[0].base_separations[0], -0.01);
        assert_eq!(d[0].normal_impulses[0], 2.5);
    }
}
