//! Focused queue-family equivalence test for the isolated backend patches.
//! Same physics, timestep, solver options and state reads on both queue families.
pub use gpu_physics::{api, mesh, render_scene};
#[allow(dead_code)]
#[path="queue_overlap.rs"]
mod setup;
use std::{io::Write,sync::atomic::{AtomicBool,Ordering}};
static ERROR:AtomicBool=AtomicBool::new(false);

fn main() {
    env_logger::Builder::from_env(env_logger::Env::default().default_filter_or("info"))
        .format(|b,r|{if r.level()==log::Level::Error{ERROR.store(true,Ordering::Relaxed);}
            writeln!(b,"{} {}: {}",r.level(),r.target(),r.args())}).init();
    let result=pollster::block_on(run());
    assert!(!ERROR.load(Ordering::Relaxed),"native validation errors");
    println!("{result}");
}
async fn run()->String {
    use gpu_physics::types::{DemoConfig,DemoScene};
    let mut rows=Vec::new();
    for scene in [DemoScene::HighResistance,DemoScene::MixedStacks,DemoScene::Dominoes] {
        let mut reference=Vec::new();let mut worst=0.0f32;let mut bodies=0;
        for mode in ["single","dedicated"] {
            let (gpu,graphics,graphics_queue,family)=setup::devices(mode).await;
            let world=gpu_physics::scenes::build_demo_world(gpu.clone(),&DemoConfig{
                body_count:600,body_count_explicit:false,contacts:true,jacobi:false,scene});
            api::b3_world_enable_sleeping(world,false);
            let ids=api::b3_world_dynamic_body_ids(world);bodies=ids.len();
            for step in 1..=120 {
                api::b3_world_step_gpu(world,1.0/60.0,4);
                api::b3_world_gpu_wait_with_mirror(world);
                assert!(!api::b3_world_physics_invalid(world),"capacity/schedule failure");
                assert_eq!(api::b3_world_physics_step(world),step);
                let state:Vec<f32>=ids.iter().flat_map(|&id|{
                    let (p,q)=api::b3_body_get_transform(id);let v=api::b3_body_get_linear_velocity(id);let w=api::b3_body_get_angular_velocity(id);
                    p.into_iter().chain(q).chain(v).chain(w)
                }).collect();
                assert!(state.iter().all(|x|x.is_finite()),"non-finite physics");
                if mode=="single" {reference.push(state);} else {
                    let old=&reference[step as usize-1];assert_eq!(state.len(),old.len());
                    for (i,(a,b)) in state.iter().zip(old).enumerate() {
                        let delta=(a-b).abs();worst=worst.max(delta);
                        assert!(delta<=1e-5,"{} step {step}, value {i}: {a} vs {b}",scene.slug());
                    }
                }
            }
            api::b3_destroy_world(world);
            gpu.device.poll(wgpu::PollType::Wait).unwrap();graphics.poll(wgpu::PollType::Wait).unwrap();
            drop(graphics_queue);eprintln!("{} {mode} family {family}: 120 steps checked",scene.slug());
        }
        rows.push(format!("{{\"scene\":\"{}\",\"dynamic_bodies\":{bodies},\"steps\":120,\"max_delta\":{worst}}}",scene.slug()));
    }
    format!("{{\"status\":\"pass\",\"native_frame_benchmark\":false,\"cases\":[{}]}}",rows.join(","))
}
