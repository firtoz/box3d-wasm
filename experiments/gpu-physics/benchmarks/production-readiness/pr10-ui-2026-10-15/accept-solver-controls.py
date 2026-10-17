from pathlib import Path
import json,hashlib,shutil
from PIL import Image,ImageDraw
D=Path(__file__).resolve().parent;labels=['cpu','ordinary-gpu','native-gpu','ordinary-both','native-both'];r={'status':'VERIFYING','routes':[]};accepted={}
def sha(p):
 with Path(p).open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()
for label in labels:
 tag=label if label=='cpu'else label+('-loading'if 'both'in label else '-policy');base=D/'raw/solver-controls-runs'/tag;a=json.loads((base/'receipt.json').read_text());assert a['status']=='PASS_SHARED_SOLVER_SLIDERS_ACTUAL_API'and a['app_terminal_exit']==0;assert all(a['checks'].values());assert len(a['screenshots'])==2
 for s in a['screenshots'].values():assert sha(s['path'])==s['sha256']
 r['routes'].append({'label':label,'tag':tag,'checks':a['checks'],'seconds':a['elapsed_seconds'],'edited_step_calls':a['edited_step_calls'],'edited_recycle_calls':a['edited_recycle_calls'],'restored_step_calls':a['restored_step_calls'],'initial_controls':a['initial_controls'],'restored_controls':a['restored_controls'],'shared_cache_before':a['shared_cache_before'],'shared_cache_after':a['shared_cache_after']});accepted[label]=base
b=json.loads((D/'solver-controls-build.json').read_text());assert b['status']=='PASS_READ_ONLY_SOLVER_CONTROL_OBSERVER';assert all(sha(p)==h for p,h in b['input_pins'].items())
im=Image.new('RGB',(2400,590),(25,25,25));draw=ImageDraw.Draw(im)
for col,label in enumerate(labels):
 for row,stage in enumerate(['edited','restored']):
  img=Image.open(accepted[label]/(stage+'.png'));img.thumbnail((480,270));im.paste(img,(col*480,row*295+25));draw.text((col*480+5,row*295+5),label+' / '+stage,fill='white')
im.save(D/'solver-controls-grid.jpg',quality=92)
r.update(status='PASS_FIVE_ROUTES_SOLVER_SLIDERS',checks=sum(len(x['checks'])for x in r['routes']),screenshots=10,full_interaction_samples_done=0,full_interaction_samples_left=165,limits='4 sharedslider callsites withgeneratedGPUavailability reconciled; no sample-specific qualification, no solverphysicalaccuracy claim. Originalharnessfailures preserved.')
(D/'solver-controls-acceptance.json').write_text(json.dumps(r,indent=2)+'\n');print(r['status'],r['checks'])
a=json.loads((D/'inventory.json').read_text());routefields={'cpu':'real_cpu','ordinary-gpu':'ordinary_gpu','native-gpu':'native_gpu','ordinary-both':'ordinary_combined','native-both':'native_combined'}
for x in a['shared_widget_calls']:
 if x['label']in ['Sub-steps##Solver','Hertz##Solver','Workers##Solver','Recycle##Solver']:
  x['status']='VERIFIED_SHARED_IMPLEMENTATION_APPLICABLE_ROUTES';x['evidence']='solver-controls-acceptance.json';x['scope']='RealCPU/SingleBoxfirst,generatednativehost shared acrossHasSolverControls samples; not automatic fullsample pass.'
  for label,field in routefields.items():x[field]='EXPLICIT_AUTOMATIC_GPU_SCHEDULING_NO_WIDGET'if x['label']=='Workers##Solver'and label in ['ordinary-gpu','native-gpu']else 'CHECKED_ACTUAL_INPUT_AND_API_EFFECT'
  if x['label']=='Workers##Solver':x['generated_alias']='CPU Workers##Solver incombined; ordinary/native standalone explicitlyautomatic'
a['counts']['shared_slider_calls_verified']=4;(D/'inventory.json').write_text(json.dumps(a,indent=2)+'\n')
removed=[];paths=list((D/'raw/ui02-after/bin').glob('*'))
for p in (D/'raw/solver-controls-runs').glob('*/*'):
 if p.name in ['observer.jsonl','api.jsonl']:paths.append(p)
for p in paths:
 assert p.is_file();removed.append({'path':str(p.relative_to(D)),'bytes':p.stat().st_size,'sha256':sha(p)});p.unlink()
cleanup={'status':'CONSUMED_INPUTS_CLEANED','removed':removed,'deleted_bytes':sum(x['bytes']for x in removed),'retained_consumer':'Currentsolver-controls-build5viewers+observer.o/UIobserver+Sample/main objects: remainingPR10. Beforeaftermedia/receipts/actualAPIwindows retained. Sharedcaches preserved.','free_bytes':shutil.disk_usage(D).free};(D/'solver-controls-cleanup.json').write_text(json.dumps(cleanup,indent=2)+'\n');print('deleted_bytes',cleanup['deleted_bytes'])
