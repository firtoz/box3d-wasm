from pathlib import Path
import json,hashlib,shutil
from PIL import Image,ImageDraw
D=Path(__file__).resolve().parent;labels=['cpu','ordinary-gpu','native-gpu','ordinary-both','native-both'];r={'status':'REVIEWED_WITH_VISUAL_FINDINGS','routes':[]};images={}
def sha(p):
 with Path(p).open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()
for label in labels:
 tag='cpu-reach'if label=='cpu'else label+'-gpu-policy';base=D/'raw/navigation-metrics-runs'/tag;a=json.loads((base/'receipt.json').read_text());assert a['app_terminal_exit']==0
 if label=='cpu':
  assert a['status']=='REVIEWED_SHARED_NAVIGATION_METRICS_WITH_FINDINGS';assert len(a['findings'])==1 and a['findings'][0]['check']=='Metrics constraints reachable';tree=json.loads((D/'raw/metrics-tree-control-runs/one-detent/receipt.json').read_text());assert tree['status']=='PASS_CPU_PROFILE_TREE_CONTROL'and all(tree['checks'].values());q=json.loads((D/'raw/metrics-tree-control-runs/one-detent/plots.ui.json').read_text());node=next(x for x in q['items']if x['kind']=='Tree'and x['label']=='constraints');r['CPU_tree_reachability_control']={'status':'PASS_WITH_ONE_DETENT_HARNESS_OVERSCROLL_CLOSED','checks':tree['checks'],'label_item':node,'windows':q['windows']}
 else:assert a['status']=='PASS_SHARED_NAVIGATION_METRICS'and all(a['checks'].values())
 assert a['checks']['R restart from world focus']and a['checks']['R restart with Info focus'];assert a['checks']['Tab restores UI']
 for sc in a['screenshots'].values():assert sha(sc['path'])==sc['sha256']
 hidden=json.loads((base/'hidden.ui.json').read_text());r['routes'].append({'label':label,'checks':a['checks'],'original_findings':a['findings'],'renderer_text':a['renderer_text'],'hidden_actual_windows':hidden['windows'],'hidden_actual_items':hidden['items'],'seconds':a['elapsed_seconds'],'shared_cache_before':a['shared_cache_before'],'shared_cache_after':a['shared_cache_after'],'visual_findings':['UI05 pausedHUD overlaps sampletext']+(['UI05 combinedComparisonViewremains/coversfooter']if 'both'in label else [])});images[label]={stage:base/(stage+'.png')for stage in ['profile','frame-time','controls','about','hidden','restored']}
r['CPU_profile_label_finding']={'issue':'UI04','source_label':r['CPU_tree_reachability_control']['label_item']['label'],'item_rect':r['CPU_tree_reachability_control']['label_item']['rect'],'clip':r['CPU_tree_reachability_control']['label_item']['clip'],'evidence':'raw/metrics-tree-control-runs/one-detent/plots.png','description':'constraints endsclipped at fixed firstcolumn edge; actualmouse expansion/reachability works.'}
r['source_policy']={'GPU_Profile':'NativeGPUtimestamp table; CPUShowPlots/ResetButton/TreeabsentwithunavailableCPUsubstageexplanation.','GPU_FrameTime':'Same native timestamp table currently, noCPUhistorychart. Explicitpresentationlimitation,notCPUflamebars.'};r['full_sample_reviews_done']=0;r['full_sample_reviews_left']=165;(D/'navigation-metrics-acceptance.json').write_text(json.dumps(r,indent=2)+'\n')
im=Image.new('RGB',(2400,1770),(25,25,25));draw=ImageDraw.Draw(im)
for col,label in enumerate(labels):
 for row,stage in enumerate(['profile','frame-time','controls','about','hidden','restored']):
  pic=Image.open(images[label][stage]);pic.thumbnail((480,270));im.paste(pic,(col*480,row*295+25));draw.text((col*480+5,row*295+5),label+' / '+stage,fill='white')
