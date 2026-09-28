// Live ordering for supported, unchanged proxy topology, using GPU state only.
// Shape metadata: proxy key, moved-list rank, reversed leaf rank, moved flag.
fn live_order_tree(kind: u32) -> u32 {
    return params.order_base + kind * (8u + 19u * params.order_node_capacity);
}
fn live_order_metadata(shape: u32) -> u32 {
    return params.order_base + 3u * (8u + 19u * params.order_node_capacity) + 4u * shape;
}
fn live_order_proxy(shape: u32) -> vec3<u32> {
    let p = live_order_metadata(shape);
    return vec3<u32>(scratch[p], scratch[p+1u], scratch[p+2u]);
}
@compute @workgroup_size(1)
fn update_contact_order() {
    if (params.order_enabled == 0u || scratch[params.order_base+6u] == params.physics_step) { return; }
    scratch[params.order_base+6u] = params.physics_step;
    let initial = params.physics_step == 1u;
    var moved_count = 0u;
    for (var shape = 0u; shape < params.shape_count; shape++) {
        let p = live_order_metadata(shape);
        let key = scratch[p];
        scratch[p+1u] = EMPTY;
        scratch[p+3u] = 0u;
        if (key == EMPTY) { continue; }
        let kind = key & 3u; let leaf = key >> 2u;
        var moved = initial;
        if (!initial && kind != 0u) {
            let fat = params.fat_bounds_base + 8u * shape;
            let lower = bitcast<vec3<f32>>(vec3<u32>(scratch[fat],scratch[fat+1u],scratch[fat+2u]));
            let upper = bitcast<vec3<f32>>(vec3<u32>(scratch[fat+4u],scratch[fat+5u],scratch[fat+6u]));
            let tree = live_order_tree(kind);
            moved = any(lower != order_lower(tree,leaf)) || any(upper != order_upper(tree,leaf));
            if (moved) { order_enlarge(tree,leaf,lower,upper); }
        }
        if (moved) { scratch[p+3u] = 1u; moved_count += 1u; }
    }
    // Creation uses shape insertion order; bound changes follow body order
    // and reverse shape insertion within a body. This is not yet exact after
    // native awake-set compaction. Explicit MoveProxy commands disable the cache.
    for (var shape = 0u; shape < params.shape_count; shape++) {
        let p = live_order_metadata(shape);
        if (scratch[p+3u] == 0u) { continue; }
        let body = load_shape(shape).body_index;
        var rank = 0u;
        for (var other = 0u; other < params.shape_count; other++) {
            if (scratch[live_order_metadata(other)+3u] == 0u) { continue; }
            let other_body = load_shape(other).body_index;
            let before = select(other_body < body || (other_body == body && other > shape), other < shape, initial);
            if (before) { rank += 1u; }
        }
        scratch[p+1u] = rank;
    }
    for (var kind = 0u; kind < 3u; kind++) {
        let tree = live_order_tree(kind);
        let count = order_query_leaves(tree);
        for (var rank = 0u; rank < count; rank++) {
            let leaf = scratch[order_leaves(tree)+rank];
            let shape = scratch[order_node(tree,leaf)+9u];
            scratch[live_order_metadata(shape)+2u] = rank;
        }
        // Native rebuilds after queries and only with a nonempty move list.
        if (moved_count != 0u && kind != 0u) { order_rebuild(tree,false); }
    }
}
