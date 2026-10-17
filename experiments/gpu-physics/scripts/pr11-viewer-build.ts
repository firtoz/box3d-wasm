import {resolve,dirname} from "node:path";
import {createHash} from "node:crypto";
import {mkdir,copyFile} from "node:fs/promises";
const root=resolve("experiments/gpu-physics"),dir=resolve(root,"benchmarks/production-readiness/pr11-queries-2026-10-17"),old=resolve(root,"benchmarks/production-readiness/pr10-ui-2026-10-15"),raw=resolve(dir,"raw"),build=resolve(raw,"viewer-build");
await mkdir(build,{recursive:true});
const sha=async(p:string)=>createHash("sha256").update(new Uint8Array(await Bun.file(p).arrayBuffer())).digest("hex");
// Use the qualified UI schema (visible/clip/windows and Selectable hooks),
// stopping before the old file's unrelated physics/renderer observers.
let ui=(await Bun.file(resolve(old,"plane-controls-observer.cpp")).text()).split('\n#include "box3d/types.h"\nextern "C" void __real_b3World_Draw')[0];
ui=ui.replace('#include "solver-controls-observer.cpp"','// Unrelated solver-control observer omitted.');
if(!ui.includes('record("Selectable"')||!ui.includes('"visible"')&&!ui.includes('\\"visible\\"')||!ui.includes('\\"windows\\"'))throw Error("Incomplete qualified UI schema");
const symbols=[...ui.matchAll(/DECL\([^,]+,"([^"]+)"/g),...ui.matchAll(/asm\("__wrap_([^"]+)"\)/g)].map(x=>x[1]);
symbols.push("DrawLine","DrawPoint","DrawSolidSphere","b3Body_CastRay","b3Body_CastShape","b3World_CastRay","b3World_CastShape","RenderFrame","_ZN6SampleC2EP13SampleContext");
const observer=resolve(raw,"viewer-observer.cpp");
await Bun.write(observer,ui+`\n#include "${resolve(dir,"viewer-observer.inc.cpp")}"\n`);
const include=resolve(raw,"observer.cmake");
await Bun.write(include,`function(pr11_observer)\n if(TARGET samples_both)\n target_sources(samples_both PRIVATE "${observer}")\n target_link_options(samples_both PRIVATE ${[...new Set(symbols)].map(s=>`"-Wl,--wrap=${s}"`).join(" ")})\n endif()\nendfunction()\ncmake_language(DEFER CALL pr11_observer)\n`);
const base=await Bun.file(resolve(old,"shared-protocol.json")).json();
await Bun.write(resolve(dir,"shared-protocol.json"),JSON.stringify({viewers_CPU_first:base.viewers_CPU_first.filter((v:any)=>v.label.endsWith("-both"))},null,2));
const driver=await Bun.file(resolve(old,"native-ui-driver.ts")).text();
await Bun.write(resolve(raw,"native-ui-driver.ts"),driver.replace('const D=dirname(import.meta.path),R=process.cwd();',`const D=${JSON.stringify(dir)},R=process.cwd();`));
const budget={status:"FROZEN",seconds_per_app:180,actions_per_app:48,screenshots_per_app:4,observer_bytes_per_app:8*1024**2,api_bytes_per_app:8*1024**2,api_tail_bytes:256*1024,temporary_bytes:1024**3,min_free_bytes:20*1024**3,shared_cache_growth_bytes:128*1024**2};
await Bun.write(resolve(dir,"viewer-budget.json"),JSON.stringify(budget,null,2));
const result:any=process.argv.includes("--resume-copy")?await Bun.file(resolve(dir,"viewer-build.json")).json():{status:"BUILDING",operations:[],viewers:[],diagnostic_ui_source:await sha(resolve(old,"plane-controls-observer.cpp")),diagnostic_ui_prefix_sha256:createHash("sha256").update(ui).digest("hex"),driver_source:await sha(resolve(old,"native-ui-driver.ts"))};
async function run(label:string,args:string[],seconds:number){const file=resolve(raw,label+".log");await Bun.write(file,"");await Bun.write(file+".stderr","");const p=Bun.spawn(["timeout","-k","5s",`${seconds}s`,...args],{stdout:Bun.file(file),stderr:Bun.file(file+".stderr")});const t=Date.now();const code=await p.exited;result.operations.push({label,args,seconds,pid:p.pid,code,elapsedMs:Date.now()-t});await Bun.write(resolve(dir,"viewer-build.json"),JSON.stringify(result,null,2));console.log(label,code);if(code)throw Error(label);}
for(const [label,lib] of [["ordinary-both",resolve(root,"target/samples/release/libgpu_physics.a")],["native-both",resolve(root,"target/native-cache-build/release/libgpu_physics.a")]]) {
 if(!(label==="ordinary-both"&&process.argv.includes("--resume-copy"))){
  await run(label+"-configure",["cmake","-S",resolve(root,"native-samples"),"-B",build,"-DCMAKE_BUILD_TYPE=Release","-DFETCHCONTENT_FULLY_DISCONNECTED=ON","-DGPU_PORTABLE_API=ON","-DGPU_SAMPLES=ON","-DBOTH_SAMPLES=ON",`-DGPU_PHYSICS_LIB=${lib}`,`-DCMAKE_PROJECT_INCLUDE=${include}`],45);
 await run(label+"-build",["cmake","--build",build,"--target","samples_both","-j4"],process.argv.includes("--incremental")?45:180);
 }
 const binary=resolve(raw,label);await copyFile(resolve(build,"bin/samples_both"),binary);
 result.viewers.push({label,binary,sha256:await sha(binary),lib_sha256:await sha(lib)});
}
result.status="PASS";await Bun.write(resolve(dir,"viewer-build.json"),JSON.stringify(result,null,2));
