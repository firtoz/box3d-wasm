// Conservative convex sweep. Mirrors api/query.rs; points are body-origin local.
// Rounded convexes represent spheres (1 core point), capsules (2), and hulls.
struct CcdProxy { first: u32, count: u32, radius: f32, unused: u32 }
struct CcdTransform { p: vec4<f32>, q: vec4<f32> }
struct CcdVertex { a: vec3<f32>, b: vec3<f32>, w: vec3<f32>, weight: f32, ia: u32, ib: u32 }
struct CcdSimplex { vertices: array<CcdVertex, 4>, count: u32 }
struct CcdDistance { a: vec3<f32>, b: vec3<f32>, normal: vec3<f32>, distance: f32 }
struct CcdHit { point: vec4<f32>, normal: vec4<f32>, fraction: f32, status: u32, unused: vec2<u32> }
fn ccd_rotate(q: vec4<f32>, v: vec3<f32>) -> vec3<f32> {
    return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}
fn ccd_point(proxy: CcdProxy, xf: CcdTransform, i: u32) -> vec3<f32> {
    return xf.p.xyz + ccd_rotate(xf.q, ccd_points[proxy.first + i].xyz);
}
fn ccd_support(proxy: CcdProxy, xf: CcdTransform, direction: vec3<f32>) -> u32 {
    let origin = ccd_point(proxy, xf, 0u);
    var best = 0u;
    var projection = 0.0;
    for (var i = 1u; i < proxy.count; i++) {
        let value = dot(direction, ccd_point(proxy, xf, i) - origin);
        if (value > projection) { best = i; projection = value; }
    }
    return best;
}
fn ccd_edge(a: vec3<f32>, b: vec3<f32>) -> vec3<f32> {
    let ab = b-a;
    return vec3<f32>(dot(b, ab), -dot(a, ab), dot(ab, ab));
}
fn ccd_tri(a: vec3<f32>, b: vec3<f32>, c: vec3<f32>) -> vec4<f32> {
    let n = cross(b-a, c-a);
    return vec4<f32>(dot(cross(b,c),n), dot(cross(c,a),n), dot(cross(a,b),n), dot(n,n));
}
fn ccd_set_vertex(s: ptr<function, CcdSimplex>, a: CcdVertex) {
    (*s).count = 1u;
    (*s).vertices[0] = a;
    (*s).vertices[0].weight = 1.0;
}
fn ccd_set_edge(s: ptr<function, CcdSimplex>, a: CcdVertex, b: CcdVertex, weights: vec3<f32>) -> bool {
    if (weights.z <= 0.0) { return false; }
    (*s).count = 2u;
    (*s).vertices[0] = a; (*s).vertices[1] = b;
    (*s).vertices[0].weight = weights.x / weights.z;
    (*s).vertices[1].weight = weights.y / weights.z;
    return true;
}
fn ccd_solve2(s: ptr<function, CcdSimplex>) -> bool {
    let a = (*s).vertices[0]; let b = (*s).vertices[1];
    let weights = ccd_edge(a.w,b.w);
    if (weights.y <= 0.0) { ccd_set_vertex(s,a); }
    else if (weights.x <= 0.0) { ccd_set_vertex(s,b); }
    else { return ccd_set_edge(s,a,b,weights); }
    return true;
}
fn ccd_solve3(s: ptr<function, CcdSimplex>) -> bool {
    let a=(*s).vertices[0]; let b=(*s).vertices[1]; let c=(*s).vertices[2];
    let ab=ccd_edge(a.w,b.w); let bc=ccd_edge(b.w,c.w); let ca=ccd_edge(c.w,a.w);
    if (ab.y <= 0.0 && ca.x <= 0.0) { ccd_set_vertex(s,a); return true; }
    if (bc.y <= 0.0 && ab.x <= 0.0) { ccd_set_vertex(s,b); return true; }
    if (ca.y <= 0.0 && bc.x <= 0.0) { ccd_set_vertex(s,c); return true; }
    let abc=ccd_tri(a.w,b.w,c.w);
    if (abc.z <= 0.0 && ab.x > 0.0 && ab.y > 0.0) { return ccd_set_edge(s,a,b,ab); }
    if (abc.x <= 0.0 && bc.x > 0.0 && bc.y > 0.0) { return ccd_set_edge(s,b,c,bc); }
    if (abc.y <= 0.0 && ca.x > 0.0 && ca.y > 0.0) { return ccd_set_edge(s,c,a,ca); }
    if (abc.w <= 0.0) { return false; }
    (*s).vertices[0].weight=abc.x/abc.w;
    (*s).vertices[1].weight=abc.y/abc.w;
    (*s).vertices[2].weight=abc.z/abc.w;
    return true;
}
fn ccd_closest(s: CcdSimplex) -> vec3<f32> {
    var point=vec3<f32>(0.0);
    for (var i=0u; i<s.count; i++) { point += s.vertices[i].weight*s.vertices[i].w; }
    return point;
}
fn ccd_solve4(s: ptr<function, CcdSimplex>) -> bool {
    let vertices=(*s).vertices;
    let faces=array<vec4<u32>,4>(vec4<u32>(0,2,1,3),vec4<u32>(0,1,3,2),vec4<u32>(0,3,2,1),vec4<u32>(1,2,3,0));
    var best=3.402823466e38;
    var found=false;
    for (var i=0u; i<4u; i++) {
        let f=faces[i]; let a=vertices[f.x]; let b=vertices[f.y]; let c=vertices[f.z]; let o=vertices[f.w];
        let n=cross(b.w-a.w,c.w-a.w);
        if (dot(n,-a.w)*dot(n,o.w-a.w) >= 0.0) { continue; }
        var face: CcdSimplex; face.count=3u;
        face.vertices[0]=a; face.vertices[1]=b; face.vertices[2]=c;
        if (!ccd_solve3(&face)) { continue; }
        let closest=ccd_closest(face);
        let distance=dot(closest,closest);
        if (!found || distance<best) { best=distance; found=true; (*s)=face; }
    }
    if (found) { return true; }
    let a=vertices[0].w; let b=vertices[1].w; let c=vertices[2].w; let d=vertices[3].w;
    let divisor=dot(cross(b-a,c-a),d-a);
    let sign=select(1.0,-1.0,divisor<0.0);
    let denominator=sign*divisor;
    if (denominator <= 0.0) { return false; }
    let weights=sign*vec4<f32>(dot(cross(b,c),d),dot(cross(a,d),c),dot(cross(a,b),d),dot(cross(a,c),b));
    if (any(weights<vec4<f32>(0.0))) { return false; }
    for (var i=0u; i<4u; i++) { (*s).vertices[i].weight=weights[i]/denominator; }
    return true;
}
fn ccd_witnesses(s: CcdSimplex) -> CcdDistance {
    var out: CcdDistance;
    for (var i=0u; i<s.count; i++) {
        out.a += s.vertices[i].weight*s.vertices[i].a;
        out.b += s.vertices[i].weight*s.vertices[i].b;
    }
    if (s.count==4u) { out.b=out.a; }
    return out;
}
fn ccd_unit(v: vec3<f32>) -> vec3<f32> {
    let length2=dot(v,v);
    if (length2 > 0.0) { return v/sqrt(length2); }
    return vec3<f32>(0.0);
}
fn ccd_distance(a: CcdProxy, xa: CcdTransform, b: CcdProxy, xb: CcdTransform) -> CcdDistance {
    var s: CcdSimplex; s.count=1u;
    s.vertices[0].a=ccd_point(a,xa,0u); s.vertices[0].b=ccd_point(b,xb,0u);
    s.vertices[0].w=s.vertices[0].b-s.vertices[0].a; s.vertices[0].weight=1.0;
    var backup=s; var previous=3.402823466e38; var normal=vec3<f32>(0.0);
    for (var iteration=0u; iteration<32u; iteration++) {
        var solved=false;
        switch s.count {
            case 1u: { s.vertices[0].weight=1.0; solved=true; }
            case 2u: { solved=ccd_solve2(&s); }
            case 3u: { solved=ccd_solve3(&s); }
            case 4u: { solved=ccd_solve4(&s); }
            default: {}
        }
        if (!solved) { s=backup; break; }
        if (s.count==4u) { return ccd_witnesses(s); }
        let closest=ccd_closest(s); let distance2=dot(closest,closest);
        if (distance2>=previous) { s=backup; break; }
        previous=distance2;
        var direction=vec3<f32>(0.0);
        switch s.count {
            case 1u: { direction=-s.vertices[0].w; }
            case 2u: { let ab=s.vertices[1].w-s.vertices[0].w; direction=cross(cross(ab,-s.vertices[0].w),ab); }
            case 3u: { let p=s.vertices[0].w; let n=cross(s.vertices[1].w-p,s.vertices[2].w-p); direction=select(-n,n,dot(n,p)<0.0); }
            default: {}
        }
        if (dot(direction,direction)<1.17549435e-35) { return ccd_witnesses(s); }
        normal=-direction;
        let ia=ccd_support(a,xa,-direction); let ib=ccd_support(b,xb,direction);
        var duplicate=false;
        for (var i=0u; i<s.count; i++) { duplicate=duplicate || (s.vertices[i].ia==ia && s.vertices[i].ib==ib); }
        if (duplicate) { break; }
        backup=s;
        var vertex: CcdVertex;
        vertex.a=ccd_point(a,xa,ia); vertex.b=ccd_point(b,xb,ib); vertex.w=vertex.b-vertex.a;
        vertex.ia=ia; vertex.ib=ib; s.vertices[s.count]=vertex; s.count++;
    }
    var out=ccd_witnesses(s);
    out.distance=length(out.b-out.a); out.normal=ccd_unit(normal);
    if (all(out.normal==vec3<f32>(0.0))) { out.normal=ccd_unit(out.b-out.a); }
    return out;
}
fn ccd_interpolate(a: CcdTransform,b: CcdTransform,t:f32) -> CcdTransform {
    var out: CcdTransform; out.p=a.p+t*(b.p-a.p);
    let qa=normalize(a.q); var qb=normalize(b.q);
    if (dot(qa,qb)<0.0) { qb=-qb; }
    out.q=normalize(qa*(1.0-t)+qb*t); return out;
}
fn ccd_radius(a:CcdProxy) -> f32 {
    var radius=a.radius;
    for (var i=0u;i<a.count;i++) { radius=max(radius,length(ccd_points[a.first+i].xyz)+a.radius); }
    return radius;
}
fn ccd_angle(a:vec4<f32>,b:vec4<f32>) -> f32 {
    return 2.0*acos(clamp(abs(dot(normalize(a),normalize(b))),0.0,1.0));
}
fn ccd_fixed_separated(a:CcdProxy,a0:CcdTransform,a1:CcdTransform,b:CcdProxy,b0:CcdTransform,b1:CcdTransform,
                      fraction:f32,normal:vec3<f32>,separation_target:f32) -> bool {
    if (!(all(a0.q==a1.q) || all(a0.q==-a1.q)) || !(all(b0.q==b1.q) || all(b0.q==-b1.q))) { return false; }
    let axis=ccd_unit(normal); if (all(axis==vec3<f32>(0.0))) { return false; }
    for (var endpoint=0u;endpoint<2u;endpoint++) {
        let t=select(0.0,fraction,endpoint==1u);
        let xa=ccd_interpolate(a0,a1,t); let xb=ccd_interpolate(b0,b1,t);
        let origin=ccd_point(a,xa,0u); var upper=-3.402823466e38; var lower=3.402823466e38; var magnitude=1.0;
        for (var i=0u;i<a.count;i++) {
            let p=ccd_point(a,xa,i); upper=max(upper,dot(p-origin,axis)); magnitude=max(magnitude,max(abs(p.x),max(abs(p.y),abs(p.z))));
        }
        for (var i=0u;i<b.count;i++) {
            let p=ccd_point(b,xb,i); lower=min(lower,dot(p-origin,axis)); magnitude=max(magnitude,max(abs(p.x),max(abs(p.y),abs(p.z))));
        }
        if (lower-upper<=separation_target+32.0*1.192092896e-7*magnitude) { return false; }
    }
    return true;
}
fn ccd_make_hit(distance:CcdDistance,ra:f32,rb:f32,fraction:f32,status:u32) -> CcdHit {
    var out:CcdHit; var normal=ccd_unit(distance.normal);
    if (all(normal==vec3<f32>(0.0))) { normal=ccd_unit(distance.b-distance.a); }
    out.point=vec4<f32>((distance.a+ra*normal+distance.b-rb*normal)*0.5,0.0);
    out.normal=vec4<f32>(normal,0.0); out.fraction=fraction; out.status=status; return out;
}
fn ccd_sweep(a:CcdProxy,a0:CcdTransform,a1:CcdTransform,b:CcdProxy,b0:CcdTransform,b1:CcdTransform,
             max_fraction:f32,iteration_limit:u32) -> CcdHit {
    var miss:CcdHit;
    let separation_target=max(0.005,a.radius+b.radius-0.005);
    let motion=length(a1.p.xyz-a0.p.xyz)+length(b1.p.xyz-b0.p.xyz)
        +ccd_angle(a0.q,a1.q)*ccd_radius(a)+ccd_angle(b0.q,b1.q)*ccd_radius(b);
    if (motion<=1.192092896e-7 || max_fraction<=0.0) { return miss; }
    let initial=ccd_distance(a,ccd_interpolate(a0,a1,0.0),b,ccd_interpolate(b0,b1,0.0));
    if (initial.distance<=separation_target+0.00125) { return miss; }
    if (ccd_fixed_separated(a,a0,a1,b,b0,b1,max_fraction,initial.normal,separation_target)) { return miss; }
    var fraction=0.0; var separated=initial;
    for (var i=0u;i<iteration_limit;i++) {
        let advance=max((separated.distance-separation_target)/motion,1.0e-6);
        let next_fraction=min(fraction+advance,max_fraction);
        if (next_fraction<=fraction) { return miss; }
        let next=ccd_distance(a,ccd_interpolate(a0,a1,next_fraction),b,ccd_interpolate(b0,b1,next_fraction));
        if (next.distance<=separation_target) {
            var lower=fraction; var upper=next_fraction; var hit=next;
            for (var refine=0u;refine<24u;refine++) {
                let middle=0.5*(lower+upper);
                let candidate=ccd_distance(a,ccd_interpolate(a0,a1,middle),b,ccd_interpolate(b0,b1,middle));
                if (candidate.distance>separation_target) { lower=middle; } else { upper=middle; hit=candidate; }
            }
            if (upper<=0.0) { return miss; }
            return ccd_make_hit(hit,a.radius,b.radius,upper,1u);
        }
        if (next_fraction>=max_fraction) { return miss; }
        fraction=next_fraction; separated=next;
    }
    // Explicit unresolved state: never turn an exhausted sweep into a miss.
    return ccd_make_hit(separated,a.radius,b.radius,fraction,2u);
}
