#include "box3d/box3d.h"
#include <cstdio>
#include <cstdlib>
extern "C" bool __real_gpu_b3_replace_mesh(b3ShapeId,const float*,int,const int32_t*,int,const uint8_t*,const uint8_t*,const void*,int,float,float,float);
extern "C" bool __wrap_gpu_b3_replace_mesh(b3ShapeId id,const float*vertices,int vertexCount,const int32_t*triangles,int triangleCount,const uint8_t*flags,const uint8_t*materials,const void*nodes,int nodeCount,float x,float y,float z){const bool result=__real_gpu_b3_replace_mesh(id,vertices,vertexCount,triangles,triangleCount,flags,materials,nodes,nodeCount,x,y,z);if(const char*path=std::getenv("GPU_PR10_API_TRACE"))if(FILE*f=std::fopen(path,"a")){std::fprintf(f,"{\"api\":\"ui23_mesh_replacement\",\"id\":[%u,%u,%u],\"scale\":[%.9g,%.9g,%.9g],\"vertices\":%d,\"triangles\":%d,\"result\":%s}\n",id.index1,id.generation,id.world0,x,y,z,vertexCount,triangleCount,result?"true":"false");std::fclose(f);}return result;}
