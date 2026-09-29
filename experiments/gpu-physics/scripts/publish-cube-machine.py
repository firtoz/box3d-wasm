#!/usr/bin/env python3
"""Add verified raw sweep batches to one machine dataset; reruns cannot replace trials."""
import argparse
import csv
import datetime
import gzip
import json
from pathlib import Path
import re
from cube_machine_data import ROOT, MODES, digest, protocol, read_dataset, validate, summaries

p=argparse.ArgumentParser(description=__doc__)
p.add_argument('inputs', type=Path, nargs='+')
p.add_argument('--machine-id', required=True)
p.add_argument('--label', required=True, help='Human-readable CPU/GPU machine label')
p.add_argument('--output-dir', type=Path, default=ROOT/'benchmarks/machines')
a=p.parse_args()
if not re.fullmatch(r'[a-z0-9][a-z0-9-]{1,79}',a.machine_id):p.error('use a short lowercase machine/run identifier')
a.output_dir.mkdir(parents=True,exist_ok=True)
path=a.output_dir/(a.machine_id+'.json')
if path.exists():
    data,bundle=read_dataset(path)
    assert data['label']==a.label, 'machine label changed; use a new machine/run ID'
else:
    data=dict(schema='cube-machine-v1',machine_id=a.machine_id,label=a.label,batches={},trials=[],stopped={})
    bundle={'files':{}}
for folder in a.inputs:
    manifest=json.loads((folder/'manifest.json').read_text())
    proto=protocol(manifest)
    if 'protocol' in data:assert data['protocol']==proto, 'protocol changed; use a new machine/run ID'
    else:data['protocol']=proto
    # Keep portable build/configuration evidence; omit absolute binary paths and GPU UUIDs.
    batch={k:manifest[k] for k in ['workload','platform','cpu','git','diff_sha256','environment']}
    gpu_rows=list(csv.DictReader(manifest.get('gpu','').splitlines(),skipinitialspace=True))
    batch['gpu_hardware']=[{k:v for k,v in row.items() if k and 'uuid' not in k.lower()} for row in gpu_rows]
    batch['arguments']={k:v for k,v in manifest['arguments'].items() if k not in ['output','gpu_binary','sokol_gpu_binary','resume','require_idle']}
    batch['binaries']={k:{'name':Path(v['path']).name,'sha256':v['sha256']} for k,v in manifest['binaries'].items()}
    key=digest(json.dumps(batch,sort_keys=True).encode())[:16]
    data['batches'][key]=batch
    data['stopped'].update(json.loads((folder/'stopped.json').read_text()))
    for original in json.loads((folder/'trials.json').read_text()):
        row={k:original[k] for k in ['count','mode','trial','status','mean_ms','p50_ms','p95_ms','framebuffer','adapter','contacts','error'] if k in original}
        row['batch']=key
        row['system_samples']={key:original[key] for key in ['before','after','idle','load_gate_enabled'] if key in original}
        stem=f"{row['count']}-{row['mode']}-{row['trial']}"
        for suffix in ['.json','.cadence.json','.log']:
            raw=folder/(stem+suffix)
            if raw.exists():
                text=raw.read_text()
                if raw.name in bundle['files']:assert bundle['files'][raw.name]==text, 'refusing to replace raw evidence'
                bundle['files'][raw.name]=text
        if row['status']=='ok':
            row['raw_file']=stem+'.json'
            row['raw_sha256']=digest(bundle['files'][row['raw_file']].encode())
        identity=(row['count'],row['mode'],row['trial'])
        previous=next((r for r in data['trials'] if (r['count'],r['mode'],r['trial'])==identity),None)
        if previous is not None:assert previous==row, 'refusing to replace an existing trial; use a new run ID'
        else:data['trials'].append(row)
validate(data,bundle)
data['trials'].sort(key=lambda r:(r['count'],r['mode'],r['trial']))
data['updated_utc']=datetime.datetime.now(datetime.timezone.utc).isoformat()
data['status']='complete' if len(summaries(data))==len(MODES)*len(data['protocol']['counts']) else 'partial'
raw=gzip.compress((json.dumps(bundle,sort_keys=True,separators=(',',':'))+'\n').encode(),mtime=0)
raw_path=path.with_name(path.stem+'.'+digest(raw)[:12]+'.raw.json.gz')
data['raw_bundle']={'file':raw_path.name,'sha256':digest(raw),'bytes':len(raw)}
raw_path.write_bytes(raw)
temporary=path.with_suffix('.json.tmp')
temporary.write_text(json.dumps(data,indent=2)+'\n')
temporary.replace(path)
print(path, data['status'], f"{len(data['trials'])} trials; raw bundle {len(raw)/1024:.1f} KiB")
