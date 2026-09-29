// Isolated candidate: preserve roots and physical counters; canonicalize children.
struct Config { capacity:u32, groups:u32, unused0:u32, unused1:u32 }
@group(0) @binding(0) var<uniform> cfg:Config;
@group(0) @binding(1) var<storage,read_write> hot:array<u32>;
@group(0) @binding(2) var<storage,read_write> persistent:array<u32>;
@group(0) @binding(3) var<storage,read_write> prepared:array<u32>;
@group(0) @binding(4) var<storage,read_write> staged:array<u32>;
@group(0) @binding(5) var<storage,read_write> work:array<atomic<u32>>;
@group(0) @binding(6) var<storage,read_write> query:array<atomic<u32>>;
@group(0) @binding(7) var<storage,read_write> atom:array<atomic<u32>>;
struct StepParameters { words:array<vec4<u32>,16> }
@group(0) @binding(8) var<uniform> parameters:StepParameters;
const EMPTY:u32=0xffffffffu;
fn totals()->u32{return 4u+3u*cfg.capacity;}
fn bases()->u32{return totals()+3u*cfg.groups;}
fn sources()->u32{return bases()+3u*cfg.groups;}
fn destinations()->u32{return sources()+cfg.capacity;}
fn sh(slot:u32,word:u32)->u32{return staged[slot*HOT+word];}
fn sp(slot:u32,word:u32)->u32{return staged[cfg.capacity*HOT+slot*PERSISTENT+word];}
fn fail(){
    if(atomicExchange(&work[1],1u)!=0u){return;}
    atomicOr(&query[66],2u);
    atomicAdd(&atom[4],1u);atomicAdd(&atom[10],1u);
    let step=max(parameters.words[STEP_WORD/4u][STEP_WORD%4u],1u);
    loop {
        let result=atomicCompareExchangeWeak(&atom[12],0u,step);
        if(result.exchanged || result.old_value!=0u){break;}
    }
}
fn load3(base:u32)->vec3<u32>{return vec3<u32>(atomicLoad(&work[base]),atomicLoad(&work[base+1u]),atomicLoad(&work[base+2u]));}
fn store3(base:u32,v:vec3<u32>){atomicStore(&work[base],v.x);atomicStore(&work[base+1u],v.y);atomicStore(&work[base+2u],v.z);}
var<workgroup> scan:array<vec3<u32>,256>;
fn inclusive(lid:u32,v:vec3<u32>)->vec3<u32>{
    scan[lid]=v;workgroupBarrier();
    for(var offset=1u;offset<256u;offset*=2u){
        var add=vec3<u32>(0u);if(lid>=offset){add=scan[lid-offset];}
        workgroupBarrier();scan[lid]+=add;workgroupBarrier();
    }
    return scan[lid];
}
@compute @workgroup_size(256)
fn snapshot(@builtin(global_invocation_id) gid:vec3<u32>,@builtin(local_invocation_index) lid:u32,@builtin(workgroup_id) group:vec3<u32>){
    let slot=gid.x;let high=min(atomicLoad(&query[73]),cfg.capacity);
    if(slot==0u){
        atomicStore(&work[2],high);
        if(atomicLoad(&query[73])>cfg.capacity){fail();}
    }
    if(slot<cfg.capacity && slot>=high && hot[slot*HOT]!=EMPTY){fail();}
    var counts=vec3<u32>(0u);
    if(slot<high){
        for(var i=0u;i<HOT;i++){staged[slot*HOT+i]=hot[slot*HOT+i];}
        for(var i=0u;i<PERSISTENT;i++){staged[cfg.capacity*HOT+slot*PERSISTENT+i]=persistent[slot*PERSISTENT+i];}
        for(var i=0u;i<PREPARED;i++){staged[cfg.capacity*(HOT+PERSISTENT)+slot*PREPARED+i]=prepared[slot*PREPARED+i];}
        let alive=hot[slot*HOT]!=EMPTY;
        let root=alive && hot[slot*HOT+LINK+1u]==0u;
        if(root){counts.x=max(hot[slot*HOT+LINK+2u],1u)-1u;}
        counts.y=u32(!root);counts.z=u32(alive && !root);
    }
    let prefix=inclusive(lid,counts);
    if(slot<cfg.capacity){store3(4u+3u*slot,prefix-counts);}
    if(lid==255u){store3(totals()+3u*group.x,prefix);}
}
@compute @workgroup_size(256)
fn block_bases(@builtin(local_invocation_index) lid:u32){
    let span=(cfg.groups+255u)/256u;let start=lid*span;
    var sum=vec3<u32>(0u);
    for(var i=start;i<min(start+span,cfg.groups);i++){sum+=load3(totals()+3u*i);}
    let prefix=inclusive(lid,sum);var base=prefix-sum;
    for(var i=start;i<min(start+span,cfg.groups);i++){
        store3(bases()+3u*i,base);base+=load3(totals()+3u*i);
    }
    if(lid==255u){
        atomicStore(&work[0],prefix.x);
        if(prefix.x!=prefix.z || prefix.x>prefix.y){fail();}
    }
}
@compute @workgroup_size(256)
fn map_children(@builtin(global_invocation_id) gid:vec3<u32>){
    let root=gid.x;let high=atomicLoad(&work[2]);if(root>=high){return;}
    let rank=load3(4u+3u*root)+load3(bases()+3u*(root/256u));
    if(sh(root,0u)==EMPTY || sh(root,LINK+1u)!=0u){
        atomicStore(&work[destinations()+rank.y],root);return;
    }
    let count=max(sh(root,LINK+2u),1u);
    if(count>high){fail();return;}
    var next=sh(root,LINK);
    for(var ordinal=1u;ordinal<count;ordinal++){
        if(next==0u || next>high){fail();return;}
        let child=next-1u;
        if(sh(child,0u)!=sh(root,0u) || sh(child,1u)!=sh(root,1u)
            || sh(child,LINK+1u)!=root+1u || sp(child,PAIR)!=sp(root,PAIR)
            || sp(child,PAIR+1u)!=sp(root,PAIR+1u)){fail();return;}
        atomicStore(&work[sources()+rank.x+ordinal-1u],child);
        next=sh(child,LINK);
    }
    if(next!=0u){fail();}
}
@compute @workgroup_size(256)
fn publish(@builtin(global_invocation_id) gid:vec3<u32>){
    let slot=gid.x;if(slot>=atomicLoad(&work[2]) || atomicLoad(&work[1])!=0u){return;}
    let rank=load3(4u+3u*slot)+load3(bases()+3u*(slot/256u));
    if(sh(slot,0u)!=EMPTY && sh(slot,LINK+1u)==0u){
        if(sh(slot,LINK)!=0u){hot[slot*HOT+LINK]=atomicLoad(&work[destinations()+rank.x])+1u;}
        return;
    }
    let generation=sp(slot,LIFE);
    if(rank.y<atomicLoad(&work[0])){
        let src=atomicLoad(&work[sources()+rank.y]);
        for(var i=0u;i<HOT;i++){hot[slot*HOT+i]=sh(src,i);}
        for(var i=0u;i<PERSISTENT;i++){persistent[slot*PERSISTENT+i]=sp(src,i);}
        for(var i=0u;i<PREPARED;i++){prepared[slot*PREPARED+i]=staged[cfg.capacity*(HOT+PERSISTENT)+src*PREPARED+i];}
        if(sh(src,LINK)!=0u){hot[slot*HOT+LINK]=atomicLoad(&work[destinations()+rank.y+1u])+1u;}
    }else{
        for(var i=0u;i<HOT;i++){hot[slot*HOT+i]=0u;}
        hot[slot*HOT]=EMPTY;hot[slot*HOT+1u]=EMPTY;
        for(var i=0u;i<PERSISTENT;i++){persistent[slot*PERSISTENT+i]=0u;}
        for(var i=0u;i<4u;i++){persistent[slot*PERSISTENT+PAIR+i]=EMPTY;}
        for(var i=0u;i<PREPARED;i++){prepared[slot*PREPARED+i]=0u;}
    }
    persistent[slot*PERSISTENT+LIFE]=generation;
}
