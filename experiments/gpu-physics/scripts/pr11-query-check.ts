import { mkdir, readFile, stat } from "node:fs/promises";
import { createHash } from "node:crypto";
import { resolve } from "node:path";

// Bounded headless diagnosis. Run from the repository root. Raw builds are
// ignored; retained receipts preserve the exact input source and library hashes.
const root = resolve("experiments/gpu-physics");
const out = resolve(process.env.GPU_PR11_CHECK_OUT ?? resolve(root, "benchmarks/production-readiness/pr11-queries-2026-10-17"));
const raw = resolve(out, "raw");
await mkdir(raw, { recursive: true });
const receipts: unknown[] = await Bun.file(resolve(out,"build-checks.json")).exists()?await Bun.file(resolve(out,"build-checks.json")).json():[];
async function run(label: string, argv: string[], seconds: number, env = {}) {
  const file = Bun.file(resolve(raw, `${label}.log`));
  await Bun.write(file, "");
  await Bun.write(resolve(raw,`${label}.stderr`), "");
  const started = Date.now();
  const child = Bun.spawn(["timeout", "-k", "5s", `${seconds}s`, ...argv], {
    cwd: process.cwd(), env: { ...process.env, ...env }, stdout: file, stderr: Bun.file(resolve(raw,`${label}.stderr`)),
  });
  const code = await child.exited;
  const receipt = { label, argv, seconds, code, pid:child.pid, elapsedMs: Date.now()-started };
  receipts.push(receipt);
  await Bun.write(resolve(out,"build-checks.json"),JSON.stringify(receipts,null,2)+"\n");
  console.log(label, code, receipt.elapsedMs);
  if ((await stat(file.name!)).size > 16*1024*1024) throw new Error(`Output cap: ${label}`);
  if(code) throw new Error(`${label} failed; see ${file.name}`);
}
const cpu = resolve(raw,"cpu-build/src/libbox3d.a");
const source = resolve(root,"c_abi/pr11_query_reference.cpp");
const include = resolve("box3d/include");
const libs = {
  ordinary: resolve(root,"target/samples/release/libgpu_physics.a"),
  native: resolve(root,"target/native-cache-build/release/libgpu_physics.a"),
};
const mode=process.argv[2]??"baseline";
if(!process.env.GPU_PR11_CHECK_OUT) throw new Error("Set GPU_PR11_CHECK_OUT to a fresh diagnostic output directory; retained milestone evidence must not be overwritten.");
if(await Bun.file(resolve(out,`${mode}-identities.json`)).exists() || mode==="callbacks"&&await Bun.file(resolve(out,"callbacks-result.json")).exists()) throw new Error("This mode already has evidence; use a fresh output directory.");
if(mode==="callbacks") {
  const fixture=resolve(root,"c_abi/query_reentrancy.cpp");
  for(const [name,lib] of Object.entries({cpu,...libs})) {
    await run(`callbacks-${name}-link`,["g++","-O2","-ffp-contract=off","-std=c++17",fixture,"-I",include,...(name==="cpu"?[cpu]:["-Wl,--allow-multiple-definition","-Wl,--wrap=b3CreateCompound",resolve(raw,"shim.o"),lib,cpu]),"-ldl","-lpthread","-lm","-lgcc_s","-o",resolve(raw,`callbacks-${name}`)],45);
    await run(`callbacks-${name}`,["bash","-c",'ulimit -c 0; exec "$1"',"pr11",resolve(raw,`callbacks-${name}`)],60,{
      WGPU_BACKEND:"vulkan",GPU_PHYSICS_ADAPTER:"nvidia",VK_DRIVER_FILES:"/usr/share/vulkan/icd.d/nvidia_icd.json",GPU_PHYSICS_PIPELINE_CACHE_DIR:resolve(process.env.HOME!,".cache/box3d-gpu-physics/pipelines"),
    });
    const output=await Bun.file(resolve(raw,`callbacks-${name}.log`)).text();
    if(output!==[0,1,2,3].map(i=>`mode=${i} valid=1\n`).join("")) throw new Error(`Unexpected callback result ${name}`);
  }
  await Bun.write(resolve(out,"callbacks-result.json"),JSON.stringify({status:"PASS",backends:["cpu","ordinary","native"],modes:["continue","terminate","ignore","clip"],nested:["body reads","closest ray","callback ray"],source:fixture},null,2));
  process.exit(0);
}
if(mode==="baseline") {
  await run("cpu-build",["cmake","--build",resolve(raw,"cpu-build"),"--target","box3d","-j4"],120);
  await run("shim",["cc","-O2","-ffp-contract=off","-I",include,"-c",resolve(root,"c_abi/shim.c"),"-o",resolve(raw,"shim.o")],45);
  await run("cpu-link",["g++","-O2","-ffp-contract=off","-std=c++17",source,"-I",include,cpu,"-lpthread","-lm","-o",resolve(raw,"cpu")],45);
}
if(mode==="repaired") await run("shim-repaired",["cc","-O2","-ffp-contract=off","-I",include,"-c",resolve(root,"c_abi/shim.c"),"-o",resolve(raw,"shim.o")],45);
if(["contract","encroach"].includes(mode)) await run(`${mode}-cpu-link`,["g++","-O2","-ffp-contract=off","-std=c++17",source,"-I",include,cpu,"-lpthread","-lm","-o",resolve(raw,"cpu")],45);
const heightPath=resolve(raw,"heightfield-inputs.txt");
if(mode==="heightfield") {
  const inputs=await Bun.file(resolve(root,"benchmarks/production-readiness/pr09-repairs-2026-10-02/PERF02/cast-cost-trace-no-json-dependency/actual-inputs-outputs.json")).json();
  await Bun.write(heightPath,inputs.map((r:any)=>[r.index,...r.origin,r.radius,...r.translation].join(" ")).join("\n")+"\n");
}
if(mode!=="heightfield")
for(const [name,lib] of Object.entries(libs)) {
  await run(`${name}-link`,["g++","-O2","-ffp-contract=off","-std=c++17","-DPR11_GPU",source,"-I",include,"-Wl,--allow-multiple-definition","-Wl,--wrap=b3CreateCompound",resolve(raw,"shim.o"),lib,cpu,"-ldl","-lpthread","-lm","-lgcc_s","-o",resolve(raw,name)],45);
}
const hashes: Record<string,string>={};
for(const path of [source,cpu,...Object.values(libs),resolve(root,"c_abi/shim.c"),resolve(root,"src/api/query.rs")]) hashes[path]=createHash("sha256").update(await readFile(path)).digest("hex");
await Bun.write(resolve(out,`${mode}-identities.json`),JSON.stringify(hashes,null,2)+"\n");
for(const name of ["cpu",...Object.keys(libs)]) {
  await run(`${mode}-${name}`,["bash","-c",'ulimit -c 0; exec "$@"',"pr11",resolve(raw,name),...(mode==="heightfield"?[heightPath]:mode==="encroach"?["encroach"]:[])],mode==="contract"?90:mode==="encroach"?30:60,{
    WGPU_BACKEND:"vulkan",GPU_PHYSICS_ADAPTER:"nvidia",VK_DRIVER_FILES:"/usr/share/vulkan/icd.d/nvidia_icd.json",GPU_PHYSICS_PIPELINE_CACHE_DIR:resolve(process.env.HOME!,".cache/box3d-gpu-physics/pipelines"),
  });
  const data=(await Bun.file(resolve(raw,`${mode}-${name}.log`)).text()).split("\n").filter(x=>x.startsWith("{"));
  await Bun.write(resolve(out,`${mode}-${name}.json`),JSON.stringify(data.map(x=>JSON.parse(x)),null,2)+"\n");
}
if(["repaired","contract","heightfield","encroach"].includes(mode)) {
  const cpuRows=await Bun.file(resolve(out,`${mode}-cpu.json`)).json();
  const results=[];
  for(const backend of Object.keys(libs)) {
    const actual=await Bun.file(resolve(out,`${mode}-${backend}.json`)).json();
    let maximum=0;const errors=[];
    if(actual.length!==cpuRows.length) errors.push({reason:"row count",cpu:cpuRows.length,gpu:actual.length});
    for(let i=0;i<Math.max(actual.length,cpuRows.length);i++) {
      const a=cpuRows[i],b=actual[i];
      if(!a||!b) continue;
      for(const key of new Set([...Object.keys(a),...Object.keys(b)])) {
        if(["fraction","point","normal"].includes(key)) {
          const x=[a[key]].flat(),y=[b[key]].flat();
          const error=Math.max(...x.map((v,j)=>Math.abs(v-y[j])));
          maximum=Math.max(maximum,error);
          if(!Number.isFinite(error)||error>1e-5) errors.push({index:i,key,cpu:a[key],gpu:b[key],error});
        } else if(JSON.stringify(a[key])!==JSON.stringify(b[key])) errors.push({index:i,key,cpu:a[key],gpu:b[key]});
      }
    }
    results.push({backend,rows:actual.length,maximum,tolerance:1e-5,errors});
  }
  await Bun.write(resolve(out,`${mode}-comparison.json`),JSON.stringify(results,null,2)+"\n");
  console.log(JSON.stringify(results.map(r=>({...r,errors:r.errors.slice(0,6),errorCount:r.errors.length}))));
  if(results.some(r=>r.errors.length)) process.exitCode=1;
}
