import{spawn,spawnSync}from'node:child_process';import{readFileSync,writeFileSync,readdirSync,existsSync,mkdirSync,rmSync,statSync,statfsSync}from'node:fs';import{resolve,join}from'node:path';import{createHash}from'node:crypto';
const p=resolve('experiments/gpu-physics/benchmarks/production-readiness/pr04-behavior-2026-10-17'),g=resolve('experiments/gpu-physics'),phase='contact-policy-batch1';const sha=(f:string)=>createHash('sha256').update(readFileSync(f)).digest('hex');
if(process.argv[2]==='physical'){
 const backend=process.argv[3],order=process.argv[4];if(!['ordinary','native'].includes(backend)||!['0','1'].includes(order))throw Error('args');
 const log=readFileSync(`${p}/checks/${phase}-${backend}-tests.log`,'utf8');const match=log.match(/Executable unittests[^\n]*\(([^)]+)\)/);if(!match)throw Error('Compiler binary receipt absent');const binary=resolve(match[1]);
 const old=JSON.parse(readFileSync(p+'/related-physical-checks-protocol.json','utf8')).jobs.find(j=>j.label===`${backend}-order${order}-physical`);
 const scratch=`${p}/raw/contact-policy-${backend}-${order}`;mkdirSync(scratch,{recursive:true});console.log('Actual compiler-selected binary',binary,sha(binary));
 try{const r=spawnSync(binary,old.argv.slice(1),{env:{...process.env,TMPDIR:scratch},encoding:'utf8',timeout:85000,killSignal:'SIGKILL',maxBuffer:16*1024**2});process.stdout.write(r.stdout??'');process.stderr.write(r.stderr??'');if(r.error||r.status!==0||!r.stdout?.includes('test result: ok. 8 passed; 0 failed;'))throw Error('Original physical suite failed');}finally{rmSync(scratch,{force:true,recursive:true})}
}else if(process.argv[2]==='run'){
 const replacements=[{path:g+'/src/sim.rs',before:p+'/contact-policy-sim-before.rs',candidate:p+'/contact-policy-sim-candidate.rs'},{path:g+'/Cargo.toml',before:p+'/contact-policy-ordinary-manifest-before.toml',candidate:p+'/contact-policy-ordinary-manifest-candidate.toml'},{path:g+'/target/native-workspace/experiments/gpu-physics/Cargo.toml',before:p+'/contact-policy-native-manifest-before.toml',candidate:p+'/contact-policy-native-manifest-candidate.toml'}];
 for(const r of replacements)if(sha(r.path)!==sha(r.before))throw Error('Preserve changed source '+r.path);
 const bytes=(d:string):number=>!existsSync(d)?0:readdirSync(d,{withFileTypes:true}).reduce((n,e)=>n+(e.isDirectory()?bytes(join(d,e.name)):e.isFile()?statSync(join(d,e.name)).size:0),0);
 const roots=[p+'/raw',g+'/target/samples',g+'/target/native-cache-build','/home/firtoz/.cache/box3d-gpu-physics/pipelines','/home/firtoz/.cache/nvidia'],initial=roots.map(bytes),start=Date.now();let peak=0,reason:string|undefined;
 const stop=()=>{const f=p+'/'+phase+'-receipt.json';if(existsSync(f)){const r=JSON.parse(readFileSync(f,'utf8'));if(r.pid)try{process.kill(-r.pid,'SIGKILL')}catch{}}};
 try{
  for(const r of replacements)writeFileSync(r.path,readFileSync(r.candidate));
  const child=spawn('bun',[p+'/run.ts',phase],{stdio:'inherit'});
  const timer=setInterval(()=>{try{const growth=roots.map(bytes).reduce((n,b,i)=>n+Math.max(0,b-initial[i]),0);peak=Math.max(peak,growth);const fs=statfsSync(p);if(growth>1024**3||fs.bavail*fs.bsize<20*1024**3||Date.now()-start>1355*1000)throw Error('Growth/free-space/outer timeout');}catch(e){reason=String(e);stop();child.kill('SIGTERM')}},5000);
  let code:number|null;try{code=await new Promise<number|null>((res,rej)=>{child.once('close',res);child.once('error',rej)})}finally{clearInterval(timer)}
  if(code!==0||reason)throw Error(reason??'Candidate phase failed');
 }finally{
  stop();const restored=[];for(const r of replacements){if(sha(r.path)===sha(r.candidate))writeFileSync(r.path,readFileSync(r.before));else if(sha(r.path)!==sha(r.before))throw Error('Concurrent edit, preserve '+r.path);restored.push({path:r.path,sha256:sha(r.path),restored:sha(r.path)===sha(r.before)});}
  const protocol=JSON.parse(readFileSync(p+'/'+phase+'-protocol.json','utf8'));let removedBytes=0;for(const f of protocol.cleanupOnFailure){if(!f.startsWith(p+'/raw/'))throw Error('cleanup boundary');if(existsSync(f)){removedBytes+=statSync(f).size;rmSync(f,{force:true})}}
  writeFileSync(p+'/'+phase+'-restoration.json',JSON.stringify({restored,peakNewBytes:peak,removedBytes,elapsedMs:Date.now()-start,reason},null,2)+'\n');
 }
}else throw Error('Choose run or physical');
