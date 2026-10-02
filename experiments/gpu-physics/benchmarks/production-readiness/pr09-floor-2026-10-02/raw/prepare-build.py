from pathlib import Path
import json,shlex,hashlib,runpy,subprocess
R=Path.cwd();D=R/'benchmarks/production-readiness/pr09-floor-2026-10-02';A=R/'artifacts/production-readiness/pr09-floor';sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
# Called only after every before cell passes. No previous objects are modified.
p=json.loads((A/'before/protocol.json').read_text());state=json.loads((A/'before/receipt.json').read_text());assert state['status']=='captured' and len(state['results'])==25 and all(r['status']=='pass' for r in state['results'])
build=A/'build';build.mkdir();cases=[]
for key,viewer in {'cpu':p['viewers']['cpu'],**p['gpu_viewers']}.items():
 d=json.loads((R/viewer['receipt']).read_text());units=d['compiled_units'];unit=next(x for x in units if x['file'].endswith('/debug_adapter.c'));cwd=Path(unit['directory']).parent
 out=build/key;out.mkdir();generated=out/'gfx';runpy.run_path(str(R/'scripts/inject-sokol-capacity.py'))['generate'](R.parents[1]/'box3d/samples/gfx',generated)
 if key.endswith('both'):
  subprocess.run(['python3',str(R/'scripts/inject-sokol-split.py'),str(generated),str(out/'split')],check=True);generated=out/'split'
 cmd=shlex.split(unit['command']);cmd[cmd.index('-o')+1]=str(out/'debug_adapter.o');cmd[cmd.index('-c')+1]=str(generated/'debug_adapter.c');cmd += ['-I'+str(R.parents[1]/'box3d/samples/gfx')]
 link=d.get('command') or shlex.split(d['link_command']);link=[x for x in link if not x.startswith('-Wl,--dependency-file=')];link[link.index('-o')+1]=str(out/'viewer')
 gfx=next(x for x in link if x.endswith('libgfx.a'));link[link.index(gfx)]=str(out/'libgfx.a')
 archive=str((cwd/gfx).resolve());copycmd=['cp',archive,str(out/'libgfx.a')];replace=['ar','r',str(out/'libgfx.a'),str(out/'debug_adapter.o')]
 # Archive member name must match old member (debug_adapter.c.o), not add a duplicate.
 cmd[cmd.index('-o')+1]=str(out/'debug_adapter.c.o');replace[-1]=str(out/'debug_adapter.c.o')
 consumed={str((cwd/x).resolve()):sha(cwd/x) for x in link if x.endswith(('.a','.o')) and str(out) not in x};consumed[archive]=sha(archive)
 inputs={n:h for n,h in d['inputs_before'].items() if not n.startswith(('src/','shaders/')) and n not in ['Cargo.toml','Cargo.lock','build.rs']};inputs['scripts/inject-sokol-capacity.py']=sha(R/'scripts/inject-sokol-capacity.py');inputs['c_abi/growable_slots.h']=sha(R/'c_abi/growable_slots.h')
 cases.append({'cell':key,'cwd':str(cwd),'compile':cmd,'copy_archive':copycmd,'replace_archive':replace,'link':link,'original_receipt':viewer['receipt'],'original_receipt_sha256':sha(R/viewer['receipt']),'original_binary_sha256':viewer['binary_sha256'],'compiled_units':units,'old_adapter':unit,'linked_unchanged':consumed,'inputs':inputs,'generated_sha256':sha(generated/'debug_adapter.c')})
plan={'budget':{'viewers':5,'translation_units':5,'Rust_builds':0,'focused_test_builds':2,'focused_checks':12,'retries':0},'watchdog':180,'cases':cases,'context':'Compile only regenerated adapter, replace its gfx archive member in a copied archive, relink against all original unchanged objects/archives and actual distance-clamp Rust providers. No clean-build or physics qualification.'}
(build/'protocol.json').write_text(json.dumps(plan,indent=2)+'\n');print('build plan frozen',sha(build/'protocol.json'))
