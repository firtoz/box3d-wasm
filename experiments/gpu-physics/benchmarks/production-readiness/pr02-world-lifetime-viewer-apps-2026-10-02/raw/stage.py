from pathlib import Path
import json,time,sys
root=Path.cwd();cell,stage=sys.argv[1:];dest=root/'artifacts/production-readiness/pr02-world-lifetime-viewer-apps'/cell
plans={
'preview':[{'type':'key','symbol':112,'label':'pause'},{'type':'screenshot','name':'paused'},{'type':'key','symbol':46,'label':'single-before'},{'type':'scroll','x':1203,'y':620,'steps':5},{'type':'screenshot','name':'controls'}],
'flags-off':[{'type':'click','x':1034,'y':587,'label':'sleep-off'},{'type':'click','x':1034,'y':616,'label':'warm-off'},{'type':'click','x':1034,'y':645,'label':'continuous-off'},{'type':'screenshot','name':'flags-off'}],
'restart':[{'type':'click','x':1052,'y':672,'label':'restart-off'},{'type':'screenshot','name':'restart-off'},{'type':'key','symbol':46,'label':'single-after'}],
'flags-on':[{'type':'click','x':1034,'y':587,'label':'sleep-on'},{'type':'click','x':1034,'y':616,'label':'warm-on'},{'type':'click','x':1034,'y':645,'label':'continuous-on'},{'type':'screenshot','name':'flags-on'},{'type':'key','symbol':109,'label':'metrics-on'},{'type':'screenshot','name':'metrics-initial'}],
'metrics':[{'type':'click','x':48,'y':525,'label':'profile'},{'type':'screenshot','name':'profile'},{'type':'click','x':123,'y':525,'label':'counters'},{'type':'screenshot','name':'counters'},{'type':'click','x':293,'y':525,'label':'frame-time'},{'type':'screenshot','name':'frame-time'},{'type':'key','symbol':109,'label':'metrics-off'},{'type':'screenshot','name':'metrics-off'},{'type':'quit','label':'quit'}]}
def receipt():return json.loads((dest/'receipt.json').read_text())
if stage=='preview':
 start=time.monotonic()
 while not (dest/'observer.jsonl').exists():
  if (dest/'receipt.json').exists():assert receipt()['status']!='failed'
  assert time.monotonic()-start<180,'first observer timeout';time.sleep(.2)
for action in plans[stage]:
 r=receipt();assert r['status']in ['starting','running'];index=len(r['actions'])+1;path=dest/f'action-{index:02}.json';assert not path.exists();path.write_text(json.dumps(action)+'\n');start=time.monotonic()
 while True:
  r=receipt()
  if len(r['actions'])==index:break
  assert r['status']not in ['failed','complete'],r
  assert time.monotonic()-start<60,action;time.sleep(.15)
 a=r['actions'][-1];s=a['state_after'];label=action.get('label','')
 if label=='pause':assert s['pause']==1
 if label.startswith('single-'):
  for f in ['sample_step','submitted','completed']:assert s[f]-a['state_before'][f]==1,(label,f,a)
 if label in ['sleep-off','warm-off','continuous-off','sleep-on','warm-on','continuous-on']:
  i={'sleep':0,'warm':1,'continuous':2}[label.rsplit('-',1)[0]];assert s['context_flags'][i]==int(label.endswith('-on'));assert s['context_flags']==s['world_flags'];
  if cell.endswith('both'):assert s['cpu_flags']==s['context_flags']
 if label=='restart-off':assert s['world']!=a['state_before']['world']and s['sample_step']==s['submitted']==0 and s['pause']==1 and s['context_flags']==[0,0,0],a
 print(index,action,s and {k:s[k]for k in ['world','sample_step','submitted','completed','context_flags','body_count','shape_count']},flush=True)
