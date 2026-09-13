#!/usr/bin/env python3
"""Create an instrumented native archive outside the clean submodule."""
import io,json,subprocess,tarfile,tempfile
from pathlib import Path
root=Path(__file__).resolve().parents[1]
out=Path(tempfile.mkdtemp(prefix='box3d-solver-trace-'))
archive=subprocess.check_output(['git','-C',str(root/'../../box3d'),'archive','HEAD'])
with tarfile.open(fileobj=io.BytesIO(archive)) as tar:tar.extractall(out,filter='data')
p=out/'src/contact_solver.c';s=p.read_text();s='#include <stdio.h>\n#include <stdlib.h>\n'+s
anchor='\t\t\t\t\tconstraint->rollingImpulse = b3MulSV( warmStartScale, manifold->rollingImpulse );'
assert s.count(anchor)==1
s=s.replace(anchor,anchor+'''
                if (getenv("B3_SOLVER_TRACE")) {
                    fprintf(stderr,"native-prepare %llu softness %.9g %.9g %.9g friction %.9g rolling %.9g centerB %.9g %.9g %.9g\\n",
                        (unsigned long long)world->stepIndex,contactConstraint->softness.biasRate,contactConstraint->softness.massScale,contactConstraint->softness.impulseScale,
                        contact->friction,contact->rollingResistance,centerB.x,centerB.y,centerB.z);
                    fprintf(stderr,"native-inertia %llu %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g\\n",(unsigned long long)world->stepIndex,
                        iB.cx.x,iB.cx.y,iB.cx.z,iB.cy.x,iB.cy.y,iB.cy.z,iB.cz.x,iB.cz.y,iB.cz.z);
                    for(int t=0;t<pointCount;t++)fprintf(stderr,"native-point %llu %d mass %.9g base %.9g\\n",(unsigned long long)world->stepIndex,t,constraint->points[t].normalMass,constraint->points[t].baseSeparation);
                }
''')
start=s.index('void b3SolveContacts_Mesh(');end=s.index('void b3ApplyRestitution_Mesh',start);chunk=s[start:end]
anchor='\t\tfloat rollingResistance = contactConstraint->rollingResistance;';assert anchor in chunk
trace='''
        if(getenv("B3_SOLVER_TRACE"))fprintf(stderr,"native-solve-%s %%llu %%d v %%.9g %%.9g %%.9g w %%.9g %%.9g %%.9g\\n",(unsigned long long)world->stepIndex,useBias,vB.x,vB.y,vB.z,wB.x,wB.y,wB.z);
'''
chunk=chunk.replace(anchor,anchor+trace%('before'),1)
anchor='\t\t\tstateB->angularVelocity = wB;';assert anchor in chunk
chunk=chunk.replace(anchor,anchor+trace%('after'),1)
s=s[:start]+chunk+s[end:];p.write_text(s)
(root/'artifacts/v18-solver-trace').mkdir(exist_ok=True)
(root/'artifacts/v18-solver-trace/native-path.json').write_text(json.dumps({'path':str(out),'revision':subprocess.check_output(['git','-C',str(root/'../../box3d'),'rev-parse','HEAD'],text=True).strip()})+'\n')
print(out)
