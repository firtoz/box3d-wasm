from pathlib import Path
import json,hashlib,subprocess
R=Path.cwd();A=R/'artifacts/production-readiness/pr03-sleeper-fixture';sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest();source=R/'src/gpu_invariants.rs';original=source.read_text();engine=json.loads((R/'artifacts/production-readiness/pr02-world-lifetime-artifact-repair/build-receipt.json').read_text());inputs=engine['inputs_after'];assert all(sha(R/n)==h for n,h in inputs.items());assert not(A/'protocol.json').exists()
start='''    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    let world = b3_create_world(gpu, &b3_default_world_def());
    create_high_resistance(world);
    b3_world_enable_sleeping(world, true);'''
replacement='''    let gpu = pollster::block_on(GpuDevice::new(None)).expect("gpu");
    // Reuse a world slot so the fixture cannot accidentally rely on generation 1.
    let prior = b3_create_world(gpu.clone(), &b3_default_world_def());
    create_high_resistance(prior);
    let stale = crate::api::b3_world_dynamic_body_ids(prior).into_iter()
        .find(|body| body.index1 == 7).expect("prior capsule");
    b3_destroy_world(prior);
    let world = b3_create_world(gpu, &b3_default_world_def());
    create_high_resistance(world);
    let sleeper = crate::api::b3_world_dynamic_body_ids(world).into_iter()
        .find(|body| body.index1 == 7).expect("live sleeper capsule");
    assert_eq!(world.index1, prior.index1, "exercise world-slot reuse");
    assert!(sleeper.generation > stale.generation, "reused child epoch must advance");
    assert!(crate::api::b3_body_is_valid(sleeper), "fixture needs a valid live body");
    assert!(!crate::api::b3_body_is_valid(stale), "retired capsule must remain invalid");
    b3_world_enable_sleeping(world, true);'''
old='''    let sleeper = crate::api::BodyId {
        index1: 7,
        world0: world.index1,
        generation: 1,
    };
    assert!(!b3_body_is_awake(sleeper), "fixture must be asleep before testing wakeup");
    b3_body_set_linear_velocity(sleeper, [0.0, 4.0, 0.0]);'''
new='''    assert!(crate::api::b3_body_is_valid(sleeper), "sleeper must still be valid after settling");
    assert!(!b3_body_is_awake(sleeper), "fixture must be asleep before testing wakeup");
    let settled_velocity = b3_body_get_linear_velocity(sleeper);
    b3_body_set_linear_velocity(stale, [0.0, 4.0, 0.0]);
    assert_eq!(b3_body_get_linear_velocity(sleeper), settled_velocity, "stale setter must not change live velocity");
    assert!(!b3_body_is_awake(sleeper), "stale setter must not wake live capsule");
    eprintln!("sleeper fixture: stale={:?} live={:?} valid=true asleep=true", stale, sleeper);
    b3_body_set_linear_velocity(sleeper, [0.0, 4.0, 0.0]);'''
a=original.index('fn high_resistance_sleeper_wakes_on_velocity()');b=original.index('\nfn overlapping_compound_world',a);body=original[a:b];assert body.count(start)==body.count(old)==1;candidate=original[:a]+body.replace(start,replacement).replace(old,new)+original[b:];(A/'original-gpu_invariants.rs').write_text(original);(A/'candidate-gpu_invariants.rs').write_text(candidate);candidate_inputs=inputs|{'src/gpu_invariants.rs':sha(A/'candidate-gpu_invariants.rs')}
recipes=[('ordinary',['cargo','test','--release','--locked','--no-run','--lib','--features','external-c-shim,replay-diagnostics','--target-dir','target/samples','--message-format=json']),('native',['bash','scripts/build-native-cache.sh','test','--release','--locked','--no-run','--lib','--features','external-c-shim,replay-diagnostics','--message-format=json'])]
baseline=json.loads((R/'artifacts/production-readiness/pr03-rust-baseline/protocol.json').read_text());cases=[]
for mode in ['0','1']:
 for backend in ['ordinary','native']:
  original_case=next(c for c in baseline['cases'] if c['cell']==backend);env=original_case['environment'].copy();env.pop('GPU_PHYSICS_STATE_TRACE',None);env['GPU_PHYSICS_LIVE_CONTACT_ORDER']=mode;env['GPU_PHYSICS_PIPELINE_CACHE_DIR']=str(A/(backend+'-pipelines'));cases.append({'name':backend+'-order'+mode,'backend':backend,'environment':env,'selector':'gpu_invariants::high_resistance_sleeper_wakes_on_velocity','settle_steps':400,'wake_steps':1,'dt':1/60,'substeps':4})
p={'purpose':'Correct stale generation1 fixture handle without changing scene/production solver or original sleep/wake assertions. Explicit prior world reuse/public live enumeration/stale setter negative control. Both original serial-suite failures remain retained.','budget':{'test_builds':2,'test_processes':4,'production_library_builds':0,'solver_candidates':0,'retries':0,'headline_timing':0},'run_after_terminal_campaign':str(R/'artifacts/production-readiness/pr03-rain-drag-remaining/receipt.json'),'original_inputs':inputs,'candidate_inputs':candidate_inputs,'test_source_cfg_gate':{'src/lib.rs_sha256':sha(R/'src/lib.rs'),'text':'#[cfg(all(test, not(target_arch = "wasm32")))]\nmod gpu_invariants;'},'recipes':recipes,'cases':cases,'build_watchdog_seconds':1800,'test_watchdog_seconds':240,'stop_rule':'No repeats;stop/retain input/build/adapter/timeout/capture failures. Retain assertion failures and run remaining declared configurations, no criterion change. Build failure restores original test source. Fixed fixture may reveal an unresolved actual physics failure; do not waive it.','source_milestone':subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip(),'engine_producer_sha256':sha(R/'artifacts/production-readiness/pr02-world-lifetime-artifact-repair/build-receipt.json'),'libraries':engine['libraries'],'ordinary_library':{'path':str(R/'artifacts/production-readiness/pr02-world-lifetime-fix/ordinary-frozen.a'),'sha256':'aba7a834443ba7174d92bcf39217929ce9af83adc28ea9c244943b5e72eb6ec6'}};(A/'protocol.json').write_text(json.dumps(p,indent=2)+'\n');print('Frozen2testbuilds/4fresh tests; no source edit/execution yet;SHA',sha(A/'protocol.json'))
