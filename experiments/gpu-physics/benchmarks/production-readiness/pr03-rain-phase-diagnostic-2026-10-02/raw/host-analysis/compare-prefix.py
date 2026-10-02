from pathlib import Path
import json,hashlib,subprocess,sys,time,os
R=Path.cwd();A=R/'artifacts/production-readiness/pr03-rain-phase-remaining';G=R/'artifacts/production-readiness/pr03-rain-phase-diagnostic/phase180/health.json';C=R/'artifacts/production-readiness/pr03-rain-drag-display-repair/cpu-rain/health.json'
def sha(f):
 h=hashlib.sha256()
 with f.open('rb') as s:
  while b:=s.read(1024*1024):h.update(b)
 return h.hexdigest()
P=A/'host-analysis-protocol.json';assert not P.exists();s=json.loads((A/'receipt.json').read_text());assert s['status']=='completed-diagnostic';q={'purpose':'Single host-only comparison of complete180 GPU diagnostic window against first180 frames of previously completed CPU600. This is a projected CPU record, not a new engine run. Original residual screens/evaluator unchanged; partialdiagnosis never600/recycling/fullstate/performance qualification.','budget':{'host_comparisons':1,'builds':0,'CPU_GPU_processes':0,'solver_candidates':0,'retries':0,'headline_timing':0},'CPU_source_sha256':sha(C),'CPU_source_bytes':C.stat().st_size,'GPU_source_sha256':sha(G),'GPU_source_bytes':G.stat().st_size,'phase_receipt_sha256':sha(A/'receipt.json'),'evaluator_sha256':sha(R/'scripts/compare-rain-lifetimes.py'),'CPU_window_header_changes':{'frames_observed':180,'timed':180,'measured':180},'frames':180,'scope':'exactmeaningfulorderedfirst180 CPUframes;originalfull600 retained in priorreport'};P.write_text(json.dumps(q,indent=2)+'\n');start=time.time();dest=A/'cpu600-prefix180.json'
with C.open() as f:
 prefix=[]
 for line in f:
  if line.strip()=='"frames": [':break
  prefix.append(line)
 h=json.loads(''.join(prefix)+'"frames": []\n}');assert h['status']=='ok' and h['timed']==h['measured']==h['frames_observed']==600
 h.update(q['CPU_window_header_changes']);h['frames']=[];head=json.dumps(h,indent=2);head=head[:head.index('  "frames": []')]+'  "frames": [\n';hasher=hashlib.sha256()
 with dest.open('w') as out:
  out.write(head)
  for i in range(180):
   line=f.readline();frame=json.loads(line.strip().removesuffix(','));assert frame['i']==i and frame['submitted_step']==i+1;hasher.update(json.dumps(frame,separators=(',',':')).encode()+b'\n');out.write(line.rstrip().removesuffix(',')+(',' if i<179 else '')+'\n')
  out.write('  ]\n}\n')
command=[sys.executable,str(R/'scripts/compare-rain-lifetimes.py'),str(dest),str(G),'--frames','180','--require-spherical','--output',str(A/'residual-comparison.json')]
with (A/'host-analysis.stdout.log').open('w') as so,(A/'host-analysis.stderr.log').open('w') as se:
 env=os.environ.copy();env['PYTHONDONTWRITEBYTECODE']='1';code=subprocess.run(command,env=env,stdout=so,stderr=se).returncode
report=json.loads((A/'residual-comparison.json').read_text());assert report['frames']==180 and report['identity_status']=='pass' and code==int(report['residual_screen_status']!='pass');receipt={'protocol_sha256':sha(P),'driver_sha256':sha(Path(__file__)),'status':'completed-host-diagnosis','exit':code,'command':command,'elapsed_seconds_incidental':time.time()-start,'projected_CPU_sha256':sha(dest),'projected_CPU_bytes':dest.stat().st_size,'ordered_CPU_frame_semantics_sha256':hasher.hexdigest(),'report_sha256':sha(A/'residual-comparison.json'),'stdout_sha256':sha(A/'host-analysis.stdout.log'),'stderr_sha256':sha(A/'host-analysis.stderr.log'),'residual_screen_status':report['residual_screen_status']};(A/'host-analysis-receipt.json').write_text(json.dumps(receipt,indent=2)+'\n');print(json.dumps(report,indent=2))
