//! Bounded current-generation transport between independent GPU devices.
//! Caller recreates the ring when topology or immutable cold metadata changes.
use std::{collections::VecDeque,sync::mpsc};
use wgpu::{Buffer,Device,Queue};
struct Slot { staging:Buffer,state:Buffer,cold:Buffer,cold_ready:bool }
struct Pending { slot:usize,step:u64,submission:wgpu::SubmissionIndex,ready:mpsc::Receiver<Result<(),wgpu::BufferAsyncError>> }
pub struct Snapshot {pub step:u64,pub state:Buffer,pub cold:Buffer}
pub struct StagedRing {
    source:Device, slots:[Slot;2],pending:VecDeque<Pending>,next:usize,last_published:Option<u64>,
    state_bytes:u64,cold_bytes:u64,pub waits:u64,
}
impl StagedRing {
    pub fn new(source:&Device,destination:&Device,state_bytes:u64,cold_bytes:u64)->Self {
        assert!(state_bytes>0 && cold_bytes>0 && state_bytes%4==0 && cold_bytes%4==0);
        let size=state_bytes.checked_add(cold_bytes).expect("snapshot size overflow");
        let slots=std::array::from_fn(|_| {
            let staging=source.create_buffer(&wgpu::BufferDescriptor{label:Some("overlap staging"),size,usage:wgpu::BufferUsages::COPY_DST|wgpu::BufferUsages::MAP_READ,mapped_at_creation:false});
            let buffer=|size|destination.create_buffer(&wgpu::BufferDescriptor{label:Some("overlap draw snapshot"),size,usage:wgpu::BufferUsages::STORAGE|wgpu::BufferUsages::COPY_DST|wgpu::BufferUsages::COPY_SRC,mapped_at_creation:false});
            Slot{staging,state:buffer(state_bytes),cold:buffer(cold_bytes),cold_ready:false}
        });
        Self{source:source.clone(),slots,pending:VecDeque::new(),next:0,last_published:None,state_bytes,cold_bytes,waits:0}
    }
    pub fn publish(&mut self,queue:&Queue,state:&Buffer,cold:&Buffer,step:u64)->Result<(),String> {
        let expected=self.last_published.map_or(Some(0),|v|v.checked_add(1)).ok_or("step overflow")?;
        if step!=expected {return Err(format!("publish step {step}, expected {expected}"));}
        if self.pending.len()==2 {return Err("snapshot ring full".into());}
        if state.size()<self.state_bytes || cold.size()<self.cold_bytes {return Err("source buffer too small".into());}
        let slot=&self.slots[self.next];
        assert!(!self.pending.iter().any(|p|p.slot==self.next));
        let mut enc=self.source.create_command_encoder(&wgpu::CommandEncoderDescriptor{label:Some("publish staged snapshot")});
        enc.copy_buffer_to_buffer(state,0,&slot.staging,0,self.state_bytes);
        if !slot.cold_ready {enc.copy_buffer_to_buffer(cold,0,&slot.staging,self.state_bytes,self.cold_bytes);}
        let submission=queue.submit([enc.finish()]);
        let (tx,ready)=mpsc::channel();
        slot.staging.slice(..).map_async(wgpu::MapMode::Read,move|r|{let _=tx.send(r);});
        self.pending.push_back(Pending{slot:self.next,step,submission,ready});
        self.next^=1;self.last_published=Some(step);Ok(())
    }
    pub fn consume(&mut self,queue:&Queue,step:u64)->Result<Snapshot,String> {
        let p=self.pending.front().ok_or("no pending snapshot")?;
        if p.step!=step {return Err(format!("consume step {step}, oldest {}",p.step));}
        self.source.poll(wgpu::PollType::Poll).map_err(|e|format!("poll: {e}"))?;
        let result=match p.ready.try_recv() {
            Ok(r)=>r,
            Err(mpsc::TryRecvError::Empty)=>{
                self.waits+=1;
                // Never wait for the newer physics/copy submission.
                self.source.poll(wgpu::PollType::WaitForSubmissionIndex(p.submission.clone())).map_err(|e|format!("wait: {e}"))?;
                p.ready.recv().map_err(|e|format!("map callback: {e}"))?
            },
            Err(e)=>return Err(format!("map callback: {e}")),
        };
        result.map_err(|e|format!("map: {e}"))?;
        let p=self.pending.pop_front().unwrap();let slot=&mut self.slots[p.slot];
        {
            let bytes=slot.staging.slice(..).get_mapped_range();
            queue.write_buffer(&slot.state,0,&bytes[..self.state_bytes as usize]);
            if !slot.cold_ready {queue.write_buffer(&slot.cold,0,&bytes[self.state_bytes as usize..]);}
        }
        slot.staging.unmap();slot.cold_ready=true;
        Ok(Snapshot{step:p.step,state:slot.state.clone(),cold:slot.cold.clone()})
    }
    pub fn pending(&self)->usize {self.pending.len()}
}

