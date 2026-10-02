#!/usr/bin/env python3
from pathlib import Path
import hashlib,json,tarfile,shlex
B=Path(__file__).resolve().parent
def read(n):return json.loads((B/n).read_text())
def sha(f):return hashlib.sha256(Path(f).read_bytes()).hexdigest()
for n,v in read('raw-index.json').items():assert sha(B/n)==v['sha256'] and (B/n).stat().st_size==v['bytes'],n
p=read('raw/protocol.json');r=read('raw/receipt.json');assert r['status']=='built' and not r.get('running')
assert p['budget']=={'viewer_relinks':4,'compiler_translation_units':0,'Rust_builds':0,'engine_processes':0,'retries':0,'headline_timing':0}
assert r['protocol_sha256']==sha(B/'raw/protocol.json') and r['driver_sha256']==sha(B/'raw/run.py')
refs=read('reference-map.json')
for key in ['candidate_Rust_producer','regression','parent']:assert sha(B/refs[p[key+'_receipt']]['portable'])==p[key+'_receipt_sha256']
provider=read('raw/candidate_Rust_producer-receipt.json');assert provider['all_original_assertions_pass'] and provider['compiled_inputs_before']==provider['compiled_inputs_after']
with tarfile.open(B/'raw/consumer-inputs.tar.gz') as t:
 for n,h in p['CPP_consumer_inputs'].items():assert hashlib.sha256(t.extractfile(n.replace('../../box3d/','box3d/')).read()).hexdigest()==h,n
source_hashes=set()
for archive in (B/'raw').rglob('*.tar.gz'):
 with tarfile.open(archive) as t:
  for m in t.getmembers():
   if m.isfile():source_hashes.add(hashlib.sha256(t.extractfile(m).read()).hexdigest())
assert all(u['source_sha256'] in source_hashes for c in p['cases'] for u in c['compiled_units'])
assert len(p['CPP_consumer_inputs'])==614 and len(r['results'])==4
for row,c in zip(r['results'],p['cases'],strict=True):
 assert row['exit']==0 and row['command']==c['command'] and row['cwd']==c['cwd']
 folder=B/'raw'/c['cell'];v=json.loads((folder/'receipt.json').read_text());orig=json.loads((folder/'original-receipt.json').read_text())
 assert v['compiled_units']==c['compiled_units']==orig['compiled_units']
 assert v['inputs_before']==v['inputs_after']==p['CPP_consumer_inputs'] and v['status']=='built'
 assert v['candidate_library_sha256']==provider['libraries'][c['backend']]['sha256']==c['candidate_library']['sha256']
 assert v['linked_inputs']==c['linked_inputs'] and sha(folder/'original-receipt.json')==c['consumer_receipt_sha256']
 assert row['stdout_sha256']==sha(folder/'stdout.log') and row['stderr_sha256']==sha(folder/'stderr.log')
 assert v['executable_sha256']==r['viewers'][c['cell']]['executable_sha256'] and sha(folder/'receipt.json')==r['viewers'][c['cell']]['receipt_sha256']
 old=shlex.split(orig['link_command']);new=[x for x in old if not x.startswith('-Wl,--dependency-file=')];lib=next(x for x in new if 'frozen.a' in x);new[new.index(lib)]=c['candidate_library']['path'];new[new.index('-o')+1]=c['binary'];assert new==v['command']
 # Preserve all actually reused archives other than the intentionally replaced
 # Rust provider. Their original identity remains in the consumer receipt.
 for n,h in orig['linked_archives'].items():
  if n in c['linked_inputs']:assert c['linked_inputs'][n]==h,n
  else:assert Path(n).name not in [Path(x).name for x in old if x.endswith('.a')],n
print('PASS:28indexed rawfiles;614CPP inputs;four artifact relinks with exact original124/178Cobjects and candidateRust hashes. Zero compiler/device/timing runs;clean builds remain PR06.')
