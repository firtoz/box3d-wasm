#!/usr/bin/env python3
"""Build a diagnostic CPU solver object in artifacts; never edit the submodule.

The caller supplies drag_trace_frame, synchronized to the GPU submit frame.
Only completed solver phases are observed; their arithmetic is unchanged.
"""
from pathlib import Path
import argparse, subprocess, shutil
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('output',type=Path)
p.add_argument('--build-dir', type=Path, help='BOTH build containing the complete CPU oracle archive')
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
        fprintf(stderr,"CPUvelocity %d %d %d %.9g %.9g %.9g %.9g %.9g %.9g\\n",drag_trace_frame,phase,context->sims[i].bodyId,
            state->linearVelocity.x,state->linearVelocity.y,state->linearVelocity.z,
            state->angularVelocity.x,state->angularVelocity.y,state->angularVelocity.z);
    }
}
'''
anchor='_Static_assert( B3_RESTITUTION_ITERATIONS >= 1'
assert source.count(anchor)==1;source=source.replace(anchor,helper+'\n'+anchor)
for field,phase in [('prepareConstraints','1'),('integrateVelocities','2+5*subStepIndex'),('warmStart','3+5*subStepIndex'),('solveImpulses','4+5*subStepIndex'),('integratePositions','5+5*subStepIndex'),('relaxImpulses','6+5*subStepIndex'),('applyRestitution','22')]:
 anchor=f'profile->{field} += b3GetMillisecondsAndReset( &ticks );'
 assert source.count(anchor)==1,(field,source.count(anchor))
 source=source.replace(anchor,anchor+f'\n        drag_capture_phase(context,{phase});')
# The joint boundary isolates motor error from the following static contacts.
# Phase 100/101 is before/after a relaxed solve, 102/103 a biased solve.
anchor='\t\tb3SolveJoint( joint, context, useBias );'
assert source.count(anchor)==1
source=source.replace(anchor, 'if (getenv("DRAG_CONSTRAINT_TRACE")) drag_capture_phase(context,100+2*useBias);\n'+anchor+'\nif (getenv("DRAG_CONSTRAINT_TRACE")) drag_capture_phase(context,101+2*useBias);')
(out/'solver.c').write_text(source)
build=a.build_dir.resolve() if a.build_dir else exp/'native-samples/build-both'
subprocess.run(['cc','-O3','-DNDEBUG','-std=gnu17','-fvisibility=hidden','-I'+str(root/'box3d/src'),'-I'+str(root/'box3d/include'),'-I'+str(build/'box3d_src'),'-c',str(out/'solver.c'),'-o',str(out/'solver.c.o')],check=True)
archive = build/'libbox3d_cpu_original.a'
if not archive.exists():
    cache = (build/'CMakeCache.txt').read_text()
    if 'GPU_PORTABLE_API:BOOL=ON' in cache:
        raise SystemExit(f'Missing complete CPU oracle: {archive}; build the BOTH targets first')
    archive = build/'box3d_src/libbox3d.a'
shutil.copy2(archive,out/'libbox3d.a')
subprocess.run(['ar','r',str(out/'libbox3d.a'),str(out/'solver.c.o')],check=True)
# Instrument the complete CPU convex solver without editing upstream sources.
contact=(root/'box3d/src/contact_solver.c').read_text()
contact = '#include <stdio.h>\n#include <stdlib.h>\nextern int drag_trace_frame;\nstatic int drag_contact_trace(void) { int a=0,b=0; const char* r=getenv("DRAG_PHASE_RANGE"); return getenv("DRAG_CONSTRAINT_TRACE") && r && sscanf(r,"%d:%d",&a,&b)==2 && drag_trace_frame>=a && drag_trace_frame<=b; }\n' + contact
anchor='( (float*)&cp->normalMasses )[lane] = kNormal > 0.0f ? 1.0f / kNormal : 0.0f;'
assert contact.count(anchor)==1
contact=contact.replace(anchor,anchor+'''
                    if (drag_contact_trace()) fprintf(stderr,"CPUprepare %d %d %d a %.9g %.9g %.9g base %.9g b %.9g %.9g %.9g mass %.9g old %.9g\\n",drag_trace_frame,indexB,pointIndex,rA.x,rA.y,rA.z,baseSeparation,rB.x,rB.y,rB.z,((float*)&cp->normalMasses)[lane],((float*)&cp->normalImpulses)[lane]);
