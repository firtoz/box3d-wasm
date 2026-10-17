from pathlib import Path
import json,hashlib,shutil
from PIL import Image,ImageDraw
D=Path(__file__).resolve().parent;labels=['cpu','ordinary-gpu','native-gpu','ordinary-both','native-both'];r={'status':'UI03_CONFIRMED_UNFIXED','routes':[],'source_scope':'gpu_samples_world_draw sharednativeGPUdraw ignoresalloverlayflags exceptdrawShapes. ActualoutputconfirmedBounds/Massonly; otheroverlaysrequireapplicablefixtures/explicitreview.','limits':'No repair oraftercapture. 20beforeoriginalscreens; all5routes realCPUfirst. Fullsample reviews0complete165remaining.'}
def sha(p):
 with Path(p).open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()
for label in labels:
 a=json.loads((D/'raw/ui03-before-runs'/label/'receipt.json').read_text());assert all(a['checks'].values())and a['app_terminal_exit']==0;assert a['status']==('UI03_CPU_OUTPUT_REFERENCE_VERIFIED'if label=='cpu'else 'UI03_ACTUAL_OUTPUT_DEFECT_CONFIRMED')
 for case in a['output_cases']:
  field='bounds_callbacks'if case['control']=='Bounds'else 'transform_callbacks';v=[x[field]for x in case['callback_outputs'][-4:]]
  assert max(v)>0 if label=='cpu'or 'both'in label else max(v)==0
  if 'both'in label:assert case['actual_gpu_outputs']and all(x[field]==0 for x in case['actual_gpu_outputs'])
 for sc in a['screenshots'].values():assert sha(sc['path'])==sc['sha256']
 r['routes'].append({k:a[k]for k in ['label','checks','output_cases','elapsed_seconds','shared_cache_before','shared_cache_after']})
b=json.loads((D/'ui03-output-build.json').read_text());assert all(sha(p)==h for p,h in b['input_pins'].items());r['source_sha256']=sha(D.parents[2]/'c_abi/samples_api.c');r['checks']=sum(len(x['checks'])for x in r['routes']);(D/'ui03-output-acceptance.json').write_text(json.dumps(r,indent=2)+'\n')
im=Image.new('RGB',(2400,1180),(25,25,25));draw=ImageDraw.Draw(im)
for col,label in enumerate(labels):
 for row,stage in enumerate(['default','bounds','mass','restored']):
  img=Image.open(D/'raw/ui03-before-runs'/label/(stage+'.png'));img.thumbnail((480,270));im.paste(img,(col*480,row*295+25));draw.text((col*480+5,row*295+5),label+' / '+stage,fill='white')
im.save(D/'ui03-before-grid.jpg',quality=92)
a=json.loads((D/'issues.json').read_text());u=next(x for x in a if x['id']=='UI03');u.update(status='CONFIRMED_UNFIXED',defect='Bounds/Mass toggles reachactualdrawflag butGPUemits0callbacks; CPUemits2bounds/2transforms. BothGPUhalves0withCPUleftpositive. SameGPUdrawfunctionhasotheroverlayflagsunconsumed; otheroutputsnotyetqualified.',evidence=['UI03.md','ui03-output-acceptance.json','ui03-before-grid.jpg'],next='Continue fullwidgetreview; recordaffectedcontrols/applicablefixtures. Do notfixthisissueuntilreviewphasecomplete.');(D/'issues.json').write_text(json.dumps(a,indent=2)+'\n')
a=json.loads((D/'inventory.json').read_text());good=['Sub-steps##Solver','Hertz##Solver','Workers##Solver','Recycle##Solver'];effects=['Pause','Single Step','Restart','Quit','Shapes','Diagnostics','Counters','Sleep##Solver','Warm Starting##Solver','Continuous##Solver'];fields=['real_cpu','ordinary_gpu','native_gpu','ordinary_combined','native_combined']
for w in a['shared_widget_calls']:
 if w['label']in effects:
  w['status']='CHECKED_SHARED_INPUT_AND_EFFECT';w['evidence']='menu-controls-routing.json'if w['label']in ['Pause','Single Step','Quit','Shapes','Diagnostics']or w['kind']=='MenuItem'else 'ui01-acceptance.json';w['scope']='SharedimplementationrepresentativeSingleBox/Jenga routes; notautomaticfullsamplepass.'
  for f in fields:w[f]='CHECKED_SHARED_INPUT_AND_EFFECT'
 if w['kind']=='BeginMenu'and w['label']in ['Sim','View','Anchor']:
  w['status']='CHECKED_SHARED_NAVIGATION';w['evidence']='menu-controls-routing.json'
  for f in fields:w[f]='CHECKED_SHARED_NAVIGATION'
 if w['kind']=='MenuItem'and w['label']in ['Bounds','Mass']:
  w['status']='REVIEWED_DEFECT_UI03';w['evidence']='ui03-output-acceptance.json';w['real_cpu']='CHECKED_ACTUAL_CALLBACK_OUTPUT'
  for f in fields[1:]:w[f]='CONFIRMED_MISSING_GPU_OVERLAY_UI03'
 elif w['kind']=='MenuItem'and w['label']in ['Joints','Joint Extras','Sleep','Body Names','Graph Colors','Islands','Contact Points','Contact Normals','Contact Features','Contact Forces','Anchor A','Anchor B']:
  w['status']='ROUTING_CHECKED_OUTPUT_UNREVIEWED_SOURCE_GAP_UI03';w['evidence']='menu-controls-routing.json';w['scope']='Actualinputandflagschecked; GPUfunctiondoesnotconsumeflag. Needapplicablegeometry/outputreview; notUIpass.'
a['counts']['shared_calls_dispositioned']=sum(x['status']in ['VERIFIED_SHARED_IMPLEMENTATION_APPLICABLE_ROUTES','CHECKED_SHARED_INPUT_AND_EFFECT','CHECKED_SHARED_NAVIGATION','REVIEWED_DEFECT_UI03']for x in a['shared_widget_calls']);a['counts']['shared_calls_without_complete_disposition']=84-a['counts']['shared_calls_dispositioned'];(D/'inventory.json').write_text(json.dumps(a,indent=2)+'\n')
removed=[]
for label in labels:
 p=D/'raw/menu-controls-clip-build'/label
 if p.exists():removed.append({'path':str(p.relative_to(D)),'bytes':p.stat().st_size});p.unlink()
for p in (D/'raw/ui03-before-runs').glob('*/*'):
 if p.name in ['observer.jsonl','api.jsonl']:removed.append({'path':str(p.relative_to(D)),'bytes':p.stat().st_size});p.unlink()
(D/'ui03-output-cleanup.json').write_text(json.dumps({'removed':removed,'deleted_bytes':sum(x['bytes']for x in removed),'retained_consumer':'Fivecurrentui03-output-build viewers/observer andrequiredmain/Sample objects: remainingPR10widgets/outputconfirmation. Screens/compactcallbackinputs retained; sharedcaches preserved.','free_bytes':shutil.disk_usage(D).free},indent=2)+'\n');print(r['status'],r['checks'],'shared_dispositioned',a['counts']['shared_calls_dispositioned'],'left',a['counts']['shared_calls_without_complete_disposition'],'deleted',sum(x['bytes']for x in removed))
