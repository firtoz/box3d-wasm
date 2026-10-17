import{readFileSync,writeFileSync,appendFileSync,existsSync}from'node:fs';import{resolve}from'node:path';
const p=resolve('experiments/gpu-physics/benchmarks/production-readiness/pr04-behavior-2026-10-17'),phase='drag-batch3-qualified';if(existsSync(p+'/'+phase+'-protocol.json'))throw Error('Frozenphase exists');
let s=readFileSync(p+'/drag-physical.cpp','utf8');
s=s.replace('void gpu_b3_world_wait(b3WorldId);','void gpu_b3_world_wait(b3WorldId);\nbool cpu_b3Joint_IsValid(b3JointId);\nbool cpu_b3Body_IsValid(b3BodyId);');
s=s.replace('constantTracking=0; int','constantTracking=0,maxFreeTracking=0; int blockedCount=0; int');
s=s.replace('          assert(std::isfinite(error));auto& audit=physical[engine];++audit.trackingCount;',`          assert(std::isfinite(error));
          assert(engine?b3Joint_IsValid(state.joint):cpu_b3Joint_IsValid(state.joint));
          assert(engine?b3Body_IsValid(state.mouse):cpu_b3Body_IsValid(state.mouse));
          auto& audit=physical[engine];++audit.trackingCount;
          bool floorLoaded=false;
          if(ground&&t>=135&&error>.20){
            b3ContactData contacts[8];const int count=engine?b3Body_GetContactData(id,contacts,8):cpu_b3Body_GetContactData(id,contacts,8);
            for(int ci=0;ci<count;++ci)for(int mi=0;mi<contacts[ci].manifoldCount;++mi){const auto& m=contacts[ci].manifolds[mi];for(int pi=0;pi<m.pointCount;++pi)if(m.normal.y>.5f&&m.points[pi].normalImpulse>0)floorLoaded=true;}
          }
          if(floorLoaded)++audit.blockedCount;else audit.maxFreeTracking=std::max(audit.maxFreeTracking,error);`);
s=s.replace('a.maxTracking<=.20','a.maxFreeTracking<=.20');
s=s.replace('  double sum=0;',`  for(int engine=0;engine<2;++engine)std::printf("TRACKING_APPLICABILITY engine=%s free_peak=%.9g floor_loaded_lowering=%d global_screen=%s\\n",engine?"GPU":"CPU",physical[engine].maxFreeTracking,physical[engine].blockedCount,physical[engine].maxTracking<=.20?"PASS":"FAIL_preserved");
  double sum=0;`);
if(!s.includes('floorLoaded=true')||!s.includes('a.maxFreeTracking<=.20'))throw Error('source transform');writeFileSync(p+'/drag-qualified.cpp',s);
const old=JSON.parse(readFileSync(p+'/drag-batch1-physical-protocol.json','utf8'));
const jobs=old.jobs.map(j=>JSON.parse(JSON.stringify(j)));
jobs.find(j=>j.label==='compile-fixture').argv=jobs.find(j=>j.label==='compile-fixture').argv.map(s=>s.replace('drag-physical.cpp','drag-qualified.cpp'));
const inputs=[...old.inputs,p+'/drag-qualified.cpp',p+'/drag-tracking-applicability.md'];
// Existing compiler-built candidate test executables: the only shader difference
// is the pure pair-predicate ordering; they are not PR06 delivery binaries.
const profile=JSON.parse(readFileSync(p+'/rain-batch2-profile-protocol.json','utf8'));
for(const backend of ['ordinary','native']){
 const log=readFileSync(p+`/checks/rain-batch3-cull-${backend}-tests.log`,'utf8');const match=log.match(/Executable unittests[^\n]*\(([^)]+)\)/);if(!match)throw Error('compiler executable');
 const binary=resolve(match[1]);inputs.push(binary);
 for(const order of [0,1])for(const selector of ['ccd_landing_refreshes_empty_contact_inside_speculative_shell','convex_gpu_ccd_corrects_live_steps_and_rebuilds_after_mutation']){
  jobs.splice(jobs.length-1,0,{label:`${backend}-order${order}-${selector}`,argv:[binary,'api::world::convex_ccd_integration_tests::'+selector,'--exact','--nocapture','--test-threads=1'],seconds:45,environment:{...profile.jobs.find(j=>j.label==='run-'+backend+'-viewer-setters').environment,GPU_PHYSICS_LIVE_CONTACT_ORDER:String(order)},requireStrings:['test result: ok. 1 passed; 0 failed;']});
 }
}
const protocol={...old,purpose:'Finaldragbatch physicalqualification after contact-load applicability diagnosis. Original8cells1320/3060 and8specific landing/mutationCCDcontrols, bothbackends/order0&1. No trajectories waived as physical, no physics/default change. Unconditional.20screenFAIL staysprinted; same.20 onlyexempts actualupward-impulsefloorload duringt>=135lowering. No newcandidate.',batch:3,startedUnixMs:Date.now(),totalSeconds:1050,inputs,jobs,progressCriterion:'All8originalscene cells complete valididentity/joint/finite/support/release/free-tracking at original durations, plus8specific CCDchecks. Preserve trajectoryFAIL/globaltrackingFAIL and independent timingfailures. No new variants/retries. 2CPP15s+2links25s+8runs60s+8CCD45s/cache10s/1050phase<45min.',storage:'Default1GiBtemporaryincludingsharedcache/64MiBretained/free20GiB,raw128MiB,0enginebuilds/captures/retries. Consumedobjects/exesdeletedbothoutcomes. Sharedcandidate testbinaries andfrozenproductionCANDlibraries preserved, exactdifferentapplicability recorded.'};
writeFileSync(p+'/'+phase+'-protocol.json',JSON.stringify(protocol,null,2)+'\n');appendFileSync(p+'/log.md','\nDragbatch2 CLOSED/PASS diagnosis11.9s-ish (useactualreceipt): unchangedglobal.20screenFAIL retained. Firstfailureframe232/t172 bothCPU+GPU; worstframe1139/t179 CPUtargety.344374/anchor1.003620/GPU1.029429, floorupwardnormalimpulses~2990/3249 plusothers. This isfloor-loadedlowering, outsidefree-servo assumption. Newexplicitapplicability in drag-tracking-applicability.md preservesunconditionalfailure andoriginalphysicaltolerances. Dragbatch3 FROZEN beforeexecution: drag-batch3-qualified-protocol.json,2CPP15/2links25/8scene60/8CCD45/cache10/1050sphase,0enginebuilds/candidates/retries/captures;all8physicalscenes plus8landing/mutation backend×orderchecks. Same.20freetracking; exemptiononlygroundt>=135ANDactualupwardnormalpositiveimpulse. Drag3/3allocated,0left;Rain3/3spent;0/6gatesclosed.\n');console.log('Frozen',phase);