''')
anchor='b3FloatW deltaImpulse = b3SubW( newImpulse, cp->normalImpulses );'
assert contact.count(anchor)==2
contact=contact.replace(anchor,anchor+r'''
            if (drag_contact_trace()) for (int lane=0;lane<B3_SIMD_WIDTH;lane++) {
                if (c->indexB[lane] && pointIndex<c->pointCounts[lane]) fprintf(stderr,"CPUimpulse %d %d %d %d %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",drag_trace_frame,c->indexB[lane]-1,useBias,pointIndex,((float*)&s)[lane],((float*)&vn)[lane],((float*)&bias)[lane],((float*)&cp->normalMasses)[lane],((float*)&pointMassScale)[lane],((float*)&pointImpulseScale)[lane],((float*)&cp->normalImpulses)[lane],((float*)&negImpulse)[lane],((float*)&newImpulse)[lane],((float*)&deltaImpulse)[lane]);
            }
''',1)
anchor='b3FloatW twistSpeed = b3DotW( c->normal, b3SubVW( bB.w, bA.w ) );'
assert contact.count(anchor)==1
contact=contact.replace(anchor,anchor+r'''
                if (drag_contact_trace()) for (int lane=0;lane<B3_SIMD_WIDTH;lane++) {
                    if(c->indexB[lane]) fprintf(stderr,"CPUtwist %d %d %.9g %.9g %.9g\n",drag_trace_frame,c->indexB[lane]-1,((float*)&c->twistMass)[lane],((float*)&twistSpeed)[lane],((float*)&c->twistImpulse)[lane]);
                }
''')
anchor='b3Vec2W newImpulse = b3AddV2W( c->frictionImpulse, deltaImpulse );'
assert contact.count(anchor)==1
contact=contact.replace(anchor,anchor+r'''
                if (drag_contact_trace()) for (int lane=0;lane<B3_SIMD_WIDTH;lane++) {
                    if(c->indexB[lane]) fprintf(stderr,"CPUfriction %d %d %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",drag_trace_frame,c->indexB[lane]-1,((float*)&rB.X)[lane],((float*)&rB.Y)[lane],((float*)&rB.Z)[lane],((float*)&vt.x)[lane],((float*)&vt.y)[lane],((float*)&newImpulse.x)[lane],((float*)&newImpulse.y)[lane],((float*)&bB.w.X)[lane],((float*)&bB.w.Y)[lane],((float*)&bB.w.Z)[lane],((float*)&c->frictionImpulse.x)[lane],((float*)&c->frictionImpulse.y)[lane]);
                }
''')
anchor='b3Vec2W newImpulse = b3AddV2W( c->frictionImpulse, deltaImpulse );'
contact=contact.replace(anchor,anchor+r'''
                if (drag_contact_trace()) for (int lane=0;lane<B3_SIMD_WIDTH;lane++) {
                    if(c->indexB[lane]) fprintf(stderr,"CPUfrictionMass %d %d %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",drag_trace_frame,c->indexB[lane]-1,((float*)&c->tangentMass.cxx)[lane],((float*)&c->tangentMass.cxy)[lane],((float*)&c->tangentMass.cxy)[lane],((float*)&c->tangentMass.cyy)[lane],((float*)&tangent1.X)[lane],((float*)&tangent1.Y)[lane],((float*)&tangent1.Z)[lane],0.0,((float*)&tangent2.X)[lane],((float*)&tangent2.Y)[lane],((float*)&tangent2.Z)[lane],0.0);
                }
