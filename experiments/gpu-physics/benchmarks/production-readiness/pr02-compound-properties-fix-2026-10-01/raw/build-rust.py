from pathlib import Path
import json,hashlib,subprocess,os
A=Path('artifacts/production-readiness/pr02-compound-properties-fix');sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
C=json.loads((A/'candidate-inputs.json').read_text());P=json.loads((A/'protocol-before-change.json').read_text());assert sha(A/'protocol-before-change.json')==C['protocol_sha256'];assert not(A/'rust-driver-receipt.json').exists()
for n,h in C['production_candidate_sha256'].items():assert sha(n)==h,n
R={'status':'building','driver_pid':os.getpid(),'protocol_sha256':C['protocol_sha256'],'candidate_inputs_sha256':sha(A/'candidate-inputs.json'),'builds':[]}
def save():(A/'rust-driver-receipt.json').write_text(json.dumps(R,indent=2)+'\n')
def restore():
 R['restoration']={}
 for n,h in C['production_candidate_sha256'].items():
  if sha(n)==h:
   Path(n).write_bytes((A/'baseline'/n).read_bytes());R['restoration'][n]=sha(n);assert sha(n)==P['production_files_before'][n]
  else:R['restoration'][n]='unexpected edits preserved'
save()
try:
 for b in ['ordinary','native']:
  cmd=['python3',str(A/'freeze.py'),b];proc=subprocess.Popen(cmd);R['running']={'backend':b,'pid':proc.pid};save();code=proc.wait();R.pop('running');R['builds'].append({'backend':b,'command':cmd,'exit':code});save()
  assert code==0,('build failed',b,code)
 R['status']='compiled'
except BaseException as e:
 R.update(status='stopped',error=str(e));restore();raise
finally:R.pop('driver_pid',None);R.pop('running',None);save()
