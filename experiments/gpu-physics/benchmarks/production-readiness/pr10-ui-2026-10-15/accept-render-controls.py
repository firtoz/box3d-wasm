from pathlib import Path
import json,hashlib,shutil
from PIL import Image,ImageDraw
D=Path(__file__).resolve().parent;labels=['cpu','ordinary-gpu','native-gpu','ordinary-both','native-both'];r={'status':'VERIFYING','routes':[]};fields=['disableShadows','disableAmbientOcclusion','ibl','hullEdges','edgeConvexity','shadowStart','shadowRange','shadowDistribution','sun','exposure','debugMode','gtaoQuality'];sources={}
def sha(p):
 with Path(p).open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()
for label in labels:
 base=D/'raw/render-controls-runs'/(label+'-popup');a=json.loads((base/'receipt.json').read_text());assert a['status']=='PASS_SHARED_RENDER_CONTROLS_ACTUAL_BOUNDARY'and a['app_terminal_exit']==0;assert all(a['checks'].values());initial=a['initial_renderer'];proofs=list(a['render_proofs']);partials=[]
 if label=='cpu':
  for tag in ['cpu','cpu-negative']:
   p=D/'raw/render-controls-runs'/tag/'receipt.json';q=json.loads(p.read_text());assert q['status']=='FAILURE_RETAINED';assert q['initial_renderer']==initial;proofs+=q['render_proofs'];partials.append({'receipt':str(p.relative_to(D)),'status':q['status'],'error':q['error'],'retained_checked_fields':list(q['checks'])})
 for f in fields:
  assert any(p['field']==f and abs(p['actual_renderer'][f]-initial[f])>1e-5 for p in proofs),(label,f,'noactualchange')
  assert any(p['field']==f and abs(p['actual_renderer'][f]-initial[f])<1e-5 for p in proofs),(label,f,'noactualrestore')
  assert abs(a['final_renderer'][f]-initial[f])<1e-5
 assert set(p['value']for p in proofs if p['field']=='debugMode')>=set(range(5))
 assert a['image_effects']['dark_mean']<a['image_effects']['baseline_mean']*.85;assert a['image_effects']['normal_mean_abs_difference']>15
 for sc in a['screenshots'].values():assert sha(sc['path'])==sc['sha256']
 ref=a['screenshots']['default']['state'];identity=['world','body_count','shape_count','context_flags','world_flags','hertz','substeps','pause']
 for sc in a['screenshots'].values():assert all(ref[k]==sc['state'][k]for k in identity)
 r['routes'].append({'label':label,'initial_renderer':initial,'final_renderer':a['final_renderer'],'actual_renderer_proofs':proofs,'image_effects':a['image_effects'],'seconds':a['elapsed_seconds'],'partial_CPU_receipts':partials,'shared_cache_before':a['shared_cache_before'],'shared_cache_after':a['shared_cache_after'],'identity_defaults_preserved':identity});sources[label]={stage:base/(stage+'.png')for stage in ['default','dark','restored']};sources[label]['normal']=D/'raw/render-controls-runs/cpu-negative/normal.png'if label=='cpu'else base/'normal.png'
b=json.loads((D/'render-popup-build.json').read_text());assert all(sha(p)==h for p,h in b['input_pins'].items());r.update(status='PASS_FIVE_ROUTES_RENDER_SETTINGS',settings_per_route=len(fields),debug_modes_per_route=5,full_sample_reviews_done=0,full_sample_reviews_left=165,limits=['CPU evidence combines retainedpartial checks and tailchecks on samefrozenbuild/defaultsetup; originalfailures remainfailures.','Nativekeyboardnavnotenabled, Combo actual mouse usesobservedpopupgeometry. Previoussame-tierweakcaseinvalidated; changed-tier2→1proved.','SourcecommonRenderFrame actualsettings verified; selectedExposure/normal actualimageeffects. Notpixelmatching, noUI03overlayrepair.'])
(D/'render-controls-acceptance.json').write_text(json.dumps(r,indent=2)+'\n')
im=Image.new('RGB',(2400,1180),(25,25,25));draw=ImageDraw.Draw(im)
for col,label in enumerate(labels):
 for row,stage in enumerate(['default','dark','normal','restored']):
  img=Image.open(sources[label][stage]);img.thumbnail((480,270));im.paste(img,(col*480,row*295+25));draw.text((col*480+5,row*295+5),label+' / '+stage,fill='white')
im.save(D/'render-controls-grid.jpg',quality=92)
a=json.loads((D/'inventory.json').read_text())
for w in a['shared_widget_calls']:
 if w['source']=='box3d/samples/sample.cpp'and (1535<=w['line']<=1655 or w['kind']=='BeginMenu'and w['label']=='Render'):
  w['status']='CHECKED_SHARED_RENDER_BOUNDARY';w['evidence']='render-controls-acceptance.json';w['scope']='Sharedrendererinput/getters sourcegroup; 5native routesrealCPUfirst, selectedExposure/normalimageeffect, sourceapplicabilitynotautomaticfullsamplepass.'
  for f in ['real_cpu','ordinary_gpu','native_gpu','ordinary_combined','native_combined']:w[f]='CHECKED_ACTUAL_INPUT_AND_RENDER_BOUNDARY'
  if w['kind']=='RadioButton':w['runtime_cases']='All5debugmodes0–4selected, actualFrameInputdebugModeverified'
a['counts']['shared_calls_dispositioned']=sum(x['status']in ['VERIFIED_SHARED_IMPLEMENTATION_APPLICABLE_ROUTES','CHECKED_SHARED_INPUT_AND_EFFECT','CHECKED_SHARED_NAVIGATION','REVIEWED_DEFECT_UI03','CHECKED_SHARED_RENDER_BOUNDARY']for x in a['shared_widget_calls']);a['counts']['shared_calls_without_complete_disposition']=84-a['counts']['shared_calls_dispositioned'];(D/'inventory.json').write_text(json.dumps(a,indent=2)+'\n')
removed=[]
for folder in ['ui03-output-build','render-controls-build']:
 for label in labels:
  p=D/'raw'/folder/label
  if p.exists():removed.append({'path':str(p.relative_to(D)),'bytes':p.stat().st_size});p.unlink()
for p in (D/'raw/render-controls-runs').glob('*/*'):
 if p.name in ['observer.jsonl','api.jsonl']:removed.append({'path':str(p.relative_to(D)),'bytes':p.stat().st_size});p.unlink()
(D/'render-controls-cleanup.json').write_text(json.dumps({'removed':removed,'deleted_bytes':sum(x['bytes']for x in removed),'retained_consumer':'Fivecurrentrender-popup-build diagnosticviewers+observer.o/main/Sample objects: remainingPR10sampleCombo andsharedcontrols. 20selectedimages+failureimages/receipts/actualstatewindows retained; sharedcaches preserved.','free_bytes':shutil.disk_usage(D).free},indent=2)+'\n');print(r['status'],'shared',a['counts']['shared_calls_dispositioned'],'left',a['counts']['shared_calls_without_complete_disposition'],'deleted',sum(x['bytes']for x in removed))
