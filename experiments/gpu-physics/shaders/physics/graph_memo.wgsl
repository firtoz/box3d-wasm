// Exact memoization of the canonical greedy coloring function. No manifold or
// impulse data is cached. Allocation is capacity-sized and private to this sim.
override GRAPH_MEMO_BASE:u32=0u;
var<workgroup> memo_masks:array<u32,8160>;
var<workgroup> memo_counts:array<u32,24>;
var<workgroup> memo_changed:atomic<u32>;
var<workgroup> memo_prefix:atomic<u32>;
var<workgroup> memo_restart:u32;
fn memo_body_base()->u32 {return GRAPH_MEMO_BASE+52u;}
fn memo_edge_base()->u32 {return memo_body_base()+2u*params.body_count;}

fn memo_choose(a:u32,b:u32)->vec2<u32> {
    if (a>=params.body_count || b>=params.body_count) {
        record_contact_drop(9u);return vec2<u32>(0xffffffffu);
    }
    let available=(~(memo_masks[a]|memo_masks[b])) & ((1u<<DYNAMIC_COLOR_COUNT)-1u);
    var col=OVERFLOW_COLOR;
    if (available!=0u) {col=firstTrailingBit(available);memo_masks[a]|=1u<<col;memo_masks[b]|=1u<<col;}
    let local=memo_counts[col];memo_counts[col]=local+1u;
    return vec2<u32>(col,local);
}

fn memo_publish(slot:u32,choice:vec2<u32>) {
    if (choice.x>=24u) {return;}
    if (choice.y>=params.contact_capacity) {record_contact_drop(9u);return;}
    scratch[color_contact_base()+choice.x*params.contact_capacity+choice.y]=slot;
    store_graph_meta(slot,choice.x,choice.y);
}

fn memo_serial(n:u32) {
    for (var i=0u;i<n;i++) {
        let slot=scratch[SCR_ACTIVE_CONTACT+scratch[SCR_NEXT_OCCUPIED+i]];
        let h=contacts[slot];memo_publish(slot,memo_choose(h.a,h.b));
    }
}

@compute @workgroup_size(64)
fn graph_assign_dynamic_memo(@builtin(local_invocation_index) lane:u32) {
    let n=min(scratch[SCR_DYN_DYN_N],PAIR_CAP);
    if (params.body_count>8160u || n>params.contact_capacity) {
        if (lane==0u) {record_contact_drop(9u);}
        return;
    }
    if (lane==0u) {
        let valid=atomicLoad(&query[GRAPH_MEMO_BASE])==1u
            && atomicLoad(&query[GRAPH_MEMO_BASE+1u])==params.body_count;
        atomicStore(&memo_prefix,select(0u,min(n,atomicLoad(&query[GRAPH_MEMO_BASE+2u])),valid));
        atomicStore(&memo_changed,select(0u,1u,
            atomicLoad(&query[GRAPH_MEMO_BASE])!=1u
            || atomicLoad(&query[GRAPH_MEMO_BASE+1u])!=params.body_count
            || atomicLoad(&query[GRAPH_MEMO_BASE+2u])!=n));
    }
    workgroupBarrier();
    for (var b=lane;b<params.body_count;b+=64u) {
        let mask=atomicLoad(&atom[ATOM_JACOBI+b]);
        memo_masks[b]=mask;
        if (atomicLoad(&query[memo_body_base()+2u*b])!=mask) {atomicStore(&memo_changed,1u);atomicMin(&memo_prefix,0u);}
    }
    if (lane<24u) {
        let count=atomicLoad(&atom[atom_graph_color()+lane]);
        memo_counts[lane]=count;
        if (atomicLoad(&query[GRAPH_MEMO_BASE+4u+lane])!=count) {atomicStore(&memo_changed,1u);atomicMin(&memo_prefix,0u);}
    }
    for (var i=lane;i<n;i+=64u) {
        let slot=scratch[SCR_ACTIVE_CONTACT+scratch[SCR_NEXT_OCCUPIED+i]];
        let h=contacts[slot];let base=memo_edge_base()+6u*i;
        if (atomicLoad(&query[base])!=slot || atomicLoad(&query[base+1u])!=h.a
            || atomicLoad(&query[base+2u])!=h.b
            || atomicLoad(&query[base+5u])!=contact_persistent[slot].lifecycle.w) {
            atomicStore(&memo_changed,1u);
            atomicMin(&memo_prefix,i);
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
    // The unused occupancy tail holds 64 (slot,a,b) records and two uniform
    // loop controls. Greedy decisions remain serial and in input order; only
    // global loads/publication are parallel. Larger spans retain the reference.
    if (params.body_count<=7966u) {
        if (lane==0u) {
            memo_masks[8158u]=n;
            memo_masks[8159u]=select(0u,1u,changed);
            memo_restart=(atomicLoad(&memo_prefix)/64u)*64u;
        }
        let batch_n=workgroupUniformLoad(&memo_masks[8158u]);
        let rebuild=workgroupUniformLoad(&memo_masks[8159u]);
        if (rebuild!=0u && batch_n>=128u) {
            let restart=workgroupUniformLoad(&memo_restart);
            // The entire prefix has identical identities, generations and input
            // masks/counts. Rebuild its monotone mask union and count maxima in
            // parallel, preserving every cached color/local index exactly.
            for (var i=lane;i<restart;i+=64u) {
                let base=memo_edge_base()+6u*i;
                let slot=atomicLoad(&query[base]);
                let a=atomicLoad(&query[base+1u]);let b=atomicLoad(&query[base+2u]);
                let col=atomicLoad(&query[base+3u]);let local=atomicLoad(&query[base+4u]);
                memo_publish(slot,vec2<u32>(col,local));
                if (col<DYNAMIC_COLOR_COUNT) {
                    atomicOr(&atom[ATOM_JACOBI+a],1u<<col);
                    atomicOr(&atom[ATOM_JACOBI+b],1u<<col);
                }
                atomicMax(&atom[atom_graph_color()+col],local+1u);
            }
            storageBarrier();
            for (var b=lane;b<params.body_count;b+=64u) {memo_masks[b]=atomicLoad(&atom[ATOM_JACOBI+b]);}
            if (lane<24u) {memo_counts[lane]=atomicLoad(&atom[atom_graph_color()+lane]);}
            workgroupBarrier();
            for (var start=restart;start<batch_n;start+=64u) {
                let dst=params.body_count+3u*lane;
                if (start+lane<batch_n) {
                    let slot=scratch[SCR_ACTIVE_CONTACT+scratch[SCR_NEXT_OCCUPIED+start+lane]];
                    let h=contacts[slot];
                    memo_masks[dst]=slot;memo_masks[dst+1u]=h.a;memo_masks[dst+2u]=h.b;
                }
                workgroupBarrier();
                if (lane==0u) {
                    for (var i=0u;i<min(64u,batch_n-start);i++) {
                        let src=params.body_count+3u*i;
                        let choice=memo_choose(memo_masks[src+1u],memo_masks[src+2u]);
                        memo_masks[src+1u]=choice.x;memo_masks[src+2u]=choice.y;
                    }
                }
                workgroupBarrier();
                if (start+lane<batch_n) {
                    memo_publish(memo_masks[dst],vec2<u32>(memo_masks[dst+1u],memo_masks[dst+2u]));
                }
                workgroupBarrier();
            }
        } else if (changed && lane==0u) {memo_serial(n);}
    } else if (changed && lane==0u) {
        memo_serial(n);
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
