from pathlib import Path
from atomic_io import atomic_json
import json,threading,time,hashlib
p=Path(__file__).resolve().parent;target=p/'host-publication.json';atomic_json(target,{'index':0,'payload':'x'*1000});done=threading.Event();errors=[];reads=[0]
def reader():
 while not done.is_set():
  try:
   obj=json.loads(target.read_text());assert obj['payload']=='x'*1000 and 0<=obj['index']<=500;reads[0]+=1
  except Exception as error:errors.append(repr(error))
t=threading.Thread(target=reader);t.start()
for i in range(1,501):atomic_json(target,{'index':i,'payload':'x'*1000})
done.set();t.join();assert not errors and reads[0]>0 and json.loads(target.read_text())['index']==500
r={'status':'pass','atomic_writes':501,'concurrent_reads':reads[0],'errors':errors,'protocol_sha256':hashlib.sha256((p/'protocol-before-runs.json').read_bytes()).hexdigest(),'helper_sha256':hashlib.sha256((p/'atomic_io.py').read_bytes()).hexdigest(),'scope':'Filesystem JSON publication only; no viewer/GPU/physics/timing test'};atomic_json(p/'host-receipt.json',r);print(r)
