extern "C" float cpu_b3Body_GetGravityScale(b3BodyId) __attribute__((weak));
extern "C" b3MotionLocks cpu_b3Body_GetMotionLocks(b3BodyId) __attribute__((weak));
extern "C" bool cpu_b3Body_IsBullet(b3BodyId) __attribute__((weak));
static int lock_bits(b3MotionLocks a){return(a.linearX?1:0)|(a.linearY?2:0)|(a.linearZ?4:0)|(a.angularX?8:0)|(a.angularY?16:0)|(a.angularZ?32:0);}
// Read-only observers: each original operation is invoked exactly once.
#include <cmath>
#include <algorithm>
extern "C" b3Quat cpu_b3Body_GetRotation(b3BodyId) __attribute__((weak));
extern "C" b3Vec3 cpu_b3Body_GetAngularVelocity(b3BodyId) __attribute__((weak));
extern "C" bool cpu_b3Body_IsAwake(b3BodyId) __attribute__((weak));
extern "C" b3ShapeType cpu_b3Shape_GetType(b3ShapeId) __attribute__((weak));
extern "C" float cpu_b3Shape_GetRestitution(b3ShapeId) __attribute__((weak));
namespace {
struct MutationShape {b3ShapeId id;std::string kind;float restitution;bool invoke;float radius;int vertices;};
struct MutationBody {WatchedBody ref;b3BodyDef def;std::vector<MutationShape> shapes;};
std::vector<MutationBody> mutation_bodies;
unsigned mutation_created=0,mutation_destroyed=0,mutation_shapes_created=0;
bool mutation_enabled(){return std::getenv("GPU_PR10_MUTATION_TRACE");}
void mutation_body_json(FILE* f,MutationBody& b){
  float gravity=b.ref.cpu?cpu_b3Body_GetGravityScale(b.ref.id):b3Body_GetGravityScale(b.ref.id);
  auto locks=b.ref.cpu?cpu_b3Body_GetMotionLocks(b.ref.id):b3Body_GetMotionLocks(b.ref.id);
  bool bullet=b.ref.cpu?cpu_b3Body_IsBullet(b.ref.id):b3Body_IsBullet(b.ref.id);
  auto p=b.ref.cpu?cpu_b3Body_GetPosition(b.ref.id):b3Body_GetPosition(b.ref.id);
  auto v=b.ref.cpu?cpu_b3Body_GetLinearVelocity(b.ref.id):b3Body_GetLinearVelocity(b.ref.id);
  int type=b.ref.cpu?cpu_b3Body_GetType(b.ref.id):b3Body_GetType(b.ref.id);
  float mass=b.ref.cpu?cpu_b3Body_GetMass(b.ref.id):b3Body_GetMass(b.ref.id);
  bool enabled=b.ref.cpu?cpu_b3Body_IsEnabled(b.ref.id):b3Body_IsEnabled(b.ref.id);
  std::fprintf(f,"{\"owner\":\"%s\",\"id\":[%u,%u,%u],\"valid\":true,\"type\":%d,\"mass\":%.9g,\"enabled\":%s,\"position\":[%.9g,%.9g,%.9g],\"velocity\":[%.9g,%.9g,%.9g]",b.ref.cpu?"mapped_cpu":"public",b.ref.id.index1,b.ref.id.generation,b.ref.id.world0,type,mass,enabled?"true":"false",(double)p.x,(double)p.y,(double)p.z,v.x,v.y,v.z);
  std::fprintf(f,",\"gravity_scale\":%.9g,\"lock_bits\":%d,\"is_bullet\":%s",gravity,lock_bits(locks),bullet?"true":"false");
  b3Quat q=b.ref.cpu?cpu_b3Body_GetRotation(b.ref.id):b3Body_GetRotation(b.ref.id);
  b3Vec3 w=b.ref.cpu?cpu_b3Body_GetAngularVelocity(b.ref.id):b3Body_GetAngularVelocity(b.ref.id);
  bool awake=b.ref.cpu?cpu_b3Body_IsAwake(b.ref.id):b3Body_IsAwake(b.ref.id);
  std::fprintf(f,",\"rotation\":[%.9g,%.9g,%.9g,%.9g],\"angular_velocity\":[%.9g,%.9g,%.9g],\"awake\":%s,\"creation_position\":[%.9g,%.9g,%.9g],\"shapes\":[",q.v.x,q.v.y,q.v.z,q.s,w.x,w.y,w.z,awake?"true":"false",(double)b.def.position.x,(double)b.def.position.y,(double)b.def.position.z);
  for(size_t i=0;i<b.shapes.size();++i){auto s=b.shapes[i];if(i)std::fputc(',',f);int type=b.ref.cpu?cpu_b3Shape_GetType(s.id):b3Shape_GetType(s.id);float restitution=b.ref.cpu?cpu_b3Shape_GetRestitution(s.id):b3Shape_GetRestitution(s.id);std::fprintf(f,"{\"id\":[%u,%u,%u],\"kind\":",s.id.index1,s.id.generation,s.id.world0);quote(f,s.kind);std::fprintf(f,",\"actual_type\":%d,\"restitution\":%.9g,\"definition_restitution\":%.9g,\"definition_invoke\":%s,\"radius\":%.9g,\"vertices\":%d}",type,restitution,s.restitution,s.invoke?"true":"false",s.radius,s.vertices);}
  std::fputs("]}",f);
}
// Serialize complete small fixtures, bounded examples + aggregate/digest for the 1089-body benchmark.
void mutation_world_json(FILE* f,bool cpu){
  int count=0,dynamic=0,nonzero=0,lock_counts[64]={};float gravity_min=INFINITY,gravity_max=-INFINITY;double speed2=0;float vmin[3]={INFINITY,INFINITY,INFINITY},vmax[3]={-INFINITY,-INFINITY,-INFINITY};uint64_t ids=1469598103934665603ull;
  for(auto& b:mutation_bodies){if(b.ref.cpu!=cpu||!body_valid(b.ref))continue;++count;for(unsigned v:{(unsigned)b.ref.id.index1,(unsigned)b.ref.id.generation,(unsigned)b.ref.id.world0})ids=(ids^v)*1099511628211ull;const int type=cpu?cpu_b3Body_GetType(b.ref.id):b3Body_GetType(b.ref.id);if(type!=b3_dynamicBody)continue;++dynamic;float gravity=cpu?cpu_b3Body_GetGravityScale(b.ref.id):b3Body_GetGravityScale(b.ref.id);gravity_min=std::min(gravity_min,gravity);gravity_max=std::max(gravity_max,gravity);++lock_counts[lock_bits(cpu?cpu_b3Body_GetMotionLocks(b.ref.id):b3Body_GetMotionLocks(b.ref.id))];auto v=cpu?cpu_b3Body_GetLinearVelocity(b.ref.id):b3Body_GetLinearVelocity(b.ref.id);const float a[]={v.x,v.y,v.z};double s=0;for(int i=0;i<3;++i){s+=(double)a[i]*a[i];vmin[i]=std::min(vmin[i],a[i]);vmax[i]=std::max(vmax[i],a[i]);}speed2+=s;if(s>1e-16)++nonzero;}
  if(!dynamic)for(int i=0;i<3;++i)vmin[i]=vmax[i]=0;
  std::fprintf(f,"{\"owner\":\"%s\",\"bodies\":%d,\"dynamic\":%d,\"moving_dynamic\":%d,\"velocity_squared_sum\":%.17g,\"velocity_min\":[%.9g,%.9g,%.9g],\"velocity_max\":[%.9g,%.9g,%.9g],\"id_digest\":\"%016llx\",\"examples\":[",cpu?"mapped_cpu":"public",count,dynamic,nonzero,speed2,vmin[0],vmin[1],vmin[2],vmax[0],vmax[1],vmax[2],(unsigned long long)ids);
  // Property aggregates cover every dynamic body, not only the64displayexamples.
  unsigned n=0;for(auto& b:mutation_bodies){if(b.ref.cpu!=cpu||!body_valid(b.ref)||n>=64)continue;if(n++)std::fputc(',',f);mutation_body_json(f,b);}std::fprintf(f,"],\"gravity_range\":[%.9g,%.9g],\"locks_histogram\":[",dynamic?gravity_min:0,dynamic?gravity_max:0);bool first_lock=true;for(int i=0;i<64;++i)if(lock_counts[i]){if(!first_lock)std::fputc(',',f);first_lock=false;std::fprintf(f,"[%d,%d]",i,lock_counts[i]);}std::fputs("]}",f);
}
void mutation_event(const char* api,const char* phase,WatchedBody body,b3Pos p={},b3Quat q=b3Quat_identity){
  if(!mutation_enabled())return;if(const char* path=std::getenv("GPU_PR10_API_TRACE"))if(FILE* f=std::fopen(path,"a")){std::fprintf(f,"{\"api\":\"%s\",\"phase\":\"%s\",\"input_position\":[%.9g,%.9g,%.9g],\"input_rotation\":[%.9g,%.9g,%.9g,%.9g],\"body\":",api,phase,(double)p.x,(double)p.y,(double)p.z,q.v.x,q.v.y,q.v.z,q.s);auto it=std::find_if(mutation_bodies.begin(),mutation_bodies.end(),[&](auto& b){return same_body(b.ref,body);});if(it!=mutation_bodies.end()&&body_valid(body))mutation_body_json(f,*it);else body_json(f,body);std::fputs("}\n",f);std::fclose(f);}
}
void mutation_shape(WatchedBody ref,b3ShapeId id,const b3ShapeDef* def,const char* kind,float radius,int vertices){if(!mutation_enabled())return;auto it=std::find_if(mutation_bodies.begin(),mutation_bodies.end(),[&](auto& b){return same_body(b.ref,ref);});if(it!=mutation_bodies.end())it->shapes.push_back({id,kind,def->baseMaterial.restitution,def->invokeContactCreation,radius,vertices});++mutation_shapes_created;}
}
#define MUT_CREATE(NAME,CPU) extern "C" b3BodyId __real_##NAME(b3WorldId,const b3BodyDef*) __attribute__((weak));extern "C" b3BodyId __wrap_##NAME(b3WorldId world,const b3BodyDef* def){assert(__real_##NAME);auto id=__real_##NAME(world,def);if(mutation_enabled()){++mutation_created;if(mutation_bodies.size()<4096)mutation_bodies.push_back({{id,CPU},*def,{}});}return id;}
#define MUT_DESTROY(NAME,CPU) extern "C" void __real_##NAME(b3BodyId) __attribute__((weak));extern "C" void __wrap_##NAME(b3BodyId id){assert(__real_##NAME);if(mutation_enabled())++mutation_destroyed;mutation_event(#NAME,"before",{id,CPU});__real_##NAME(id);mutation_event(#NAME,"after",{id,CPU});}
#define MUT_TRANSFORM(NAME,CPU) extern "C" void __real_##NAME(b3BodyId,b3Pos,b3Quat) __attribute__((weak));extern "C" void __wrap_##NAME(b3BodyId id,b3Pos p,b3Quat q){assert(__real_##NAME);mutation_event(#NAME,"before",{id,CPU},p,q);__real_##NAME(id,p,q);mutation_event(#NAME,"after",{id,CPU},p,q);}
#define MUT_AWAKE(NAME,CPU) extern "C" void __real_##NAME(b3BodyId,bool) __attribute__((weak));extern "C" void __wrap_##NAME(b3BodyId id,bool v){assert(__real_##NAME);mutation_event(#NAME,"before",{id,CPU});__real_##NAME(id,v);mutation_event(#NAME,"after",{id,CPU});}
#define MUT_SPHERE(NAME,CPU) extern "C" b3ShapeId __real_##NAME(b3BodyId,const b3ShapeDef*,const b3Sphere*) __attribute__((weak));extern "C" b3ShapeId __wrap_##NAME(b3BodyId id,const b3ShapeDef* def,const b3Sphere* sphere){assert(__real_##NAME);auto shape=__real_##NAME(id,def,sphere);mutation_shape({id,CPU},shape,def,"Sphere",sphere->radius,0);return shape;}
#define MUT_HULL(NAME,CPU) extern "C" b3ShapeId __real_##NAME(b3BodyId,const b3ShapeDef*,const b3HullData*) __attribute__((weak));extern "C" b3ShapeId __wrap_##NAME(b3BodyId id,const b3ShapeDef* def,const b3HullData* hull){assert(__real_##NAME);auto shape=__real_##NAME(id,def,hull);mutation_shape({id,CPU},shape,def,"Hull",0,hull->vertexCount);return shape;}
#define MUT_EXPLODE(NAME,CPU) extern "C" void __real_##NAME(b3WorldId,const b3ExplosionDef*) __attribute__((weak));extern "C" void __wrap_##NAME(b3WorldId id,const b3ExplosionDef* def){assert(__real_##NAME);FILE* f=nullptr;if(mutation_enabled())if(const char* p=std::getenv("GPU_PR10_API_TRACE"))f=std::fopen(p,"a");if(f){std::fprintf(f,"{\"api\":\"%s\",\"phase\":\"before\",\"input\":{\"position\":[%.9g,%.9g,%.9g],\"radius\":%.9g,\"falloff\":%.9g,\"impulsePerArea\":%.9g},\"world\":",#NAME,(double)def->position.x,(double)def->position.y,(double)def->position.z,def->radius,def->falloff,def->impulsePerArea);mutation_world_json(f,CPU);std::fputs("}\n",f);std::fclose(f);}__real_##NAME(id,def);if(mutation_enabled())if(const char* p=std::getenv("GPU_PR10_API_TRACE"))if(FILE* f=std::fopen(p,"a")){std::fprintf(f,"{\"api\":\"%s\",\"phase\":\"after\",\"world\":",#NAME);mutation_world_json(f,CPU);std::fputs("}\n",f);std::fclose(f);}}
MUT_CREATE(b3CreateBody,false) MUT_CREATE(cpu_b3CreateBody,true)
MUT_DESTROY(b3DestroyBody,false) MUT_DESTROY(cpu_b3DestroyBody,true)
MUT_TRANSFORM(b3Body_SetTransform,false) MUT_TRANSFORM(cpu_b3Body_SetTransform,true)
MUT_AWAKE(b3Body_SetAwake,false) MUT_AWAKE(cpu_b3Body_SetAwake,true)
MUT_SPHERE(b3CreateSphereShape,false) MUT_SPHERE(cpu_b3CreateSphereShape,true)
MUT_HULL(b3CreateHullShape,false) MUT_HULL(cpu_b3CreateHullShape,true)
MUT_EXPLODE(b3World_Explode,false) MUT_EXPLODE(cpu_b3World_Explode,true)
// Body IDs have a world slot, not a world generation. Drop observations at world destruction.
#define MUT_DESTROY_WORLD(NAME,CPU) extern "C" void __real_##NAME(b3WorldId) __attribute__((weak));extern "C" void __wrap_##NAME(b3WorldId id){assert(__real_##NAME);mutation_bodies.erase(std::remove_if(mutation_bodies.begin(),mutation_bodies.end(),[&](auto& b){return b.ref.cpu==CPU&&b.ref.id.world0==id.index1-1;}),mutation_bodies.end());__real_##NAME(id);}
MUT_DESTROY_WORLD(b3DestroyWorld,false) MUT_DESTROY_WORLD(cpu_b3DestroyWorld,true)
static void mutation_frame(){
 if(!mutation_enabled())return;
 if(const char* ui=std::getenv("GPU_PR10_UI_TRACE")){static unsigned last=0;unsigned request=0;std::string p=std::string(ui)+".request";FILE* rf=std::fopen(p.c_str(),"r");if(!rf)return;const int read=std::fscanf(rf,"%u",&request);std::fclose(rf);if(read!=1||request==last)return;last=request;
 if(const char* path=std::getenv("GPU_PR10_API_TRACE"))if(FILE* f=std::fopen(path,"a")){const b3Quat reference=b3MakeQuatFromAxisAngle(b3Vec3_axisZ,0.95f*B3_PI);std::fprintf(f,"{\"api\":\"actual_mutation_frame\",\"request\":%u,\"created\":%u,\"destroyed\":%u,\"shapes_created\":%u,\"teleport_reference\":[%.9g,%.9g,%.9g,%.9g],\"worlds\":[",request,mutation_created,mutation_destroyed,mutation_shapes_created,reference.v.x,reference.v.y,reference.v.z,reference.s);mutation_world_json(f,false);if(cpu_b3Body_IsValid){std::fputc(',',f);mutation_world_json(f,true);}std::fputs("]}\n",f);std::fclose(f);}
 mutation_bodies.erase(std::remove_if(mutation_bodies.begin(),mutation_bodies.end(),[](auto& b){return !body_valid(b.ref);}),mutation_bodies.end());
 }
}
