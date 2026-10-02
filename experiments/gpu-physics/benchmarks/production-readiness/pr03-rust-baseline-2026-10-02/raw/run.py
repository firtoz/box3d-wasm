from pathlib import Path
import json,hashlib,subprocess,time,os,signal,re,gzip,shutil,resource
R=Path.cwd();A=R/'artifacts/production-readiness/pr03-rust-baseline';P=A/'protocol.json';sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest();p=json.loads(P.read_text());assert not(A/'receipt.json').exists();s={'status':'running','protocol_sha256':sha(P),'driver_sha256':sha(__file__),'started_unix':time.time(),'results':[]};resource.setrlimit(resource.RLIMIT_CORE,(0,0))
def save():
 t=A/'receipt.tmp';t.write_text(json.dumps(s,indent=2)+'\n');os.replace(t,A/'receipt.json')
def check():
 for f,h in p['producer_receipts'].items():
  assert sha(f)==h;d=json.loads(Path(f).read_text());assert d['inputs_before']==d['inputs_after']
  for n,h in d['inputs_after'].items():assert sha(R/n)==h,n
 for cell,d in p['cells'].items():assert sha(d['test_executable']['path'])==d['test_executable']['sha256']
save()
try:
 check()
 for c in p['cases']:
  out=A/c['name'];out.mkdir(parents=True);start=time.time()
  with (out/'stdout.log').open('w') as so,(out/'stderr.log').open('w') as se:
   proc=subprocess.Popen(c['command'],cwd=R,env=c['environment'],stdout=so,stderr=se,start_new_session=True);s['running']={'name':c['name'],'pid':proc.pid};save()
   try:code=proc.wait(timeout=p['watchdog_seconds_per_process'])
   except subprocess.TimeoutExpired:os.killpg(proc.pid,signal.SIGKILL);proc.wait();code='timeout'
  s.pop('running',None);text=(out/'stdout.log').read_text();err=(out/'stderr.log').read_text();adapter='NVIDIA GeForce RTX 4070 SUPER' in err and 'Vulkan' in err;result=re.search(r'test result: (ok|FAILED)\. (\d+) passed; (\d+) failed; (\d+) ignored;',text);row={'name':c['name'],'exit':code,'selected_count':len(c['selectors']),'harness_result':result.groups() if result else None,'actual_NVIDIA_Vulkan':adapter,'elapsed_seconds_incidental':time.time()-start,'stdout_sha256':sha(out/'stdout.log'),'stderr_sha256':sha(out/'stderr.log'),'valid':code==0 and result is not None and result.group(1)=='ok' and int(result.group(2))==len(c['selectors']) and int(result.group(3))==int(result.group(4))==0}
  trace=out/'state.jsonl'
  if trace.exists():
   row['raw_trace_sha256']=sha(trace);row['raw_trace_bytes']=trace.stat().st_size
   with trace.open() as f:
    count=0
    for count,line in enumerate(f,1):
     frame=json.loads(line);assert frame.get('schema')=='gpu-core-state-v24' and frame.get('frame')==count
     assert all(isinstance(frame.get(k),list) for k in ['bodies','joints','contacts'])
   row['trace_frames']=count;compressed=out/'state.jsonl.gz'
   with trace.open('rb') as inp,gzip.open(compressed,'wb',compresslevel=1) as dest:shutil.copyfileobj(inp,dest)
   with gzip.open(compressed,'rb') as f:assert hashlib.sha256(f.read()).hexdigest()==row['raw_trace_sha256']
   row['trace_gzip_sha256']=sha(compressed);trace.unlink()
  hits=re.search(r'FULL_STATE_REPLAY_HITS (\[[^\n]+\])',err)
  if hits:row['replay_hits']=json.loads(hits.group(1))
  s['results'].append(row);save();print(c['name'],code,'checks',row['harness_result'],'frames',row.get('trace_frames'),flush=True)
  assert code!='timeout' and adapter,row
  if row['valid'] and 'expected_trace_frames' in c:assert row.get('trace_frames')==c['expected_trace_frames'],row
  check()
 s.update(status='completed-baseline',finished_unix=time.time(),all_assertions_pass=all(r['valid'] for r in s['results']),selected_checks=sum(r['selected_count'] for r in s['results']));save()
except BaseException as ex:s.update(status='stopped',error=repr(ex),finished_unix=time.time(),unlaunched=[c['name'] for c in p['cases'][len(s['results']):]]);save();raise
