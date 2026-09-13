// Exact memoization of the canonical greedy coloring function. No manifold or
// impulse data is cached. Allocation is capacity-sized and private to this sim.
override GRAPH_MEMO_BASE:u32=0u;
var<workgroup> memo_masks:array<u32,8160>;
var<workgroup> memo_counts:array<u32,24>;
var<workgroup> memo_changed:atomic<u32>;
fn memo_body_base()->u32 {return GRAPH_MEMO_BASE+52u;}
fn memo_edge_base()->u32 {return memo_body_base()+2u*params.body_count;}

@compute @workgroup_size(64)
fn graph_assign_dynamic_memo(@builtin(local_invocation_index) lane:u32) {
    let n=min(scratch[SCR_DYN_DYN_N],PAIR_CAP);
    if (params.body_count>8160u || n>params.contact_capacity) {
        if (lane==0u) {record_contact_drop(9u);}
        return;
    }
    if (lane==0u) {
        atomicStore(&memo_changed,select(0u,1u,
            atomicLoad(&query[GRAPH_MEMO_BASE])!=1u
            || atomicLoad(&query[GRAPH_MEMO_BASE+1u])!=params.body_count
            || atomicLoad(&query[GRAPH_MEMO_BASE+2u])!=n));
    }
    workgroupBarrier();
    for (var b=lane;b<params.body_count;b+=64u) {
        let mask=atomicLoad(&atom[ATOM_JACOBI+b]);
        memo_masks[b]=mask;
        if (atomicLoad(&query[memo_body_base()+2u*b])!=mask) {atomicStore(&memo_changed,1u);}
    }
    if (lane<24u) {
        let count=atomicLoad(&atom[atom_graph_color()+lane]);
        memo_counts[lane]=count;
        if (atomicLoad(&query[GRAPH_MEMO_BASE+4u+lane])!=count) {atomicStore(&memo_changed,1u);}
    }
    for (var i=lane;i<n;i+=64u) {
        let slot=scratch[SCR_ACTIVE_CONTACT+scratch[SCR_NEXT_OCCUPIED+i]];
        let h=contacts[slot];let base=memo_edge_base()+6u*i;
        if (atomicLoad(&query[base])!=slot || atomicLoad(&query[base+1u])!=h.a
            || atomicLoad(&query[base+2u])!=h.b
            || atomicLoad(&query[base+5u])!=contact_persistent[slot].lifecycle.w) {
            atomicStore(&memo_changed,1u);
        }
    }
    workgroupBarrier();
    let changed=atomicLoad(&memo_changed)!=0u;
    if (!changed) {
        for (var b=lane;b<params.body_count;b+=64u) {
            memo_masks[b]=atomicLoad(&query[memo_body_base()+2u*b+1u]);
        }
        if (lane<24u) {memo_counts[lane]=atomicLoad(&query[GRAPH_MEMO_BASE+28u+lane]);}
        for (var i=lane;i<n;i+=64u) {
            let base=memo_edge_base()+6u*i;
            let slot=atomicLoad(&query[base]);let col=atomicLoad(&query[base+3u]);let local=atomicLoad(&query[base+4u]);
            scratch[color_contact_base()+col*params.contact_capacity+local]=slot;
            store_graph_meta(slot,col,local);
        }
        if (lane==0u) {atomicAdd(&query[GRAPH_MEMO_BASE+3u],1u);}
    } else {
        // Save initial state before the serial reference walk mutates it.
        for (var b=lane;b<params.body_count;b+=64u) {atomicStore(&query[memo_body_base()+2u*b],memo_masks[b]);}
        if (lane<24u) {atomicStore(&query[GRAPH_MEMO_BASE+4u+lane],memo_counts[lane]);}
    }
    workgroupBarrier();
    if (changed && lane==0u) {
        for (var i=0u;i<n;i++) {
            let slot=scratch[SCR_ACTIVE_CONTACT+scratch[SCR_NEXT_OCCUPIED+i]];
            let h=contacts[slot];
            if (h.a>=params.body_count || h.b>=params.body_count) {record_contact_drop(9u);continue;}
            let available=(~(memo_masks[h.a]|memo_masks[h.b])) & ((1u<<DYNAMIC_COLOR_COUNT)-1u);
            var col=OVERFLOW_COLOR;
            if (available!=0u) {col=firstTrailingBit(available);memo_masks[h.a]|=1u<<col;memo_masks[h.b]|=1u<<col;}
            let local=memo_counts[col];memo_counts[col]=local+1u;
            if (local<params.contact_capacity) {
                scratch[color_contact_base()+col*params.contact_capacity+local]=slot;
                store_graph_meta(slot,col,local);
            } else {record_contact_drop(9u);}
        }
    }
    workgroupBarrier();
    storageBarrier();
    if (changed) {
        for (var b=lane;b<params.body_count;b+=64u) {atomicStore(&query[memo_body_base()+2u*b+1u],memo_masks[b]);}
        if (lane<24u) {atomicStore(&query[GRAPH_MEMO_BASE+28u+lane],memo_counts[lane]);}
        for (var i=lane;i<n;i+=64u) {
            let slot=scratch[SCR_ACTIVE_CONTACT+scratch[SCR_NEXT_OCCUPIED+i]];let h=contacts[slot];
            let base=memo_edge_base()+6u*i;
            atomicStore(&query[base],slot);atomicStore(&query[base+1u],h.a);atomicStore(&query[base+2u],h.b);
            atomicStore(&query[base+3u],h.color);atomicStore(&query[base+4u],contact_persistent[slot].lifecycle.z);
            atomicStore(&query[base+5u],contact_persistent[slot].lifecycle.w);
        }
        if (lane==0u) {
            atomicStore(&query[GRAPH_MEMO_BASE],1u);
            atomicStore(&query[GRAPH_MEMO_BASE+1u],params.body_count);
            atomicStore(&query[GRAPH_MEMO_BASE+2u],n);
        }
    }
    for (var b=lane;b<params.body_count;b+=64u) {atomicStore(&atom[ATOM_JACOBI+b],memo_masks[b]);}
    if (lane<24u) {atomicStore(&atom[atom_graph_color()+lane],memo_counts[lane]);}
    storageBarrier();
    if (lane==0u) {finish_dynamic_graph();}
}
