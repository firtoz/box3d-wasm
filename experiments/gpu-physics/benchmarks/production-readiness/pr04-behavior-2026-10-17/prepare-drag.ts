import { readFileSync,writeFileSync,existsSync,appendFileSync } from 'node:fs';
import { resolve } from 'node:path';
const p=resolve('experiments/gpu-physics/benchmarks/production-readiness/pr04-behavior-2026-10-17'),g=resolve('experiments/gpu-physics'),r=resolve('.');
const phase='drag-batch1-physical';if(existsSync(p+'/'+phase+'-protocol.json'))throw Error('Frozen phase exists');
let s=readFileSync(g+'/c_abi/both_drag_test.cpp','utf8');
s=s.replace('#include <cstring>','#include <cstring>\n#include <chrono>\n#include <vector>');
s=s.replace('  int frame = 0;',`  int frame = 0;
  struct Physical { double minCorner=1e9,maxSpeed=0,maxAbsY=0,maxTracking=0,constantTracking=0; int worstFrame=-1,worstBody=-1,finiteCount=0,trackingCount=0; } physical[2];
  std::vector<double> clocks;
  auto inspect=[&](){
    for(int engine=0;engine<2;++engine)for(int index=0;index<5;++index){
      auto id=engine?bodies[index]:both_cpu_body(bodies[index]);
      auto x=engine?b3Body_GetTransform(id):cpu_b3Body_GetTransform(id);
      auto v=engine?b3Body_GetLinearVelocity(id):cpu_b3Body_GetLinearVelocity(id);
      auto wv=engine?b3Body_GetAngularVelocity(id):cpu_b3Body_GetAngularVelocity(id);
      auto& a=physical[engine];
      double values[]={x.p.x,x.p.y,x.p.z,x.q.v.x,x.q.v.y,x.q.v.z,x.q.s,v.x,v.y,v.z,wv.x,wv.y,wv.z};
      for(double value:values)assert(std::isfinite(value));++a.finiteCount;
      a.maxSpeed=std::max(a.maxSpeed,double(b3Length(v)));a.maxAbsY=std::max(a.maxAbsY,std::abs(double(x.p.y)));
      if(ground)for(float vx:{-.5f,.5f})for(float vy:{-.5f,.5f})for(float vz:{-.5f,.5f}){
        double y=x.p.y+b3RotateVector(x.q,{vx,vy,vz}).y;
        if(y<a.minCorner){a.minCorner=y;a.worstFrame=frame;a.worstBody=index;}
      }
    }
  };`);
s=s.replace('    b3World_Step(w, 1.f / 60, 4);\n    gpu_b3_world_wait(w);',`    auto start=std::chrono::steady_clock::now();
    b3World_Step(w, 1.f / 60, 4);
    gpu_b3_world_wait(w);
    clocks.push_back(std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count());
    inspect();
    if((frame+1)%120==0)std::printf("progress physical steps=%d CPUcorner=%.9g GPUcorner=%.9g\\n",frame+1,physical[0].minCorner,physical[1].minCorner);`);
s=s.replace('        held.add(bodies[i]);',`        held.add(bodies[i]);
        if(t>=45)for(int engine=0;engine<2;++engine){
          const auto state=engine?b:a;const auto id=engine?bodies[i]:both_cpu_body(bodies[i]);
          const auto x=engine?b3Body_GetTransform(id):cpu_b3Body_GetTransform(id);
          const auto mouse=engine?b3Body_GetTransform(state.mouse):cpu_b3Body_GetTransform(state.mouse);
          const auto offset=b3RotateVector(x.q,state.local_anchor);
          const double error=std::hypot(x.p.x+offset.x-mouse.p.x,x.p.y+offset.y-mouse.p.y,x.p.z+offset.z-mouse.p.z);
          assert(std::isfinite(error));auto& audit=physical[engine];++audit.trackingCount;
          audit.maxTracking=std::max(audit.maxTracking,error);
          if(t>=105&&t<135)audit.constantTracking=std::max(audit.constantTracking,error);
        }`);
s=s.replace('  return pass ? 0 : 1;',`  bool physicalPass=frame==(ground?3060:1320);
  for(int engine=0;engine<2;++engine){
    const auto& a=physical[engine];
    physicalPass=physicalPass&&a.finiteCount==frame*5&&a.maxSpeed<=120&&a.maxAbsY<=200&&a.trackingCount>0&&a.maxTracking<=.20&&(!ground||a.minCorner>=-.025);
    std::printf("PHYSICAL engine=%s steps=%d finite=%d min_corner=%.9g worst_frame=%d worst_body=%d max_speed=%.9g max_abs_y=%.9g tracking=%.9g constant_tracking=%.9g\\n",engine?"GPU":"CPU",frame,a.finiteCount,a.minCorner,a.worstFrame,a.worstBody,a.maxSpeed,a.maxAbsY,a.maxTracking,a.constantTracking);
  }
  double sum=0;for(double x:clocks)sum+=x;std::sort(clocks.begin(),clocks.end());
  std::printf("TIMING combined_completed steps=%zu mean_ms=%.9g SPS=%.9g simulated_sps=%.9g p95_ms=%.9g max_ms=%.9g\\n",clocks.size(),sum/clocks.size(),1000*clocks.size()/sum,1000*clocks.size()/sum/60,clocks[size_t(std::ceil(clocks.size()*.95))-1],clocks.back());
  std::printf("Original trajectory result=%s (deferred for loaded scenes; unchanged limits/diagnostics)\\n",pass?"PASS":"FAIL");
  std::printf("%s independent physical dragging\\n",physicalPass?"PASS":"FAIL");
  return physicalPass?0:1;`);
