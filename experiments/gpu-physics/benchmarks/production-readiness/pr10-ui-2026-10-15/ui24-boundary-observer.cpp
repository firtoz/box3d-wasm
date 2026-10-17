#include "box3d/box3d.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstdint>
extern "C" b3ShapeId __real_gpu_b3_create_height_field(b3BodyId,const uint16_t*,int,int,float,float,float,float,float,const uint8_t*,const uint8_t*,bool,float,float,float,float);
extern "C" b3ShapeId __wrap_gpu_b3_create_height_field(b3BodyId body,const uint16_t* heights,int columns,int rows,float minHeight,float heightScale,float x,float y,float z,const uint8_t* materials,const uint8_t* flags,bool clockwise,float density,float friction,float restitution,float rolling)
{
 const b3ShapeId id=__real_gpu_b3_create_height_field(body,heights,columns,rows,minHeight,heightScale,x,y,z,materials,flags,clockwise,density,friction,restitution,rolling);
 if(const char* path=std::getenv("GPU_PR10_API_TRACE"))if(FILE* f=std::fopen(path,"a")){
  float lower=INFINITY,upper=-INFINITY;bool finite=true;int holes=0;
  uint64_t digest=1469598103934665603ull;
  for(int i=0;i<columns*rows;++i){const float h=y*(minHeight+heightScale*float(heights[i]));finite=finite&&std::isfinite(h);lower=std::fmin(lower,h);upper=std::fmax(upper,h);digest=(digest^heights[i])*1099511628211ull;}
  for(int i=0;i<(columns-1)*(rows-1);++i){holes+=materials[i]==255;digest=(digest^materials[i])*1099511628211ull;}
  for(int i=0;i<2*(columns-1)*(rows-1);++i)digest=(digest^flags[i])*1099511628211ull;
  std::fprintf(f,"{\"api\":\"ui24_gpu_heightfield_create\",\"body\":[%u,%u,%u],\"id\":[%u,%u,%u],\"columns\":%d,\"rows\":%d,\"min_height\":%.9g,\"height_scale\":%.9g,\"scale\":[%.9g,%.9g,%.9g],\"decoded_y_bounds\":[%.9g,%.9g],\"decoded_finite\":%s,\"holes\":%d,\"clockwise\":%s,\"payload_digest\":\"%016llx\",\"density\":%.9g,\"friction\":%.9g,\"restitution\":%.9g,\"rolling\":%.9g,\"accepted\":%s}\n",body.index1,body.generation,body.world0,id.index1,id.generation,id.world0,columns,rows,minHeight,heightScale,x,y,z,lower,upper,finite?"true":"false",holes,clockwise?"true":"false",(unsigned long long)digest,density,friction,restitution,rolling,id.index1?"true":"false");std::fclose(f);
 }
 return id;
}
