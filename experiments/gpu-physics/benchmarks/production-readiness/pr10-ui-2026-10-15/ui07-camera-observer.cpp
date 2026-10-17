// Read-only actual native Camera::Frame input/output boundary.
#include "host/camera.h"
#include "sokol_app.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
extern void real_frame(Camera*,b3AABB,float,float) asm("__real__ZN6Camera5FrameE6b3AABBff");
extern void wrap_frame(Camera*,b3AABB,float,float) asm("__wrap__ZN6Camera5FrameE6b3AABBff");
static b3AABB observed_display(const Mat4& s,b3AABB a){
 const Vec4 lo=MulMV4(s,MakeVec4(a.lowerBound.x,a.lowerBound.y,a.lowerBound.z,1)),hi=MulMV4(s,MakeVec4(a.upperBound.x,a.upperBound.y,a.upperBound.z,1));
 return {{std::fmin(lo.x,hi.x),std::fmin(lo.y,hi.y),std::fmin(lo.z,hi.z)},{std::fmax(lo.x,hi.x),std::fmax(lo.y,hi.y),std::fmax(lo.z,hi.z)}};
}
void wrap_frame(Camera* c,b3AABB box,float aspect,float padding){
 const int width=c->m_width,height=c->m_height;const float before=c->m_radius,fov=c->m_fov;
 const b3AABB display=observed_display(c->m_renderXform,box);const b3Vec3 e=b3AABB_Extents(display);
 const float r=std::sqrt(e.x*e.x+e.y*e.y+e.z*e.z),v=r/std::tan(.5f*fov),h=v/(aspect>0?aspect:1);
 real_frame(c,box,aspect,padding);
 if(!std::getenv("GPU_PR10_CAMERA_TRACE"))return;const char* path=std::getenv("GPU_PR10_API_TRACE");if(!path)return;
 if(FILE* f=std::fopen(path,"a")){std::fprintf(f,"{\"api\":\"ui07_camera_frame\",\"width\":%d,\"height\":%d,\"window\":[%d,%d],\"aspect\":%.9g,\"padding\":%.9g,\"fov\":%.9g,\"aabb\":[[%.9g,%.9g,%.9g],[%.9g,%.9g,%.9g]],\"display_half\":[%.9g,%.9g,%.9g],\"sphere_radius\":%.9g,\"dist_v\":%.9g,\"dist_h\":%.9g,\"expected_unclamped_radius\":%.9g,\"radius_before\":%.9g,\"radius_after\":%.9g}\n",width,height,sapp_width(),sapp_height(),aspect,padding,fov,box.lowerBound.x,box.lowerBound.y,box.lowerBound.z,box.upperBound.x,box.upperBound.y,box.upperBound.z,e.x,e.y,e.z,r,v,h,(v>h?v:h)*padding,before,c->m_radius);std::fclose(f);}
}