if(!s.includes('PHYSICAL engine=')||!s.includes('inspect();'))throw Error('fixture transformation');
writeFileSync(p+'/drag-physical.cpp',s);
const profile=JSON.parse(readFileSync(p+'/rain-batch2-profile-protocol.json','utf8'));
const includes=['-I',r+'/box3d/include','-I',g+'/c_abi','-I',g+'/native-samples','-I',r+'/box3d/samples','-I',r+'/box3d/extern/sokol'];
const jobs=[profile.jobs[0],{label:'compile-fixture',argv:['g++','-O2','-std=c++17',...includes,'-c',p+'/drag-physical.cpp','-o',p+'/raw/drag-physical.o'],seconds:15},{label:'compile-camera',argv:['g++','-O2','-std=c++17',...includes,'-c',r+'/box3d/samples/host/camera.cpp','-o',p+'/raw/drag-camera.o'],seconds:15}];
const inputs=[p+'/drag-physical.cpp',p+'/drag-acceptance.md',g+'/c_abi/both_drag_test.cpp',g+'/c_abi/both_dual.c',g+'/c_abi/both_view.h',r+'/box3d/samples/host/camera.cpp'];
for(const backend of ['ordinary','native']){
 const base=g+'/artifacts/pr05-pr06-release-candidate-2026-10-17',route=base+'/'+backend+'-both';
 const libs=[route+'/libgpu_both_api.a',route+'/libgpu_samples_api.a',base+'/'+backend+'/libgpu_physics.a',route+'/libbox3d_cpu.a',route+'/box3d_src-libbox3d.a'];inputs.push(...libs);
 jobs.push({label:'link-'+backend,argv:['g++',p+'/raw/drag-physical.o',p+'/raw/drag-camera.o','-Wl,--whole-archive',libs[0],libs[1],'-Wl,--no-whole-archive',...libs.slice(2),'-ldl','-lpthread','-lm','-lgcc_s','-lGL','-o',p+'/raw/drag-'+backend],seconds:25});
 const environment=profile.jobs.find(j=>j.label==='run-'+backend+'-viewer-setters').environment;
 for(const order of [0,1])for(const mode of ['isolated','ground'])jobs.push({label:`${backend}-order${order}-${mode}`,argv:[p+'/raw/drag-'+backend,...(mode==='ground'?['--ground']:[])],seconds:60,environment:{...environment,GPU_PHYSICS_LIVE_CONTACT_ORDER:String(order)},requireStrings:['PASS independent physical dragging']});
}
jobs.push(profile.jobs.at(-1));const cleanup=['drag-physical.o','drag-camera.o','drag-ordinary','drag-native'].map(f=>p+'/raw/'+f);
const protocol={purpose:'Dragging/CCD batch1: independent physical audit of existing original scene windows, not cross-engine trajectory rerun. H1 archived failures solely trajectory vs H2 actual support/tracking/finite/release failure. No source/default/geometry edits. CPUfirst reports in combined realCPU+GPU eachcell; bothbackends/order0&1/isolated1320+ground3060.',batch:1,startedUnixMs:Date.now(),totalSeconds:640,maxRawBytes:128*1024**2,environment:profile.environment,inputs,jobs,cleanup,cleanupOnFailure:cleanup,progressCriterion:'Complete1320/3060 physical checks or first concrete independent physical failure/worstframe/body. Preserve original trajectory FAIL diagnostics,do not claim repairedsolver. Newtracking.20m derivedbeforeexecution in drag-acceptance,originalrelease<.1/support-.025 unchanged.',storage:'Default1GiBtemporaryincludingcachegrowth/64MiBretained/free20GiB.2CPP15s+2links25s+8runs60s+2cache10s,640sphase;0enginebuilds,captures,retries. Streamprogress/aggregate only,deleteobjects/exes bothoutcomes. Frozenreleaseinputs preserved.'};
writeFileSync(p+'/'+phase+'-protocol.json',JSON.stringify(protocol,null,2)+'\n');appendFileSync(p+'/log.md','\nIndependent dragging/CCD batch1 FROZEN in drag-batch1-physical-protocol.json: newindependent physicalaudit withunchanged original1320/3060steps, actualCPU/GPUandbothbackends/order0&1;2CPP15s+2links25s+8runs60s/cache10s/640sphase,0enginebuilds/candidates/retries/captures,default1GiBtemporary/64MiBretained/free20GiB. Actualmotor7.5Hz/damping1.0 correction tohistorical5Hz assumption in drag-acceptance.md; independentanchortrackinglimit.20m derivedfromramp+discretelag+gravitybeforeexecution,originalrelease.1/support.025retained. No Rainbudgetrenewal; separatedrag1/3allocated,2left;0/6gatesclosed. FrozenPR06productionlibs used,NOT cullcandidate.\n');
console.log('Frozen',phase);
