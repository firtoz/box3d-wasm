#!/usr/bin/env python3
"""Build a diagnostic CPU solver object in artifacts; never edit the submodule.

The caller supplies drag_trace_frame, synchronized to the GPU submit frame.
Only completed solver phases are observed; their arithmetic is unchanged.
"""
from pathlib import Path
import argparse, subprocess, shutil
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('output',type=Path)
a=p.parse_args(); out=a.output.resolve();out.mkdir(parents=True,exist_ok=True)
root=Path(__file__).resolve().parents[3]; exp=root/'experiments/gpu-physics'
source=(root/'box3d/src/solver.c').read_text()
helper='''
#include <stdlib.h>
extern int drag_trace_frame;
static void drag_capture_phase(b3StepContext* context, int phase) {
    const char* range=getenv("DRAG_PHASE_RANGE");
    int first=660,last=900;
    if (!range || sscanf(range,"%d:%d",&first,&last)!=2 || drag_trace_frame<first || drag_trace_frame>last) return;
    int count=context->world->solverSets.data[b3_awakeSet].bodyStates.count;
    for(int i=0;i<count;++i) {
        b3BodyState* state=context->states+i;
        fprintf(stderr,"CPUphase %d %d %d %.9g %.9g %.9g %u\\n",drag_trace_frame,phase,context->sims[i].bodyId,
            state->linearVelocity.x,state->linearVelocity.z,b3Length(state->angularVelocity),state->flags);
    }
}
'''
anchor='_Static_assert( B3_RESTITUTION_ITERATIONS >= 1'
assert source.count(anchor)==1;source=source.replace(anchor,helper+'\n'+anchor)
for field,phase in [('prepareConstraints','1'),('integrateVelocities','2+5*subStepIndex'),('warmStart','3+5*subStepIndex'),('solveImpulses','4+5*subStepIndex'),('integratePositions','5+5*subStepIndex'),('relaxImpulses','6+5*subStepIndex'),('applyRestitution','22')]:
 anchor=f'profile->{field} += b3GetMillisecondsAndReset( &ticks );'
 assert source.count(anchor)==1,(field,source.count(anchor))
 source=source.replace(anchor,anchor+f'\n        drag_capture_phase(context,{phase});')
(out/'solver.c').write_text(source)
build=exp/'native-samples/build-both'
subprocess.run(['cc','-O3','-DNDEBUG','-std=gnu17','-fvisibility=hidden','-I'+str(root/'box3d/src'),'-I'+str(root/'box3d/include'),'-I'+str(build/'box3d_src'),'-c',str(out/'solver.c'),'-o',str(out/'solver.c.o')],check=True)
shutil.copy2(build/'box3d_src/libbox3d.a',out/'libbox3d.a')
subprocess.run(['ar','r',str(out/'libbox3d.a'),str(out/'solver.c.o')],check=True)
subprocess.run(['objcopy','--redefine-syms='+str(build/'libbox3d_cpu.syms'),str(out/'libbox3d.a'),str(out/'libbox3d_cpu.a')],check=True)
fixture=(exp/'c_abi/both_drag_test.cpp').read_text()
fixture=fixture.replace('#include <algorithm>','extern "C" { int drag_trace_frame = 0; void gpu_b3_world_dump_phases(b3WorldId, unsigned); }\n#include <algorithm>')
fixture=fixture.replace('    b3World_Step(w, 1.f / 60, 4);','    drag_trace_frame = frame;\n    b3World_Step(w, 1.f / 60, 4);')
fixture=fixture.replace('    dump();','''    const char* range = std::getenv("DRAG_PHASE_RANGE");
    int first=0,last=0;
    if (range && std::sscanf(range,"%d:%d",&first,&last)==2 && frame>=first && frame<=last)
      gpu_b3_world_dump_phases(w,frame);
    dump();''')
(out/'both_drag_phase.cpp').write_text(fixture)
