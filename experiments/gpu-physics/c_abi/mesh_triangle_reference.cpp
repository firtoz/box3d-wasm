// Captured Mesh Drop body 258 pose, querying each terrain triangle in isolation.
#include "box3d/collision.h"
#include <cstdio>
int main() {
    b3Transform pose={{-3.55281305f,0.294953436f,-8.53022671f},{{-0.504609466f,-0.783914804f,-0.358624011f},0.0472847596f}};
    b3Vec3 v[]={{-4,0.279481441f,-9},{-4,0.451831192f,-8},{-3,0.279481351f,-8},{-3,0.172873959f,-9}};
    int indices[][3]={{0,1,2},{2,3,0}};
    b3BoxHull hull=b3MakeBoxHull(0.02f,0.2f,0.04f);
    for(int t=0;t<2;++t) {
        b3LocalManifoldPoint points[4]={};
        b3LocalManifold m={};m.points=points;
        b3SATCache cache={};
        b3CollideTriangleAndHull(&m,4,b3InvTransformPoint(pose,v[indices[t][0]]),
            b3InvTransformPoint(pose,v[indices[t][1]]),b3InvTransformPoint(pose,v[indices[t][2]]),0,&hull.base,&cache,true);
        auto n=b3RotateVector(pose.q,m.normal);
        std::printf("{\"triangle\":%d,\"count\":%d,\"normal\":[%.9g,%.9g,%.9g],\"points\":[",t,m.pointCount,n.x,n.y,n.z);
        for(int i=0;i<m.pointCount;++i){auto p=b3TransformPoint(pose,m.points[i].point);
            std::printf("%s[%.9g,%.9g,%.9g,%.9g]",i?",":"",p.x,p.y,p.z,m.points[i].separation);}
        std::puts("]}");
    }
}
