from pathlib import Path
import re,json,hashlib
R=Path(__file__).resolve().parents[6]; E=R/'experiments/gpu-physics'; O=Path(__file__).resolve().parents[1]
# The actual CMake sample list is authoritative, including generated copies.
cm=(E/'native-samples/CMakeLists.txt').read_text();listed=set(re.findall(r'BOX3D_DIR\}/samples/(sample_[^"/]+\.cpp)',cm))|{'sample.cpp','sample_character.cpp','sample_continuous.cpp','sample_joint.cpp'}
def clean(s):
 s=re.sub(r'//[^\n]*|/\*.*?\*/',lambda m:'\n'*m.group(0).count('\n'),s,flags=re.S)
 return re.sub(r'#if 0\b.*?#endif',lambda m:'\n'*m.group(0).count('\n'),s,flags=re.S)
def norm(s):return re.sub('[^a-z0-9]','',s.lower())
rows=[];inputs={}
for path in sorted([R/'box3d/samples'/x for x in listed]+list((E/'native-samples').glob('sample*.cpp'))):
 s=clean(path.read_text());inputs[str(path.relative_to(R))]=hashlib.sha256(path.read_bytes()).hexdigest()
 for m in re.finditer(r'Register(?:Sample|ReplayViewer|Replay)\(\s*"([^"]+)"\s*,\s*"([^"]+)"\s*,\s*([^;]+?)\s*\)',s):
  cat,name,fcn=m.groups();prefix=s[:m.start()];classes=list(re.finditer(r'class\s+(\w+)\s*:',prefix));body=prefix[classes[-1].start():] if classes else prefix
  feats=sorted(set(re.findall(r'b3\w+\(',body))); feats=[x[:-1] for x in feats];floor=[x for x in feats if any(z in x for z in ['Compound','MeshShape','HeightFieldShape','HullShape'])]
  excluded=[x for x in feats if x.startswith(('b3Rec','b3World_SetTask','b3World_GetTask','b3World_RebuildStatic'))]
  rows.append({'cpu_identity':cat+'/'+name,'class':fcn.split('::')[0].strip(),'source':str(path.relative_to(R)), 'registration_line':s[:m.start()].count('\n')+1,'gpu_counterpart':cat+'/'+name,'availability':'registered-shared-scene; runtime unqualified' if not excluded else 'registered but uses excluded API: '+', '.join(excluded),'disposition':'PR09 runtime review pending' if not excluded else 'PR02 excluded native recording/player/static-tree operations; explicit unavailable gap, review UI at PR10','floor_source_calls':floor,'scene_settings':'unchanged upstream constructor/defaults; runtime settings not reviewed','interactions':'constructor/UI/Step source: '+', '.join(x for x in feats if any(y in x for y in ['Cast','Overlap','Motor','Force','Explosion'])),'ordinary_review':'UNREVIEWED','native_review':'UNREVIEWED','combined_review':'UNREVIEWED','behavior':'UNREVIEWED','performance':'UNREVIEWED; no timing run','evidence':[]})
