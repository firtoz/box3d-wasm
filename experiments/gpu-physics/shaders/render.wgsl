struct Camera {
    view_proj: mat4x4<f32>,
}

@group(0) @binding(0) var<uniform> camera: Camera;

struct VsOut {
    @builtin(position) clip: vec4<f32>,
    @location(0) world_normal: vec3<f32>,
    @location(1) color: vec3<f32>,
}

fn quat_rotate(q: vec4<f32>, v: vec3<f32>) -> vec3<f32> {
    let u = q.xyz;
    let s = q.w;
    return 2.0 * dot(u, v) * u + (s * s - dot(u, u)) * v + 2.0 * s * cross(u, v);
}

@vertex
fn vs_sphere(
    @location(0) local_pos: vec3<f32>,
    @location(1) local_normal: vec3<f32>,
    @location(2) inst_pos_mass: vec4<f32>,
    @location(3) _vel: vec3<f32>,
    @location(4) kind: u32,
    @location(5) half: vec3<f32>,
    @location(6) flags: u32,
    @location(7) rot: vec4<f32>,
    @builtin(instance_index) instance: u32,
) -> VsOut {
    var out: VsOut;
    if (kind != 0u || (flags & 2u) != 0u) {
        out.clip = vec4<f32>(0.0, 0.0, 2.0, 1.0);
        out.world_normal = local_normal;
        out.color = vec3<f32>(0.0);
        return out;
    }
    let center = inst_pos_mass.xyz;
    let world = center + quat_rotate(rot, local_pos * half.x);
    out.clip = camera.view_proj * vec4<f32>(world, 1.0);
    out.world_normal = quat_rotate(rot, local_normal);
    let hash = f32((instance * 2654435761u) & 255u) / 255.0;
    out.color = vec3<f32>(0.25 + 0.55 * hash, 0.45, 0.85 - 0.4 * hash);
    return out;
}

@vertex
fn vs_box(
    @location(0) local_pos: vec3<f32>,
    @location(1) local_normal: vec3<f32>,
    @location(2) inst_pos_mass: vec4<f32>,
    @location(3) _vel: vec3<f32>,
    @location(4) kind: u32,
    @location(5) half: vec3<f32>,
    @location(6) flags: u32,
    @location(7) rot: vec4<f32>,
    @builtin(instance_index) instance: u32,
) -> VsOut {
    var out: VsOut;
    if (kind != 1u || (flags & 2u) != 0u) {
        out.clip = vec4<f32>(0.0, 0.0, 2.0, 1.0);
        out.world_normal = local_normal;
        out.color = vec3<f32>(0.0);
        return out;
    }
    let center = inst_pos_mass.xyz;
    let world = center + quat_rotate(rot, local_pos * half);
    out.clip = camera.view_proj * vec4<f32>(world, 1.0);
    out.world_normal = quat_rotate(rot, local_normal);
    let hash = f32((instance * 2654435761u) & 255u) / 255.0;
    out.color = vec3<f32>(0.75, 0.35 + 0.4 * hash, 0.2);
    return out;
}

@vertex
fn vs_capsule(
    @location(0) local_pos: vec3<f32>,
    @location(1) local_normal: vec3<f32>,
    @location(2) inst_pos_mass: vec4<f32>,
    @location(3) _vel: vec3<f32>,
    @location(4) kind: u32,
    @location(5) half: vec3<f32>,
    @location(6) flags: u32,
    @location(7) rot: vec4<f32>,
    @builtin(instance_index) instance: u32,
) -> VsOut {
    var out: VsOut;
    if (kind != 2u || (flags & 2u) != 0u) {
        out.clip = vec4<f32>(0.0, 0.0, 2.0, 1.0);
        out.world_normal = local_normal;
        out.color = vec3<f32>(0.0);
        return out;
    }
    let center = inst_pos_mass.xyz;
    let r = half.x;
    let h = half.y;
    var lp = local_pos;
    var ln = local_normal;
    if (abs(lp.x) <= 1.0) {
        lp = vec3<f32>(lp.x * h, lp.y * r, lp.z * r);
        ln = vec3<f32>(0.0, ln.y, ln.z);
    } else {
        let s = sign(lp.x);
        let sph = lp - vec3<f32>(s, 0.0, 0.0);
        lp = vec3<f32>(s * h, 0.0, 0.0) + sph * r;
        ln = sph;
    }
    let world = center + quat_rotate(rot, lp);
    out.clip = camera.view_proj * vec4<f32>(world, 1.0);
    out.world_normal = quat_rotate(rot, ln);
    let hash = f32((instance * 2654435761u) & 255u) / 255.0;
    out.color = vec3<f32>(0.2 + 0.3 * hash, 0.7, 0.45);
    return out;
}

@fragment
fn fs_main(in: VsOut) -> @location(0) vec4<f32> {
    let light = normalize(vec3<f32>(0.35, 0.9, 0.25));
    let n = normalize(in.world_normal);
    let diff = max(dot(n, light), 0.12);
    return vec4<f32>(in.color * diff, 1.0);
}

struct GroundIn {
    @builtin(position) clip: vec4<f32>,
    @location(0) world_xz: vec2<f32>,
}

@vertex
fn vs_ground(@location(0) pos: vec3<f32>) -> GroundIn {
    var out: GroundIn;
    out.clip = camera.view_proj * vec4<f32>(pos, 1.0);
    out.world_xz = pos.xz;
    return out;
}

@fragment
fn fs_ground(in: GroundIn) -> @location(0) vec4<f32> {
    let checker = 2.0 * fract(0.5 * (floor(in.world_xz.x) + floor(in.world_xz.y)));
    let shade = select(0.045, 0.065, checker < 0.5);
    let footprint = max(fwidth(in.world_xz.x), fwidth(in.world_xz.y));
    let filtered = mix(shade, 0.055, smoothstep(0.3, 1.0, footprint));
    return vec4<f32>(vec3<f32>(filtered), 1.0);
}