''')
anchor='b3Vec3W P = b3AddVW( b3MulSVW( deltaImpulse.x, tangent1 ), b3MulSVW( deltaImpulse.y, tangent2 ) );'
assert contact.count(anchor)==1
contact=contact.replace(anchor,anchor+r'''
                if (drag_contact_trace()) for (int lane=0;lane<B3_SIMD_WIDTH;lane++) {
                    if(c->indexB[lane]) fprintf(stderr,"CPUfrictionResult %d %d %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",drag_trace_frame,c->indexB[lane]-1,((float*)&lengthSquared)[lane],((float*)&maxImpulse)[lane],((float*)&scale)[lane],0.0,((float*)&newImpulse.x)[lane],((float*)&newImpulse.y)[lane],((float*)&deltaImpulse.x)[lane],((float*)&deltaImpulse.y)[lane],((float*)&P.X)[lane],((float*)&P.Y)[lane],((float*)&P.Z)[lane],0.0);
                }
''')
(out/'contact_solver.c').write_text(contact)
subprocess.run(['cc','-O3','-DNDEBUG','-std=gnu17','-I'+str(root/'box3d/src'),'-I'+str(root/'box3d/include'),'-c',str(out/'contact_solver.c'),'-o',str(out/'contact_solver.c.o')],check=True)
subprocess.run(['ar','r',str(out/'libbox3d.a'),str(out/'contact_solver.c.o')],check=True)
motor=(root/'box3d/src/motor_joint.c').read_text()
motor = '#include <stdio.h>\n#include <stdlib.h>\nextern int drag_trace_frame;\nstatic int drag_motor_trace(void) { int a=0,b=0; const char* r=getenv("DRAG_PHASE_RANGE"); return getenv("DRAG_CONSTRAINT_TRACE") && r && sscanf(r,"%d:%d",&a,&b)==2 && drag_trace_frame>=a && drag_trace_frame<=b; }\n' + motor
anchor='b3Vec3 b = b3Solve3( k, b3Add( cdot, bias ) );'
assert motor.count(anchor)==1
motor=motor.replace(anchor,anchor+r'''
        if (drag_motor_trace()) {
            b3Vec3 rhs = b3Add(cdot,bias);
            fprintf(stderr,"CPUmotorRotation %d %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",drag_trace_frame,stateB->deltaRotation.v.x,stateB->deltaRotation.v.y,stateB->deltaRotation.v.z,stateB->deltaRotation.s,joint->frameB.p.x,joint->frameB.p.y,joint->frameB.p.z);

            fprintf(stderr,"CPUmotor %d rA %.9g %.9g %.9g rB %.9g %.9g %.9g c %.9g %.9g %.9g rhs %.9g %.9g %.9g solution %.9g %.9g %.9g\n",drag_trace_frame,rA.x,rA.y,rA.z,rB.x,rB.y,rB.z,c.x,c.y,c.z,rhs.x,rhs.y,rhs.z,b.x,b.y,b.z);
            fprintf(stderr,"CPUmotorMatrix %d %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",drag_trace_frame,k.cx.x,k.cx.y,k.cx.z,k.cy.x,k.cy.y,k.cy.z,k.cz.x,k.cz.y,k.cz.z);
        }
''')
anchor='impulse = b3Sub( joint->linearSpringImpulse, oldImpulse );'
assert motor.count(anchor)==1
motor=motor.replace(anchor,anchor+r'''
        if (drag_motor_trace()) fprintf(stderr,"CPUmotorImpulse %d %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",drag_trace_frame,oldImpulse.x,oldImpulse.y,oldImpulse.z,joint->linearSpringImpulse.x,joint->linearSpringImpulse.y,joint->linearSpringImpulse.z,impulse.x,impulse.y,impulse.z,massScale,impulseScale,joint->linearSpring.biasRate);
''')
anchor='impulse = b3Sub( joint->linearSpringImpulse, oldImpulse );'
motor=motor.replace(anchor,anchor+r'''
        if (drag_motor_trace()) {
            b3Vec3 torque = b3Cross(rB,impulse);
            b3Vec3 delta = b3MulMV(iB,torque);
            fprintf(stderr,"CPUmotorTorque %d %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",drag_trace_frame,torque.x,torque.y,torque.z,delta.x,delta.y,delta.z,wB.x,wB.y,wB.z);
            fprintf(stderr,"CPUmotorInertia %d %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",drag_trace_frame,iB.cx.x,iB.cx.y,iB.cx.z,iB.cy.x,iB.cy.y,iB.cy.z,iB.cz.x,iB.cz.y,iB.cz.z);
        }
