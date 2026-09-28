// Persistent ordering metadata, independent of broadphase pair generation.
// Header: root, high-water, free head, leaves, capacity, error, reserved[2].
// Node: lower[3], upper[3], parent, children[2], shape, enlarged, free next.
// Workspace: traversal stack, rebuild leaves, five-word build frames.
// Proxy metadata: native proxy key, moved-list position (EMPTY if unmoved),
// reversed traversal rank within its body-type tree. Native queries prepend
// results, so dynamic results precede static, then kinematic results.
fn order_pair_priority(a: vec3<u32>, b: vec3<u32>) -> vec3<u32> {
    let empty = 0xffffffffu;
    let at = a.x & 3u; let bt = b.x & 3u;
    let am = a.y != empty; let bm = b.y != empty;
    if ((!am && !bm) || (at != 2u && bt != 2u)) { return vec3<u32>(empty); }
    var owner_a = am;
    if (am && bm) {
        // A moved dynamic proxy owns a cross-type pair. Two moved dynamic
        // proxies deduplicate using proxy keys, not their moved-list positions.
        owner_a = at == 2u && (bt != 2u || a.x < b.x);
    }
    let owner = select(b,a,owner_a);
    let other = select(a,b,owner_a);
    let tree = other.x & 3u;
    let priority = select(select(2u,1u,tree==0u),0u,tree==2u);
    return vec3<u32>(owner.y,priority,other.z);
}

