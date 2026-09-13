//! Isolated coloring + complete list construction probe. No live engine selection.
use wgpu::util::DeviceExt;
fn words(bytes:&[u8])->Vec<u32>{bytes.chunks_exact(4).map(|b|u32::from_le_bytes(b.try_into().unwrap())).collect()}
fn read(device:&wgpu::Device,buffer:&wgpu::Buffer)->Vec<u8>{
 let (tx,rx)=std::sync::mpsc::channel();buffer.slice(..).map_async(wgpu::MapMode::Read,move|r|tx.send(r).unwrap());device.poll(wgpu::PollType::Wait).unwrap();rx.recv().unwrap().unwrap();let v=buffer.slice(..).get_mapped_range().to_vec();buffer.unmap();v
}
fn main(){env_logger::init();pollster::block_on(run());}
async fn run(){
 let args:Vec<_>=std::env::args().collect();assert_eq!(args.len(),3,"input.bin output.json");
 let input=words(&std::fs::read(&args[1]).unwrap());let(b,n,cap)=(input[0] as usize,input[1] as usize,input[2] as usize);assert!(b>0&&b<=6000&&n<=65536);
 let payload_end=5+b+24+4*n;assert_eq!(input.len(),payload_end+2*n);
 let instance=wgpu::Instance::new(&wgpu::InstanceDescriptor{backends:wgpu::Backends::VULKAN,flags:wgpu::InstanceFlags::default().with_env(),..Default::default()});
 let(adapter,_)=gpu_physics::adapter::pick_adapter(&instance,None).await.unwrap();let supported=adapter.limits();assert!(supported.max_compute_workgroup_storage_size>=49152);
 let(device,queue)=adapter.request_device(&wgpu::DeviceDescriptor{label:Some("coloring-probe"),required_features:wgpu::Features::TIMESTAMP_QUERY,required_limits:wgpu::Limits{max_compute_workgroup_storage_size:49152,..Default::default()},memory_hints:wgpu::MemoryHints::MemoryUsage,trace:wgpu::Trace::Off}).await.unwrap();
 device.on_uncaptured_error(Box::new(|e|panic!("{e}")));
 let shader=device.create_shader_module(wgpu::ShaderModuleDescriptor{label:Some("parallel-color-probe"),source:wgpu::ShaderSource::Wgsl(include_str!("parallel_coloring.wgsl").into())});
 let pipeline=device.create_compute_pipeline(&wgpu::ComputePipelineDescriptor{label:Some("coloring"),layout:None,module:&shader,entry_point:Some("schedule"),compilation_options:Default::default(),cache:None});
 let src=device.create_buffer_init(&wgpu::util::BufferInitDescriptor{label:Some("graph"),contents:bytemuck::cast_slice(&input[5..payload_end]),usage:wgpu::BufferUsages::STORAGE});
 let total=28+2*n+24*cap;let output=device.create_buffer_init(&wgpu::util::BufferInitDescriptor{label:Some("color lists"),contents:bytemuck::cast_slice(&vec![u32::MAX;total]),usage:wgpu::BufferUsages::STORAGE|wgpu::BufferUsages::COPY_SRC});
 let staging=device.create_buffer(&wgpu::BufferDescriptor{label:None,size:(total*4) as u64,usage:wgpu::BufferUsages::COPY_DST|wgpu::BufferUsages::MAP_READ,mapped_at_creation:false});
 let query=device.create_query_set(&wgpu::QuerySetDescriptor{label:None,ty:wgpu::QueryType::Timestamp,count:240});
 let resolve=device.create_buffer(&wgpu::BufferDescriptor{label:None,size:240*8,usage:wgpu::BufferUsages::QUERY_RESOLVE|wgpu::BufferUsages::COPY_SRC,mapped_at_creation:false});
 let times=device.create_buffer(&wgpu::BufferDescriptor{label:None,size:240*8,usage:wgpu::BufferUsages::MAP_READ|wgpu::BufferUsages::COPY_DST,mapped_at_creation:false});
 let check_only=std::env::var("COLOR_PROBE_CHECK_ONLY").as_deref()==Ok("1");let repeats=if check_only{2u32}else{120u32};
 let mut rows=Vec::new();
 for trial in 0..3 {for mode in if trial%2==0{[0u32,1]}else{[1,0]} {
  let params=device.create_buffer_init(&wgpu::util::BufferInitDescriptor{label:None,contents:bytemuck::cast_slice(&[b as u32,n as u32,cap as u32,mode]),usage:wgpu::BufferUsages::UNIFORM});
  let bg=device.create_bind_group(&wgpu::BindGroupDescriptor{label:None,layout:&pipeline.get_bind_group_layout(0),entries:&[wgpu::BindGroupEntry{binding:0,resource:params.as_entire_binding()},wgpu::BindGroupEntry{binding:1,resource:src.as_entire_binding()},wgpu::BindGroupEntry{binding:2,resource:output.as_entire_binding()}]});
  let mut encoder=device.create_command_encoder(&Default::default());
  for repeat in 0..repeats {let mut pass=encoder.begin_compute_pass(&wgpu::ComputePassDescriptor{label:Some("color and emit"),timestamp_writes:Some(wgpu::ComputePassTimestampWrites{query_set:&query,beginning_of_pass_write_index:Some(2*repeat),end_of_pass_write_index:Some(2*repeat+1)})});pass.set_pipeline(&pipeline);pass.set_bind_group(0,&bg,&[]);pass.dispatch_workgroups(1,1,1);}
  encoder.resolve_query_set(&query,0..2*repeats,&resolve,0);encoder.copy_buffer_to_buffer(&resolve,0,&times,0,240*8);encoder.copy_buffer_to_buffer(&output,0,&staging,0,(total*4) as u64);queue.submit([encoder.finish()]);
  let actual=words(&read(&device,&staging));let expected=&input[payload_end+mode as usize*n..payload_end+(mode as usize+1)*n];
  let mut masks=input[5..5+b].to_vec();let mut seen=vec![false;n];let initial=&input[5+b..5+b+24];
  for i in 0..n {let col=actual[28+2*i];assert_eq!(col,expected[i],"mode{mode}, edge{i}");assert!(col<20||col==23);let at=5+b+24+4*i;let a=input[at] as usize;let bb=input[at+1] as usize;if col<20{assert_eq!((masks[a]|masks[bb])&(1<<col),0);masks[a]|=1<<col;masks[bb]|=1<<col;}}
  for col in 0..24usize {let begin=initial[col] as usize;let end=actual[4+col] as usize;assert!(end>=begin&&end<=cap);for local in 0..begin{assert_eq!(actual[28+2*n+col*cap+local],u32::MAX,"static prefix overwritten");}let mut last=None;for local in begin..end {let i=actual[28+2*n+col*cap+local] as usize;assert!(i<n&&!seen[i]);seen[i]=true;assert_eq!(actual[28+2*i],col as u32);assert_eq!(actual[28+2*i+1],local as u32);if col==23{if let Some(old)=last{assert!(i>old);}last=Some(i);}}}
  assert!(seen.iter().all(|&x|x));if mode==1 {assert_eq!(actual[0],input[3]);assert_eq!(actual[1],input[4]);}
  let raw=read(&device,&times);let ticks:Vec<u64>=raw[..repeats as usize*16].chunks_exact(8).map(|a|u64::from_le_bytes(a.try_into().unwrap())).collect();let mut samples:Vec<f64>=ticks.chunks_exact(2).skip(if check_only{0}else{20}).map(|t|{assert!(t[1]>=t[0]);(t[1]-t[0])as f64*queue.get_timestamp_period() as f64/1e6}).collect();samples.sort_by(f64::total_cmp);
  rows.push(format!("{{\"mode\":{mode},\"trial\":{trial},\"p50_ms\":{},\"p95_ms\":{},\"rounds\":{},\"fallback\":{}}}",samples[samples.len()/2],samples[samples.len()*95/100],actual[0],actual[1]));
 }}
 std::fs::write(&args[2],format!("{{\"bodies\":{b},\"edges\":{n},\"checks\":\"pass\",\"cpu_win_validated\":false,\"trials\":[{}]}}",rows.join(","))).unwrap();println!("PASS {}: {b} bodies, {n} edges",args[1]);
}
