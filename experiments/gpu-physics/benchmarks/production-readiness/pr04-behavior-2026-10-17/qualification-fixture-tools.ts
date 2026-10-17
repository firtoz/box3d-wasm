import {spawnSync} from 'node:child_process';
import {readFileSync,writeFileSync,existsSync,mkdirSync} from 'node:fs';
import {createHash} from 'node:crypto';
import {resolve} from 'node:path';
const p=resolve('experiments/gpu-physics/benchmarks/production-readiness/pr04-behavior-2026-10-17');
const protocol=JSON.parse(readFileSync(p+'/mesh-ragdoll-qualification-protocol.json','utf8'));
const [engine,mode,order]=process.argv.slice(2);
if(!['cpu','ordinary','native'].includes(engine)||!['0','1'].includes(order))throw Error('args');
const spec=protocol.modes.find(m=>m.mode===mode);if(!spec)throw Error('mode');
const binary=p+'/raw/'+(spec.kind==='mesh'?'mesh-':'ragdoll-')+engine;
const hash=(bytes:any)=>createHash('sha256').update(bytes).digest('hex');
const args=spec.kind==='mesh'?[]:[mode,String(spec.steps)];
const fixtureEnv={...process.env};for(const k of Object.keys(fixtureEnv))if(k.startsWith('RAIN_FROZEN_'))delete fixtureEnv[k];if(mode==='grid')fixtureEnv.RAIN_FROZEN_GRID='1';
const result=spawnSync(binary,args,{env:fixtureEnv,input:spec.input?readFileSync(spec.input):undefined,encoding:'utf8',timeout:(engine==='cpu'?25:spec.steps===600?85:25)*1000,killSignal:'SIGKILL',maxBuffer:16*1024**2});
mkdirSync(p+'/qualification-results',{recursive:true});
const label=engine+'-order'+order+'-'+mode;
const diagnostic={binary,binarySha256:hash(readFileSync(binary)),stdoutSha256:hash(result.stdout??''),stdoutBytes:Buffer.byteLength(result.stdout??''),stderrSha256:hash(result.stderr??''),stderrBytes:Buffer.byteLength(result.stderr??''),exit:result.status,signal:result.signal,error:result.error?String(result.error):null};
const retained=p+'/qualification-results/'+label+'.json';
mkdirSync(p+'/qualification-references',{recursive:true});
const reference=p+'/qualification-references/cpu-'+mode+'.json';
let report:any={label,diagnostic,status:'FAIL',limits:'Original physical/calculation limits; cross-engine chaotic trajectory matching deferred.'};
try{
 if(result.error||result.status!==0)throw Error('Fixture process failed');
 const stdout=result.stdout??'',stderr=result.stderr??'';
 if(spec.kind==='mesh'){
  const cases:any={},manifolds:any={};
  for(const line of stdout.trim().split('\n')){
   const v=line.trim().split(/\s+/);const id=Number(v[1]);
   if(v[0]==='case'){if(cases[id])throw Error('duplicate case');cases[id]=v.slice(2).map(Number);}
   else if(v[0]==='manifold'){
    const count=Number(v[2]),normal=v.slice(3,6).map(Number),separations=v.slice(6).map(s=>Number(s.split(':')[0])).sort((a,b)=>a-b);
    if(normal.length!==3||separations.length!==count||[...normal,...separations].some(x=>!Number.isFinite(x)))throw Error('nonfinite manifold');
    (manifolds[id]??=[]).push([...normal,...separations]);
   }else throw Error('Unknown fixture record');
  }
  if(Object.keys(cases).length!==spec.count||Array.from({length:spec.count},(_,i)=>i).some(i=>!cases[i]))throw Error('Missing mesh cases');
  for(let i=0;i<spec.count;++i){const c=cases[i],ms=manifolds[i]??[],n=ms.reduce((n,m)=>n+m.length-3,0);if(c.length!==2||c[1]!==n||Boolean(c[0])!==Boolean(n))throw Error('Inconsistent mesh case');
   if(mode==='grid'&&(n===0||ms.some(m=>Math.max(Math.abs(m[0]),Math.abs(m[1]-1),Math.abs(m[2]))>1e-5)))throw Error('Grid physical contact/upnormal');
  }
  report={...report,cases:spec.count,maxComponentError:0};
  if(engine==='cpu')writeFileSync(reference,JSON.stringify({cases,manifolds})+'\n');
  else{
   const cpu=JSON.parse(readFileSync(reference,'utf8'));
   for(let i=0;i<spec.count;++i){if(JSON.stringify(cpu.cases[i])!==JSON.stringify(cases[i]))throw Error('Mesh count mismatch case'+i);
    const remaining=[...(manifolds[i]??[])];
    for(const m of cpu.manifolds[i]??[]){let best=-1,error=Infinity;for(let j=0;j<remaining.length;++j){if(remaining[j].length!==m.length)continue;const e=Math.max(...m.map((x,k)=>Math.abs(x-remaining[j][k])));if(e<error){error=e;best=j}}
     if(best<0||error>1e-5)throw Error('Frozen manifold mismatch '+i+' error='+error);remaining.splice(best,1);report.maxComponentError=Math.max(report.maxComponentError,error);
    }if(remaining.length)throw Error('Extra manifold');
   }
  }
 }else{
  const bodies=spec.bodies,rows=stdout.trim().split('\n').map(line=>line.trim().split(/\s+/).map(Number));
  if(rows.length!==(spec.steps+1)*bodies)throw Error('Row count '+rows.length);
  for(let i=0;i<rows.length;++i)if(rows[i].length!==16||rows[i].some(x=>!Number.isFinite(x))||rows[i][0]!==Math.floor(i/bodies)||rows[i][1]!==i%bodies)throw Error('Nonfinite/unordered row '+i);
  const setup:any={};let counts:any=null;
  if(mode.startsWith('ragdolls')){
   for(const line of stderr.split('\n')){
    const v=line.trim().split(/\s+/);if(v[0]==='joint-setup-count')counts=v.slice(1).map(Number);
    if(v[0]!=='joint-setup')continue;const key=v[1]+':'+v[2];if(setup[key])throw Error('duplicate setup');const values:any={};for(const s of v.slice(6)){const [k,raw]=s.split('='),n=Number(raw);if(!Number.isFinite(n)||k in values)throw Error('bad setup value');values[k]=new Uint32Array(new Float32Array([n]).buffer)[0]}
    const [type,a,b]=v.slice(3,6).map(Number);if(a===b||a<0||a>=112||b<0||b>=112||Object.keys(values).length!==({2:19,5:29,6:36}[type]))throw Error('bad joint setup');setup[key]={type,a,b,values};
   }
   if(JSON.stringify(counts)!=='[112,112,224]'||Object.keys(setup).length!==112)throw Error('Incomplete joint setup');
   for(let h=0;h<8;++h)for(let slot=1;slot<15;++slot)if(!setup[h+':'+slot])throw Error('Missing typed joint');
   const histogram=Object.values(setup).reduce((n:any,r:any)=>(n[r.type]=(n[r.type]??0)+1,n),{});if(JSON.stringify(histogram)!=='{"2":8,"5":32,"6":72}')throw Error('Joint type counts');
  }
  const spine:any={};if(mode==='ragdolls')for(const line of stderr.split('\n')){const v=line.trim().split(/\s+/);if(v[0]==='spine-contact'){if(v.length!==8||v.slice(1).some(x=>!Number.isFinite(Number(x))))throw Error('Invalid spine record');const key=v.slice(1,5).join(':');if(key in spine)throw Error('duplicate spine');spine[key]=v.slice(5).map(Number)}}
  let cpu:any;
  if(engine!=='cpu'){
   cpu=JSON.parse(readFileSync(reference,'utf8'));
   if(mode.startsWith('ragdolls')&&JSON.stringify(cpu.setup)!==JSON.stringify(setup))throw Error('Joint setup float32/type/endpoint mismatch');
  }
  report={...report,rows:rows.length,jointSetup:mode.startsWith('ragdolls')?{status:'PASS',joints:112,types:{filter:8,revolute:32,spherical:72},fieldComparison:'float32 exact against CPU'}:null};
  if(mode==='ragdolls'){
   const metrics:any={};for(const key of['peak_joint_separation','peak_angular_speed','tail_joint_separation','tail_linear_speed','tail_angular_speed'])metrics[key]={value:-Infinity};
   const put=(key:string,value:number,row:number[])=>{if(value>metrics[key].value)metrics[key]={value,frame:row[0],body:row[1],row}};
   for(const row of rows){put('peak_joint_separation',row[15],row);put('peak_angular_speed',Math.hypot(...row.slice(12,15)),row);if(row[0]>=480){put('tail_joint_separation',row[15],row);put('tail_linear_speed',Math.hypot(...row.slice(9,12)),row);put('tail_angular_speed',Math.hypot(...row.slice(12,15)),row)}}
   report.metrics=metrics;
   const limits={peak_joint_separation:.25,tail_joint_separation:.005,tail_linear_speed:.05,tail_angular_speed:.1};
   for(const [k,limit]of Object.entries(limits))if(metrics[k].value>=limit)throw Error('Physical '+k+'='+metrics[k].value+' limit='+limit);
   if(Object.keys(spine).length!==3)throw Error('Incomplete frame2 spine contacts');
   if(engine==='cpu')writeFileSync(reference,JSON.stringify({metrics,setup,initial:rows.slice(0,112),firstStep:rows.slice(112,224),spine})+'\n');
   else{
    if(metrics.peak_angular_speed.value>1.2*cpu.metrics.peak_angular_speed.value)throw Error('Peak angular speed exceeds original CPU multiplier');
    let initialError=0,firstStep=0,spineError=0;
    for(let i=0;i<112;++i){initialError=Math.max(initialError,...rows[i].slice(2).map((x,k)=>Math.abs(x-cpu.initial[i][k+2])));firstStep=Math.max(firstStep,Math.hypot(...rows[112+i].slice(2,5).map((x,k)=>x-cpu.firstStep[i][k+2])))}
    if(initialError>1e-5||firstStep>2e-4)throw Error('Initial/first-step calculation screen');
    if(JSON.stringify(Object.keys(cpu.spine).sort())!==JSON.stringify(Object.keys(spine).sort()))throw Error('Frame2 spine keys');
    for(const key of Object.keys(spine))spineError=Math.max(spineError,Math.hypot(...spine[key].map((x,k)=>x-cpu.spine[key][k])));
    if(spineError>1e-4)throw Error('Frame2 stale contact normal screen');report.initialError=initialError;report.firstStepError=firstStep;report.frame2SpineNormalError=spineError;
   }
  }else if(mode==='ragdolls-no-contacts'){
   if(engine==='cpu')writeFileSync(reference,JSON.stringify({setup,positions:rows.map(r=>r.slice(2,5))})+'\n');
   else{let error=0;for(let i=0;i<rows.length;++i)error=Math.max(error,Math.hypot(...rows[i].slice(2,5).map((x,k)=>x-cpu.positions[i][k])));report.maxPositionError=error;if(error>5e-5)throw Error('Isolated joint integration '+error);}
  }else{
   if(engine==='cpu')writeFileSync(reference,JSON.stringify({rows})+'\n');
   else{let error=0;for(let i=0;i<rows.length;++i)for(let k=2;k<16;++k)error=Math.max(error,Math.abs(rows[i][k]-cpu.rows[i][k]));report.maxStateError=error;if(error>1e-5)throw Error('Isolated spherical calculation '+error);}
  }
 }
 report.status='PASS';
}catch(error){report.error=String(error);report.stderrTail=(result.stderr??'').slice(-4000);}
if(existsSync(reference))report.cpuReference={path:reference,sha256:hash(readFileSync(reference)),bytes:Buffer.byteLength(readFileSync(reference))};
writeFileSync(retained,JSON.stringify(report,null,2)+'\n');console.log(JSON.stringify(report));if(report.status!=='PASS')process.exit(1);
