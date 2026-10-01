from pathlib import Path
import hashlib,json,subprocess,tarfile,os
r=Path.cwd();a=r/'artifacts/production-readiness/pr02-compound-aabb-captures';sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
inputs=[];repos=[]
for dep in ['imgui','implot','nfd']:
 folder=r.parents[1]/'box3d/.fetchcontent-cache'/f'{dep}-src'
 dirty=subprocess.check_output(['git','-C',str(folder),'status','--porcelain'],text=True);assert not dirty,dirty
 repos.append({'name':dep,'revision':subprocess.check_output(['git','-C',str(folder),'rev-parse','HEAD'],text=True).strip(),'clean_status':dirty})
 inputs.extend(p for p in folder.rglob('*')if p.is_file()and p.suffix in ['.cpp','.c','.h','.hpp','.inl']and '.git'not in p.parts)
proof={'purpose':'supplement portable viewer source archives with third-party compiled translation units and clean pinned dependency trees; post-build source audit, not a new build or GPU trial','dependencies':repos,'files':{os.path.relpath(p,r):sha(p)for p in sorted(inputs)},'matching_compiled_units':{}}
for kind,b in [('cpu',a/'cpu-viewer/receipt.json'),('gpu',r/'artifacts/production-readiness/pr02-compound-aabb-validation/native-viewer/receipt.json')]:
 d=json.loads(b.read_text());matches=[]
 for unit in d['compiled_units']:
  if '.fetchcontent-cache'in unit['file']:
   assert sha(Path(unit['file']))==unit['source_sha256'];matches.append(unit)
 proof['matching_compiled_units'][kind]=matches
with tarfile.open(a/'viewer-dependencies.tar.gz','w:gz')as t:
 for p in sorted(inputs):t.add(p,arcname='box3d/'+str(p.relative_to(r.parents[1]/'box3d')))
proof['archive_sha256']=sha(a/'viewer-dependencies.tar.gz')
(a/'viewer-dependencies.json').write_text(json.dumps(proof,indent=2)+'\n');print('Supplemented exact compiled source bytes for nine third-party units per viewer; dependency header trees are a post-build audit.')
