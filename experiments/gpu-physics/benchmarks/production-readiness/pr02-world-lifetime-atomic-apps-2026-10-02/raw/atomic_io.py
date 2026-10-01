from pathlib import Path
import json,os
def atomic_json(path,data):
 path=Path(path);temporary=path.with_name(path.name+"."+str(os.getpid())+".tmp");temporary.write_text(json.dumps(data,indent=2)+"\n");os.replace(temporary,path)
