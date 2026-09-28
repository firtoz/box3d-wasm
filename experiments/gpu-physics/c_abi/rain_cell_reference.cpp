// Isolated upstream Rain cell (defaults to row 6, column 1), retaining world coordinates,
// three Human definitions, terrain, spawn time, materials and joint defaults.
// Diagnostic fixture: global contact ordering differs from the complete Rain scene.
#include "box3d/box3d.h"
#include "box3d/collision.h"
#include "human.h"
#include "human_joint_audit.h"
#include "spherical_limit_health.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <initializer_list>
#include <vector>
extern "C" bool reference_set_spherical_cache(b3JointId,const float*) __attribute__((weak));
extern "C" bool reference_set_revolute_cache(b3JointId,const float*) __attribute__((weak));
extern "C" void b3_world_gpu_wait_with_mirror(b3WorldId) __attribute__((weak));
extern "C" bool gpu_b3_world_write_core_state(b3WorldId,const char*,unsigned) __attribute__((weak));
int main(int argc,char** argv) {
    const int steps=argc>1?std::atoi(argv[1]):210;
    const bool continuous=!(argc>2 && std::strcmp(argv[2],"no-ccd")==0);
    const int column=std::getenv("RAIN_CELL_COLUMN")?std::atoi(std::getenv("RAIN_CELL_COLUMN")):1;
    const int row=std::getenv("RAIN_CELL_ROW")?std::atoi(std::getenv("RAIN_CELL_ROW")):6;
    const int base=std::getenv("RAIN_REPLAY_BASE")?std::atoi(std::getenv("RAIN_REPLAY_BASE")):673;
    if(column<0 || column>=10 || row<0 || row>=10 || base<1)return 3;
    const float cellX=-75.0f+15.0f*(column+0.5f);
    const float cellZ=-75.0f+15.0f*(row+0.5f);
    auto wd=b3DefaultWorldDef(); wd.enableContinuous=continuous;
    // Explicit diagnostic ablation only; absent means upstream defaults.
    if(const char* sleep=std::getenv("RAIN_REPLAY_SLEEP")) {
        if(std::strcmp(sleep,"0")!=0 && std::strcmp(sleep,"1")!=0)return 3;
        wd.enableSleep=std::strcmp(sleep,"1")==0;
    }
    auto world=b3CreateWorld(&wd);
    auto grid=b3CreateGridMesh(8,8,15.0f/8.0f,1,true);
    auto torus=b3CreateTorusMesh(16,16,3.75f,1.0f);
    auto bd=b3DefaultBodyDef(); bd.position={cellX,0.0f,cellZ};
    auto ground=b3CreateBody(world,&bd); auto sd=b3DefaultShapeDef();
    b3CreateMeshShape(ground,&sd,grid,b3Vec3_one);
    b3CreateMeshShape(ground,&sd,torus,b3Vec3_one);
    Human humans[3]={};
    const char* replay=std::getenv("RAIN_REPLAY_STATE");
    const int spawn=replay?0:64*(column/2)+16*(column%2);
    std::fprintf(stderr,"rain-cell row=%d column=%d group=%d spawn=%d humans=3 continuous=%d\n",row,column,10*row+column,spawn,int(continuous));
    std::fprintf(stderr,"rain-cell-sleep %d\n",int(wd.enableSleep));
    for(int frame=0;frame<steps;++frame) {
        if(frame==spawn) {
            b3Pos position={cellX,20.0f,cellZ};
            for(auto& human:humans) {
                CreateHuman(&human,world,position,5.0f,1.0f,0.7f,10*row+column,nullptr,false);
                position.x+=0.75f;
            }
            if(std::getenv("RAIN_AUDIT_JOINTS")) dumpHumanJointSetup({&humans[0],&humans[1],&humans[2]});
            if(replay) {
                // Diagnostic state transplant: joints retain upstream local
                // frames but start with fresh impulses and contact history.
                FILE* input=std::fopen(replay,"r"); if(!input)return 3;
                for(int i=0;i<3*bone_count;++i) {
                    int sourceFrame,id; float x[14];
                    if(std::fscanf(input,"%d %d",&sourceFrame,&id)!=2 || id!=base+i)return 4;
                    for(float& value:x)if(std::fscanf(input,"%f",&value)!=1 || !std::isfinite(value))return 4;
                    auto body=humans[i/bone_count].bones[i%bone_count].bodyId;
                    b3Body_SetTransform(body,{x[0],x[1],x[2]},{{x[3],x[4],x[5]},x[6]});
                    b3Body_SetLinearVelocity(body,{x[7],x[8],x[9]});
                    b3Body_SetAngularVelocity(body,{x[10],x[11],x[12]});
                    auto p0=b3Body_GetPosition(body);auto q0=b3Body_GetRotation(body);
                    auto v0=b3Body_GetLinearVelocity(body);auto w0=b3Body_GetAngularVelocity(body);
                    std::fprintf(stderr,"replay-initial %d",id);
                    for(float value:{p0.x,p0.y,p0.z,q0.v.x,q0.v.y,q0.v.z,q0.s,v0.x,v0.y,v0.z,w0.x,w0.y,w0.z})std::fprintf(stderr," %.9g",value);
                    std::fputc('\n',stderr);
                }
                std::fclose(input);
                std::fprintf(stderr,"rain-replay fresh history from %s\n",replay);
            }
            if(const char* cache=std::getenv("RAIN_REPLAY_SPHERICAL_CACHE")) {
                if(!replay || !reference_set_spherical_cache)return 6;
                FILE* input=std::fopen(cache,"r");if(!input)return 6;
                bool seen[3*bone_count]={};int count=0,a,b;
                while(true) {
                    const int fields=std::fscanf(input,"%d %d",&a,&b);
                    if(fields==EOF)break;
                    if(fields!=2 || a<0 || a>=3*bone_count || b<0 || b>=3*bone_count || seen[b])return 6;
                    float values[12];for(float& v:values)if(std::fscanf(input,"%f",&v)!=1 || !std::isfinite(v))return 6;
                    const auto joint=humans[b/bone_count].bones[b%bone_count].jointId;
                    const auto parent=humans[a/bone_count].bones[a%bone_count].bodyId;
                    const auto child=humans[b/bone_count].bones[b%bone_count].bodyId;
                    if(B3_IS_NULL(joint) || !B3_ID_EQUALS(b3Joint_GetBodyA(joint),parent)
                        || !B3_ID_EQUALS(b3Joint_GetBodyB(joint),child)
                        || !reference_set_spherical_cache(joint,values))return 6;
                    seen[b]=true;++count;
                    std::fprintf(stderr,"replay-spherical-cache %d %d",a,b);
                    for(float v:values)std::fprintf(stderr," %.9g",v);
                    std::fputc('\n',stderr);
                }
                std::fclose(input);
                for(int i=0;i<3*bone_count;++i) {
                    const auto j=humans[i/bone_count].bones[i%bone_count].jointId;
                    if(!B3_IS_NULL(j) && b3Joint_GetType(j)==b3_sphericalJoint && !seen[i])return 6;
                }
                std::fprintf(stderr,"replay-spherical-cache-count %d\n",count);
            }
            if(const char* cache=std::getenv("RAIN_REPLAY_REVOLUTE_CACHE")) {
                if(!replay || !reference_set_revolute_cache)return 6;
                FILE* input=std::fopen(cache,"r");if(!input)return 6;
                bool seen[3*bone_count]={};int count=0,a,b;
                while(true) {
                    const int fields=std::fscanf(input,"%d %d",&a,&b);
                    if(fields==EOF)break;
                    if(fields!=2 || a<0 || a>=3*bone_count || b<0 || b>=3*bone_count || seen[b])return 6;
                    float values[9];for(float& v:values)if(std::fscanf(input,"%f",&v)!=1 || !std::isfinite(v))return 6;
                    const auto joint=humans[b/bone_count].bones[b%bone_count].jointId;
                    const auto parent=humans[a/bone_count].bones[a%bone_count].bodyId;
                    const auto child=humans[b/bone_count].bones[b%bone_count].bodyId;
                    if(B3_IS_NULL(joint) || !B3_ID_EQUALS(b3Joint_GetBodyA(joint),parent)
                        || !B3_ID_EQUALS(b3Joint_GetBodyB(joint),child)
                        || !reference_set_revolute_cache(joint,values))return 6;
                    seen[b]=true;++count;
                    std::fprintf(stderr,"replay-revolute-cache %d %d",a,b);
                    for(float v:values)std::fprintf(stderr," %.9g",v);
                    std::fputc('\n',stderr);
                }
                std::fclose(input);
                for(int i=0;i<3*bone_count;++i) {
                    const auto j=humans[i/bone_count].bones[i%bone_count].jointId;
                    if(!B3_IS_NULL(j) && b3Joint_GetType(j)==b3_revoluteJoint && !seen[i])return 6;
                }
                std::fprintf(stderr,"replay-revolute-cache-count %d\n",count);
            }
            if(const char* mode=std::getenv("RAIN_REPLAY_TARGET_DRIVE")) {
                const char* targetText=std::getenv("RAIN_REPLAY_TARGET_BODY");
                if(!targetText)return 5;
                const int index=std::atoi(targetText)-base;
                if(index<0 || index>=3*bone_count)return 5;
                const auto joint=humans[index/bone_count].bones[index%bone_count].jointId;
                if(B3_IS_NULL(joint) || b3Joint_GetType(joint)!=b3_sphericalJoint)return 5;
                const bool sham=std::strcmp(mode,"sham")==0;
                const bool noSpring=std::strcmp(mode,"no-spring")==0;
                const bool noMotor=std::strcmp(mode,"no-motor")==0;
                if(!sham && !noSpring && !noMotor)return 5;
                const bool spring=b3SphericalJoint_IsSpringEnabled(joint);
                const bool motor=b3SphericalJoint_IsMotorEnabled(joint);
                if(!spring || !motor)return 5;
                // Matched setter calls in all arms. Only the requested drive
                // differs; normal fixture runs never enter this ablation.
                b3SphericalJoint_EnableSpring(joint,spring && !noSpring);
                b3SphericalJoint_EnableMotor(joint,motor && !noMotor);
                if(b3SphericalJoint_IsSpringEnabled(joint)!=(spring && !noSpring)
                    || b3SphericalJoint_IsMotorEnabled(joint)!=(motor && !noMotor))return 5;
                std::fprintf(stderr,"replay-target-drive %d %s\n",base+index,mode);
            }
            if(const char* mode=std::getenv("RAIN_REPLAY_COLLISIONS")) {
                if(std::strcmp(mode,"0")!=0 && std::strcmp(mode,"1")!=0)return 5;
                const bool enabled=std::strcmp(mode,"1")==0;
                for(auto& human:humans)for(auto& bone:human.bones) {
                    std::vector<b3ShapeId> shapes(b3Body_GetShapeCount(bone.bodyId));
                    const int count=b3Body_GetShapes(bone.bodyId,shapes.data(),int(shapes.size()));
                    if(count!=int(shapes.size()))return 5;
                    for(auto shape:shapes) {
                        auto filter=b3Shape_GetFilter(shape);
                        if(!enabled) {filter.maskBits=0;filter.groupIndex=0;}
                        // The enabled arm repeats the original filter setter.
                        b3Shape_SetFilter(shape,filter,true);
                        const auto actual=b3Shape_GetFilter(shape);
                        if(actual.maskBits!=filter.maskBits || actual.categoryBits!=filter.categoryBits
                            || actual.groupIndex!=filter.groupIndex)return 5;
                    }
                }
                std::fprintf(stderr,"replay-collisions %d\n",int(enabled));
            }
        }
        if(const char* resetStep=std::getenv("RAIN_CONE_RESET_STEP")) {
            if(frame==std::atoi(resetStep)) {
                const char* targetText=std::getenv("RAIN_CONE_RESET_BODY");
                const char* mode=std::getenv("RAIN_CONE_RESET_MODE");
                if(!targetText || !mode)return 5;
                const int target=std::atoi(targetText), index=target-base;
                if(index<0 || index>=3*bone_count)return 5;
                const auto joint=humans[index/bone_count].bones[index%bone_count].jointId;
                if(B3_IS_NULL(joint) || b3Joint_GetType(joint)!=b3_sphericalJoint
                    || !b3SphericalJoint_IsConeLimitEnabled(joint))return 5;
                const bool clear=std::strcmp(mode,"clear")==0;
                if(!clear && std::strcmp(mode,"sham")!=0)return 5;
                // Both arms perform two setter calls (including upload/wake
                // effects); only the clear arm resets the cached cone impulse.
                b3SphericalJoint_EnableConeLimit(joint,!clear);
                b3SphericalJoint_EnableConeLimit(joint,true);
                if(!b3SphericalJoint_IsConeLimitEnabled(joint))return 5;
                std::fprintf(stderr,"cone-cache-control %d %d %s\n",frame,target,mode);
            }
        }
        if(frame==0 && std::getenv("RAIN_REPLAY_ZERO_STEP")) {
            if(!replay || std::strcmp(std::getenv("RAIN_REPLAY_ZERO_STEP"),"1")!=0)return 7;
            auto snapshot=[&]() {
                std::vector<float> data;
                for(auto& human:humans)for(auto& bone:human.bones) {
                    auto p=b3Body_GetPosition(bone.bodyId);auto q=b3Body_GetRotation(bone.bodyId);
                    auto v=b3Body_GetLinearVelocity(bone.bodyId);auto w=b3Body_GetAngularVelocity(bone.bodyId);
                    for(float x:{p.x,p.y,p.z,q.v.x,q.v.y,q.v.z,q.s,v.x,v.y,v.z,w.x,w.y,w.z,float(b3Body_IsAwake(bone.bodyId))})data.push_back(x);
                }
                return data;
            };
            auto before=snapshot();b3World_Step(world,0.0f,4);auto after=snapshot();
            if(before.size()!=after.size() || std::memcmp(before.data(),after.data(),before.size()*sizeof(float))!=0)return 7;
            auto index=[&](b3BodyId id) {
                if(B3_ID_EQUALS(id,ground))return -1;
                for(int i=0;i<3*bone_count;++i)if(B3_ID_EQUALS(id,humans[i/bone_count].bones[i%bone_count].bodyId))return base+i;
                return -2;
            };
            std::vector<b3ContactId> seen;
            int patches=0;
            for(auto& human:humans)for(auto& bone:human.bones) {
                std::vector<b3ContactData> contacts(b3Body_GetContactCapacity(bone.bodyId));
                const int n=b3Body_GetContactData(bone.bodyId,contacts.data(),int(contacts.size()));
                for(int i=0;i<n;++i) {
                    const auto& c=contacts[i];bool duplicate=false;
                    for(auto id:seen)if(B3_ID_EQUALS(id,c.contactId))duplicate=true;
                    if(duplicate)continue;seen.push_back(c.contactId);
                    int a=index(b3Shape_GetBody(c.shapeIdA)),b=index(b3Shape_GetBody(c.shapeIdB));
                    if(a==-2 || b==-2)return 7;
                    for(int m=0;m<c.manifoldCount;++m) {
                        const auto& patch=c.manifolds[m];++patches;
                        std::fprintf(stderr,"zero-step-manifold %d %d %d %.9g %.9g %.9g\n",a,b,patch.pointCount,patch.normal.x,patch.normal.y,patch.normal.z);
                        for(int k=0;k<patch.pointCount;++k) {
                            const auto& p=patch.points[k];
                            std::fprintf(stderr,"zero-step-point %d %d %u %d %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",a,b,p.featureId,p.triangleIndex,p.anchorA.x,p.anchorA.y,p.anchorA.z,p.anchorB.x,p.anchorB.y,p.anchorB.z,p.separation);
                        }
                    }
                }
            }
            std::fprintf(stderr,"zero-step-preserved-bodies 42 contacts=%zu patches=%d\n",seen.size(),patches);
        }
        b3World_Step(world,1.0f/60.0f,4);
        if(b3_world_gpu_wait_with_mirror)b3_world_gpu_wait_with_mirror(world);
        if(const char* trace=std::getenv("GPU_PHYSICS_STATE_TRACE")) {
            if(!gpu_b3_world_write_core_state || !gpu_b3_world_write_core_state(world,trace,frame+1))return 2;
        }
        if(frame<spawn)continue;
        for(int h=0;h<3;++h)for(int b=0;b<bone_count;++b) {
            auto body=humans[h].bones[b].bodyId;auto joint=humans[h].bones[b].jointId;
            auto p=b3Body_GetPosition(body);auto q=b3Body_GetRotation(body);
            auto v=b3Body_GetLinearVelocity(body);auto w=b3Body_GetAngularVelocity(body);
            float error=0;
            if(B3_IS_NON_NULL(joint)) {
                const auto& bone=humans[h].bones[b];
                auto a=b3Body_GetWorldPoint(humans[h].bones[bone.parentIndex].bodyId,bone.localFrameA.p);
                auto z=b3Body_GetWorldPoint(body,bone.localFrameB.p);
                double dx=a.x-z.x,dy=a.y-z.y,dz=a.z-z.z;
                error=float(std::sqrt(dx*dx+dy*dy+dz*dz));
            }
            if(std::getenv("RAIN_REPLAY_HEALTH") && humans[h].bones[b].jointType==b3_revoluteJoint) {
                const auto& bone=humans[h].bones[b];
                auto qa=b3NormalizeQuat(b3MulQuat(b3Body_GetRotation(humans[h].bones[bone.parentIndex].bodyId),bone.localFrameA.q));
                auto qb=b3NormalizeQuat(b3MulQuat(q,bone.localFrameB.q));
                float dot=b3Dot(b3RotateVector(qa,{0,0,1}),b3RotateVector(qb,{0,0,1}));
                float angular=std::acos(std::fmax(-1.0f,std::fmin(1.0f,dot)));
                std::fprintf(stderr,"replay-health %d %d %.9g %.9g %d\n",frame,base+h*bone_count+b,error,angular,int(b3Body_IsAwake(body)));
            }
            if(std::getenv("RAIN_REPLAY_LIMITS") && B3_IS_NON_NULL(joint)
                && humans[h].bones[b].jointType==b3_sphericalJoint) {
                const auto fa=b3Joint_GetLocalFrameA(joint), fb=b3Joint_GetLocalFrameB(joint);
                const auto qa=b3MulQuat(b3Body_GetRotation(b3Joint_GetBodyA(joint)),fa.q);
                const auto qb=b3MulQuat(q,fb.q);
                const bool cone=b3SphericalJoint_IsConeLimitEnabled(joint);
                const bool twist=b3SphericalJoint_IsTwistLimitEnabled(joint);
                const float limit=b3SphericalJoint_GetConeLimit(joint);
                const float lower=b3SphericalJoint_GetLowerTwistLimit(joint);
                const float upper=b3SphericalJoint_GetUpperTwistLimit(joint);
                const auto health=measureSphericalLimits(b3InvMulQuat(qa,qb),cone,limit,twist,lower,upper);
                std::fprintf(stderr,"replay-spherical-limit %d %d %d %.9g %.9g %.9g %d %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",
                    frame,base+h*bone_count+b,int(cone),health.swing,limit,health.coneExcess,
                    int(twist),health.twist,lower,upper,health.lowerTwistExcess,health.upperTwistExcess,
                    b3SphericalJoint_GetConeAngle(joint),b3SphericalJoint_GetTwistAngle(joint));
                std::fprintf(stderr,"replay-spherical-awake %d %d %d %d\n",
                    frame,base+h*bone_count+b,int(b3Body_IsAwake(b3Joint_GetBodyA(joint))),
                    int(b3Body_IsAwake(body)));
            }
            std::printf("%d %d",frame,base+h*bone_count+b);
            for(float x:{p.x,p.y,p.z,q.v.x,q.v.y,q.v.z,q.s,v.x,v.y,v.z,w.x,w.y,w.z,error})std::printf(" %.9g",x);
            std::puts("");
        }
        if(frame>=145 && frame<=180) {
            auto body=humans[0].bones[bone_calf_r].bodyId;
            std::vector<b3ContactData> contacts(b3Body_GetContactCapacity(body));
            int count=b3Body_GetContactData(body,contacts.data(),int(contacts.size()));
            for(int c=0;c<count;++c)for(int m=0;m<contacts[c].manifoldCount;++m) {
                const auto& manifold=contacts[c].manifolds[m];
                std::fprintf(stderr,"calf-contact %d %d %d %d %d %.9g %.9g %.9g\n",frame,
                    contacts[c].shapeIdA.index1,contacts[c].shapeIdB.index1,m,manifold.pointCount,
                    manifold.normal.x,manifold.normal.y,manifold.normal.z);
            }
        }
    }
    for(auto& human:humans)if(human.isSpawned)DestroyHuman(&human);
    b3DestroyWorld(world);b3DestroyMesh(grid);b3DestroyMesh(torus);
}
