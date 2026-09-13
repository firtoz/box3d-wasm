#include "box3d/box3d.h"
#include "box3d/collision.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
typedef b3BodyId GpuBodyId;
typedef b3ShapeId GpuShapeId;
static int creates, clones, destroys, fault;
static void* cache_alloc(size_t n, size_t size) { return fault == 1 ? NULL : calloc(n, size); }
#define GPU_MESH_CACHE_CALLOC cache_alloc
static GpuShapeId gpu_b3_create_mesh(GpuBodyId b, const float* v, int nv,
 const int* t, int nt, const uint8_t* f, const uint8_t* m, const void* nodes,
 int nn, float sx,float sy,float sz,float d,float friction,float r,float rolling,bool mass)
{
 (void)b;(void)v;(void)t;(void)f;(void)m;(void)nodes;(void)nn;(void)d;(void)friction;(void)r;(void)rolling;
 assert(nv == 3 && nt == 1 && sx == 1 && sy == 1 && sz == 1 && !mass);
 ++creates;
 return (GpuShapeId){fault == 2 ? 0 : creates, 0, 1};
}
static void gpu_b3_destroy_shape(GpuShapeId id, bool mass) { assert(id.index1 > 0 && !mass); ++destroys; }
#include "compound_mesh_instances.h"
GpuShapeId gpu_b3_create_mesh_instance(GpuBodyId b, GpuShapeId src,
 const float* p,const float* q,const float* scale,float d,float f,float r,float roll)
{
 (void)b;(void)p;(void)q;(void)d;(void)f;(void)r;(void)roll;
 assert(src.index1 > 0 && scale[0] == -2);
 ++clones;
 return (GpuShapeId){fault == 3 ? 0 : 100 + clones, 0, 1};
}
int main(void)
{
 struct { b3MeshData header; b3Vec3 vertices[3]; b3MeshTriangle triangles[1]; } raw = {0};
 raw.header.vertexCount=3; raw.header.triangleCount=1;
 raw.header.vertexOffset=(int)((char*)raw.vertices-(char*)&raw);
 raw.header.triangleOffset=(int)((char*)raw.triangles-(char*)&raw);
 b3ChildShape child={0}; child.mesh.data=&raw.header; child.mesh.scale=(b3Vec3){-2,1,1};
 b3ShapeDef def=b3DefaultShapeDef();
 for(fault=0;fault<=3;++fault) {
  creates=clones=destroys=0;
  GpuMeshCache cache;
  bool initialized=gpu_mesh_cache_init(&cache,8);
  assert(initialized == (fault != 1));
  if(initialized) {
   for(int i=0;i<8;++i) {
    GpuShapeId id=gpu_mesh_cache_instance(&cache,(GpuBodyId){1,0,1},&child,&def);
    assert((id.index1>0) == (fault==0));
   }
   assert(creates==1);
   assert(clones==(fault==2?0:8));
  }
  gpu_mesh_cache_free(&cache);
  assert(destroys==((fault==0||fault==3)?1:0));
  gpu_mesh_cache_free(&cache);
 }
 GpuMeshCache cache;
 assert(!gpu_mesh_cache_init(&cache,-1)); gpu_mesh_cache_free(&cache);
 assert(!gpu_mesh_cache_init(&cache,65537)); gpu_mesh_cache_free(&cache);
 assert(gpu_mesh_cache_init(&cache,0)); gpu_mesh_cache_free(&cache);
 puts("shared source, allocation/source/clone failure and idempotent cleanup pass");
}
