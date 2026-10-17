extern "C" bool cpu_b3World_IsValid(b3WorldId) __attribute__((weak));
// Read-only boundary observations. Each wrapped original runs exactly once.
namespace {
struct SceneTuning {b3WorldId world;bool cpu;float hertz,damping,speed;unsigned calls;};
std::vector<SceneTuning> scene_tunings;
struct SceneForce {const char* api;bool cpu;unsigned index,generation,world; b3Vec3 value;float drag,lift,maxSpeed;bool wake;unsigned calls;};
std::vector<SceneForce> scene_forces;
bool scene_trace(){return std::getenv("GPU_PR10_SCENE_CONTROLS");}
void scene_tuning(b3WorldId id,bool cpu,float hz,float dr,float speed){if(!scene_trace())return;for(auto& t:scene_tunings)if(t.cpu==cpu&&t.world.index1==id.index1&&t.world.generation==id.generation){t.hertz=hz;t.damping=dr;t.speed=speed;++t.calls;return;}if(scene_tunings.size()<32)scene_tunings.push_back({id,cpu,hz,dr,speed,1});}
void scene_force(const char* api,bool cpu,unsigned index,unsigned generation,unsigned world,b3Vec3 value,bool wake,float drag=0,float lift=0,float maxSpeed=0){if(!scene_trace())return;for(auto& q:scene_forces)if(q.cpu==cpu&&!std::strcmp(q.api,api)){q.index=index;q.generation=generation;q.world=world;q.value=value;q.wake=wake;q.drag=drag;q.lift=lift;q.maxSpeed=maxSpeed;++q.calls;return;}if(scene_forces.size()<8)scene_forces.push_back({api,cpu,index,generation,world,value,drag,lift,maxSpeed,wake,1});}
}
#define SCENE_TUNE(NAME,CPU) extern "C" void __real_##NAME(b3WorldId,float,float,float) __attribute__((weak));extern "C" void __wrap_##NAME(b3WorldId id,float hz,float dr,float speed){assert(__real_##NAME);__real_##NAME(id,hz,dr,speed);scene_tuning(id,CPU,hz,dr,speed);}
#define SCENE_TORQUE(NAME,CPU) extern "C" void __real_##NAME(b3BodyId,b3Vec3,bool) __attribute__((weak));extern "C" void __wrap_##NAME(b3BodyId id,b3Vec3 v,bool wake){assert(__real_##NAME);__real_##NAME(id,v,wake);scene_force(#NAME,CPU,id.index1,id.generation,id.world0,v,wake);}
#define SCENE_WIND(NAME,CPU) extern "C" void __real_##NAME(b3ShapeId,b3Vec3,float,float,float,bool) __attribute__((weak));extern "C" void __wrap_##NAME(b3ShapeId id,b3Vec3 v,float drag,float lift,float maxSpeed,bool wake){assert(__real_##NAME);__real_##NAME(id,v,drag,lift,maxSpeed,wake);scene_force(#NAME,CPU,id.index1,id.generation,id.world0,v,wake,drag,lift,maxSpeed);}
SCENE_TUNE(b3World_SetContactTuning,false) SCENE_TUNE(cpu_b3World_SetContactTuning,true)
SCENE_TORQUE(b3Body_ApplyTorque,false) SCENE_TORQUE(cpu_b3Body_ApplyTorque,true)
SCENE_WIND(b3Shape_ApplyWind,false) SCENE_WIND(cpu_b3Shape_ApplyWind,true)
static void scene_controls_frame(){
 if(!scene_trace())return;const char* ui=trace_path(),*path=std::getenv("GPU_PR10_API_TRACE");if(!ui||!path)return;static unsigned last=0;unsigned request=0;FILE* rf=std::fopen((std::string(ui)+".request").c_str(),"r");if(!rf)return;int got=std::fscanf(rf,"%u",&request);std::fclose(rf);if(got!=1||request==last)return;
 if(FILE* f=std::fopen(path,"a")){std::fprintf(f,"{\"api\":\"sample_scene_controls\",\"request\":%u,\"tunings\":[",request);bool comma=false;for(auto t:scene_tunings){bool valid=t.cpu?(cpu_b3World_IsValid&&cpu_b3World_IsValid(t.world)):b3World_IsValid(t.world);if(!valid)continue;if(comma)std::fputc(',',f);comma=true;std::fprintf(f,"{\"owner\":\"%s\",\"world\":[%u,%u],\"hertz\":%.9g,\"damping\":%.9g,\"speed\":%.9g,\"calls\":%u}",t.cpu?"mapped_cpu":"public",t.world.index1,t.world.generation,t.hertz,t.damping,t.speed,t.calls);}
 std::fputs("],\"forces\":[",f);comma=false;for(auto q:scene_forces){if(comma)std::fputc(',',f);comma=true;std::fprintf(f,"{\"api\":\"%s\",\"owner\":\"%s\",\"id\":[%u,%u,%u],\"value\":%s,\"wake\":%s,\"drag\":%.9g,\"lift\":%.9g,\"maxSpeed\":%.9g,\"calls\":%u}",q.api,q.cpu?"mapped_cpu":"public",q.index,q.generation,q.world,vec_json(q.value).c_str(),q.wake?"true":"false",q.drag,q.lift,q.maxSpeed,q.calls);}
 std::fputs("]}\n",f);std::fclose(f);last=request;}
}
