from pathlib import Path
import json,hashlib,os
from PIL import Image,ImageDraw
D=Path(__file__).resolve().parent;labels=['cpu','ordinary-gpu','native-gpu','ordinary-both','native-both'];r={'status':'VERIFYING','routes':[]};accept={}
def sha(p):
 with Path(p).open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()
for label in labels:
 tag=label if label in ['cpu','ordinary-gpu']else label+'-timing-observed';bs=D/'raw/ui02-before-runs'/label;af=D/'raw/ui02-after-runs'/tag;b=json.loads((bs/'receipt.json').read_text());a=json.loads((af/'receipt.json').read_text());assert b['status']=='UI02_DEFECT_REPRODUCED_ACTUAL_HANDLER';assert a['status']=='UI02_REPAIR_VERIFIED_ACTUAL_INPUT';assert all(b['checks'].values())and all(a['checks'].values());assert b['app_terminal_exit']==a['app_terminal_exit']==0
 for v in [a,b]:
  assert abs(v['enter_case']['radius_after']-v['enter_case']['radius_before'])<.05 if v is a else abs(v['enter_case']['radius_after']-v['enter_case']['radius_before'])>.05
  assert abs(v['leave_case']['radius_after']-v['leave_case']['radius_before'])>.05 if v is a else abs(v['leave_case']['radius_after']-v['leave_case']['radius_before'])<.05
  for name,s in v['screenshots'].items():assert sha(s['path'])==s['sha256']
 fields=['world','body_count','shape_count','context_flags','world_flags','hertz','substeps','pause']
 for phase,v in [('before',b),('after',a)]:
  ref=v['screenshots']['before']['state']
  for s in v['screenshots'].values():assert all(ref[k]==s['state'][k]for k in fields)
 assert all(b['screenshots']['before']['state'][k]==a['screenshots']['before']['state'][k]for k in fields)
 row={'label':label,'after_tag':tag,'before_seconds':b['elapsed_seconds'],'after_seconds':a['elapsed_seconds'],'before_checks':len(b['checks']),'after_checks':len(a['checks']),'before_enter':b['enter_case'],'before_leave':b['leave_case'],'after_enter':a['enter_case'],'after_leave':a['leave_case'],'held_case':a['held_case'],'popup_case':a['popup_case'],'identity_defaults_preserved':fields,'cache_before':a['shared_cache_before'],'cache_after':a['shared_cache_after']};r['routes'].append(row);accept[label]=af
build=json.loads((D/'ui02-build-after.json').read_text());assert build['status']=='PASS_UI02_ONLY_CANDIDATE_BUILD';assert all(sha(p)==h for p,h in build['input_pins'].items());assert sha(D.parent.parent.parent/'native-samples/sokol_ui_input.h')==build['helper_sha256']
for phase in ['before','after']:
 im=Image.new('RGB',(2400,885),(25,25,25));draw=ImageDraw.Draw(im)
 for col,label in enumerate(labels):
  base=D/'raw/ui02-before-runs'/label if phase=='before'else accept[label]
  for row,stage in enumerate(['before','enter','leave']):
   img=Image.open(base/(stage+'.png'));img.thumbnail((480,270));im.paste(img,(col*480,row*295+25));draw.text((col*480+5,row*295+5),label+' / '+stage,fill='white')
 im.save(D/('ui02-'+phase+'-grid.jpg'),quality=92)
r.update(status='PASS_FIVE_ROUTES_UI02',before_checks=sum(x['before_checks']for x in r['routes']),after_checks=sum(x['after_checks']for x in r['routes']),screenshots=30,full_sample_interaction_reviews_complete=0,full_sample_interaction_reviews_left=165,limitations=['The XTEST motion/wheel flush can cross UI frames; paired actual events/flags are retained rather than assuming one batch. All5 before reproduce stale capture; after CPU/ordinary directly exercise stale-flag repaired path.','Specific mouse drag/body picking interactions remain PR10 open. Held mouse path is byte-preserved by helper early return and actual held-world+panel wheel tested.','Full affected-scene precommit recordings remain required; no commit authorization.'])
(D/'ui02-acceptance.json').write_text(json.dumps(r,indent=2)+'\n');print(r['status'],r['before_checks'],r['after_checks'],r['screenshots'])
