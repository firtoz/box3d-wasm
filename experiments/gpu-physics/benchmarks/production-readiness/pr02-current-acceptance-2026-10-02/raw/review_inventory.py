from pathlib import Path
import json,re,hashlib
R=Path.cwd();A=R/'artifacts/production-readiness/pr02-current-acceptance';P=json.loads((A/'protocol-before-work.json').read_text());sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest();digest=lambda s:hashlib.sha256(s.encode()).hexdigest();old=R/'benchmarks/production-readiness/pr02-api-2026-10-01';currentaudit=R/'benchmarks/production-readiness/pr02-world-lifetime-scene-captures-2026-10-02/raw/capture/api-applicability'
def funcs(path):
 s=Path(path).read_text();out={}
 # Expand the finite BOTH_* function-generating macros for source indexing only.
 lines=s.splitlines();original_lines=len(lines)
 for lineindex,line in enumerate(lines):
  macro=re.match(r'#define (BOTH_\w+)\(([^)]*)\)',line)
  if not macro:continue
  parts=[line[macro.end():]];end=lineindex
  while lines[end].rstrip().endswith(chr(92)):
   end+=1;parts.append(lines[end])
  body='\n'.join(part.rstrip().removesuffix(chr(92)).strip() for part in parts);params=[x.strip() for x in macro.group(2).split(',')]
  for call in re.finditer(r'^'+macro.group(1)+r'\(([^)]*)\)',s,re.M):
   args=[x.strip() for x in call.group(1).split(',')];expanded=body
   for key,value in zip(params,args):expanded=re.sub(r'\b'+key+r'\b',value,expanded)
   expanded=re.sub(r'\s*##\s*','',expanded);s+='\n'+expanded+'\n'
 for m in re.finditer(r'^B3_API\s+[^;{]*?\b(b3\w+)\([^;]*?\)\s*\{',s,re.M|re.S):
  a=m.end()-1;i=a+1;level=1
  while level and i<len(s):level+=(s[i]=='{')-(s[i]=='}');i+=1
  out[m.group(1)]={'line':min(s[:m.start()].count('\n')+1,original_lines),'text':s[m.start():i]}
 return out
idx=json.loads((old/'compiled-source-index.json').read_text());historical={}
for item in idx:
 if item['receipt'] in ['closed-stderr/receipt.json','unavailable-routing/ordinary-both/receipt.json']:
  for n,d in item['compiled_source_files'].items():
   if Path(n).name in ['samples_api.c','samples_stubs.c']:
    f=old/d['portable_path'];assert sha(f)==d['sha256'];historical[(item['receipt'],Path(n).name)]=(f,funcs(f))
