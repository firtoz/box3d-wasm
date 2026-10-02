from pathlib import Path
import json,hashlib,subprocess,shlex,shutil
R=Path('/home/firtoz/work/2026/box3d-wasm');E=R/'experiments/gpu-physics';A=E/'artifacts/production-readiness/pr04-drag-ccd-boundary';A.mkdir()
sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest()
producer=E/'artifacts/production-readiness/pr04-distance-clamp/receipt.json';pr=json.loads(producer.read_text())
consumer=E/'artifacts/production-readiness/pr04-distance-viewers/ordinary-both/receipt.json';cr=json.loads(consumer.read_text())
cp=E/'artifacts/production-readiness/pr04-distance-viewers/protocol.json';headers=json.loads(cp.read_text())['CPP_consumer_inputs']
for n,h in headers.items():assert sha(E/n)==h,n
for u in cr['compiled_units']:assert sha(u['file'])==u['source_sha256'] and sha(u['object'])==u['object_sha256']
current={n:sha(E/n) for n in pr['compiled_inputs_after']};assert [n for n,h in current.items() if h!=pr['compiled_inputs_after'][n]]==['src/gpu_invariants.rs']
base=E/'artifacts/production-readiness/pr02-world-lifetime-viewer-builds/ordinary-both/cmake';original=base/'libbox3d_cpu_original.a';syms=base/'libbox3d_cpu.syms';assert original.exists() and syms.exists()
solver=(R/'box3d/src/solver.c').read_text();fixture=(E/'c_abi/both_drag_test.cpp').read_text()
helper='''
extern int drag_trace_frame;
static bool drag_boundary_selected(int bodyId) {
    return bodyId == 1 && drag_trace_frame >= 220 && drag_trace_frame <= 232;
}
'''
anchor='_Static_assert( B3_RESTITUTION_ITERATIONS >= 1';assert solver.count(anchor)==1;solver=solver.replace(anchor,helper+'\n'+anchor)
anchor='\t\t\tfloat maxMotion = b3MaxFloat( maxDeltaPosition, maxVelocity * timeStep );';assert solver.count(anchor)==1
solver=solver.replace(anchor,anchor+'''
            if (drag_boundary_selected(sim->bodyId)) {
                fprintf(stderr,"CPUclass frame=%d body=%d motion=%.9g delta=%.9g velocity_dt=%.9g threshold=%.9g fast=%d p=%.9g,%.9g,%.9g q=%.9g,%.9g,%.9g,%.9g v=%.9g,%.9g,%.9g w=%.9g,%.9g,%.9g\\n",
                    drag_trace_frame,sim->bodyId,maxMotion,maxDeltaPosition,maxVelocity*timeStep,safetyFactor*sim->minExtent,
                    body->type==b3_dynamicBody && enableContinuous && maxMotion>safetyFactor*sim->minExtent,
                    sim->transform.p.x,sim->transform.p.y,sim->transform.p.z,sim->transform.q.v.x,sim->transform.q.v.y,sim->transform.q.v.z,sim->transform.q.s,
                    v.x,v.y,v.z,w.x,w.y,w.z);
            }
''')
anchor='\tif ( context.fraction < 1.0f )';assert solver.count(anchor)==1
solver=solver.replace(anchor,'''
    if (drag_boundary_selected(fastBodySim->bodyId)) {
        fprintf(stderr,"CPUtoi frame=%d body=%d fraction=%.9g start=%.9g,%.9g,%.9g end=%.9g,%.9g,%.9g\\n",
            drag_trace_frame,fastBodySim->bodyId,context.fraction,
            fastBodySim->center0.x,fastBodySim->center0.y,fastBodySim->center0.z,
            fastBodySim->center.x,fastBodySim->center.y,fastBodySim->center.z);
    }
'''+anchor)
(A/'solver.c').write_text(solver)
assert fixture.count('#include <algorithm>')==1;fixture=fixture.replace('#include <algorithm>','extern "C" { int drag_trace_frame = 0; }\n#include <algorithm>')
anchor='    b3World_Step(w, 1.f / 60, 4);';assert fixture.count(anchor)==1;fixture=fixture.replace(anchor,'    drag_trace_frame = frame;\n'+anchor)
anchor='    frame++;';assert fixture.count(anchor)==1
fixture=fixture.replace(anchor,anchor+'''
    if (frame == 240) {
      if (trace) { std::fclose(trace); trace = nullptr; }
      b3DestroyWorld(w);
      std::printf("DIAGNOSTIC completed 240-step original drag prefix; not full dragging acceptance\\n");
      std::exit(0);
    }
''');(A/'drag-prefix.cpp').write_text(fixture)
u=next(x for x in cr['compiled_units'] if x['file'].endswith('/src/solver.c'));cmd=shlex.split(u['command']);cmd[cmd.index('-o')+1]=str(A/'solver.c.o');cmd[cmd.index('-c')+1]=str(A/'solver.c');cmd += ['-MD','-MF',str(A/'solver-dependencies.d')]
links=[];cases=[];oldp=json.loads((E/'artifacts/production-readiness/pr04-distance-focused-test/protocol.json').read_text())
for name in ['ordinary','native']:
 old=next(x for x in pr['links'] if x['name']==name+'-link');link=old['command'].copy();link[6]=str(A/'drag-prefix.cpp');assert old['command'][6].endswith('/distance-original.cpp')
 oldcpu=next(x for x in link if x.endswith('/libbox3d_cpu.a'));link[link.index(oldcpu)]=str(A/'libbox3d_cpu_observer.a');link[link.index('-o')+1]=str(A/(name+'-drag-prefix'))
 reused={n:h for n,h in old['reused_inputs'].items() if n!=oldcpu};library=pr['libraries'][name];reused[library['path']]=library['sha256']
 for n,h in reused.items():assert sha(n)==h,n
 links.append({'name':name,'command':link,'reused_inputs':reused,'Rust_library_sha256':library['sha256']})
 env=next(x['environment_overrides'].copy() for x in oldp['cases'] if x['backend']==name and x['ordering']==0);env['GPU_PHYSICS_PIPELINE_CACHE_DIR']=str(A/(name+'-pipelines'));env['GPU_PHYSICS_TRACE_BODY']='2';env['BOTH_DRAG_TRACE']=str(A/name/'comparison.txt');env['BOTH_DRAG_TRACE_BODY']='-1'
 prior=E/'benchmarks/production-readiness/pr03-drag-order-native-timeout-2026-10-02/raw'/f'{name}-drag-ground/comparison.txt.gz'
 cases.append({'name':name,'command':[str(A/(name+'-drag-prefix')),'--ground'],'environment_overrides':env,'baseline_trace':str(prior),'baseline_trace_sha256':sha(prior)})
