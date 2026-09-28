// Discrete collision diagnostic for Rain calf poses. Each input gets a fresh
// body/contact lifetime; zero gravity/velocity and a tiny step isolate geometry.
#include "box3d/box3d.h"
#include "box3d/collision.h"
#include "human.h"
#include <cstdio>
#include <cstdlib>
#include <vector>
extern "C" void b3_world_gpu_wait_with_mirror(b3WorldId) __attribute__((weak));
extern "C" void gpu_b3_world_dump_mesh_candidates(b3WorldId) __attribute__((weak));
int main() {
    auto wd=b3DefaultWorldDef(); wd.gravity={0,0,0}; wd.enableContinuous=false;
    auto world=b3CreateWorld(&wd);
    Human human={}; CreateHuman(&human,world,{0,20,0},5,1,.7f,61,nullptr,false);
    b3ShapeId shape; b3Body_GetShapes(human.bones[bone_calf_r].bodyId,&shape,1);
    auto capsule=b3Shape_GetCapsule(shape); DestroyHuman(&human);
    const bool grid=std::getenv("RAIN_FROZEN_GRID")!=nullptr;
    auto mesh=grid?b3CreateGridMesh(2,2,2,1,true):b3CreateTorusMesh(16,16,3.75f,1);
    if(grid && std::getenv("RAIN_FROZEN_TRACE")) {
        for(int i=0;i<mesh->triangleCount;++i) {
            auto t=b3GetMeshTriangles(mesh)[i];
            std::fprintf(stderr,"grid-triangle %d vertices %d %d %d flags %u\n",i,t.index1,t.index2,t.index3,unsigned(b3GetMeshFlags(mesh)[i]));
        }
    }
    auto bd=b3DefaultBodyDef(); bd.position=grid?b3Pos{0,0,0}:b3Pos{-52.5f,0,22.5f};
    auto ground=b3CreateBody(world,&bd); auto sd=b3DefaultShapeDef();
    b3CreateMeshShape(ground,&sd,mesh,b3Vec3_one);
    int id; b3Pos p; b3Quat q;
    while(std::scanf("%d %f %f %f %f %f %f %f",&id,&p.x,&p.y,&p.z,&q.v.x,&q.v.y,&q.v.z,&q.s)==8) {
        if(std::getenv("RAIN_FROZEN_TRIANGLE")) {
          for(int ti=grid?0:412; ti<(grid?mesh->triangleCount:413); ++ti) {
            auto t=b3GetMeshTriangles(mesh)[ti]; auto vertices=b3GetMeshVertices(mesh);
            b3Vec3 tri[3]; int indices[3]={t.index1,t.index2,t.index3};
            auto delta=b3Sub(grid?b3Vec3{0,0,0}:b3Vec3{-52.5f,0,22.5f},b3Vec3{p.x,p.y,p.z});
            auto translation=b3InvRotateVector(q,delta);
            for(int i=0;i<3;++i)tri[i]=b3Add(b3InvRotateVector(q,vertices[indices[i]]),translation);
            b3LocalManifoldPoint points[4]={}; b3LocalManifold m={};m.points=points;b3SimplexCache cache={};
            b3CollideTriangleAndCapsule(&m,4,tri,&capsule,&cache);
            std::fprintf(stderr,"triangle-case %d triangle %d flags %u count %d feature %d\n",id,ti,unsigned(b3GetMeshFlags(mesh)[ti]),m.pointCount,int(m.feature));
            for(auto v:tri)std::fprintf(stderr,"triangle-vertex %.9g %.9g %.9g\n",v.x,v.y,v.z);
          }
        }
        bd=b3DefaultBodyDef(); bd.type=b3_dynamicBody; bd.position=p; bd.rotation=q;
        auto body=b3CreateBody(world,&bd); b3CreateCapsuleShape(body,&sd,&capsule);
        b3World_Step(world,1e-6f,1);
        if(b3_world_gpu_wait_with_mirror)b3_world_gpu_wait_with_mirror(world);
        if(gpu_b3_world_dump_mesh_candidates && std::getenv("RAIN_FROZEN_TRACE")) {
            std::fprintf(stderr,"frozen-case %d\n",id);
            gpu_b3_world_dump_mesh_candidates(world);
        }
        std::vector<b3ContactData> contacts(b3Body_GetContactCapacity(body));
        int n=b3Body_GetContactData(body,contacts.data(),int(contacts.size()));
        int points=0;
        for(int c=0;c<n;++c)for(int m=0;m<contacts[c].manifoldCount;++m) {
            const auto& v=contacts[c].manifolds[m]; points+=v.pointCount;
            std::printf("manifold %d %d %.9g %.9g %.9g",id,v.pointCount,v.normal.x,v.normal.y,v.normal.z);
            for(int i=0;i<v.pointCount;++i)std::printf(" %.9g:%d:%u",v.points[i].separation,v.points[i].triangleIndex,v.points[i].featureId);
            std::puts("");
        }
        std::printf("case %d %d %d\n",id,n,points);
        b3DestroyBody(body);
    }
    b3DestroyWorld(world); b3DestroyMesh(mesh);
}
