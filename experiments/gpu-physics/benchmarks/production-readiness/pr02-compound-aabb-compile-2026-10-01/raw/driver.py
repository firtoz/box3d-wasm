from pathlib import Path
import subprocess,json,os
out=Path('artifacts/production-readiness/pr02-compound-aabb-compile')
r={'status':'building','builds':[]}
def save():(out/'driver-receipt.json').write_text(json.dumps(r,indent=2)+'\n')
save()
for backend in ['ordinary','native']:
 cmd=['python3',str(out/'freeze.py'),backend]
 proc=subprocess.Popen(cmd);r['running']={'pid':proc.pid,'backend':backend};save();code=proc.wait();r.pop('running');r['builds'].append({'backend':backend,'command':cmd,'exit':code});save()
 if code:r['status']='failed';save();raise SystemExit(code)
r['status']='compiled';save()