# Reconcile every browser registration, rather than trusting docs sample counts.
index=R/'demo/src/samples/index.ts';s=index.read_text();imports={symbol.strip():(index.parent/(rel+'.ts')).resolve() for names,rel in re.findall(r'import\s*\{([^}]+)\}\s*from\s*"(\.[^"]+)"',s) for symbol in names.split(',')};arr=s.split('export const samples = [',1)[1].split(']',1)[0];symbols=re.findall(r'\b(\w+Sample)\b',arr);browser=[]
for sym in symbols:
 path=imports[sym];t=path.read_text();pair=re.search(r'(?:createGenericSample|createShaderInstancedSample)\(\s*"([^"]+)"\s*,\s*"([^"]+)"',t)
 if not pair:pair=re.search(r'id:\s*"([^"]+)".*?(?:name|label):\s*"([^"]+)"',t,re.S)
 identity,title=pair.groups() if pair else (None,None)
 if not title:
  title_match=re.search(r'(?:name|label):\s*"([^"]+)"',t);title=title_match.group(1) if title_match else sym
 if not identity:
  id_match=re.search(r'id:\s*"([^"]+)"',t);identity=id_match.group(1) if id_match else sym
 cpp=[]
 for rel in re.findall(r'from\s*"(\.[^"]+scene)"',t):
  scene=(path.parent/(rel+'.ts')).resolve()
  if scene.exists():cpp+=re.findall(r'dumpCppSampleName\s*=\s*"([^"]+)"',scene.read_text())
 matches=[r['cpu_identity'] for r in rows if norm(title)==norm(r['cpu_identity']) or (r['class'] in cpp or r['cpu_identity'].split('/',1)[1] in cpp)]
 if sym.endswith('Sample') and '/manifold/' in str(path):
  scene=path.with_name(path.stem+'-scene.ts').read_text();pair=re.search(r'id:\s*"([^"]+)".*?name:\s*"([^"]+)"',scene,re.S)
  if pair:identity,title=pair.groups()
  matches=[r['cpu_identity'] for r in rows if norm(title)==norm(r['cpu_identity'])]
 if sym in ['dominoesSample','createDominoesSample']:
  identity='dominoes' if sym=='dominoesSample' else 'dominoes-2x';title='Stacking / Dominoes' if sym=='dominoesSample' else 'Bench / Dominoes 2×';matches=['Stacking/Dominoes']
 if sym=='createWasherSample': identity='washer';title='Benchmark / Washer';matches=[r['cpu_identity'] for r in rows if r['cpu_identity'].endswith('/Washer')]
 browser.append({'symbol':sym,'id':identity,'title':title,'source':str(path.relative_to(R)),'native_cpu_matches':matches,'gpu_counterpart':matches or None,'review':'UNREVIEWED browser; native review recorded separately','disposition':'shared native scene available, runtime unqualified' if matches else 'browser-only custom scene; no native CPU/GPU viewer registration. Native port gap at PR09; browser/WASM outside initial native release'})
# Every registered CPU scene has a row, including duplicates disambiguated by source.
assert len(symbols)==len(browser) and len(set(symbols))==len(symbols)
report={'scope':'Static exhaustive native registrations from actual CMake scene inputs + full registered CPU browser catalog. Registration proves selectable counterpart, not feature correctness. No sample equivalence from UNREVIEWED.', 'native_registration_count':len(rows),'unique_native_identities':len(set(r['cpu_identity'] for r in rows)),'browser_count':len(browser),'source_sha256':inputs,'native':sorted(rows,key=lambda x:(x['cpu_identity'],x['source'])),'browser':browser}
oracle=R/'experiments/gpu-physics/oracle/oracle.cpp';oracle_text=oracle.read_text().split('static const char* kScenes[] = {',1)[1].split('};',1)[0];fixtures=re.findall(r'"([^"]+)"',oracle_text)
report['oracle_cpu_fixtures']=[{'cpu_identity':'oracle/'+n,'source':'experiments/gpu-physics/oracle/oracle.cpp','gpu_counterpart':'Rust demo --scene '+n,'review':'UNREVIEWED in this native-viewer milestone','disposition':'Existing short fixture catalog; retained prior evidence is historical, not a new PR09 sweep pass'} for n in fixtures]
report['oracle_fixture_count']=len(fixtures)
report['source_sha256']['demo/src/samples/index.ts']=hashlib.sha256(index.read_bytes()).hexdigest()
report['source_sha256']['experiments/gpu-physics/native-samples/CMakeLists.txt']=hashlib.sha256((E/'native-samples/CMakeLists.txt').read_bytes()).hexdigest()
report['source_sha256']['experiments/gpu-physics/oracle/oracle.cpp']=hashlib.sha256(oracle.read_bytes()).hexdigest()
(O/'inventory.json').write_text(json.dumps(report,indent=2)+'\n');print(len(rows),'native rows',len(browser),'browser',sum(bool(b['native_cpu_matches']) for b in browser),'matched')
