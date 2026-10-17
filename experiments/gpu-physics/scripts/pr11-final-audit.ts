import {resolve} from "node:path";
import {createHash} from "node:crypto";
import {readdir,lstat,mkdir,copyFile,rm} from "node:fs/promises";
import assert from "node:assert/strict";
const root=resolve("experiments/gpu-physics"),dir=resolve(root,"benchmarks/production-readiness/pr11-queries-2026-10-17"),raw=resolve(dir,"raw");
const json=(name:string)=>Bun.file(resolve(dir,name)).json();
const sha=async(path:string)=>createHash("sha256").update(new Uint8Array(await Bun.file(path).arrayBuffer())).digest("hex");
async function bytes(path:string):Promise<number>{const s=await lstat(path);if(s.isSymbolicLink())return 0;if(!s.isDirectory())return s.size;let n=0;for(const f of await readdir(path))n+=await bytes(resolve(path,f));return n;}
async function command(args:string[]){const p=Bun.spawn(["timeout","-k","2s","20s",...args],{stdout:"pipe",stderr:"pipe"});const text=await new Response(p.stdout).text(),code=await p.exited;assert.equal(code,0);return text.trim();}
const result:any={status:"AUDITING",gates:[],scope:"PR11 only",limits:{temporary:1024**3,retained:64*1024**2,originalNumericTolerance:1e-5},tests:{},hashes:{}};
for(const file of ["repaired-comparison.json","contract-comparison.json","encroach-comparison.json"]){const reports=await json(file);assert.equal(reports.length,2);for(const r of reports){assert.deepEqual(r.errors,[]);assert(r.maximum<=1e-5);}result.tests[file]=reports;}
assert.equal((await json("callbacks-result.json")).status,"PASS");
for(const file of ["facet-oracle-comparison.json","vertex-order-comparison.json"]){const r=await json(file);assert.deepEqual(r.errors,[]);assert(r.max<=1e-5);result.tests[file]=r;}
const cpu=await json("heightfield-cpu.json"),ordinary=await json("heightfield-ordinary.json"),native=await json("heightfield-native.json");
assert.deepEqual(ordinary,native);assert.equal(cpu.length,27618);assert.equal(ordinary.length,cpu.length);
const summary=(rows:any[])=>rows.filter(r=>r.kind==="summary");assert.equal(summary(cpu).length,14278);assert.deepEqual(summary(cpu),summary(ordinary));
const hits=(rows:any[])=>rows.filter(r=>r.kind==="cast");assert.equal(hits(cpu).length,13340);assert(hits(ordinary).every(r=>r.identity===1));
let maxFraction=0;for(let i=0;i<cpu.length;i++){assert.equal(cpu[i].case,ordinary[i].case);if(cpu[i].kind==="cast")maxFraction=Math.max(maxFraction,Math.abs(cpu[i].fraction-ordinary[i].fraction));}
const maxCenterTravel=maxFraction*Math.hypot(80000,-80000,8);assert(maxCenterTravel<=1e-5);
result.tests.heightfield={inputs:14278,hits:13340,backendsIdentical:true,maxFraction,maxCenterTravel,primitiveOracleCases:95,adjacentFacetCases:93,cyclicVertexAnchorCases:2,wholeGridPointNormalIdentity:false};
for(const file of ["heightfield-cpu.json","heightfield-ordinary.json","heightfield-native.json","heightfield-cpu-reference.json"])result.hashes[file]=await sha(resolve(dir,file));
const ids=await json("encroach-identities.json");for(const path of [resolve(root,"src/api/query.rs"),resolve(root,"c_abi/shim.c"),resolve(root,"c_abi/pr11_query_reference.cpp"),resolve(root,"target/samples/release/libgpu_physics.a"),resolve(root,"target/native-cache-build/release/libgpu_physics.a")])assert.equal(await sha(path),ids[path]);
result.sourceLibraryIdentities=ids;
for(const [log,count]of [["B5-query-tests.log",25],["B7-native-query-tests.log",25]]as const){const text=await Bun.file(resolve(raw,log)).text();assert(text.includes(`test result: ok. ${count} passed; 0 failed`));result.tests[log]={passed:count,failed:0};}
assert.equal((await Bun.file(resolve(raw,"B7-diagnostic-typecheck-corrected.log")).text()).trim(),"");
assert(!(await Bun.file(resolve(raw,"B7-workspace-typecheck.log")).text()).includes("error TS"));
const media=await json("media.json");for(const row of media.after)assert.equal(await sha(resolve(dir,row.published)),row.sha256);for(const path of media.before)assert(await Bun.file(resolve(dir,path)).exists());
for(const route of ["ordinary-both","native-both"]){const r=await json(`${route}-visual-result.json`);assert.equal(r.status,"PASS_CONTROLS");assert.equal(r.app_terminal_exit,0);assert.equal(r.findings.length,0);assert(Object.values(r.checks).every(Boolean));assert.equal(Object.keys(r.checks).length,14);try{process.kill(r.app_pid,0);throw Error(`Owned viewer still live: ${r.app_pid}`);}catch(e:any){assert.equal(e.code,"ESRCH");}}
assert.equal(await command(["git","-C","box3d","status","--porcelain"]),"");assert.equal(await command(["git","diff","--name-only","--","box3d","packages/box3d-wasm","demo"]),"");
result.git={head:await command(["git","rev-parse","HEAD"]),divergence:await command(["git","rev-list","--left-right","--count","HEAD...origin/feat/gpu"]),box3d:await command(["git","-C","box3d","rev-parse","HEAD"])};assert.equal(result.git.divergence,"0\t0");
// Source applicability, not another runtime survey. Shared defaults also apply
// to all callers using the default shape filter, including non-query samples.
const calls=["b3World_CastShape","b3Body_CastShape","b3World_OverlapShape","b3Body_OverlapShape","b3World_CastMover"];
const affected:any[]=[];for(const folder of [resolve("box3d/samples"),resolve(root,"native-samples")])for(const file of (await readdir(folder)).filter(x=>x.endsWith(".cpp"))){const source=await Bun.file(resolve(folder,file)).text();const classes=[...source.matchAll(/class\s+(\w+)\s*:\s*public\s+Sample/g)];for(let i=0;i<classes.length;i++){const c=classes[i],body=source.slice(c.index,classes[i+1]?.index??source.length),uses=calls.filter(x=>body.includes(x+"(" )||body.includes(x+" ("));if(!uses.length)continue;const registration=new RegExp('RegisterSample\\(\\s*"([^"]+)"\\s*,\\s*"([^"]+)"\\s*,\\s*'+c[1]+'::Create');const m=source.match(registration);affected.push({class:c[1],sample:m?m[1]+"/"+m[2]:"Unresolved registration; source applicability only",file,apis:uses,review:"Source applicability; shared independent primitive cases, not a new scene runtime pass"});}}
await Bun.write(resolve(dir,"affected-scenes.json"),JSON.stringify({directQueryCallers:affected,sharedDefaultFilter:"All ordinary/native sample callers using b3DefaultFilter/b3DefaultShapeDef; no-body tools without these calls unaffected. Shared masks1/2/bit63 checked; no165sample resweep.",runtimeScenes:["Collision/Initial Overlap","Collision/Shape Cast","Bodies/Cast"],frozenExactScenes:["Collision/Initial Overlap","Collision/Overlap World","Benchmark/Height Field","Bodies/Cast"]},null,2));
result.rawBytesBefore=await bytes(raw);assert(result.rawBytesBefore<1024**3);
await mkdir(resolve(dir,"checks"),{recursive:true});
for(const file of ["B5-query-tests.log","B7-native-query-tests.log","B7-diagnostic-typecheck.log","B7-diagnostic-typecheck-corrected.log","B7-workspace-typecheck.log","B2-ordinary-build.log","B2-native-build.log","B4-ordinary-build.log","B4-native-build.log","gdb.log"]){await copyFile(resolve(raw,file),resolve(dir,"checks",file));}
// Full duplicated cast outputs have been consumed into the validated compact
// CPU reference, difference/oracle inputs and hashes above.
for(const file of ["heightfield-cpu.json","heightfield-ordinary.json","heightfield-native.json"])await rm(resolve(dir,file));
await rm(raw,{recursive:true});
result.rawRemoved=true;result.retainedBytes=await bytes(dir);assert(result.retainedBytes<64*1024**2);
result.gates=["A1","A2","A3","A4","A5","A6","A7"].map(id=>({id,status:"PASS"}));result.status="PASS";
result.cleanup="Task-owned raw builds/viewers/helpers/duplicate traces removed. Shared Cargo/driver/pipeline caches preserved. No PR11 commit/push.";
await Bun.write(resolve(dir,"final-acceptance.json"),JSON.stringify(result,null,2)+"\n");console.log(JSON.stringify({status:result.status,gates:result.gates.length,rawRemovedBytes:result.rawBytesBefore,retainedBytes:result.retainedBytes}));
