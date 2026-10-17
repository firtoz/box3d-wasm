// Read-only native Human lifetime boundary. The original helper executes once.
#include "human.h"
#include "box3d/box3d.h"
#include <cstdio>
#include <cstdlib>
extern "C" void __real_DestroyHuman(Human*);
static void body(FILE*f,b3BodyId id){std::fprintf(f,"{\"id\":[%u,%u,%u],\"valid\":%s}",id.index1,id.generation,id.world0,b3Body_IsValid(id)?"true":"false");}
static void joint(FILE*f,b3JointId id){std::fprintf(f,"{\"id\":[%u,%u,%u],\"valid\":%s}",id.index1,id.generation,id.world0,b3Joint_IsValid(id)?"true":"false");}
static void record(const char* phase,const Human& old,const Human& current){const char*path=std::getenv("GPU_PR10_API_TRACE");if(!path)return;FILE*f=std::fopen(path,"a");if(!f)return;std::fprintf(f,"{\"api\":\"ui21_human_lifetime\",\"phase\":\"%s\",\"spawned\":%s,\"bones\":[",phase,current.isSpawned?"true":"false");for(int i=0;i<bone_count;i++){if(i)std::fputc(',',f);std::fputs("{\"body\":",f);body(f,old.bones[i].bodyId);std::fputs(",\"anchor\":",f);body(f,old.bones[i].anchorId);std::fputs(",\"anchor_joint\":",f);joint(f,old.bones[i].anchorJointId);std::fprintf(f,",\"cleared\":%s}",(B3_IS_NULL(current.bones[i].bodyId)&&B3_IS_NULL(current.bones[i].anchorId)&&B3_IS_NULL(current.bones[i].anchorJointId))?"true":"false");}std::fputs("]}\n",f);std::fclose(f);}
extern "C" void __wrap_DestroyHuman(Human* human){const Human before=*human;record("before",before,*human);__real_DestroyHuman(human);record("after",before,*human);}
