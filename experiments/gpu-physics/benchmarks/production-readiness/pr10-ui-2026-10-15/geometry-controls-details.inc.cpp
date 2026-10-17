extern "C" b3Capsule cpu_b3Shape_GetCapsule(b3ShapeId) __attribute__((weak));
extern "C" b3Sphere cpu_b3Shape_GetSphere(b3ShapeId) __attribute__((weak));
extern "C" const b3HullData* cpu_b3Shape_GetHull(b3ShapeId) __attribute__((weak));
extern "C" b3Mesh cpu_b3Shape_GetMesh(b3ShapeId) __attribute__((weak));
extern "C" b3AABB cpu_b3Shape_GetAABB(b3ShapeId) __attribute__((weak));
static void shape_vec(FILE* f,b3Vec3 a){std::fprintf(f,"[%.9g,%.9g,%.9g]",a.x,a.y,a.z);}
static void heightfield_details(FILE*,const b3HeightFieldData*);
extern "C" const b3HeightFieldData* cpu_b3Shape_GetHeightField(b3ShapeId) __attribute__((weak));
static void mesh_shape_details(FILE* f,bool cpu,b3ShapeId id,int type){
 auto a=cpu?cpu_b3Shape_GetAABB(id):b3Shape_GetAABB(id);std::fputs(",\"actual_aabb\":[",f);shape_vec(f,a.lowerBound);std::fputc(',',f);shape_vec(f,a.upperBound);std::fputc(']',f);
 if(type==b3_sphereShape){auto s=cpu?cpu_b3Shape_GetSphere(id):b3Shape_GetSphere(id);std::fputs(",\"sphere\":{\"center\":",f);shape_vec(f,s.center);std::fprintf(f,",\"radius\":%.9g}",s.radius);}
 if(type==b3_capsuleShape){auto c=cpu?cpu_b3Shape_GetCapsule(id):b3Shape_GetCapsule(id);std::fputs(",\"capsule\":{\"center1\":",f);shape_vec(f,c.center1);std::fputs(",\"center2\":",f);shape_vec(f,c.center2);std::fprintf(f,",\"radius\":%.9g}",c.radius);}
 if(type==b3_hullShape){const auto* h=cpu?cpu_b3Shape_GetHull(id):b3Shape_GetHull(id);if(h){b3Vec3 lo={INFINITY,INFINITY,INFINITY},hi={-INFINITY,-INFINITY,-INFINITY};auto* points=b3GetHullPoints(h);for(int i=0;i<h->vertexCount;++i){auto p=points[i];lo=b3Min(lo,p);hi=b3Max(hi,p);}std::fprintf(f,",\"hull\":{\"vertex_count\":%d,\"bounds\":[",h->vertexCount);shape_vec(f,lo);std::fputc(',',f);shape_vec(f,hi);std::fputs("]}",f);}else std::fputs(",\"hull\":null",f);}
 if(type==b3_meshShape){auto m=cpu?cpu_b3Shape_GetMesh(id):b3Shape_GetMesh(id);std::fputs(",\"mesh\":{\"scale\":",f);shape_vec(f,m.scale);std::fprintf(f,",\"data_valid\":%s,\"triangles\":%d,\"vertices\":%d}",m.data?"true":"false",m.data?m.data->triangleCount:0,m.data?m.data->vertexCount:0);}
 if(type==b3_heightShape){std::fputs(",\"heightfield\":",f);heightfield_details(f,cpu?cpu_b3Shape_GetHeightField(id):b3Shape_GetHeightField(id));}
}

#include "geometry-heightfield.inc.cpp"
