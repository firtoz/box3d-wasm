// Appended only when the device supports the complete 32 KiB cache.
var<workgroup> graph_occupancy_cache: array<u32,8168>;

var<workgroup> graph_color_counts: array<u32,24>;


fn graph_shared_choice(a:u32,b:u32) -> vec2<u32> {
    if (a>=params.body_count || b>=params.body_count) {
        record_contact_drop(9u);
        return vec2<u32>(0xffffffffu);
    }
    let available=(~(graph_occupancy_cache[a]|graph_occupancy_cache[b]))
        & ((1u<<DYNAMIC_COLOR_COUNT)-1u);
    var col=OVERFLOW_COLOR;
    if (available!=0u) {
        col=firstTrailingBit(available);
        graph_occupancy_cache[a]|=1u<<col;
        graph_occupancy_cache[b]|=1u<<col;
    }
    let local=graph_color_counts[col];
    graph_color_counts[col]=local+1u;
    return vec2<u32>(col,local);
}

fn graph_shared_publish(slot:u32,choice:vec2<u32>) {
    if (choice.x<24u && choice.y<params.contact_capacity) {
        scratch[color_contact_base()+choice.x*params.contact_capacity+choice.y]=slot;
        store_graph_meta(slot,choice.x,choice.y);
    }
}

fn graph_shared_edge(slot:u32,a:u32,b:u32) {
    graph_shared_publish(slot,graph_shared_choice(a,b));
}

@compute @workgroup_size(64)
fn graph_assign_dynamic_shared(@builtin(local_invocation_index) lane:u32) {
    if (params.body_count>8168u) {
        if (lane==0u) {record_contact_drop(9u);}
        return;
    }
    for (var b=lane;b<params.body_count;b+=64u) {
        graph_occupancy_cache[b]=atomicLoad(&atom[atom_jacobi()+b]);
    }
    if (lane<24u) {graph_color_counts[lane]=atomicLoad(&atom[atom_graph_color()+lane]);}
    workgroupBarrier();
    // Reserve 192 endpoint words plus one uniform loop bound in the unused
    // occupancy tail. Body spans above this limit retain the original walk.
    if (params.body_count<=7975u) {
        if (lane==0u) {graph_occupancy_cache[8167u]=min(scratch[SCR_DYN_DYN_N],pair_cap());}
        let n=workgroupUniformLoad(&graph_occupancy_cache[8167u]);
        if (n>=128u) {
            for (var base=0u;base<n;base+=64u) {
                if (base+lane<n) {
                    let unique_i=scratch[scr_next_occupied()+base+lane];
                    let slot=scratch[scr_active_contact()+unique_i];
                    let h=contacts[slot];
                    let dst=params.body_count+3u*lane;
                    graph_occupancy_cache[dst]=slot;
                    graph_occupancy_cache[dst+1u]=h.a;
                    graph_occupancy_cache[dst+2u]=h.b;
                }
                workgroupBarrier();
                if (lane==0u) {
                    for (var i=0u;i<min(64u,n-base);i++) {
                        let src=params.body_count+3u*i;
                        let choice=graph_shared_choice(graph_occupancy_cache[src+1u],graph_occupancy_cache[src+2u]);
                        graph_occupancy_cache[src+1u]=choice.x;
                        graph_occupancy_cache[src+2u]=choice.y;
                    }
                }
                workgroupBarrier();
                // Each unique contact and (color,local) output has one owner.
                // Decisions remain canonical; only their publication is parallel.
                if (base+lane<n) {
                    let src=params.body_count+3u*lane;
                    graph_shared_publish(graph_occupancy_cache[src],vec2<u32>(graph_occupancy_cache[src+1u],graph_occupancy_cache[src+2u]));
                }
                workgroupBarrier();
            }
        } else if (lane==0u) {
            for (var i=0u;i<n;i++) {
                let slot=scratch[scr_active_contact()+scratch[scr_next_occupied()+i]];
                let h=contacts[slot];graph_shared_edge(slot,h.a,h.b);
            }
        }
    } else if (lane==0u) {
        let n=min(scratch[SCR_DYN_DYN_N],pair_cap());
        for (var i=0u;i<n;i++) {
            let slot=scratch[scr_active_contact()+scratch[scr_next_occupied()+i]];
            let h=contacts[slot];graph_shared_edge(slot,h.a,h.b);
        }
    }
    workgroupBarrier();
    // Preserve the global state, including reserved static-color bits.
    for (var b=lane;b<params.body_count;b+=64u) {
        atomicStore(&atom[atom_jacobi()+b],graph_occupancy_cache[b]);
    }
    if (lane<24u) {atomicStore(&atom[atom_graph_color()+lane],graph_color_counts[lane]);}
    storageBarrier();
    if (lane==0u) {finish_dynamic_graph();}
}
