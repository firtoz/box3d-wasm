import {createHash} from 'node:crypto';
import {readFileSync,writeFileSync,existsSync,mkdirSync,copyFileSync,constants,statSync} from 'node:fs';
import {resolve,dirname} from 'node:path';
const p=resolve('experiments/gpu-physics/benchmarks/production-readiness/pr04-behavior-2026-10-17');
const protocol=JSON.parse(readFileSync(p+'/joint-c-qualification-protocol.json','utf8'));
const sha=(f:string)=>createHash('sha256').update(readFileSync(f)).digest('hex');
if(!['freeze','verify'].includes(process.argv[2]))throw Error('Choose freeze or verify');
for(const library of protocol.candidateLibraries){
 if(process.argv[2]==='freeze'){
  if(sha(library.source)!==library.sha256)throw Error('Produced library changed');
  mkdirSync(dirname(library.path),{recursive:true});
  if(existsSync(library.path)){if(sha(library.path)!==library.sha256)throw Error('Preserve existing different candidate');}
  else copyFileSync(library.source,library.path,constants.COPYFILE_FICLONE);
 }
 if(sha(library.path)!==library.sha256)throw Error('Frozen candidate changed');
 console.log(JSON.stringify({...library,bytes:statSync(library.path).size}));
}
if(process.argv[2]==='freeze')writeFileSync(p+'/current-policy-libraries.json',JSON.stringify({status:'FROZEN_DEVELOPMENT_CANDIDATE_NOT_PR06',sourcePolicy:protocol.sourcePolicy,candidateLibraries:protocol.candidateLibraries,producer:'contact-policy-batch1 library compiler receipts; empty artifact-isolation feature has no cfg consumer',consumer:'original C fixtures, articulated qualification and PR06 applicability/refreeze; remove when replaced/consumed'},null,2)+'\n');
