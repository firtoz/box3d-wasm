// Diagnostic link wrappers: original operation/callback/context forwarded once.
namespace {
b3RecPlayer* replay_player=nullptr;
unsigned replay_creates=0,replay_destroys=0,replay_steps=0,replay_substeps=0,replay_restarts=0,replay_seeks=0,replay_workers=0,replay_policies=0;
int replay_last_seek=-1,replay_last_worker=0,replay_last_interval=0;
size_t replay_last_budget=0;
std::vector<std::string> replay_draw_calls;
bool replay_trace(){return std::getenv("GPU_PR10_REPLAY_CONTROLS");}
}
extern "C" b3RecPlayer* __real_b3CreatePlayer(const void*,int,int);
extern "C" b3RecPlayer* __wrap_b3CreatePlayer(const void* data,int size,int workers){auto* p=__real_b3CreatePlayer(data,size,workers);if(replay_trace()){replay_player=p;++replay_creates;replay_last_worker=workers;}return p;}
extern "C" void __real_b3DestroyPlayer(b3RecPlayer*);
extern "C" void __wrap_b3DestroyPlayer(b3RecPlayer* p){if(replay_trace()){++replay_destroys;if(replay_player==p)replay_player=nullptr;}__real_b3DestroyPlayer(p);}
extern "C" bool __real_b3RecPlayer_StepFrame(b3RecPlayer*);
extern "C" bool __wrap_b3RecPlayer_StepFrame(b3RecPlayer* p){bool result=__real_b3RecPlayer_StepFrame(p);if(replay_trace())++replay_steps;return result;}
#define REPLAY_VOID(NAME,COUNT) extern "C" void __real_##NAME(b3RecPlayer*);extern "C" void __wrap_##NAME(b3RecPlayer* p){__real_##NAME(p);if(replay_trace())++COUNT;}
REPLAY_VOID(b3RecPlayer_SubStepFrame,replay_substeps)
REPLAY_VOID(b3RecPlayer_Restart,replay_restarts)
extern "C" void __real_b3RecPlayer_SeekFrame(b3RecPlayer*,int);
extern "C" void __wrap_b3RecPlayer_SeekFrame(b3RecPlayer* p,int frame){__real_b3RecPlayer_SeekFrame(p,frame);if(replay_trace()){++replay_seeks;replay_last_seek=frame;}}
extern "C" void __real_b3RecPlayer_SetWorkerCount(b3RecPlayer*,int);
extern "C" void __wrap_b3RecPlayer_SetWorkerCount(b3RecPlayer* p,int count){__real_b3RecPlayer_SetWorkerCount(p,count);if(replay_trace()){++replay_workers;replay_last_worker=count;}}
extern "C" void __real_b3RecPlayer_SetKeyframePolicy(b3RecPlayer*,size_t,int);
extern "C" void __wrap_b3RecPlayer_SetKeyframePolicy(b3RecPlayer* p,size_t bytes,int interval){__real_b3RecPlayer_SetKeyframePolicy(p,bytes,interval);if(replay_trace()){++replay_policies;replay_last_budget=bytes;replay_last_interval=interval;}}
extern "C" void __real_b3RecPlayer_DrawFrameQueries(b3RecPlayer*,b3DebugDraw*,int,int);
extern "C" void __wrap_b3RecPlayer_DrawFrameQueries(b3RecPlayer* p,b3DebugDraw* draw,int index,int selected){if(!replay_trace()||!draw){__real_b3RecPlayer_DrawFrameQueries(p,draw,index,selected);return;}b3DebugDraw local=*draw;DrawOutputs output;output.original=*draw;observe_callbacks(local);auto* previous=active_output;active_output=&output;__real_b3RecPlayer_DrawFrameQueries(p,&local,index,selected);active_output=previous;std::string s="{\"index\":"+std::to_string(index)+",\"selected\":"+std::to_string(selected)+",\"callbacks\":[";for(int i=0;i<9;++i){if(i)s+=",";s+=std::to_string(output.count[i]);}s+="],\"digests\":[";for(int i=0;i<9;++i){if(i)s+=",";s+="\""+std::to_string(output.digest[i])+"\"";}s+="]}";if(replay_draw_calls.size()<16)replay_draw_calls.push_back(s);}
static void replay_controls_frame(){
 if(!replay_trace())return;
 const char* ui=trace_path(),*path=std::getenv("GPU_PR10_API_TRACE");static unsigned last=0;unsigned request=0;bool publish=false;if(ui&&path){FILE* rf=std::fopen((std::string(ui)+".request").c_str(),"r");if(rf){int got=std::fscanf(rf,"%u",&request);std::fclose(rf);publish=got==1&&request!=last;}}
 if(publish)if(FILE* f=std::fopen(path,"a")){
  std::fprintf(f,"{\"api\":\"sample_replay_controls\",\"request\":%u,\"calls\":{\"create\":%u,\"destroy\":%u,\"step\":%u,\"substep\":%u,\"restart\":%u,\"seek\":%u,\"workers\":%u,\"policy\":%u},\"last_seek\":%d,\"last_worker\":%d,\"last_policy\":[%llu,%d],\"draw_calls\":[",request,replay_creates,replay_destroys,replay_steps,replay_substeps,replay_restarts,replay_seeks,replay_workers,replay_policies,replay_last_seek,replay_last_worker,(unsigned long long)replay_last_budget,replay_last_interval);
  for(size_t i=0;i<replay_draw_calls.size();++i){if(i)std::fputc(',',f);std::fputs(replay_draw_calls[i].c_str(),f);}std::fputs("],\"camera\":",f);
  if(observed_camera){auto* c=observed_camera;float transform[16];static_assert(sizeof(transform)==sizeof(c->m_renderXform));std::memcpy(transform,&c->m_renderXform,sizeof(transform));std::fprintf(f,"{\"z_up\":%s,\"size\":[%d,%d],\"pivot\":",c->m_zUp?"true":"false",c->m_width,c->m_height);host_pos(f,c->m_pivot);std::fputs(",\"render_transform\":[",f);for(int i=0;i<16;++i){if(i)std::fputc(',',f);std::fprintf(f,"%.9g",transform[i]);}std::fputs("]}",f);}else std::fputs("null",f);
  std::fputs(",\"player\":",f);if(replay_player){auto* p=replay_player;auto w=b3RecPlayer_GetWorldId(p);auto info=b3RecPlayer_GetInfo(p);auto count=b3World_GetCounters(w);std::fprintf(f,"{\"frame\":%d,\"frame_count\":%d,\"pre_step\":%s,\"at_end\":%s,\"diverged\":%s,\"diverge_frame\":%d,\"world\":[%u,%u],\"world_counts\":[%d,%d,%d,%d],\"workers\":%d,\"bounds\":[",b3RecPlayer_GetFrame(p),b3RecPlayer_GetFrameCount(p),b3RecPlayer_IsAtPreStep(p)?"true":"false",b3RecPlayer_IsAtEnd(p)?"true":"false",b3RecPlayer_HasDiverged(p)?"true":"false",b3RecPlayer_GetDivergeFrame(p),w.index1,w.generation,count.bodyCount,count.shapeCount,count.jointCount,count.contactCount,info.workerCount);shape_vec(f,info.bounds.lowerBound);std::fputc(',',f);shape_vec(f,info.bounds.upperBound);std::fprintf(f,"],\"keyframe_policy\":[%llu,%d,%d,%llu],\"bodies\":[",(unsigned long long)b3RecPlayer_GetKeyframeBudget(p),b3RecPlayer_GetKeyframeMinInterval(p),b3RecPlayer_GetKeyframeInterval(p),(unsigned long long)b3RecPlayer_GetKeyframeBytes(p));
   for(int i=0;i<b3RecPlayer_GetBodyCount(p)&&i<64;++i){if(i)std::fputc(',',f);auto b=b3RecPlayer_GetBodyId(p,i);std::fprintf(f,"{\"ordinal\":%d,\"body\":",i);body_json(f,{b,false});if(B3_IS_NON_NULL(b)&&b3Body_IsValid(b)){std::fputs(",\"world_center\":",f);host_pos(f,b3Body_GetWorldCenter(b));std::fprintf(f,",\"shape_count\":%d,\"joint_count\":%d,\"contact_capacity\":%d",b3Body_GetShapeCount(b),b3Body_GetJointCount(b),b3Body_GetContactCapacity(b));}std::fputc('}',f);}
   std::fputs("],\"queries\":[",f);for(int i=0;i<b3RecPlayer_GetFrameQueryCount(p)&&i<128;++i){if(i)std::fputc(',',f);auto q=b3RecPlayer_GetFrameQuery(p,i);std::fprintf(f,"{\"index\":%d,\"kind\":%d,\"id\":%llu,\"key\":\"%llu\",\"name\":",i,q.type,(unsigned long long)q.id,(unsigned long long)q.key);quote(f,q.name?q.name:"");std::fprintf(f,",\"hits\":%d,\"origin\":",q.hitCount);host_pos(f,q.origin);std::fputs(",\"translation\":",f);shape_vec(f,q.translation);std::fputs(",\"bounds\":[",f);shape_vec(f,q.aabb.lowerBound);std::fputc(',',f);shape_vec(f,q.aabb.upperBound);std::fputs("]}",f);}std::fputs("]}",f);
  }else std::fputs("null",f);std::fputs("}\n",f);std::fclose(f);last=request;
 }
 replay_draw_calls.clear();
}
