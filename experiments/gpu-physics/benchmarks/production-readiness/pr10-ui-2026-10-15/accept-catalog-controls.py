from pathlib import Path
import json,hashlib,shutil
from PIL import Image,ImageDraw
D=Path(__file__).resolve().parent;labels=['cpu','ordinary-gpu','native-gpu','ordinary-both','native-both'];r={'status':'VERIFYING','routes':[]};images={}
def sha(p):
 with Path(p).open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()
for label in labels:
 base=D/'raw/catalog-controls-runs'/label;a=json.loads((base/'receipt.json').read_text());assert a['status']=='PASS_FULL_CATALOG_NAVIGATION_AND_SPLIT'and a['app_terminal_exit']==0;assert all(a['checks'].values());assert len(a['catalog_categories'])==22 and sum(len(x['actual_items'])for x in a['catalog_categories'])==165;assert len(a['picker_all_labels'])==165;assert [x['after_state']['body_count']for x in a['scene_transitions']]==[61,2,31,2]
 for transition in a['scene_transitions']:assert transition['before_world']!=transition['after_state']['world']
 for sc in a['screenshots'].values():assert sha(sc['path'])==sc['sha256']
 r['routes'].append({k:a[k]for k in ['label','checks','catalog_categories','picker_all_labels','scene_transitions','memory_dump_output','elapsed_seconds','shared_cache_before','shared_cache_after']}|({'split_image_effects':a['split_image_effects']}if 'both'in label else {}));images[label]=base
b=json.loads((D/'render-popup-build.json').read_text());assert all(sha(p)==h for p,h in b['input_pins'].items());r.update(status='PASS_FIVE_ROUTES_FULL165_CATALOG_NAVIGATION',counts={'registrations_per_route':165,'categories_per_route':22,'routes':5,'factory_transition_counts_each':[61,2,31,2]},limits=['Allcatalog/pickerentriesverified, notall165sample-specificwidgets. Fullinteractionreviews0done165remaining.','SplitlayoutusesrealCPUleft/GPUright andoverlaybothworlds; actualown-imageactorcentroidsandstableworld/countschecked, picking/draggingstillpending.','No production/engine changes ornewbinarybuilds.'])
(D/'catalog-controls-acceptance.json').write_text(json.dumps(r,indent=2)+'\n')
im=Image.new('RGB',(2400,1180),(25,25,25));draw=ImageDraw.Draw(im)
for col,label in enumerate(labels):
 for row,stage in enumerate(['catalog','picker','jenga','returned']):
  pic=Image.open(images[label]/(stage+'.png'));pic.thumbnail((480,270));im.paste(pic,(col*480,row*295+25));draw.text((col*480+5,row*295+5),label+' / '+stage,fill='white')
im.save(D/'catalog-controls-grid.jpg',quality=92)
im=Image.new('RGB',(1440,590),(25,25,25));draw=ImageDraw.Draw(im)
for row,label in enumerate(['ordinary-both','native-both']):
 for col,stage in enumerate(['returned','overlay','resplit']):
  pic=Image.open(images[label]/(stage+'.png'));pic.thumbnail((480,270));im.paste(pic,(col*480,row*295+25));draw.text((col*480+5,row*295+5),label+' / '+stage,fill='white')
im.save(D/'split-controls-grid.jpg',quality=92)
a=json.loads((D/'inventory.json').read_text());fields=['real_cpu','ordinary_gpu','native_gpu','ordinary_combined','native_combined']
for w in a['shared_widget_calls']:
 if w['label']in ['Previous Sample','Next Sample','Dump Mem Stats','Samples','##q']or w['kind']in ['BeginMenu','MenuItem','Selectable']and w['label']is None:
  w['status']='REVIEWED_SHARED_CATALOG_AND_LIFECYCLE';w['evidence']='catalog-controls-acceptance.json';w['scope']='22categories/165actualmenuandpickerentries; Jenga/SingleBox/SphereStack factorytransitions. Doesnotcertifyeachsample-specificwidget.'
  for f in fields:w[f]='CHECKED_ACTUAL_INPUT_AND_EFFECT'
 if w.get('generated_gpu_only')and w['label']=='Split: CPU left / GPU right':
  w['status']='REVIEWED_COMBINED_LAYOUT_CONTROL_WITH_UI05';w['evidence']='catalog-controls-acceptance.json';w['scope']='Overlay/splitimageeffectandstableidentity/countsverifiedbothroutes; remainsvisibleonHideUI/UI05.'
  w['ordinary_combined']=w['native_combined']='CHECKED_LAYOUT_EFFECT_HIDE_UI_DEFECT_UI05'
closed=['VERIFIED_SHARED_IMPLEMENTATION_APPLICABLE_ROUTES','CHECKED_SHARED_INPUT_AND_EFFECT','CHECKED_SHARED_NAVIGATION','REVIEWED_DEFECT_UI03','CHECKED_SHARED_RENDER_BOUNDARY','REVIEWED_SHARED_NAVIGATION_METRICS','REVIEWED_DEFECT_UI04','REVIEWED_DEFECT_UI05','REVIEWED_SHARED_CATALOG_AND_LIFECYCLE','REVIEWED_COMBINED_LAYOUT_CONTROL_WITH_UI05'];a['counts']['shared_calls_dispositioned']=sum(x['status']in closed for x in a['shared_widget_calls']);a['counts']['shared_calls_without_complete_disposition']=85-a['counts']['shared_calls_dispositioned'];(D/'inventory.json').write_text(json.dumps(a,indent=2)+'\n')
removed=[]
for p in (D/'raw/catalog-controls-runs').glob('*/*'):
 if p.name in ['observer.jsonl','api.jsonl']:removed.append({'path':str(p.relative_to(D)),'bytes':p.stat().st_size});p.unlink()
(D/'catalog-controls-cleanup.json').write_text(json.dumps({'removed':removed,'deleted_bytes':sum(x['bytes']for x in removed),'retained_consumer':'Current5render-popup diagnosticviewers/objects forremainingPR10. Originalscreens/catalognames/transitions/actualdumpoutput/receiptsretained; sharedcaches preserved.','free_bytes':shutil.disk_usage(D).free},indent=2)+'\n');print(r['status'],'reviewed',a['counts']['shared_calls_dispositioned'],'left',a['counts']['shared_calls_without_complete_disposition'],'deleted',sum(x['bytes']for x in removed))