review={'scope':'Current-source PR02 named contract reconciliation, not full physics/release qualification','compiled_source_milestone':'abc0a54a3ce3b285c9984f2c7c6b5baefff00d1b','excluded_methods':[],'support_counts':{},'sources':{}}
for backend in ['ordinary','native']:
 audit=json.loads((currentaudit/(backend+'.json')).read_text());receipt=json.loads((currentaudit/'receipt.json').read_text());assert audit['status']=='complete' and audit['required_stateful_header_symbols']==415
 for n,h in receipt['inputs_after'].items():assert sha(R/n)==h
 rows=[]
 for cell in [backend+'-gpu',backend+'-both']:
  d=P['cells'][cell];build=Path(d['build']);providers={}
  for f in sorted((build/'api-sources').glob('*.c')):
   if f.name not in ['samples_api.c','samples_stubs.c','shim.c']:continue
   review['sources'][str(f)] = sha(f)
   for name,value in funcs(f).items():providers[name]={'source':str(f),'source_sha256':sha(f),'line':value['line'],'definition_sha256':digest(value['text']),'GPU_calls':sorted(set(re.findall(r'\bgpu_\w+(?=\s*\()',value['text'])))}
  if cell.endswith('both'):
   for f in [R/'c_abi/both_dual.c',build/'both_passthrough.c']:
    assert f.exists(),f
    review['sources'][str(f)]=sha(f)
    for name,value in funcs(f).items():providers[name]={'source':str(f),'source_sha256':sha(f),'line':value['line'],'definition_sha256':digest(value['text']),'GPU_calls':sorted(set(re.findall(r'\bgpu_\w+(?=\s*\()',value['text'])))}
  rust=R/'src/c_abi.rs';rs=rust.read_text();review['sources'][str(rust)]=sha(rust)
  for m in re.finditer(r'pub\s+(?:unsafe\s+)?extern\s+"C"\s+fn\s+(b3\w+)\s*\(',rs):
   name=m.group(1)
   if name in providers:continue
   a=rs.find('{',m.end());i=a+1;level=1
   while level and i<len(rs):level+=(rs[i]=='{')-(rs[i]=='}');i+=1
   body=rs[m.start():i];providers[name]={'source':'src/c_abi.rs','source_sha256':sha(rust),'line':rs[:m.start()].count('\n')+1,'definition_sha256':digest(body),'Rust_API_calls':sorted(set(re.findall(r'\bb3_\w+(?=\s*\()',body)))}
  # Every declared exclusion is directly supplied with an explicit error hook.
  for name in audit['declared_unavailable_symbols']:
   f=next(f for f in (build/'api-sources').glob('*.c') if name in funcs(f));cur=funcs(f)[name]['text'];assert 'gpu_native_api_unavailable(__func__)' in cur
   key=('closed-stderr/receipt.json' if cell.endswith('gpu') else 'unavailable-routing/ordinary-both/receipt.json',f.name);priorpath,prior=historical[key];previous=prior[name]['text']
   # Save's closed-stderr repair republishes the same error after fprintf.
   same=cur==previous
   if name=='b3SaveRecordingToFile' and cell.endswith('both'):
    closed=historical[('closed-stderr/receipt.json','samples_api.c')][1][name]['text'];assert cur==closed;same=True;priorpath=historical[('closed-stderr/receipt.json','samples_api.c')][0]
   assert same,(cell,name)
   review['excluded_methods'].append({'cell':cell,'symbol':name,'current_definition_sha256':digest(cur),'historical_source':str(priorpath),'historical_source_sha256':sha(priorpath),'exact_definition_match':True})
  for row in audit['initial_release_stateful_inventory']:
   name=row['symbol'];assert name in providers,(cell,name)
   rows.append({'cell':cell,**row,'declared_contract':'explicitly unavailable' if row['classification']=='declared-unavailable' else 'implemented candidate API; final physical/runtime support qualification pending PR03–PR08','implementation':providers[name]})
 review['support_counts'][backend]={'stateful_header_symbols':415,'implemented':376,'declared_unavailable':39,'final_release_qualified':False}
 (A/('api-inventory-'+backend+'.json')).write_text(json.dumps(rows,indent=2)+'\n')
review['manual_findings']=['Macro-generated BOTH_* definitions are indexed after a finite textual token-paste expansion; source line is capped at original EOF for these expansions. Source SHA and definition hash identify the source and generated body, not an independent compiler invocation.','Current portable b3Body_GetType comes from shim/gpu_b3_body_get_type; legacy samples_api two-class fallback is removed by generation, not the linked implementation.','b3GetCompoundMaterials NULL branch is only invalid/no-table input; live compound material pointer follows native offsets and property oracle passes1016 observations per cell.','Rust ids.rs *_id_is_valid helpers only test non-null representation; actual public C IsValid routes live generation/world checks in world.rs. No public lifetime claim is based on non-null helpers.','No TODO/unimplemented/todo/stub marker found in current src/api; short-function review is a lexical aid, not behavior proof. Short getter/setter delegations read/mutate live world fields or geometry/query implementations.','b3GetMaxWorldCount32 is the advertised C world limit, not an allocated-world counter. Long-running retirement/capacity/concurrency validation remains PR05.','Wheel angular-separation zero follows documented upstream fallback; no angular physical qualification inferred.','Unavailability source definitions match successful historical compiled implementations, including additional idempotent save error publication; combined error routing/current link inventories independently unchanged.','376 implemented symbols retain behavior-unqualified classification except explicitly named tested contracts; no universal Box3D API or production claim.']
(A/'source-contract-review.json').write_text(json.dumps(review,indent=2)+'\n');print('Exact current inventory written;156 excluded definitions reconcile to successful compiled controls,415 names/backend/linkage')
