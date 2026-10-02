#!/usr/bin/env python3
"""Verify clamp attribution and original-limit candidate comparisons offline."""
from pathlib import Path
import json,hashlib,tarfile,math
B=Path(__file__).resolve().parent;R=B/'raw'
def read(n):return json.loads((B/n).read_text())
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
idx=read('raw-index.json')
for n,v in idx.items():assert sha(B/n)==v['sha256'] and (B/n).stat().st_size==v['bytes'],n
p=read('raw/protocol.json');r=read('raw/receipt.json');refs=read('reference-map.json')
assert r['protocol_sha256']==sha(R/'protocol.json') and r['driver_sha256']==sha(R/'run.py')
for n,h in p['references'].items():assert refs[n]['sha256']==h and sha(B/refs[n]['portable'])==h,n
assert p['budget']=={'Rust_library_builds':2,'C_fixture_links':2,'engine_processes_maximum':4,'solver_candidates':1,'retries':0,'headline_timing':0,'test_builds':0}
assert r['status']=='completed-diagnostic' and r['production_shader_restored'] and r['all_original_assertions_pass'] and not r['unlaunched_cases'] and not r.get('running')
assert r['compiled_inputs_before']==r['compiled_inputs_after']==p['candidate_inputs']
assert r['original_inputs_after']==p['original_inputs'] and len(p['original_inputs'])==107
assert [n for n,h in p['original_inputs'].items() if p['candidate_inputs'][n]!=h]==['shaders/physics/solve.wgsl']
with tarfile.open(R/'candidate-source-inputs.tar.gz') as t:
 for n,h in p['candidate_inputs'].items():assert hashlib.sha256(t.extractfile(n).read()).hexdigest()==h,n
with tarfile.open(R/'native-generated-inputs.tar.gz') as t:
 for n,h in r['native_generated_inputs'].items():assert hashlib.sha256(t.extractfile(n).read()).hexdigest()==h,n
assert len(r['native_generated_inputs'])==455
orig=(R/'original-solve.wgsl').read_text();candidate=(R/'candidate-solve.wgsl').read_text()
a='''            } else {
                let omega = 6.2831853 * max(jn.hertz, 1.0);
                let a1 = 2.0 * max(jn.damping, 0.25) + h * omega;'''
b='''            } else {
                // Match the prepared rigid constraint's timestep hertz clamp.
                let omega = 6.2831853 * min(max(jn.hertz, 1.0), 0.25 / max(h, 1e-8));
                let a1 = 2.0 * max(jn.damping, 0.25) + h * omega;'''
assert orig.count(a)==1 and candidate==orig.replace(a,b)
assert sha(R/'original-solve.wgsl')==p['original_inputs']['shaders/physics/solve.wgsl'] and sha(R/'candidate-solve.wgsl')==p['candidate_shader_sha256']
assert len(r['builds'])==len(r['links'])==2 and all(b['exit']==0 for b in r['builds']+r['links'])
for build,recipe in zip(r['builds'],p['builds'],strict=True):
 assert build['command']==recipe['command']
 assert build['stdout_sha256']==sha(R/build['name']/'stdout.log') and build['stderr_sha256']==sha(R/build['name']/'stderr.log')
 backend=recipe['backend'];lib=r['libraries'][backend];assert lib['compiler_artifacts'] and len(lib['sha256'])==64
 assert all('external-c-shim' in a['features'] and 'replay-diagnostics' in a['features'] for a in lib['compiler_artifacts'])
 assert ('native-command-cache' in lib['compiler_artifacts'][0]['features'])==(backend=='native')
for link,recipe in zip(r['links'],p['links'],strict=True):
 assert link['command']==recipe['command'] and '-DGPU_API_DUAL' in link['command']
 assert link['reused_inputs']==recipe['reused_inputs'] and link['source_sha256']==recipe['source_sha256']
 assert link['Rust_library_sha256']==r['libraries'][recipe['backend']]['sha256'] and len(link['binary_sha256'])==64
 assert link['stdout_sha256']==sha(R/link['name']/'stdout.log') and link['stderr_sha256']==sha(R/link['name']/'stderr.log')
expected=[{'scene':scene,'substeps':sub,'recorded_postwarmup_steps':180,'timing_only':False} for scene in ['sphere-ground','distance-joint'] for sub in [1,4]]
for row,c in zip(r['results'],p['cases'],strict=True):
 assert row['name']==c['name'] and row['command']==c['command'] and row['environment_overrides']==c['environment_overrides']
 assert row['exit']==0 and row['pass'] and row['actual_NVIDIA_Vulkan'] and not row['first_discrepancy']
 assert row['completed_original_branches']==expected
 stderr=(R/c['name']/'stderr.log').read_text();assert 'NVIDIA GeForce RTX 4070 SUPER' in stderr and 'backend=Vulkan driver=NVIDIA' in stderr
 assert row['stdout_sha256']==sha(R/c['name']/'stdout.log') and row['stderr_sha256']==sha(R/c['name']/'stderr.log')
assert len(r['results'])==4 and {(c['backend'],c['ordering']) for c in p['cases']}=={(b,o) for b in ['ordinary','native'] for o in [0,1]}
for v,y in zip(read('raw/algebra.json')['cases'],[-1.00004232,-1.00028491],strict=True):assert abs(v['predicted_initial_y']-y)<1e-7
# Independent unchanged original baseline receipt and fixture constraints.
baseline=next(json.loads((B/v['portable']).read_text()) for n,v in refs.items() if n.endswith('pr03-distance-baseline/receipt.json'))
assert all(v['exit']==-6 and v['first_discrepancy']==['distance-joint','0','1','-1.00004232','-1.00028491'] for v in baseline['results'])
print(f'PASS: {len(idx)} raw files; exact one-clamp shader change; two builds/links; four NVIDIA Vulkan original-limit fixture passes (3,840 completed steps/49,920 finite lane comparisons). Original campaign restored source; see retention follow-up for applicable regressions/scene review. Full release qualification remains OPEN. No performance claim.')
