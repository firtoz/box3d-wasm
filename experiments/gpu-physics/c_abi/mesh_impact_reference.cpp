// Seed 52977, shared pre-impact state (step 55): body 258, or argv[1]=233/842.
#include "box3d/box3d.h"
#include "box3d/collision.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <limits>
#include <vector>
#include <cstring>
#ifdef GPU_REFERENCE
extern "C" void gpu_b3_world_wait(b3WorldId);
struct Health { int dynamic_count,nan_count,worst_body; float min_y,max_y,max_speed; int exploded; };
extern "C" Health gpu_b3_world_health_scan(b3WorldId);
#endif
// Diagnostic only: exact cooked terrain triangles under the body's XZ position.
static double terrainHeight(const b3MeshData* mesh, double x, double z) {
    const auto* vertices=b3GetMeshVertices(mesh);
    const auto* triangles=b3GetMeshTriangles(mesh);
    for(int i=0;i<mesh->triangleCount;++i) {
        auto a=vertices[triangles[i].index1], b=vertices[triangles[i].index2], c=vertices[triangles[i].index3];
        double ux=b.x-a.x, uz=b.z-a.z, vx=c.x-a.x, vz=c.z-a.z;
        double det=ux*vz-uz*vx;
        if(std::abs(det)<1e-12)continue;
        double dx=x-a.x,dz=z-a.z;
        double u=(dx*vz-dz*vx)/det, v=(ux*dz-uz*dx)/det;
        if(u>=-1e-7 && v>=-1e-7 && u+v<=1.0+1e-7)
            return a.y+u*(b.y-a.y)+v*(c.y-a.y);
    }
    return std::numeric_limits<double>::quiet_NaN();
}
int main(int argc, char** argv) {
    const int case_id = argc > 1 ? std::atoi(argv[1]) : 258;
    if(case_id != 258 && case_id != 233 && case_id != 842) { std::fprintf(stderr,"expected body 258, 233, or 842\n"); return 64; }
    const bool case233 = case_id == 233;
    const bool split = argc > 2 && std::strcmp(argv[2],"split")==0;
    const bool frozen57 = argc > 2 && std::strcmp(argv[2],"frozen57")==0;
    if(frozen57 && case_id != 842) { std::fprintf(stderr,"frozen57 is body 842 only\n"); return 64; }
    const int start_step = frozen57 ? 57 : 55;
    std::fprintf(stderr,"reference_start=%d\n",start_step);

    std::fprintf(stderr,"reference_case=%d diagnostic_split_mesh=%d\n",case_id,int(split));
    b3WorldDef wd=b3DefaultWorldDef(); wd.enableSleep=false;
    b3WorldId world=b3CreateWorld(&wd);
    b3BodyDef gd=b3DefaultBodyDef(); auto ground=b3CreateBody(world,&gd);
    b3Vec3 v[]={{-4,0.279481441f,-9},{-4,0.451831192f,-8},{-3,0.279481351f,-8},{-3,0.172873959f,-9}};
    int indices[]={0,1,2,2,3,0};
    b3MeshDef md={};md.vertices=v;md.vertexCount=4;md.indices=indices;md.triangleCount=2;md.identifyEdges=true;
    auto mesh=case_id != 258 ? b3CreateWaveMesh(40,40,1.0f,0.5f,0.1f,0.2f) : b3CreateMesh(&md,nullptr,0);
    b3ShapeDef sd=b3DefaultShapeDef();
    std::vector<b3MeshData*> pieces;
    if(split) {
        const auto* vertices=b3GetMeshVertices(mesh);
        const auto* triangles=b3GetMeshTriangles(mesh);
        const auto* flags=b3GetMeshFlags(mesh);
        for(int i=0;i<mesh->triangleCount;++i) {
            b3Vec3 points[]={vertices[triangles[i].index1],vertices[triangles[i].index2],vertices[triangles[i].index3]};
            int index[]={0,1,2};
            b3MeshDef part={};part.vertices=points;part.indices=index;part.vertexCount=3;part.triangleCount=1;part.identifyEdges=true;
            auto* piece=b3CreateMesh(&part,nullptr,0);
            // These are owned diagnostic allocations; retain the full mesh's edge semantics.
            const_cast<uint8_t*>(b3GetMeshFlags(piece))[0]=flags ? flags[i] : 0;
            pieces.push_back(piece);
            b3CreateMeshShape(ground,&sd,piece,b3Vec3_one);
        }
    } else {
        b3CreateMeshShape(ground,&sd,mesh,b3Vec3_one);
    }
    b3BodyDef bd=b3DefaultBodyDef();bd.type=b3_dynamicBody;
    bd.position={-3.53273058f,0.575324297f,-8.47112465f};
    bd.rotation={{-0.576700628f,-0.544579446f,-0.599283516f},0.10820777f};
    bd.linearVelocity={0.509750605f,-9.38941956f,-0.513962209f};
    bd.angularVelocity={-1.19869685f,1.00277746f,-3.39283967f};
    if(case233) {
        bd.position={-4.72423267f,0.0754624307f,-4.62318373f};
        bd.rotation={{0.577766478f,0.534366727f,0.606414497f},0.113576852f};
        bd.linearVelocity={-0.244605839f,-9.93472862f,-0.134373009f};
        bd.angularVelocity={-5.8978343f,1.27596903f,-0.77363658f};
    }
    if(case_id == 842) {
        bd.position={5.83642721f,-0.0566934943f,-3.24710274f};
        bd.rotation={{-0.663498938f,0.56556958f,0.449735373f},-0.194006249f};
        bd.linearVelocity={0.912472963f,-10.0788984f,0.821344614f};
        bd.angularVelocity={-0.725532293f,-0.880873442f,4.30312109f};
    }
    if(frozen57) {
        // CPU frame 57, before the three-patch collision is generated. Compare
        // narrowphase from identical transforms, not after divergent impacts.
        bd.position={5.884583f,-0.31813094f,-3.25575447f};
        bd.rotation={{-0.512286186f,0.728431702f,0.433896542f},-0.136688843f};
        bd.linearVelocity={2.83454585f,-5.53687859f,-2.01074696f};
        bd.angularVelocity={-32.8008461f,-4.80442047f,-18.9829712f};
    }

    auto body=b3CreateBody(world,&bd);
    auto hull=b3MakeBoxHull(0.02f,0.2f,0.04f);
    sd.baseMaterial.rollingResistance=0.1f; // Native Mesh Drop box default.
    b3CreateHullShape(body,&sd,&hull.base);
    for(int step=start_step;step<=start_step+45;++step){
        if(step>start_step)b3World_Step(world,1.0f/60,4);
#ifdef GPU_REFERENCE
        gpu_b3_world_wait(world);
#endif
        if(step<=start_step+45) {
            std::fprintf(stderr,"reference_step=%d\n",step);
#ifdef GPU_REFERENCE
            gpu_b3_world_health_scan(world);
#else
            b3ContactData data[16];int count=b3Body_GetContactData(body,data,16);
            for(int i=0;i<count;++i)for(int j=0;j<data[i].manifoldCount;++j){
                const auto& m=data[i].manifolds[j];
                std::fprintf(stderr,"manifold n=(%g,%g,%g) count=%d\n",m.normal.x,m.normal.y,m.normal.z,m.pointCount);
                for(int k=0;k<m.pointCount;++k){const auto& p=m.points[k];
                    std::fprintf(stderr," p rb=(%g,%g,%g) sep=%g base=%g impulse=%g tri=%d\n",p.anchorB.x,p.anchorB.y,p.anchorB.z,p.separation,p.baseSeparation,p.normalImpulse,p.triangleIndex);}
            }
#endif
        }
        auto p=b3Body_GetPosition(body);auto q=b3Body_GetRotation(body);
        auto v=b3Body_GetLinearVelocity(body);auto w=b3Body_GetAngularVelocity(body);
        std::fprintf(stderr,"support_height step=%d y=%.12g\n",step,terrainHeight(mesh,p.x,p.z));
        std::printf("%d %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",step,
            double(p.x),double(p.y),double(p.z),q.v.x,q.v.y,q.v.z,q.s,v.x,v.y,v.z,w.x,w.y,w.z);
    }
    b3DestroyWorld(world);
    for(auto* piece:pieces)b3DestroyMesh(piece);
    b3DestroyMesh(mesh);
}
