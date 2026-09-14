#!/usr/bin/env python3
"""Build the CPU snapshot exporter and one-step replay driver outside the source tree.

Use a library built with --features replay-diagnostics. Ordinary viewer binaries
have neither the state-transfer symbol nor the before-step injection.
"""
from pathlib import Path
import argparse,subprocess,shutil,sys
p=argparse.ArgumentParser(description=__doc__);p.add_argument('output',type=Path)
a=p.parse_args();out=a.output.resolve();root=Path(__file__).resolve().parents[3];exp=root/'experiments/gpu-physics';build=exp/'native-samples/build-both'
subprocess.run([sys.executable,str(exp/'scripts/prepare-drag-phase-oracle.py'),str(out)],check=True)
source=(exp/'c_abi/both_dual.c').read_text();anchor='    both_drag_step(worldId, timeStep);';assert source.count(anchor)==1
source=source.replace(anchor,anchor+'\n    extern void drag_before_step(b3WorldId);\n    drag_before_step(worldId);');(out/'both_dual.c').write_text(source)
subprocess.run(['cc','-O3','-DNDEBUG','-std=gnu17','-w','-I'+str(root/'box3d/include'),'-I'+str(exp/'c_abi'),'-c',str(out/'both_dual.c'),'-o',str(out/'both_dual.c.o')],check=True)
shutil.copy2(build/'libgpu_both_api.a',out/'libgpu_both_api.a')
subprocess.run(['ar','r',str(out/'libgpu_both_api.a'),str(out/'both_dual.c.o')],check=True)
subprocess.run(['cc','-O3','-I'+str(root/'box3d/src'),'-I'+str(root/'box3d/include'),'-I'+str(build/'box3d_src'),'-c',str(exp/'c_abi/drag_snapshot.c'),'-o',str(out/'snapshot-original.o')],check=True)
subprocess.run(['objcopy','--redefine-syms='+str(build/'libbox3d_cpu.syms'),str(out/'snapshot-original.o'),str(out/'drag_snapshot.o')],check=True)
s=(out/'both_drag_phase.cpp').read_text();at=s.index('struct Errors')
s=s[:at]+'''extern "C" void drag_export_snapshot(b3WorldId,b3JointId,int,const char*);
extern "C" bool gpu_b3_world_seed_drag_snapshot(b3WorldId,const char*);
extern "C" void drag_before_step(b3WorldId world) {
    const char* frame=std::getenv("DRAG_SEED_FRAME");
    if(!frame || drag_trace_frame!=std::atoi(frame))return;
    const char* path=std::getenv("DRAG_SNAPSHOT");assert(path);
    auto c=both_pointer_state(0),g=both_pointer_state(1);
    assert(bool(c.joint.index1)==bool(g.joint.index1) && c.mouse.index1==g.mouse.index1);
    drag_export_snapshot(both_cpu_world(world),c.joint,g.joint.index1-1,path);
    assert(gpu_b3_world_seed_drag_snapshot(world,path));
}
'''+s[at:]
anchor='    frame++;';assert s.count(anchor)==1
s=s.replace(anchor,'''    const char* seed=std::getenv("DRAG_SEED_FRAME");
    if(seed && frame==std::atoi(seed)) {
      Errors one; for (auto body : bodies) one.add(body);
      std::printf("DIAGNOSTIC seeded step %d position %.9g quaternion %.9g velocity %.9g\\n",frame,one.position,one.rotation,one.velocity);
      b3DestroyWorld(w); std::exit(0);
    }
'''+anchor)
(out/'both_drag_phase.cpp').write_text(s)