#[cfg(all(test, feature = "native-command-cache"))] mod tests {
    use super::*;
    #[test] fn cross_adapter_ring_preserves_generations_and_bytes() {
        pollster::block_on(async {
            let instance=crate::sim::GpuDevice::instance_new();
            let adapters=instance.enumerate_adapters(wgpu::Backends::VULKAN);
            let source=adapters.iter().find(|a|a.get_info().vendor==0x10de).expect("NVIDIA source");
            let destination=adapters.iter().find(|a|a.get_info().vendor==0x1002).expect("AMD destination");
            let (sd,sq)=source.request_device(&wgpu::DeviceDescriptor::default()).await.unwrap();
            let (dd,dq)=destination.request_device(&wgpu::DeviceDescriptor::default()).await.unwrap();
            sd.push_error_scope(wgpu::ErrorFilter::Validation);dd.push_error_scope(wgpu::ErrorFilter::Validation);
            let create=|device:&Device,size,usage|device.create_buffer(&wgpu::BufferDescriptor{label:Some("ring test"),size,usage,mapped_at_creation:false});
            let hot=create(&sd,64,wgpu::BufferUsages::COPY_SRC|wgpu::BufferUsages::COPY_DST);
            let cold=create(&sd,32,wgpu::BufferUsages::COPY_SRC|wgpu::BufferUsages::COPY_DST);
            let cold_bytes=[0xabcddcbau32;8];sq.write_buffer(&cold,0,bytemuck::cast_slice(&cold_bytes));
            let mut ring=StagedRing::new(&sd,&dd,64,32);
            let upload=|step:u32| {let words=std::array::from_fn::<_,16,_>(|i|step*100+i as u32);sq.write_buffer(&hot,0,bytemuck::cast_slice(&words));};
            let verify=|snapshot:Snapshot,expected_cold:&[u32]| {
                let out=create(&dd,96,wgpu::BufferUsages::COPY_DST|wgpu::BufferUsages::MAP_READ);
                let mut enc=dd.create_command_encoder(&wgpu::CommandEncoderDescriptor::default());
                enc.copy_buffer_to_buffer(&snapshot.state,0,&out,0,64);enc.copy_buffer_to_buffer(&snapshot.cold,0,&out,64,32);
                let done=dq.submit([enc.finish()]);let (tx,rx)=mpsc::channel();out.slice(..).map_async(wgpu::MapMode::Read,move|r|tx.send(r).unwrap());
                dd.poll(wgpu::PollType::WaitForSubmissionIndex(done)).unwrap();rx.recv().unwrap().unwrap();
                {let bytes=out.slice(..).get_mapped_range();let words:&[u32]=bytemuck::cast_slice(&bytes);
                 for i in 0..16 {assert_eq!(words[i],snapshot.step as u32*100+i as u32);}assert_eq!(&words[16..],expected_cold);}
                out.unmap();
            };
            assert!(ring.publish(&sq,&hot,&cold,1).is_err());
            upload(0);ring.publish(&sq,&hot,&cold,0).unwrap();
            for step in 1..=64u64 {
                upload(step as u32);ring.publish(&sq,&hot,&cold,step).unwrap();
                assert_eq!(ring.pending(),2);assert!(ring.publish(&sq,&hot,&cold,step+1).is_err());
                assert!(ring.consume(&dq,step).is_err());
                let previous=ring.consume(&dq,step-1).unwrap();verify(previous,&cold_bytes);
                assert_eq!(ring.pending(),1);
            }
            verify(ring.consume(&dq,64).unwrap(),&cold_bytes);assert_eq!(ring.pending(),0);assert!(ring.consume(&dq,64).is_err());
            // A new generation resets sequence and metadata, even at identical sizes.
            drop(ring);let mut ring=StagedRing::new(&sd,&dd,64,32);upload(0);ring.publish(&sq,&hot,&cold,0).unwrap();
            // Destroy with an outstanding mapped snapshot, then change metadata.
            drop(ring);let new_cold=[0x12345678u32;8];sq.write_buffer(&cold,0,bytemuck::cast_slice(&new_cold));
            let mut ring=StagedRing::new(&sd,&dd,64,32);upload(0);ring.publish(&sq,&hot,&cold,0).unwrap();verify(ring.consume(&dq,0).unwrap(),&new_cold);
            assert!(sd.pop_error_scope().await.is_none());assert!(dd.pop_error_scope().await.is_none());
        });
    }
}
