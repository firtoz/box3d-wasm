import {D,read,write,sha} from './review-closeout';
import {join} from 'node:path';
import {readFileSync,writeFileSync,appendFileSync,existsSync} from 'node:fs';
import assert from 'node:assert/strict';
assert(!existsSync(join(D,'mutation-force-build-budget.json')));
write('mutation-force-build-budget.json',{status:'FROZEN',cpp_compiles:1,compile_seconds:30,links:5,link_seconds_each:25,apps:0,retries:0,temporary_bytes:230686720,retained_metadata_bytes:8388608,log_bytes_each:1048576,min_free_bytes:21474836480,scope:'Diagnostic summary mode for native control review. Suppress verbose per-body mutation events, retire destroyed observations immediately, retain full aggregate counts/IDs/filter inputs and actual getters, bounded examples. Original operations forward once. No engine/Box3D/WASM source changes.',outputs:[{path:'raw/mutation-force-build',consumer:'Remaining mutation/force controls, then Replay review and repair qualification',delete:'Retire previous cast pool after new five routes qualify; current pool at qualified replacement or goal completion'}]});
let body=readFileSync(join(D,'mesh-controls-body.inc.cpp'),'utf8');
body='extern "C" b3Filter cpu_b3Shape_GetFilter(b3ShapeId) __attribute__((weak));\n'+body;
body=body.replace('int vertices;};','int vertices;b3Filter filter;};');
body=body.replace('radius,vertices});','radius,vertices,def->filter});');
body=body.replace('if(!mutation_enabled())return;if(const char* path=', 'if(!mutation_enabled()||std::getenv("GPU_PR10_MUTATION_SUMMARY"))return;if(const char* path=');
body=body.replace('n>=64','n>=(std::getenv("GPU_PR10_MUTATION_SUMMARY")?8:64)');
const marker='mutation_event(#NAME,"after",{id,CPU});}';
assert(body.includes(marker));
body=body.replace(marker,'if(std::getenv("GPU_PR10_MUTATION_SUMMARY"))mutation_bodies.erase(std::remove_if(mutation_bodies.begin(),mutation_bodies.end(),[&](auto& b){return same_body(b.ref,{id,CPU});}),mutation_bodies.end());mutation_event(#NAME,"after",{id,CPU});}');
const point='mesh_shape_details(f,b.ref.cpu,s.id,type);';
assert(body.includes(point));
body=body.replace(point,'auto filter=b.ref.cpu?cpu_b3Shape_GetFilter(s.id):b3Shape_GetFilter(s.id);std::fprintf(f,",\\\"definition_filter\\\":[%llu,%llu,%d],\\\"actual_filter\\\":[%llu,%llu,%d]",(unsigned long long)s.filter.categoryBits,(unsigned long long)s.filter.maskBits,s.filter.groupIndex,(unsigned long long)filter.categoryBits,(unsigned long long)filter.maskBits,filter.groupIndex);'+point);
const aggregate='std::fputs("]}",f);\n}\nvoid mutation_event';
assert(body.includes(aggregate));
body=body.replace(aggregate,`std::fputs("],\\\"shape_totals\\\":[",f);int types[5]={};unsigned filters_match=0,filter_total=0;bool first=true;for(auto& b:mutation_bodies){if(b.ref.cpu!=cpu||!body_valid(b.ref))continue;for(auto s:b.shapes){int type=cpu?cpu_b3Shape_GetType(s.id):b3Shape_GetType(s.id);if(type>=0&&type<5)++types[type];auto filter=cpu?cpu_b3Shape_GetFilter(s.id):b3Shape_GetFilter(s.id);++filter_total;if(filter.categoryBits==s.filter.categoryBits&&filter.maskBits==s.filter.maskBits&&filter.groupIndex==s.filter.groupIndex)++filters_match;}}for(int i=0;i<5;++i){if(i)std::fputc(',',f);std::fprintf(f,"%d",types[i]);}std::fprintf(f,"],\\\"filter_count\\\":%u,\\\"filters_match_definition\\\":%u}",filter_total,filters_match);\n}\nvoid mutation_event`);
writeFileSync(join(D,'mutation-summary-body.inc.cpp'),body);
let script=readFileSync(join(D,'build-cast-controls.ts'),'utf8')
.replaceAll('cast-controls-build-budget.json','mutation-force-build-budget.json')
.replace("base=read('plane-controls-build.json')","base=read('cast-controls-build.json')")
.replaceAll("'raw/cast-controls-build'","'raw/mutation-force-build'")
.replaceAll("'cast-controls-observer.cpp'","'mutation-force-observer.cpp'")
.replaceAll("'plane-controls-observer.cpp'","'cast-controls-observer.cpp'")
.replaceAll('raw/plane-controls-build/','raw/cast-controls-build/')
.replaceAll("join(D,'cast-controls-build.json'),JSON.stringify","join(D,'mutation-force-build.json'),JSON.stringify");
const begin=script.indexOf('let s=readFileSync'),end=script.indexOf('const extra=',begin);
assert(begin>=0&&end>begin);
script=script.slice(0,begin)+`let s=readFileSync(join(D,'cast-controls-observer.cpp'),'utf8').replace('#include "mesh-controls-body.inc.cpp"','#include "mutation-summary-body.inc.cpp"');writeFileSync(src,s);\n`+script.slice(end);
const incBegin=script.indexOf('const extra='),incEnd=script.indexOf('save();for(const[p,h]',incBegin);
assert(incBegin>=0&&incEnd>incBegin);
script=script.slice(0,incBegin)+`const extra=[],inc={...base.include_pins,[join(D,'mutation-summary-body.inc.cpp')]:sha(join(D,'mutation-summary-body.inc.cpp'))},m:any={status:'BUILDING',source_sha256:sha(src),include_pins:inc,input_pins:base.input_pins,omitted_diagnostic_input:base.omitted_diagnostic_input,operations:[],viewers:[]},save=()=>writeFileSync(join(D,'mutation-force-build.json'),JSON.stringify(m,null,2)+'\\n');delete m.include_pins[join(D,'mesh-controls-body.inc.cpp')];`+script.slice(incEnd);
writeFileSync(join(D,'build-mutation-force.ts'),script);
appendFileSync(join(D,'log.md'),'\nMutation/force observer build FROZEN: one C++ compile30s/five links25s,230MiB/log1MiB,0 engine builds/apps/retries. Summary-only observation forwards every original call once; filters and counts retained for all bodies,8 examples; destroy records retired immediately. No product fix. Counts297/45widgets,160/5samples,22/1custom,26open/2verified.\n');
appendFileSync('docs/goals/gpu-production-readiness.md','\nPR10 mutation/force diagnostic build budget FROZEN:1CPP30s/5links25s/230MiB/0apps/enginebuild/retries. Summary mutation observer/all-body filters/bounded examples; native original calls unchanged. Current297widgets/45left,160samples/5left,22custom/1left,26open/2verified.\n');
console.log('FROZEN diagnostic build; source helpers prepared');
