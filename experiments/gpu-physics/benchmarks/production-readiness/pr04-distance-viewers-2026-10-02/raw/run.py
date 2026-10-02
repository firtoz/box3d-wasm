from pathlib import Path
import json,hashlib,subprocess,os,time,signal,tarfile
R=Path.cwd();A=R/'artifacts/production-readiness/pr04-distance-viewers';P=A/'protocol.json';p=json.loads(P.read_text());assert not(A/'receipt.json').exists();sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest();s={'status':'linking','protocol_sha256':sha(P),'driver_sha256':sha(__file__),'started_unix':time.time(),'toolchain':subprocess.check_output(['c++','--version'],text=True),'results':[],'viewers':{}}
def save():
 t=A/'receipt.tmp';t.write_text(json.dumps(s,indent=2)+'\n');os.replace(t,A/'receipt.json')
def check(c):
 for n,h in p['CPP_consumer_inputs'].items():assert sha(R/n)==h,n
 for n,h in c['linked_inputs'].items():assert sha(n)==h,n
 for u in c['compiled_units']:assert sha(u['file'])==u['source_sha256'] and sha(u['object'])==u['object_sha256'],u['file']
 assert sha(c['consumer_receipt'])==c['consumer_receipt_sha256']
 for key in ['candidate_Rust_producer','regression','parent']:
  assert sha(p[key+'_receipt'])==p[key+'_receipt_sha256']
save()
try:
 with tarfile.open(A/'consumer-inputs.tar.gz','w:gz') as t:
  for n in p['CPP_consumer_inputs']:t.add(R/n,arcname=n.replace('../../box3d/','box3d/'),recursive=False)
 for c in p['cases']:
  check(c);out=A/c['cell'];out.mkdir();start=time.time()
  with (out/'stdout.log').open('w') as so,(out/'stderr.log').open('w') as se:
   proc=subprocess.Popen(c['command'],cwd=c['cwd'],stdout=so,stderr=se,start_new_session=True);s['running']={'pid':proc.pid,'cell':c['cell'],'command':c['command']};save()
   try:code=proc.wait(timeout=p['watchdog_seconds_per_link'])
   except subprocess.TimeoutExpired:os.killpg(proc.pid,signal.SIGKILL);proc.wait();code='timeout'
  s.pop('running',None);row={'cell':c['cell'],'exit':code,'command':c['command'],'cwd':c['cwd'],'stdout_sha256':sha(out/'stdout.log'),'stderr_sha256':sha(out/'stderr.log'),'elapsed_seconds_incidental':time.time()-start};s['results'].append(row);save();assert code==0,row;check(c)
  v={'status':'built','executable':c['binary'],'executable_sha256':sha(c['binary']),'inputs_before':p['CPP_consumer_inputs'],'inputs_after':p['CPP_consumer_inputs'],'compiled_units':c['compiled_units'],'linked_inputs':c['linked_inputs'],'command':c['command'],'cwd':c['cwd'],'original_consumer_receipt':c['consumer_receipt'],'original_consumer_receipt_sha256':c['consumer_receipt_sha256'],'actual_Rust_producer_receipt':p['candidate_Rust_producer_receipt'],'actual_Rust_producer_receipt_sha256':p['candidate_Rust_producer_receipt_sha256'],'candidate_library_sha256':c['candidate_library']['sha256'],'disclosure':'Relink only,zero compiler translation units. ActualCPP object/source/header provenance from original producer; Rust provider compiledinputs separately preserved. Not cleanbuild qualification.'};(out/'receipt.json').write_text(json.dumps(v,indent=2)+'\n');s['viewers'][c['cell']]={'receipt_sha256':sha(out/'receipt.json'),'executable_sha256':v['executable_sha256']};save();print(c['cell'],'relinked',flush=True)
 s.update(status='built',finished_unix=time.time());save()
except BaseException as e:s.update(status='stopped',error=repr(e),finished_unix=time.time());save();raise
