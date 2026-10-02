from pathlib import Path
import json,hashlib,subprocess,os,shutil,struct
R=Path.cwd();A=R/'artifacts/production-readiness/pr04-distance-clamp';P=A/'protocol.json';assert not P.exists()
sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest()
Q=R/'benchmarks/production-readiness';contract=json.loads((Q/'pr03-contract-reconciliation-2026-10-02/qualification-contract.json').read_text());inputs={n:sha(R/n) for n in contract['current_inputs']};assert inputs==contract['current_inputs']
shader=R/'shaders/physics/solve.wgsl';s=shader.read_text();old='''            } else {
                let omega = 6.2831853 * max(jn.hertz, 1.0);
                let a1 = 2.0 * max(jn.damping, 0.25) + h * omega;''';new='''            } else {
                // Match the prepared rigid constraint's timestep hertz clamp.
                let omega = 6.2831853 * min(max(jn.hertz, 1.0), 0.25 / max(h, 1e-8));
                let a1 = 2.0 * max(jn.damping, 0.25) + h * omega;''';assert s.count(old)==1
(A/'original-solve.wgsl').write_bytes(shader.read_bytes());(A/'candidate-solve.wgsl').write_text(s.replace(old,new));candidate=dict(inputs);candidate['shaders/physics/solve.wgsl']=sha(A/'candidate-solve.wgsl')
base=json.loads((R/'artifacts/production-readiness/pr03-distance-baseline/protocol.json').read_text());links=json.loads((R/'artifacts/production-readiness/pr03-baseline-cpu-link-repair/receipt.json').read_text());builds=[];linkrecipes=[]
for b in ['ordinary','native']:
 cmd=['cargo','build','--release','--locked','--lib','--features','external-c-shim,replay-diagnostics','--target-dir','target/samples','--message-format=json'] if b=='ordinary' else ['bash','scripts/build-native-cache.sh','build','--release','--locked','--lib','--features','external-c-shim,replay-diagnostics','--message-format=json']
 builds.append({'backend':b,'command':cmd})
 row=next(c for c in links['commands'] if c['output'].endswith('/'+b+'-both/distance-original'));cmd=list(row['command']);oldlib=base['cases'][0 if b=='ordinary' else 1]['fixture']['linked_inputs'];libs=[v for v in cmd if v.endswith('.a')];gpu=[v for v in libs if 'frozen.a' in v];assert len(gpu)==1;cmd[cmd.index(gpu[0])]=str(A/(b+'-candidate.a'));cmd[-1]=str(A/(b+'-distance'));linkrecipes.append({'backend':b,'command':cmd,'reused_inputs':{v:sha(v) for v in libs if v not in gpu},'source':cmd[6],'source_sha256':sha(cmd[6])})
fixturepath=Path(linkrecipes[0]['source']);portable=Q/'pr03-baseline-fixtures-2026-10-02/raw/first-build/distance-original.cpp';assert sha(fixturepath)==sha(portable)
# Explicit environment overrides only: do not publish inherited secrets or write
# any trace/cache file in a closed campaign directory.
cases=[]
for order in [0,1]:
 for b in ['ordinary','native']:
  orig=next(c for c in base['cases'] if c['name']==b);env={k:v for k,v in orig['environment'].items() if k.startswith(('GPU_PHYSICS_','VK_','WGPU_')) or k=='RUST_LOG'}
  env['GPU_PHYSICS_LIVE_CONTACT_ORDER']=str(order);env['GPU_PHYSICS_PIPELINE_CACHE_PATH']=str(A/(b+'-order'+str(order)+'-pipeline.bin'));env.pop('GPU_PHYSICS_TEST_TRACE',None)
  cases.append({'name':b+'-order'+str(order),'backend':b,'ordering':order,'command':[str(A/(b+'-distance'))],'environment_overrides':env})
# Float32 reconstruction: body origin coincides with COM, axis is down, initial
# distance error and cached impulse are zero, so biased pass leaves gravity*a3.
f=lambda v:struct.unpack('f',struct.pack('f',v))[0]
audit=[]
for hz in [60.,15.]:
 h=f(1/60);omega=f(f(2*f(3.141592653589793))*f(hz));a1=f(f(4)+f(h*omega));a2=f(f(h*omega)*a1);a3=f(1/f(1+a2));mass=f(a2*a3);v=f(-10*h);remaining=f(v-f(mass*v));y=f(-1+f(h*remaining));audit.append({'hertz':hz,'h':h,'bias_mass_scale':mass,'impulse_scale':a3,'predicted_initial_y':y})
assert abs(audit[0]['predicted_initial_y']-(-1.00004232))<1e-7 and abs(audit[1]['predicted_initial_y']-(-1.00028491))<1e-7
(A/'algebra.json').write_text(json.dumps({'scope':'host float32 reconstruction of recorded first biased-pass displacement; not GPU/full physical acceptance','cases':audit},indent=2)+'\n')
refs={str(p):sha(p) for p in [Q/'pr03-contract-reconciliation-2026-10-02/qualification-contract.json',R/'artifacts/production-readiness/pr03-distance-baseline/protocol.json',R/'artifacts/production-readiness/pr03-distance-baseline/receipt.json',R/'artifacts/production-readiness/pr03-baseline-cpu-link-repair/receipt.json',R/'scripts/build-native-cache.sh',R/'scripts/prepare-native-backend.py',R.parents[1]/'box3d/src/joint.c',R.parents[1]/'box3d/src/distance_joint.c',R.parents[1]/'box3d/src/solver.h',A/'algebra.json']}
p={'purpose':'PR04 single rigid-distance timestep-clamp candidate: isolate recorded first-step discrepancy. No tolerance/default/scene/scheduling change. Source staged only for artifact build and restored before execution; no production commit. Full original comparison still decides candidate sufficiency.','invocation_revision_context_only':subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip(),'budget':{'Rust_library_builds':2,'C_fixture_links':2,'engine_processes_maximum':4,'solver_candidates':1,'retries':0,'headline_timing':0,'test_builds':0},'original_inputs':inputs,'candidate_inputs':candidate,'candidate_shader_sha256':sha(A/'candidate-solve.wgsl'),'references':refs,'builds':builds,'links':linkrecipes,'cases':cases,'build_watchdog_seconds':1800,'link_watchdog_seconds':180,'engine_watchdog_seconds':240,'criteria':'Every original four sphere-ground/distance ×1/4substep branch,240steps each,13finite lanes absolute1e-5, original force/warmstart transitions. Bothorder0 cells execute once even if assertion fails; onlybothpass permits the two previouslyunlaunched order1 cells. No --timing. Incidental fixture clocks are not benchmarkdata.','stop_retain':'Retain every build/engine failure. Stop on build/link/infrastructure failure, restore shader in finally; no retries. Failed physical assertion does not pass. If either order0 fails, do not run order1. Passing allfour onlypermits a separatelyfrozen regression/recording qualification before any productioncommit.','actual_adapter':'Require NVIDIA GeForce RTX4070SUPER / Vulkan banner in every GPU process; driver610.57.04'}
P.write_text(json.dumps(p,indent=2)+'\n');print('Frozen protocol',sha(P));print('Algebra:',audit)
