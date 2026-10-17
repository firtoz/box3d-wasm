import{readFileSync,writeFileSync,existsSync}from'node:fs';import{resolve}from'node:path';
const p=resolve('experiments/gpu-physics/benchmarks/production-readiness/pr04-behavior-2026-10-17'),phase='joint-kind-qualification';if(existsSync(p+'/'+phase+'-protocol.json'))throw Error('Frozenphase exists');
const old=JSON.parse(readFileSync(p+'/joint-c-remaining-protocol.json','utf8')),jobs:any[]=[old.jobs.find(j=>j.label==='caches-before'),old.jobs.find(j=>j.label==='verify-frozen-libraries'),...old.jobs.filter(j=>j.label.startsWith('compile-'))],cleanup=[p+'/raw/joint-reaction.o',p+'/raw/joint-separation.o'];
for(const backend of['ordinary','native'])for(const kind of['reaction','separation']){
 let link=old.jobs.find(j=>j.label==='link-'+backend+'-'+kind);
 if(!link){const template=old.jobs.find(j=>j.label==='link-'+backend+'-'+(kind==='reaction'?'separation':'reaction'));link={...template,label:'link-'+backend+'-'+kind,argv:template.argv.map(s=>s.replace(/joint-(reaction|separation)\.o$/,'joint-'+kind+'.o').replace(new RegExp(backend+'-joint-(reaction|separation)$'),backend+'-joint-'+kind))};}
 jobs.push(link);const binary=link.argv.at(-1);cleanup.push(binary);
 const template=old.jobs.find(j=>j.label==='run-'+backend+'-'+(backend==='ordinary'?'separation':'reaction'));
 for(const order of backend==='native'&&kind==='reaction'?['0','1']:['0']){
  const env={...template.environment,GPU_PHYSICS_LIVE_CONTACT_ORDER:order};
  if(backend==='native'&&kind==='reaction')for(let jointKind=0;jointKind<9;++jointKind){jobs.push({label:backend+'-order'+order+'-reaction-kind'+jointKind,argv:[binary],seconds:20,environment:{...env,GPU_REACTION_KIND:String(jointKind)},requireStrings:['joint reaction: '+(jointKind===0?360:312)+' independent CPU/GPU scalar comparisons passed']});}
  else jobs.push({label:backend+'-order'+order+'-'+kind,argv:[binary],seconds:30,environment:env,requireStrings:[kind==='reaction'?'joint reaction: 2856 independent CPU/GPU scalar comparisons passed':'joint separation: 2686 independent CPU/GPU checks passed across nine joint types; wheel angular unsupported upstream']});
 }
 jobs.at(-1).cleanup=[binary];
}
jobs.push(old.jobs.find(j=>j.label==='verify-frozen-libraries'),old.jobs.find(j=>j.label==='caches-after'));
writeFileSync(p+'/'+phase+'-protocol.json',JSON.stringify({phase,totalSeconds:650,maxRawBytes:128*1024**2,inputs:[...old.inputs,p+'/prepare-joint-kind.ts'],jobs,cleanup,cleanupOnFailure:cleanup,progressCriterion:'Five C fixture/configuration completions: original ordinary order0 reaction/separation, native order0 separation, native reactionorder0/1 split only by pre-existing GPU_REACTION_KIND selector. Eachkind retainsall4configs/8steps/mutations/zero/sleep/nonzeroforcechecks;counts360+8*312=2856 perordering. Total13940unchangedscalarchecks.2CPP15/4links25/18kindruns20/3wholeruns30/2verify15/cache10/650phase.0Rustbuilds/candidates/Rainruns/performancehypotheses/newobserver/captures/automatic retries; stopfirstfailure. Original60sfullnative timeout staysfailed, not convertedto speedpass.',storage:'Default1GiBtemporary including observed sharedcachegrowth/raw128MiB/retained64MiB/free20GiB;2objects4executables deletedafterphase on success/failure. Existingtwo frozenlibs196MiBexception unchanged; no cache/buildcopies.'},null,2)+'\n');
console.log(JSON.stringify({phase,jobs:jobs.length,scalarComparisons:13940,totalSeconds:650}));
