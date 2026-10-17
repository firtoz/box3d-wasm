import{existsSync,mkdirSync,readFileSync,writeFileSync,readdirSync,statSync,lstatSync,rmSync,symlinkSync,unlinkSync}from"node:fs";
import{resolve,join}from"node:path";import{createHash}from"node:crypto";
const root=process.cwd(),out=resolve("experiments/gpu-physics/benchmarks/production-readiness/pr05-pr06-native-2026-10-17"),raw=join(out,"raw/wasm"),sdk=join(raw,"sdk"),patched=join(root,"packages/box3d-wasm/.box3d-patched"),publicDir=join(root,"demo/public/wasm");
const sha=(p:string)=>createHash("sha256").update(readFileSync(p)).digest("hex");
const files=(p:string):string[]=>readdirSync(p,{withFileTypes:true}).flatMap(e=>e.isDirectory()?files(join(p,e.name)):e.isFile()?[join(p,e.name)]:[]);
const size=(p:string)=>files(p).reduce((n,f)=>n+statSync(f).size,0);
const mode=process.argv[2];
if(mode==="freeze"){
 if(existsSync(raw)||existsSync(patched)||existsSync(publicDir))throw Error("WASM scratch/input path exists");
 const pins=Object.fromEntries([...files(join(root,"packages/box3d-wasm/cmake")),...files(join(root,"patches/box3d")),join(root,"packages/box3d-wasm/src/index.ts"),join(root,"scripts/wasm-dump.ts"),join(root,"tools/emsdk-version")].map(p=>[p,sha(p)]));
 const previous=JSON.parse(readFileSync(join(out,"../pr10-ui-2026-10-15/final-wasm-compatibility.json"),"utf8"));
 for(const[p,h]of Object.entries(previous.pins_after))if(pins[p]!==h)throw Error("Changed WASM source versus PR10 "+p);
 mkdirSync(sdk,{recursive:true});mkdirSync(join(raw,"out"));
 writeFileSync(join(out,"wasm-inputs.json"),JSON.stringify({pins,previousSourceApplicability:true,initial:{raw:false,patched:false,publicDir:false},version:readFileSync(join(root,"tools/emsdk-version"),"utf8").trim(),sourceArchiveSha256:previous.sdk_source_sha256},null,2)+"\n");
 console.log("WASM unchanged source pinned; missing generated paths owned by this task");
}else if(mode==="archive"){
 const frozen=JSON.parse(readFileSync(join(out,"wasm-inputs.json"),"utf8"));
 if(sha(join(raw,"emsdk.tar.gz"))!==frozen.sourceArchiveSha256)throw Error("SDK source hash changed");
 console.log("Pinned SDK archive matches retained PR10 SHA256 before execution");
}else if(mode==="publish"){
 const frozen=JSON.parse(readFileSync(join(out,"wasm-inputs.json"),"utf8"));if(sha(join(raw,"emsdk.tar.gz"))!==frozen.sourceArchiveSha256)throw Error("SDK source hash changed");
 if(existsSync(publicDir))throw Error("Public path occupied");symlinkSync(join(raw,"out"),publicDir);
 writeFileSync(join(out,"wasm-generated.json"),JSON.stringify({artifacts:files(join(raw,"out")).map(p=>({path:p,sha256:sha(p),bytes:statSync(p).size})),configSha256:sha(join(sdk,".emscripten"))},null,2)+"\n");
 console.log("Temporary generated WASM linked for original runtime consumer");
}else if(mode==="finish"){
 const frozen=JSON.parse(readFileSync(join(out,"wasm-inputs.json"),"utf8"));
 for(const[p,h]of Object.entries(frozen.pins))if(sha(p)!==h)throw Error("WASM source changed "+p);
 const dump=JSON.parse(readFileSync(join(out,"wasm-single-box.json"),"utf8"));
 if(JSON.stringify(dump.checkpoints.map((c:any)=>c.frame))!=="[0,1,2]"||!dump.checkpoints.every((c:any)=>c.bodies.length===2&&c.bodies.every((b:any)=>[...b.p,...b.q,...b.v,...b.w].every(Number.isFinite))))throw Error("WASM runtime checks");
 if(!lstatSync(publicDir).isSymbolicLink())throw Error("Public path ownership");unlinkSync(publicDir);
 const removed={sdkAndBuildBytes:size(raw),patchedBytes:size(patched)};
 rmSync(patched,{recursive:true});rmSync(raw,{recursive:true});
 writeFileSync(join(out,"wasm-compatibility.json"),JSON.stringify({pass:true,frames:[0,1,2],bodyCount:2,sourceHashesUnchanged:true,previousSourceApplicability:true,removed,pathsRestoredAbsent:[publicDir,patched,raw].every(p=>!existsSync(p))},null,2)+"\n");
 console.log("PASS actual WASM workflow; SDK/build/generated artifacts consumed and removed");
}else throw Error("mode");
