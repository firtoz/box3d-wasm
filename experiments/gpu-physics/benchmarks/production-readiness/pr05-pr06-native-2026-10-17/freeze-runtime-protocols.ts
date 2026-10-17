import {readFileSync,writeFileSync,mkdirSync,copyFileSync} from "node:fs";
import{resolve}from"node:path";
const root=process.cwd(),g=resolve("experiments/gpu-physics"),p=g+"/benchmarks/production-readiness/pr05-pr06-native-2026-10-17",raw=p+"/raw",candidate=raw+"/candidate";
const routes=["cpu","ordinary-gpu","ordinary-both","native-gpu","native-both"];
const common={WGPU_BACKEND:"vulkan",GPU_PHYSICS_ADAPTER:"nvidia",GPU_PHYSICS_BACKEND:"vulkan",VK_DRIVER_FILES:"/usr/share/vulkan/icd.d/nvidia_icd.json",GPU_PHYSICS_LIVE_CONTACT_ORDER:"1"};
const native=JSON.parse(readFileSync(g+"/benchmarks/production-readiness/pr02-contract-refresh-2026-10-17/native-policy-protocol.json","utf8")).environment;
const cache={GPU_PHYSICS_PIPELINE_CACHE_DIR:process.env.HOME+"/.cache/box3d-gpu-physics/pipelines"};
function protocol(name:string,jobs:any[],seconds:number,inputs:string[]=[]){writeFileSync(p+"/"+name+"-protocol.json",JSON.stringify({totalSeconds:seconds,maxRawBytes:6*1024**3,environment:common,inputs,jobs},null,2)+"\n");}
function env(route:string,on=true){const e={...common,...(route.startsWith("native")?native:{}),...(on?cache:{GPU_PHYSICS_PIPELINE_CACHE:"0"})};if(!on&&route.startsWith("native"))for(const k of["GPU_PHYSICS_NATIVE_RADIX_CACHE","GPU_PHYSICS_NATIVE_CONTACT_CACHE","GPU_PHYSICS_NATIVE_GRAPH_CACHE","GPU_PHYSICS_NATIVE_TAIL_CACHE","GPU_PHYSICS_NATIVE_PAIR_CACHE","GPU_PHYSICS_NATIVE_RESET_CACHE","GPU_PHYSICS_FULL_REPLAY","GPU_PHYSICS_GRAPH_MEMO"])e[k]="0";return e;}
function link(route:string,source:string,dest:string,extra:string[]=[]){const dir=candidate+"/"+route,both=route.endsWith("both");return["g++","-O2","-std=c++17",source,"-I",root+"/box3d/include",...extra,...(both?["-DGPU_API_DUAL"]:[]),...(route==="cpu"?[]:["-Wl,--whole-archive",...(both?[dir+"/libgpu_both_api.a"]:[]),dir+"/libgpu_samples_api.a",candidate+"/"+(route.startsWith("native")?"native":"ordinary")+"/libgpu_physics.a","-Wl,--no-whole-archive",...(both?[dir+"/libbox3d_cpu.a"]:[])]),dir+"/box3d_src-libbox3d.a","-ldl","-lpthread","-lm","-lGL","-lgcc_s","-o",dest];}
const external=raw+"/external-native-consumer";mkdirSync(external,{recursive:true});copyFileSync(p+"/external-consumer.cpp",external+"/main.cpp");
const jobs:any[]=[];
for(const route of routes){const exe=external+"/consumer";jobs.push({label:"link-"+route,argv:link(route,external+"/main.cpp",exe),seconds:25});jobs.push({label:"run-"+route,argv:[exe],seconds:90,environment:env(route),requireStrings:["\"destroyed\":true"],cleanup:[exe]});}
jobs.push({label:"review",argv:["bun",g+"/scripts/pr06-consumer-review.ts"],seconds:10,cleanup:[external]});
for(const route of routes.slice(1)){const exe=raw+"/final-callback";jobs.push({label:"link-callback-"+route,argv:link(route,g+"/c_abi/pr05_callback_contract.cpp",exe,["-I",g+"/c_abi","-DPR05_GPU"]),seconds:25});jobs.push({label:"run-callback-"+route,argv:[exe],seconds:60,environment:env(route),requireStrings:["PASS callback identity and rejection"],cleanup:[exe]});}
protocol("release-consumer",jobs,900,[p+"/external-consumer.cpp",g+"/scripts/pr06-consumer-review.ts",g+"/c_abi/pr05_callback_contract.cpp"]);
const launches:any[]=[];
for(const route of routes)for(const on of route==="cpu"?[true]:[true,false]){const label=route+(on?"-cache-on":"-cache-off"),target=route==="cpu"?"cpu":route.endsWith("both")?"both":"gpu";launches.push({label,argv:[candidate+"/"+route+"/bin-samples_"+target,"--sample-name","Bodies/Body Type","--warmup","1","--timed","18","--pause-script","--completed-step","--paced","--switch-sample-name","Bodies/Disable","--switch-after","13","--bench-json",p+"/viewer-"+label+".json"],cwd:root+"/box3d",seconds:120,environment:env(route,on)});}
protocol("release-viewers",launches,1200);
const closes:any[]=[{label:"link-close-helper",argv:["g++",p+"/close-window.cpp","-lX11","-o",raw+"/close-window"],seconds:15}];
for(const route of routes.slice(1))closes.push({label:route,argv:["bun",g+"/scripts/pr05-loading-close.ts",route],seconds:100,environment:env(route,false),requireStrings:["PASS loading-worker shutdown"]});
closes[closes.length-1].cleanup=[raw+"/close-window"];
protocol("release-loading-close",closes,430,[g+"/scripts/pr05-loading-close.ts",p+"/close-window.cpp"]);
protocol("release-compatibility",[{label:"typecheck",argv:["bun","run","typecheck"],seconds:90},{label:"demo-build",argv:["bun","run","--cwd","demo","build","--outDir",raw+"/demo-dist","--emptyOutDir"],seconds:90,cleanup:[raw+"/demo-dist"]}],190,[root+"/packages/box3d-wasm/src/index.ts"]);
console.log("Frozen five external consumers, four callback guards, nine lifecycle viewers, four loading closes and three compatibility commands; no retry/capture/sweep");