inputs={str(p):sha(p) for p in [A/'solver.c',A/'drag-prefix.cpp',producer,consumer,cp,original,syms,R/'box3d/src/solver.c',E/'c_abi/both_drag_test.cpp']}
members=subprocess.check_output(['ar','t',str(original)],text=True).splitlines();assert members.count('solver.c.o')==1
hashes={n:hashlib.sha256(subprocess.check_output(['ar','p',str(original),n])).hexdigest() for n in members}
p={'purpose':'Read-only CPU CCD boundaries plus existing GPU CCD trace at first loaded-drag impact; artifact sources only, no solver candidate.','budget':{'CPU_C_translation_unit_builds':1,'CPP_fixture_compilations_and_links':2,'archive_copy_replace_prefix_operations':3,'fresh_diagnostic_processes':2,'completed_steps_per_process':240,'Rust_builds':0,'solver_candidates':0,'retries':0,'headline_timing':0},'compiled_Rust_inputs_current':current,'prior_Rust_producer':str(producer),'prior_Rust_producer_sha256':sha(producer),'current_exception':'Only cfg(test) gpu_invariants.rs differs from retained production provider; distance solver clamp is identical and no distance joint exists in drag fixture.','CPP_consumer_inputs':headers,'consumer_receipt':str(consumer),'consumer_receipt_sha256':sha(consumer),'compiled_units':cr['compiled_units'],'inputs':inputs,'CPU_archive':str(original),'CPU_archive_members':hashes,'symbol_map':str(syms),'compile_command':cmd,'compile_cwd':u['directory'],'links':links,'cases':cases,'watchdog_seconds':300,'criteria':'Both 240step prefix streams must match every original B/F/M/P line exactly (printed9sig body/contact records) before attribution. ActualNVIDIA Vulkan and completedprefix required. Observeronly diagnosis never full3060 physical acceptance.','stop_retain':'Stop on any build/infrastructure/adapter/watchdog/observer-neutrality failure; retain every result and all unlaunchedcells. No repeats or replacements. Check source/object/archive hashes beforeandafter; do not alter original C/Rust/archive/binaries. No scene/default/tolerance change.','invocation_revision_context_only':subprocess.check_output(['git','rev-parse','HEAD'],text=True,cwd=R).strip()}
(A/'protocol.json').write_text(json.dumps(p,indent=2)+'\n');shutil.copyfile(__file__,A/'prepare.py');print('Frozen',sha(A/'protocol.json'))
