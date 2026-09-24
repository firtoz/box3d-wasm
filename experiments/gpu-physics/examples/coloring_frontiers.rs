//! Offline schedule analysis of actual active contact graphs; no runtime changes.
use gpu_physics::{api::*,sim::GpuDevice,types::*};
struct Report {widths:Vec<usize>,changed:usize,overflow:usize}
impl Report {fn json(&self)->String {format!("{{\"rounds\":{},\"widths\":{:?},\"changed_colors\":{},\"overflow\":{}}}",self.widths.len(),self.widths,self.changed,self.overflow)}}
#[derive(Clone,Copy)]
struct Edge {a:usize,b:usize,color:u32,key:u64}
fn greedy(edges:&[Edge],initial:&[u32])->Vec<u32>{
    let mut masks=initial.to_vec();
    edges.iter().map(|e|{let available=(!(masks[e.a]|masks[e.b]))&((1<<20)-1);
        if available==0 {23} else {let c=available.trailing_zeros();masks[e.a]|=1<<c;masks[e.b]|=1<<c;c}}).collect()
}
fn hash(mut x:u32)->u32{x^=x>>16;x=x.wrapping_mul(0x7feb352d);x^=x>>15;x=x.wrapping_mul(0x846ca68b);x^(x>>16)}
fn frontier(edges:&[Edge],initial:&[u32],random:bool)->Report{
    let mut masks=initial.to_vec();let mut colors=vec![u32::MAX;edges.len()];let mut widths=Vec::new();
    let priorities:Vec<(u32,usize)>=(0..edges.len()).map(|i|(if random {hash(edges[i].key)} else {i as u32},i)).collect();
    loop {
        let mut owner=vec![(u32::MAX,usize::MAX);initial.len()];
        for (i,e) in edges.iter().enumerate(){if colors[i]==u32::MAX{owner[e.a]=owner[e.a].min(priorities[i]);owner[e.b]=owner[e.b].min(priorities[i]);}}
        let ready:Vec<usize>=edges.iter().enumerate().filter_map(|(i,e)|(colors[i]==u32::MAX&&owner[e.a]==priorities[i]&&owner[e.b]==priorities[i]).then_some(i)).collect();
        if ready.is_empty(){assert!(colors.iter().all(|&c|c!=u32::MAX));break;}
        widths.push(ready.len());
        for i in ready {let e=edges[i];let available=(!(masks[e.a]|masks[e.b]))&((1<<20)-1);let col=if available==0 {23}else{available.trailing_zeros()};colors[i]=col;if col<20{masks[e.a]|=1<<col;masks[e.b]|=1<<col;}}
    }
    // Verify every edge, reserved color, and writable endpoint independently.
    let mut seen=initial.to_vec();
    for (e,&col) in edges.iter().zip(&colors){assert!(col<20||col==23);if col<20 {assert_eq!((seen[e.a]|seen[e.b])&(1<<col),0);seen[e.a]|=1<<col;seen[e.b]|=1<<col;}}
    let reference=greedy(edges,initial);if !random{assert_eq!(colors,reference);}
    Report{changed:colors.iter().zip(reference).filter(|(a,b)|**a!=*b).count(),overflow:colors.iter().filter(|&&c|c==23).count(),widths}
}
fn main(){
    let out=std::env::args().nth(1).expect("output.json");
    let gpu=pollster::block_on(GpuDevice::new(None)).unwrap();let mut results=Vec::new();
    for scene in [DemoScene::MixedStacks,DemoScene::Dominoes]{
        let world=gpu_physics::scenes::build_demo_world(gpu.clone(),&DemoConfig{scene,body_count:600,body_count_explicit:false,contacts:true,jacobi:false});
        b3_world_enable_sleeping(world,false);
        for step in 1..=320 {
            b3_world_step_gpu(world,1.0/60.0,4);
            if ![1,30,120,200,320].contains(&step){continue;}
            let bodies=pollster::block_on(b3_world_sync_from_gpu(world));
            let contacts=pollster::block_on(b3_world_sync_contacts(world));
            assert!(!pollster::block_on(b3_world_live_step_stats(world)).unwrap().capacity_loss());
            let writable=|i:usize|bodies[i].flags&(FLAG_STATIC|FLAG_KINEMATIC|FLAG_DISABLED)==0;
            let mut initial=vec![0u32;bodies.len()];let mut edges=Vec::new();
            for c in contacts.iter().filter(|c|c.a!=u32::MAX&&c.count>0&&c.manifold_link[1]==0&&c.lifecycle[1]&2!=0){
                let (a,b)=(c.a as usize,c.b as usize);assert!(a<bodies.len()&&b<bodies.len());
                match (writable(a),writable(b)){
                    (true,true)=>edges.push(Edge{a,b,color:c.color,key:c.pair_key()}),
                    (true,false)|(false,true)=>if c.color<23{initial[if writable(a){a}else{b}]|=1<<c.color},
                    _=>{}
                }
            }
            edges.sort_by_key(|e|e.key);assert!(edges.windows(2).all(|e|e[0].key<e[1].key));
            let colors=greedy(&edges,&initial);assert_eq!(colors,edges.iter().map(|e|e.color).collect::<Vec<_>>(),"captured graph must reproduce actual colors, {} step{step}",scene.slug());
            let mut batches=Vec::new();for batch in edges.chunks(64){let mut depth=vec![0u32;bodies.len()];let mut max=0;for e in batch{let d=1+depth[e.a].max(depth[e.b]);depth[e.a]=d;depth[e.b]=d;max=max.max(d);}batches.push(max);}
            let canonical=frontier(&edges,&initial,false);let hashed=frontier(&edges,&initial,true);
            let pairs:Vec<_>=edges.iter().map(|e|[e.a as u32,e.b as u32,e.key,e.color]).collect();
            let row=format!("{{\"scene\":\"{}\",\"step\":{},\"bodies\":{},\"edges\":{},\"initial_masks\":{:?},\"pairs\":{:?},\"canonical\":{},\"hashed\":{},\"batch64_depths\":{:?}}}",scene.slug(),step,bodies.len(),edges.len(),initial,pairs,canonical.json(),hashed.json(),batches);
            println!("{} step{step}: {} edges, canonical {} rounds, hashed {} rounds, batch64 {} rounds total",scene.slug(),edges.len(),canonical.widths.len(),hashed.widths.len(),batches.iter().sum::<u32>());
            results.push(row);
        }
        b3_destroy_world(world);
    }
    std::fs::write(out,format!("{{\"snapshots\":[{}],\"cpu_win_validated\":false,\"scope\":\"CPU scheduling analysis of captured GPU graphs; not GPU speed or changed physics\"}}",results.join(","))).unwrap();
}