im.save(D/'navigation-metrics-grid.jpg',quality=92)
a=json.loads((D/'issues.json').read_text());a.extend([{'id':'UI04','status':'CONFIRMED_UNFIXED','defect':'CPUProfile firstcolumn clips long indented labels, includingconstraints; rowmouseinteractionworks afterproperone-detentscroll.','cause':'Fixed8fontSize firstcolumn cannotfit someindentedTree labels; exacttextboundsneedsrepair-stage trace.','affected':'RealCPUsharedProfile table; GPU/combinedProfiletimestampviewdoesnotshowtheseCPUtreecontrols. SourceapplicabilityallCPUHasMetrics scenes, actualSingleBoxconfirmed.','evidence':['UI04.md','raw/metrics-tree-control-runs/one-detent/plots.png','navigation-metrics-acceptance.json'],'next':'Do notfixuntilfullwidgetreview; traceactualTree textoffset/width/clip atsmallestoperation.'},{'id':'UI05','status':'CONFIRMED_UNFIXED','defect':'HiddenUI pausedHUD andsampletext overlap at3fontSize Y; combinedComparisonViewcheckboxstaysvisibleandcoversfooter.','cause':'Sample::ResetText hiddenstart=3fontSize equalsDrawHud pausedrow3fontSize; generatedcombinedComparisonView drawnunconditionallybeforeDrawUI hiddencheck.','affected':'All5routes hidden+paused sampletext, actualSingleBox; bothcombined toggleelementshownwhilehidden acrosssharedhost.','evidence':['UI05.md','navigation-metrics-grid.jpg','navigation-metrics-acceptance.json'],'next':'Recordallaffectedsourcegroups; fixafterfullreview, traceactualText drawcoordinates andComparisonView showUIcondition.'}]);(D/'issues.json').write_text(json.dumps(a,indent=2)+'\n')
a=json.loads((D/'inventory.json').read_text());fields=['real_cpu','ordinary_gpu','native_gpu','ordinary_combined','native_combined'];labels_done=['Profile','Reset','Show plots','Renderer','Frame Time','Reset Profile','Help','Controls','About','Solver']
for w in a['shared_widget_calls']:
 if w['label']in labels_done or w['kind']=='Shortcut':
  w['status']='REVIEWED_SHARED_NAVIGATION_METRICS';w['evidence']='navigation-metrics-acceptance.json'
  for f in fields:w[f]='CHECKED_ACTUAL_INPUT_AND_EFFECT'
  if w['label']in ['Reset','Show plots']:
   for f in fields[1:]:w[f]='EXPLICIT_NOT_AVAILABLE_NATIVE_GPU_PROFILE'
  if w['label']=='Frame Time':w['GPU_presentation']='ActualnativeGPUtimestamps, sameasProfile/nohistorychart; CPUchartverified.'
 if w['kind']=='TreeNodeEx'and w['label']is None:
  w['status']='REVIEWED_DEFECT_UI04';w['evidence']='navigation-metrics-acceptance.json';w['real_cpu']='INTERACTION_CHECKED_LABEL_CLIPPED_UI04'
  for f in fields[1:]:w[f]='EXPLICIT_NOT_AVAILABLE_NATIVE_GPU_PROFILE'
 if w['label']=='Hide UI':
  w['status']='REVIEWED_DEFECT_UI05';w['evidence']='navigation-metrics-acceptance.json'
  for f in fields:w[f]='UI_HIDES_RESTORES_BUT_HUD_OVERLAP_UI05'
a['counts']['shared_calls_dispositioned']=sum(x['status']in ['VERIFIED_SHARED_IMPLEMENTATION_APPLICABLE_ROUTES','CHECKED_SHARED_INPUT_AND_EFFECT','CHECKED_SHARED_NAVIGATION','REVIEWED_DEFECT_UI03','CHECKED_SHARED_RENDER_BOUNDARY','REVIEWED_SHARED_NAVIGATION_METRICS','REVIEWED_DEFECT_UI04','REVIEWED_DEFECT_UI05']for x in a['shared_widget_calls']);a['counts']['shared_calls_without_complete_disposition']=85-a['counts']['shared_calls_dispositioned'];(D/'inventory.json').write_text(json.dumps(a,indent=2)+'\n')
removed=[]
for folder in ['navigation-metrics-runs','metrics-tree-control-runs']:
 for p in (D/'raw'/folder).glob('*/*'):
  if p.name in ['observer.jsonl','api.jsonl']:removed.append({'path':str(p.relative_to(D)),'bytes':p.stat().st_size});p.unlink()
(D/'navigation-metrics-cleanup.json').write_text(json.dumps({'removed':removed,'deleted_bytes':sum(x['bytes']for x in removed),'retained_consumer':'Onecurrentrender-popup5viewerpool andobjects: remainingPR10; images/actualwidget/windowstate/receiptseffects retained,sharedcaches preserved.','free_bytes':shutil.disk_usage(D).free},indent=2)+'\n');print('reviewed',a['counts']['shared_calls_dispositioned'],'left',a['counts']['shared_calls_without_complete_disposition'],'open_issues3','deleted',sum(x['bytes']for x in removed))