fn order_node(base: u32, index: u32) -> u32 { return base + 8u + 12u * index; }
fn order_stack(base: u32) -> u32 { return base + 8u + 12u * scratch[base+4u]; }
fn order_leaves(base: u32) -> u32 { return order_stack(base) + scratch[base+4u]; }
fn order_frames(base: u32) -> u32 { return order_leaves(base) + scratch[base+4u]; }
fn order_lower(base: u32, index: u32) -> vec3<f32> {
    let p = order_node(base,index);
    return bitcast<vec3<f32>>(vec3<u32>(scratch[p],scratch[p+1u],scratch[p+2u]));
}
fn order_upper(base: u32, index: u32) -> vec3<f32> {
    let p = order_node(base,index)+3u;
    return bitcast<vec3<f32>>(vec3<u32>(scratch[p],scratch[p+1u],scratch[p+2u]));
}
fn order_bounds(base: u32, index: u32, lower: vec3<f32>, upper: vec3<f32>) {
    let p = order_node(base,index);
    let a = bitcast<vec3<u32>>(lower);
    let b = bitcast<vec3<u32>>(upper);
    scratch[p]=a.x; scratch[p+1u]=a.y; scratch[p+2u]=a.z;
    scratch[p+3u]=b.x; scratch[p+4u]=b.y; scratch[p+5u]=b.z;
}
fn order_allocate(base: u32) -> u32 {
    var index = scratch[base+2u];
    if (index != 0xffffffffu) {
        scratch[base+2u] = scratch[order_node(base,index)+11u];
    } else {
        index = scratch[base+1u];
        if (index >= scratch[base+4u]) { scratch[base+5u]=1u; return 0xffffffffu; }
        scratch[base+1u]=index+1u;
    }
    let p=order_node(base,index);
    for (var i=6u;i<12u;i++) { scratch[p+i]=0xffffffffu; }
    scratch[p+10u]=0u;
    return index;
}
fn order_free(base: u32, index: u32) {
    let p=order_node(base,index);
    scratch[p+11u]=scratch[base+2u];
    scratch[base+2u]=index;
}
fn order_enlarge(base: u32, leaf: u32, lower: vec3<f32>, upper: vec3<f32>) {
    order_bounds(base,leaf,lower,upper);
    var parent=scratch[order_node(base,leaf)+6u];
    loop {
        if (parent==0xffffffffu) { break; }
        let a=order_lower(base,parent);
        let b=order_upper(base,parent);
        let lo=min(a,lower); let hi=max(b,upper);
        order_bounds(base,parent,lo,hi);
        let p=order_node(base,parent);
        scratch[p+10u]=1u;
        parent=scratch[p+6u];
        if (all(lo==a) && all(hi==b)) { break; }
    }
    loop {
        if (parent==0xffffffffu) { break; }
        let p=order_node(base,parent);
        if (scratch[p+10u]!=0u) { break; }
        scratch[p+10u]=1u;
        parent=scratch[p+6u];
    }
}
fn order_center(base: u32, index: u32) -> vec3<f32> {
    return 0.5*(order_lower(base,index)+order_upper(base,index));
}
fn order_partition(base: u32, start: u32, count: u32) -> u32 {
    if (count<=2u) { return count/2u; }
    let list=order_leaves(base)+start;
    var lower=order_center(base,scratch[list]);
    var upper=lower;
    for (var i=1u;i<count;i++) {
        let center=order_center(base,scratch[list+i]);
        lower=min(lower,center); upper=max(upper,center);
    }
    let d=upper-lower;
    var axis=2u;
    if (d.x>=d.y && d.x>=d.z) { axis=0u; }
    else if (d.y>=d.z) { axis=1u; }
    let pivot=0.5*(lower[axis]+upper[axis]);
    var first=0u; var last=count;
    loop {
        if (first>=last) { break; }
        loop {
            if (first>=last) { break; }
            if (order_center(base,scratch[list+first])[axis]>=pivot) { break; }
            first+=1u;
        }
        loop {
            if (first>=last) { break; }
            if (order_center(base,scratch[list+last-1u])[axis]<pivot) { break; }
            last-=1u;
        }
        if (first<last) {
            let value=scratch[list+first];
            scratch[list+first]=scratch[list+last-1u];
            scratch[list+last-1u]=value;
            first+=1u; last-=1u;
        }
    }
    return select(count/2u,first,first>0u && first<count);
}
// Returns node indices in reversed native query order (native prepends pairs).
fn order_query_leaves(base: u32) -> u32 {
    if (scratch[base]==0xffffffffu) { return 0u; }
    let stack=order_stack(base); let list=order_leaves(base);
    scratch[stack]=scratch[base];
    var top=1u; var count=0u;
    loop {
        if (top==0u) { break; }
        top-=1u;
        let index=scratch[stack+top]; let p=order_node(base,index);
        if (scratch[p+7u]==0xffffffffu) {
            scratch[list+count]=index; count+=1u;
        } else {
            scratch[stack+top]=scratch[p+8u];
            scratch[stack+top+1u]=scratch[p+7u];
            top+=2u;
        }
    }
    return count;
}
fn order_build_frame(base: u32, top: u32, index: u32, start: u32, end: u32) {
    let p=order_frames(base)+5u*top;
    scratch[p]=index; scratch[p+1u]=start; scratch[p+2u]=end;
    scratch[p+3u]=start+order_partition(base,start,end-start);
    scratch[p+4u]=0u;
}
fn order_rebuild(base: u32, full: bool) {
    if (scratch[base]==0xffffffffu) { return; }
    let stack=order_stack(base); let list=order_leaves(base);
    scratch[stack]=scratch[base];
    var top=1u; var count=0u;
    loop {
        if (top==0u) { break; }
        top-=1u;
        let index=scratch[stack+top]; let p=order_node(base,index);
        if (scratch[p+7u]==0xffffffffu || (!full && scratch[p+10u]==0u)) {
            scratch[list+count]=index; count+=1u;
            scratch[p+6u]=0xffffffffu;
        } else {
            scratch[stack+top]=scratch[p+8u];
            scratch[stack+top+1u]=scratch[p+7u];
            top+=2u;
            order_free(base,index);
        }
    }
    if (count==1u) { scratch[base]=scratch[list]; return; }
    let root=order_allocate(base);
    if (root==0xffffffffu) { return; }
    scratch[base]=root;
    top=0u;
    order_build_frame(base,top,root,0u,count);
    loop {
        let frame=order_frames(base)+5u*top;
        let index=scratch[frame]; let p=order_node(base,index);
        let stage=scratch[frame+4u];
        if (stage==2u) {
            let a=scratch[p+7u]; let b=scratch[p+8u];
            order_bounds(base,index,min(order_lower(base,a),order_lower(base,b)),
                max(order_upper(base,a),order_upper(base,b)));
            if (top==0u) { break; }
            top-=1u;
        } else {
            let start=select(scratch[frame+1u],scratch[frame+3u],stage==1u);
            let end=select(scratch[frame+3u],scratch[frame+2u],stage==1u);
            scratch[frame+4u]=stage+1u;
            var child=scratch[list+start];
            if (end-start>1u) { child=order_allocate(base); }
            if (child==0xffffffffu) { return; }
            scratch[p+7u+stage]=child;
            scratch[order_node(base,child)+6u]=index;
            if (end-start>1u) {
                top+=1u;
                order_build_frame(base,top,child,start,end);
            }
        }
    }
}
