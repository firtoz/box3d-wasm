import { spawn, spawnSync } from 'node:child_process';
import { createHash } from 'node:crypto';
import { readFileSync, writeFileSync, existsSync, readdirSync, statSync, statfsSync, rmSync } from 'node:fs';
import { resolve, join } from 'node:path';
const p=resolve('experiments/gpu-physics/benchmarks/production-readiness/pr04-behavior-2026-10-17');
const g=resolve('experiments/gpu-physics');
const phase='rain-batch3-cull';
const sha=(p:string)=>createHash('sha256').update(readFileSync(p)).digest('hex');
if(process.argv[2]==='test'){
 const backend=process.argv[3],selector=process.argv[4];
 if(!['ordinary','native'].includes(backend)||!selector?.includes('joint_filter'))throw Error('test args');
 const log=readFileSync(`${p}/checks/${phase}-${backend}-tests.log`,'utf8');
 const match=log.match(/Executable unittests[^\n]*\(([^)]+)\)/);
 if(!match)throw Error('No actual compiler test executable receipt');
 const binary=resolve(match[1]);
 console.log('Compiler-selected test executable',binary,'sha256',sha(binary));
 const r=spawnSync(binary,[selector,'--exact','--nocapture','--test-threads=1'],{timeout:55000,killSignal:'SIGKILL',encoding:'utf8',maxBuffer:16*1024**2});
 process.stdout.write(r.stdout??'');process.stderr.write(r.stderr??'');
 if(r.error||r.status!==0||!r.stdout?.includes('test result: ok. 1 passed; 0 failed;'))throw Error('Required test failed');
}else if(process.argv[2]==='run'){
 const source=g+'/shaders/physics/broadphase.wgsl',before=p+'/broadphase-before.wgsl',candidate=p+'/broadphase-cull-candidate.wgsl';
 if(sha(source)!==sha(before))throw Error('Production shader changed; preserve user edit');
 const protocol=JSON.parse(readFileSync(p+'/'+phase+'-protocol.json','utf8'));
 const roots=[p+'/raw',g+'/target/samples',g+'/target/native-cache-build','/home/firtoz/.cache/box3d-gpu-physics/pipelines','/home/firtoz/.cache/nvidia'];
 const bytes=(dir:string):number=>!existsSync(dir)?0:readdirSync(dir,{withFileTypes:true}).reduce((n,e)=>n+(e.isDirectory()?bytes(join(dir,e.name)):e.isFile()?statSync(join(dir,e.name)).size:0),0);
 const startBytes=roots.map(bytes),started=Date.now();let peakGrowth=0,reason:string|undefined;
 const stop=()=>{const f=p+'/'+phase+'-receipt.json';if(existsSync(f)){const r=JSON.parse(readFileSync(f,'utf8'));if(r.pid)try{process.kill(-r.pid,'SIGKILL')}catch{}}};
 try{
  writeFileSync(source,readFileSync(candidate));
  const child=spawn('bun',[p+'/run.ts',phase],{stdio:'inherit'});
  const timer=setInterval(()=>{
   try{
    const growth=roots.map(bytes).reduce((n,b,i)=>n+Math.max(0,b-startBytes[i]),0);peakGrowth=Math.max(peakGrowth,growth);
    const fs=statfsSync(p);
    if(growth>1.5*1024**3||fs.bavail*fs.bsize<20*1024**3||Date.now()-started>1255*1000)throw Error('Storage/free-space/outer timeout');
   }catch(e){reason=String(e);stop();child.kill('SIGTERM')}
  },5000);
  let code:number|null=null;
  try{code=await new Promise<number|null>((res,rej)=>{child.once('close',res);child.once('error',rej)})}finally{clearInterval(timer)}
  if(code!==0||reason)throw Error(reason??'Candidate phase failed');
 }finally{
  stop();
  // Never overwrite an unrelated source edit during source restoration.
  if(sha(source)===sha(candidate))writeFileSync(source,readFileSync(before));
  else if(sha(source)!==sha(before))throw Error('Concurrent shader edit; manual restoration needed');
  let removedBytes=0;
  for(const f of protocol.cleanupOnFailure){if(!f.startsWith(p+'/raw/'))throw Error('cleanup boundary');if(existsSync(f)){removedBytes+=statSync(f).size;rmSync(f,{force:true})}}
  writeFileSync(p+'/'+phase+'-restoration.json',JSON.stringify({sourceRestored:sha(source)===sha(before),sourceSha256:sha(source),peakNewBytes:peakGrowth,removedBytes,elapsedMs:Date.now()-started,reason},null,2)+'\n');
 }
}else throw Error('Choose run or test');
