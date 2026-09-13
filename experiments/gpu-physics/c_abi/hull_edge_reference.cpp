// Same-pose Box3D/GPU hull-edge regression, captured from Gear Lift rock geometry.
#include "box3d/box3d.h"
#include "box3d/collision.h"
#include <cmath>
#include <cstdio>
#include <vector>

static const b3Vec3 vertices[] = {
{0.174166054f,-0.227081954f,0.0900000036f},
{0.251808524f,0.160288826f,-0.0300000012f},
{-0.120846272f,-0.0499619171f,-0.270000011f},
{0.130766988f,0.0f,0.270000011f},
{0.0226820186f,0.258815616f,0.150000006f},
{-0.15796712f,-0.14472869f,0.210000008f},
{0.201279074f,-0.0733945295f,-0.210000008f},
{-0.119846463f,0.230514303f,-0.150000006f},
{-0.0741920024f,-0.276397526f,-0.0900000036f},
{-0.293946117f,0.0519203208f,0.0300000012f}
};
struct Case { b3Pos p; b3Quat q; int points; b3Vec3 normal; float separation; };
static const Case cases[] = {
{{0.147663489f,0.173645422f,0.509700239f},{{-0.552520335f,-0.451367319f,-0.00953615364f},-0.700641036f},0,{0.0f,0.0f,0.0f},0.0f},
{{-0.0846484676f,-0.370581865f,-0.0927294567f},{{0.274154693f,-0.831894338f,0.0297139101f},0.481568426f},1,{-0.0637530982f,-0.629021406f,-0.774769425f},-0.150585055f},
{{0.131756812f,0.148400565f,0.469579486f},{{-0.552520335f,-0.451367319f,-0.00953615364f},-0.700641036f},4,{0.257813305f,0.695307136f,0.670880258f},-0.0141032785f},
};
int main() {
    for (int index=0; index<3; ++index) {
        auto wd=b3DefaultWorldDef(); wd.gravity=b3Vec3_zero;
        wd.enableSleep=false; wd.enableContinuous=false;
        auto world=b3CreateWorld(&wd);
        auto hull=b3CreateHull(vertices, sizeof(vertices)/sizeof(vertices[0]), sizeof(vertices)/sizeof(vertices[0]));
        auto bd=b3DefaultBodyDef(); bd.type=b3_dynamicBody;
        auto a=b3CreateBody(world,&bd);
        auto sd=b3DefaultShapeDef(); sd.baseMaterial.rollingResistance=0.3f;
        b3CreateHullShape(a,&sd,hull);
        bd.position=cases[index].p; bd.rotation=cases[index].q;
        auto b=b3CreateBody(world,&bd); b3CreateHullShape(b,&sd,hull);
        b3World_Step(world,1e-7f,4);
        int capacity=b3Body_GetContactCapacity(a);
        if(capacity<0 || capacity>16) return 10+index;
        std::vector<b3ContactData> data(capacity);
        int count=b3Body_GetContactData(a,data.data(),capacity);
        int points=0; float minimum=1e30f;
        for(int i=0;i<count;i++) for(int j=0;j<data[i].manifoldCount;j++) {
            auto& m=data[i].manifolds[j];
            float sign=b3Shape_GetBody(data[i].shapeIdA).index1==a.index1 ? 1.0f : -1.0f;
            if(m.pointCount && sign*b3Dot(m.normal,cases[index].normal)<0.99999f) return 20+index;
            points+=m.pointCount;
            for(int k=0;k<m.pointCount;k++) minimum=fminf(minimum,m.points[k].separation);
        }
        if(points!=cases[index].points) { fprintf(stderr,"case %d expected %d points, got %d\n",index,cases[index].points,points); return 30+index; }
        if(points && fabsf(minimum-cases[index].separation)>1e-5f) return 40+index;
        printf("case %d pass: %d points\n",index,points);
        b3DestroyWorld(world); b3DestroyHull(hull);
    }
}