''')
anchor='impulse = b3Sub( joint->angularVelocityImpulse, oldImpulse );'
assert motor.count(anchor)==1
motor=motor.replace(anchor,anchor+r'''
        if (drag_motor_trace()) {
            b3Vec3 solved = b3MulMV(joint->angularMass,cdot);
            b3Matrix3 m = joint->angularMass;
            fprintf(stderr,"CPUangularInput %d %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",drag_trace_frame,oldImpulse.x,oldImpulse.y,oldImpulse.z,maxImpulse,cdot.x,cdot.y,cdot.z,solved.x,solved.y,solved.z);
            fprintf(stderr,"CPUangularOutput %d %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",drag_trace_frame,joint->angularVelocityImpulse.x,joint->angularVelocityImpulse.y,joint->angularVelocityImpulse.z,impulse.x,impulse.y,impulse.z,wB.x,wB.y,wB.z);
            fprintf(stderr,"CPUangularMatrix %d %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",drag_trace_frame,m.cx.x,m.cx.y,m.cx.z,m.cy.x,m.cy.y,m.cy.z,m.cz.x,m.cz.y,m.cz.z);
        }
''')
(out/'motor_joint.c').write_text(motor)
subprocess.run(['cc','-O3','-DNDEBUG','-std=gnu17','-I'+str(root/'box3d/src'),'-I'+str(root/'box3d/include'),'-c',str(out/'motor_joint.c'),'-o',str(out/'motor_joint.c.o')],check=True)
subprocess.run(['ar','r',str(out/'libbox3d.a'),str(out/'motor_joint.c.o')],check=True)
subprocess.run(['objcopy','--redefine-syms='+str(build/'libbox3d_cpu.syms'),str(out/'libbox3d.a'),str(out/'libbox3d_cpu.a')],check=True)
fixture=(exp/'c_abi/both_drag_test.cpp').read_text()
fixture=fixture.replace('#include <algorithm>','extern "C" { int drag_trace_frame = 0; void gpu_b3_world_dump_phases(b3WorldId, unsigned); void gpu_b3_world_dump_mesh_candidates(b3WorldId); }\n#include <algorithm>')
fixture=fixture.replace('    b3World_Step(w, 1.f / 60, 4);','    drag_trace_frame = frame;\n    b3World_Step(w, 1.f / 60, 4);')
fixture=fixture.replace('        both_pointer_move(target, ray);','''        both_pointer_move(target, ray);
        const char* inputRange = std::getenv("DRAG_PHASE_RANGE");
        int inputFirst=0,inputLast=0;
        if (inputRange && std::sscanf(inputRange,"%d:%d",&inputFirst,&inputLast)==2 && frame>=inputFirst && frame<=inputLast) {
          for (int engine=0; engine<2; ++engine) {
            const auto pointer=both_pointer_state(engine);
            fprintf(stderr,"DRAGinput %d %d fraction %.9g target %.9g %.9g %.9g\\n",frame,engine,
                pointer.fraction,target.x+pointer.fraction*ray.x,target.y+pointer.fraction*ray.y,target.z+pointer.fraction*ray.z);
          }
        }''')
fixture=fixture.replace('    dump();','''    const char* range = std::getenv("DRAG_PHASE_RANGE");
    int first=0,last=0;
    if (range && std::sscanf(range,"%d:%d",&first,&last)==2 && frame>=first && frame<=last) {
      gpu_b3_world_dump_phases(w,frame);
      if (std::getenv("DRAG_CONSTRAINT_TRACE")) gpu_b3_world_dump_mesh_candidates(w);
    }
    dump();''')
(out/'both_drag_phase.cpp').write_text(fixture)
