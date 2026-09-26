// CPU-ordered rotational integration. Native Vulkan preserves explicit
// scalar arithmetic throughout the physics shader with NoContraction.
fn gyro_quat_rotate(q: vec4<f32>, v: vec3<f32>) -> vec3<f32> {
    // Native b3RotateVector form. Unlike the homogeneous expansion, this
    // preserves the vector along the rotation axis when |q| rounds off unity.
    return v + 2.0 * gyro_cross(q.xyz, gyro_cross(q.xyz, v) + q.w * v);
}

fn gyro_quat_mul(a: vec4<f32>, b: vec4<f32>) -> vec4<f32> {
    return vec4<f32>(
        (gyro_cross(a.xyz, b.xyz) + a.w * b.xyz) + b.w * a.xyz,
        a.w * b.w - gyro_dot3(a.xyz, b.xyz),
    );
}

fn gyro_quat_inv_rotate(q: vec4<f32>, v: vec3<f32>) -> vec3<f32> {
    return v + 2.0 * gyro_cross(q.xyz, gyro_cross(q.xyz,v) - q.w*v);
}

fn gyro_solve3(col0: vec3<f32>, col1: vec3<f32>, col2: vec3<f32>, b: vec3<f32>) -> vec3<f32> {
    let det = gyro_dot3(col0, gyro_cross(col1, col2));
    if (abs(det) < 1e-12) {
        return vec3<f32>(0.0);
    }
    let inv = gyro_recip(det);
    return vec3<f32>(
        gyro_dot3(b, gyro_cross(col1, col2)) * inv,
        gyro_dot3(gyro_cross(col2,col0), b) * inv,
        gyro_dot3(gyro_cross(col0,col1), b) * inv,
    );
}

fn gyro_local_inertia_matrix(b: Body) -> mat3x3<f32> {
    let m0=vec3<f32>(b.inv_inertia.x,b.inv_inertia_offdiag.x,b.inv_inertia_offdiag.y);
    let m1=vec3<f32>(b.inv_inertia_offdiag.x,b.inv_inertia.y,b.inv_inertia_offdiag.z);
    let m2=vec3<f32>(b.inv_inertia_offdiag.y,b.inv_inertia_offdiag.z,b.inv_inertia.z);
    let det=gyro_dot3(m0,gyro_cross(m1,m2));
    if (abs(det)<1e-12) {return mat3x3<f32>(vec3<f32>(0.0),vec3<f32>(0.0),vec3<f32>(0.0));}
    let invDet=gyro_recip(det);
    return transpose(mat3x3<f32>(invDet*gyro_cross(m1,m2),invDet*gyro_cross(m2,m0),invDet*gyro_cross(m0,m1)));
}

fn gyro_apply_gyro(b: ptr<function, Body>, h: f32) {
    let inv = (*b).inv_inertia;
    if (inv.x < 1e-12 || inv.y < 1e-12 || inv.z < 1e-12) {
        return;
    }
    let inertia = gyro_local_inertia_matrix(*b);
    let q = gyro_quat_mul((*b).dq, (*b).rot);
    var omega1 = gyro_quat_inv_rotate(q, (*b).omega);
    var omega2 = omega1;
    for (var it = 0u; it < 1u; it++) {
        let i00=inertia[0].x;let i01=inertia[1].x;let i02=inertia[2].x;
        let i11=inertia[1].y;let i12=inertia[2].y;let i22=inertia[2].z;
        let w1=omega2.x;let w2=omega2.y;let w3=omega2.z;
        let Iw1=i00*w1+i01*w2+i02*w3;
        let Iw2=i01*w1+i11*w2+i12*w3;
        let Iw3=i02*w1+i12*w2+i22*w3;
        let dw=omega2-omega1;
        let residual=vec3<f32>(i00*dw.x+i01*dw.y+i02*dw.z+h*(w2*Iw3-w3*Iw2),i01*dw.x+i11*dw.y+i12*dw.z+h*(w3*Iw1-w1*Iw3),i02*dw.x+i12*dw.y+i22*dw.z+h*(w1*Iw2-w2*Iw1));
        let j0=vec3<f32>(i00+h*(w2*i02-w3*i01),i01+h*(w3*i00-w1*i02-Iw3),i02+h*(w1*i01-w2*i00+Iw2));
        let j1=vec3<f32>(i01+h*(w2*i12-w3*i11+Iw3),i11+h*(w3*i01-w1*i12),i12+h*(w1*i11-w2*i01-Iw1));
        let j2=vec3<f32>(i02+h*(w2*i22-w3*i12-Iw2),i12+h*(w3*i02-w1*i22+Iw1),i22+h*(w1*i12-w2*i02));
        omega2 = omega2 - gyro_solve3(j0, j1, j2, residual);
    }
    (*b).omega = gyro_quat_rotate(q, omega2);
}

