import { createHash } from "node:crypto";
import { existsSync, readdirSync, readFileSync, writeFileSync, mkdirSync, copyFileSync, statSync, symlinkSync } from "node:fs";
import { resolve, join, relative } from "node:path";
import { execFileSync } from "node:child_process";

const root = process.cwd(), g = join(root, "experiments/gpu-physics");
const out = join(g, "benchmarks/production-readiness/pr04-behavior-2026-10-17");
const raw = join(out, "raw"), mode = process.argv[2];
const candidateRoot=join(g,"artifacts/pr04-release-candidate-2026-10-17");
const sha = (p: string) => createHash("sha256").update(readFileSync(p)).digest("hex");
function walk(dir: string): string[] {
  return readdirSync(dir, { withFileTypes: true }).flatMap(e => e.isFile() ? [join(dir,e.name)] : e.isDirectory() ? walk(join(dir,e.name)) : []);
}
function json(name: string, value: unknown) { writeFileSync(join(out,name), JSON.stringify(value,null,2)+"\n"); }
const paths = (d: string) => execFileSync("git", ["-C", d, "ls-files", "-z"], {encoding:"utf8"}).split("\0").filter(Boolean).map(p=>join(d,p)).filter(p=>existsSync(p)&&statSync(p).isFile());
if(mode === "inputs") {
  const files = new Set<string>();
  for(const d of ["src","shaders","c_abi","scripts"]) for(const p of walk(join(g,d))) files.add(p);
  for(const p of readdirSync(join(g,"native-samples"))) { const f=join(g,"native-samples",p); if(statSync(f).isFile()) files.add(f); }
  for(const p of ["Cargo.toml","Cargo.lock","build.rs"]) files.add(join(g,p));
  for(const p of paths(join(root,"box3d"))) files.add(p);
  for(const d of ["naga","wgpu","wgpu-hal","wgpu-core"]) for(const p of walk(join(g,"target/native-backend",d))) files.add(p);
  for(const p of ["target/native-backend/cargo-config.toml","target/native-workspace/experiments/gpu-physics/Cargo.lock","target/native-workspace/experiments/gpu-physics/Cargo.toml"]) files.add(join(g,p));
  const sourceHashes = Object.fromEntries([...files].sort().map(p=>[relative(root,p),sha(p)]));
  const deps = Object.fromEntries(["imgui","implot","nfd"].map(n=>{ const d=join(root,"box3d/.fetchcontent-cache",n+"-src"); return [n,{revision:execFileSync("git",["-C",d,"rev-parse","HEAD"],{encoding:"utf8"}).trim(),dirty:execFileSync("git",["-C",d,"status","--porcelain"],{encoding:"utf8"}),hashes:Object.fromEntries(paths(d).map(p=>[relative(d,p),sha(p)]))}]; }));
  const toolchain = Object.fromEntries([["rustc",["-vV"]],["cargo",["-V"]],["cmake",["--version"]],["g++",["--version"]],["bun",["--version"]],["uname",["-a"]],["nvidia-smi",["--query-gpu=name,driver_version,pci.bus_id","--format=csv,noheader"]]].map(([c,a])=>[c,execFileSync(c as string,a as string[],{encoding:"utf8"}).trim()]));
  json("release-inputs.json",{base:execFileSync("git",["rev-parse","HEAD"],{encoding:"utf8"}).trim(),box3d:execFileSync("git",["-C","box3d","rev-parse","HEAD"],{encoding:"utf8"}).trim(),sourceHashes,deps,toolchain});
  console.log("Frozen",files.size,"source inputs and three native dependency trees");
} else if(mode === "prepare-viewers") {
  const protocol=JSON.parse(readFileSync(join(out,"refreeze-viewers-protocol.json"),"utf8"));
  for(const job of protocol.jobs)if(job.cwd){mkdirSync(job.cwd,{recursive:true});const data=join(job.cwd,"data");if(!existsSync(data))symlinkSync(join(root,"box3d/data"),data);}
  console.log("Owned viewer working directories prepared");
} else if(mode === "carry-cpu") {
  const source=join(g,"artifacts/pr05-pr06-release-candidate-2026-10-17/cpu");
  const target=join(candidateRoot,"cpu");mkdirSync(target,{recursive:true});
  for(const name of ["bin-samples_cpu","box3d_src-libbox3d.a"])copyFileSync(join(source,name),join(target,name));
  const consumer=join(raw,"external-native-consumer");mkdirSync(consumer,{recursive:true});copyFileSync(join(root,"experiments/gpu-physics/benchmarks/production-readiness/pr05-pr06-native-2026-10-17/external-consumer.cpp"),join(consumer,"main.cpp"));
  console.log("Unchanged CPU candidate carried; external consumer input copied");
} else if(mode === "rust") {
  const name=process.argv[3]; if(!["ordinary","native"].includes(name)) throw Error("backend");
  const target=join(raw,"rust-"+name), lib=join(target,"release/libgpu_physics.a");
  const dir=join(candidateRoot,name); mkdirSync(dir,{recursive:true}); copyFileSync(lib,join(dir,"libgpu_physics.a"));
  const dependencies: Record<string,string>={};
  for(const d of walk(target).filter(p=>p.endsWith(".d"))) {
    const text=readFileSync(d,"utf8").replace(/\\\n/g,"");
    for(const line of text.split("\n")) {
      const split=line.indexOf(": "); if(split<0) continue;
      for(const p of line.slice(split+2).split(/\s+/)) if(p.startsWith("/")&&existsSync(p)&&statSync(p).isFile()&&!p.startsWith(target+"/")) dependencies[p]=sha(p);
    }
  }
  json("release-rust-"+name+".json",{library:{path:relative(root,join(dir,"libgpu_physics.a")),sha256:sha(lib),bytes:statSync(lib).size},dependencies});
  console.log(name,"library frozen",Object.keys(dependencies).length,"actual dep-info inputs");
} else if(mode === "native") {
  const route=process.argv[3], dir=join(raw,"cmake-"+route), candidate=join(candidateRoot,route);
  mkdirSync(candidate,{recursive:true});
  const target=route==="cpu"?"samples_cpu":route.endsWith("both")?"samples_both":"samples_gpu";
  const copies=["bin/"+target,"box3d_src/libbox3d.a",...(route!=="cpu"?["libgpu_samples_api.a"]:[]),...(route.endsWith("both")?["libgpu_both_api.a","libbox3d_cpu.a"]:[])];
  const artifacts=copies.map(p=>{ const from=join(dir,p), dest=join(candidate,p.replaceAll("/","-")); copyFileSync(from,dest);return {path:relative(root,dest),sha256:sha(dest),bytes:statSync(dest).size}; });
  const commands=JSON.parse(readFileSync(join(dir,"compile_commands.json"),"utf8"));
  const inputs:Record<string,string>={}; const generated:string[]=[];
  for(const row of commands) { if(existsSync(row.file)) inputs[row.file]=sha(row.file); if(row.file.startsWith(dir+"/"))generated.push(row.file); }
  for(const p of walk(dir).filter(p=>p.endsWith(".o.d"))) {
    const t=readFileSync(p,"utf8").replace(/\\\n/g," ");const s=t.indexOf(": ");
    for(const f of t.slice(s+2).split(/\s+/)) if(f.startsWith("/")&&existsSync(f)&&statSync(f).isFile()) inputs[f]=sha(f);
  }
  const gen=join(out,"generated",route);mkdirSync(gen,{recursive:true});
  for(const p of new Set(generated)) { const dest=join(gen,relative(dir,p));mkdirSync(resolve(dest,".."),{recursive:true});copyFileSync(p,dest); }
  json("release-native-"+route+".json",{artifacts,commands,inputs,cache:readFileSync(join(dir,"CMakeCache.txt"),"utf8")});
  console.log(route,"frozen",commands.length,"compilation commands",Object.keys(inputs).length,"actual inputs");
} else if(mode === "verify") {
  const before=JSON.parse(readFileSync(join(out,"release-inputs.json"),"utf8"));
  for(const [p,h] of Object.entries(before.sourceHashes))if(sha(join(root,p))!==h) throw Error("Changed release input "+p);
  for(const [n,dep] of Object.entries(before.deps) as any)for(const [p,h]of Object.entries(dep.hashes))if(sha(join(root,"box3d/.fetchcontent-cache",n+"-src",p))!==h)throw Error("Changed dependency "+n+"/"+p);
  console.log("All frozen source/dependency bytes unchanged");
} else throw Error("mode");
