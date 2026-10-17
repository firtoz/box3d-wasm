import {readFileSync,writeFileSync,existsSync} from 'node:fs';import {resolve} from 'node:path';
const p=resolve('experiments/gpu-physics/benchmarks/production-readiness/pr04-behavior-2026-10-17'),phase='joint-native-separation';if(existsSync(p+'/'+phase+'-protocol.json'))throw Error('Frozen phase exists');
const old=JSON.parse(readFileSync(p+'/joint-c-remaining-protocol.json','utf8'));
const labels=['caches-before','compile-separation','link-native-separation','run-native-separation','verify-frozen-libraries','caches-after'];
const jobs=labels.map(label=>{const job=old.jobs.find(j=>j.label===label);if(!job)throw Error(label);return job});
const protocol={...old,phase,jobs,totalSeconds:145,cleanup:old.cleanup.filter(p=>p.endsWith('joint-separation.o')),progressCriterion:'ONLY original unlaunched native separation fixture2686checks; compile15/link25/run60/verify15/cache10/145phase.0reaction retries/newcandidates/enginebuilds/captures. Stopfirstfailure; originalreactiontimeout remainsfailed.',inputs:[...old.inputs,p+'/prepare-unlaunched-joint.ts']};
writeFileSync(p+'/'+phase+'-protocol.json',JSON.stringify(protocol,null,2)+'\n');console.log('Frozen',phase,jobs.length,'jobs');