// Match CPU normalization: rounded sqrt, then rounded reciprocal, then scale.
// A residual correction removes the native shader sqrt/divide approximation.
// This is not an added damping term or a change to the Newton iteration count.
fn gyro_norm3(v: vec3<f32>) -> vec3<f32> {
    let squared = gyro_dot3(v, v);
    if (squared <= 1.17549435e-35) {
        return vec3<f32>(0.0);
    }
    return gyro_recip(gyro_sqrt(squared)) * v;
}

fn gyro_norm4(q: vec4<f32>) -> vec4<f32> {
    let length_squared = ((q.x*q.x + q.y*q.y) + q.z*q.z) + q.w*q.w;
    if (length_squared <= 1.17549435e-35) {
        return vec4<f32>(0.0, 0.0, 0.0, 1.0);
    }
    let scale = gyro_recip(gyro_sqrt(length_squared));
    return scale * q;
}

fn gyro_dot3(a: vec3<f32>, b: vec3<f32>) -> f32 {
    return (a.x*b.x + a.y*b.y) + a.z*b.z;
}

fn gyro_recip(x: f32) -> f32 {
    let estimate = 1.0 / x;
    let residual = fma(-estimate, x, 1.0);
    return fma(residual, estimate, estimate);
}

fn gyro_divide(a: f32, b: f32) -> f32 {
    let estimate = a / b;
    let residual = fma(-estimate, b, a);
    return fma(residual, gyro_recip(b), estimate);
}

fn gyro_sqrt(x: f32) -> f32 {
    let bits = bitcast<u32>(x);
    let exponent = (bits >> 23u) & 255u;
    if (x <= 0.0 || exponent == 0u || exponent == 255u) { return sqrt(x); }
    // Scale by an even power of two so midpoint residuals cannot underflow.
    let e = i32(exponent) - 127;
    let half_e = e >> 1;
    let scaled = bitcast<f32>((bits & 0x007fffffu) | (u32(e - 2 * half_e + 127) << 23u));
    let estimate = sqrt(scaled);
    let residual = fma(-estimate, estimate, scaled);
    let root = fma(residual, 0.5 / estimate, estimate);
    // A rounded Newton update can land on the wrong side of a sqrt midpoint.
    // Compare squared midpoints using residuals rather than rounding the midpoint.
    let root_bits = bitcast<u32>(root);
    let upper = bitcast<f32>(root_bits + 1u);
    let lower = bitcast<f32>(root_bits - 1u);
    let up_gap = upper - root;
    let down_gap = root - lower;
    let remainder = fma(-root, root, scaled);
    let above = fma(-root, up_gap, remainder) - 0.25 * up_gap * up_gap;
    let below = fma(root, down_gap, remainder) - 0.25 * down_gap * down_gap;
    let odd = (root_bits & 1u) != 0u;
    var rounded = root;
    if (above > 0.0 || (above == 0.0 && odd)) { rounded = upper; }
    else if (below < 0.0 || (below == 0.0 && odd)) { rounded = lower; }
    return rounded * bitcast<f32>(u32(half_e + 127) << 23u);
}

// Keep component operations explicit: a native cross builtin can contract its
// products even when neighboring arithmetic has NoContraction decorations.
fn gyro_cross(a: vec3<f32>, b: vec3<f32>) -> vec3<f32> {
    return vec3<f32>(a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x);
}

fn gyro_integrate_rotation(q:vec4<f32>,w:vec3<f32>,h:f32)->vec4<f32> {
    let qd=gyro_quat_mul(vec4<f32>(0.5*(h*w),0.0),q);
    return gyro_norm4(q+qd);
}
fn gyro_finish_rotation(dq:vec4<f32>,q:vec4<f32>)->vec4<f32> {
    return gyro_norm4(gyro_quat_mul(dq,q));
}


// Box3D b3Atan2's deterministic polynomial is part of joint-limit behavior.
// Keep its scalar evaluation order under the Vulkan precision compiler.
fn gyro_atan2(y: f32, x: f32) -> f32 {
    if (x == 0.0 && y == 0.0) { return 0.0; }
    let ax = abs(x);
    let ay = abs(y);
    let a = gyro_divide(min(ay, ax), max(ay, ax));
    let s = a * a;
    let c = s * a;
    let q = s * s;
    var r = 0.024840285 * q + 0.18681418;
    let t = -0.094097948 * q - 0.33213072;
    r = r * s + t;
    r = r * c + a;
    if (ay > ax) { r = 1.57079637 - r; }
    if (x < 0.0) { r = 3.14159274 - r; }
    if (y < 0.0) { r = -r; }
    return r;
}
