struct Params { bodies:u32, edges:u32, capacity:u32, mode:u32 }
@group(0) @binding(0) var<uniform> p:Params;
@group(0) @binding(1) var<storage,read> source:array<u32>;
@group(0) @binding(2) var<storage,read_write> result:array<u32>;
var<workgroup> masks:array<u32,6000>;
var<workgroup> owners:array<atomic<u32>,6000>;
var<workgroup> counts:array<atomic<u32>,24>;
var<workgroup> left:atomic<u32>;
var<workgroup> uniform_left:u32;
fn endpoint(i:u32,which:u32)->u32{return source[p.bodies+24u+4u*i+which];}
fn hash(value:u32)->u32 {var x=value;x^=x>>16u;x*=0x7feb352du;x^=x>>15u;x*=0x846ca68bu;return x^(x>>16u);}
fn priority(i:u32)->u32 {return ((hash(endpoint(i,2u))&65535u)<<16u)|i;}
fn color_at(i:u32)->u32 {return result[28u+2u*i];}
fn assign(i:u32) {
    let a=endpoint(i,0u);let b=endpoint(i,1u);
    let free=(~(masks[a]|masks[b]))&0xfffffu;
    var col=23u;
    if(free!=0u){col=firstTrailingBit(free);masks[a]|=1u<<col;masks[b]|=1u<<col;}
    result[28u+2u*i]=col;
}
fn emit(i:u32) {
    let col=color_at(i);let local=atomicAdd(&counts[col],1u);
    result[28u+2u*i+1u]=local;
    result[28u+2u*p.edges+col*p.capacity+local]=i;
}
@compute @workgroup_size(256)
fn schedule(@builtin(local_invocation_index) lane:u32) {
    for(var b=lane;b<p.bodies;b+=256u){masks[b]=source[b];}
    for(var i=lane;i<p.edges;i+=256u){result[28u+2u*i]=0xffffffffu;}
    if(lane<24u){atomicStore(&counts[lane],source[p.bodies+lane]);}
    if(lane==0u){result[0]=0u;result[1]=0u;}
    workgroupBarrier();storageBarrier();
    if(p.mode==0u){
        if(lane==0u){for(var i=0u;i<p.edges;i++){assign(i);emit(i);}}
    }else{
        if(p.edges>0u){
            for(var round=0u;round<64u;round++){
                if(lane==0u){atomicStore(&left,0u);}
                for(var b=lane;b<p.bodies;b+=256u){atomicStore(&owners[b],0xffffffffu);}
                workgroupBarrier();
                for(var i=lane;i<p.edges;i+=256u){
                    if(color_at(i)==0xffffffffu){
                        let rank=priority(i);
                        atomicMin(&owners[endpoint(i,0u)],rank);atomicMin(&owners[endpoint(i,1u)],rank);
                    }
                }
                workgroupBarrier();
                for(var i=lane;i<p.edges;i+=256u){
                    if(color_at(i)==0xffffffffu){
                        let rank=priority(i);
                        if(atomicLoad(&owners[endpoint(i,0u)])==rank && atomicLoad(&owners[endpoint(i,1u)])==rank){assign(i);}
                        else {atomicAdd(&left,1u);}
                    }
                }
                workgroupBarrier();storageBarrier();
                if(lane==0u){uniform_left=atomicLoad(&left);result[0]=round+1u;}
                workgroupBarrier();
                if(workgroupUniformLoad(&uniform_left)==0u){break;}
            }
        }
        if(lane==0u){
            for(var i=0u;i<p.edges;i++){if(color_at(i)==0xffffffffu){assign(i);result[1]++;}}
        }
        workgroupBarrier();storageBarrier();
        for(var i=lane;i<p.edges;i+=256u){if(color_at(i)<23u){emit(i);}}
        workgroupBarrier();storageBarrier();
        // Overflow is always emitted in canonical pair order, after its static prefix.
        if(lane==0u){for(var i=0u;i<p.edges;i++){if(color_at(i)==23u){emit(i);}}}
    }
    workgroupBarrier();storageBarrier();
    if(lane<24u){result[4u+lane]=atomicLoad(&counts[lane]);}
}
