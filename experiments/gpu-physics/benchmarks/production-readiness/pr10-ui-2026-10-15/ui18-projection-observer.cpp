// Read the actual native camera and preview bounds; preserve the previous key/UI observer chain.
#include "host/camera.h"
#include "gfx/draw.h"
#include "sokol_preview_bounds.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>

static Camera* observed;
static int observedWidth, observedHeight;
static b3WorldId observedWorld;
static bool previewEnabled;
extern "C" void ui18_previous_camera_update(Camera*,float,int,int);
extern void wrap_update(Camera*,float,int,int) asm("__wrap__ZN6Camera6UpdateEfii");
void wrap_update(Camera* camera,float dt,int width,int height)
{
    ui18_previous_camera_update(camera,dt,width,height);
    observed=camera;observedWidth=width;observedHeight=height;
}
extern "C" void __real_sample_preview_begin_frame(b3WorldId,bool);
extern "C" void __wrap_sample_preview_begin_frame(b3WorldId world,bool enabled)
{
    __real_sample_preview_begin_frame(world,enabled);observedWorld=world;previewEnabled=enabled;
}
extern "C" void __real_sample_preview_end_frame(void);
extern "C" void __wrap_sample_preview_end_frame(void)
{
    __real_sample_preview_end_frame();
    if(!observed||!std::getenv("GPU_PR10_PREVIEW_TRACE"))return;
    const char* ui=std::getenv("GPU_PR10_UI_TRACE"),*path=std::getenv("GPU_PR10_API_TRACE");
    if(!ui||!path)return;
    static unsigned last=0;unsigned request=0;
    FILE* rf=std::fopen((std::string(ui)+".request").c_str(),"r");if(!rf)return;
    const int got=std::fscanf(rf,"%u",&request);std::fclose(rf);
    if(got!=1||request==last)return;last=request;
    b3AABB bounds;const bool valid=sample_preview_get_bounds(observedWorld,&bounds);
    FILE* f=std::fopen(path,"a");if(!f)return;
    const b3Pos origin=GetDrawOrigin();
    std::fprintf(f,"{\"api\":\"ui18_preview_projection\",\"request\":%u,\"world\":[%u,%u],\"enabled\":%s,\"valid\":%s,\"size\":[%d,%d],\"origin\":[%.17g,%.17g,%.17g],\"pivot\":[%.17g,%.17g,%.17g],\"radius\":%.9g,\"yaw\":%.9g,\"pitch\":%.9g,\"fov\":%.9g,\"near\":%.9g,\"far\":%.9g,\"z_up\":%s,\"bounds\":",request,observedWorld.index1,observedWorld.generation,previewEnabled?"true":"false",valid?"true":"false",observedWidth,observedHeight,origin.x,origin.y,origin.z,observed->m_pivot.x,observed->m_pivot.y,observed->m_pivot.z,observed->m_radius,observed->m_yaw,observed->m_pitch,observed->m_fov,observed->m_near,observed->m_far,observed->m_zUp?"true":"false");
    if(valid)std::fprintf(f,"[[%.9g,%.9g,%.9g],[%.9g,%.9g,%.9g]]",bounds.lowerBound.x,bounds.lowerBound.y,bounds.lowerBound.z,bounds.upperBound.x,bounds.upperBound.y,bounds.upperBound.z);
    else std::fputs("null",f);
    std::fputs(",\"requested_point\":",f);
    const char* pointPath=std::getenv("GPU_PR10_PROJECTION_POINT");
    FILE* pf=pointPath?std::fopen(pointPath,"r"):nullptr;
    double px=0,py=0,pz=0;const bool havePoint=pf&&std::fscanf(pf,"%lf %lf %lf",&px,&py,&pz)==3;
    if(pf)std::fclose(pf);
    if(havePoint)
    {
        const b3Vec3 point{static_cast<float>(px),static_cast<float>(py),static_cast<float>(pz)};
        const b3Vec3 relative=b3SubPos(b3ToPos(point),origin);
        const Vec4 clip=MulMV4(observed->Proj(),MulMV4(observed->View(),MakeVec4(relative.x,relative.y,relative.z,1)));
        const bool inside=clip.w>0&&std::fabs(clip.x)<clip.w&&std::fabs(clip.y)<clip.w;
        const float nx=clip.w!=0?clip.x/clip.w:0,ny=clip.w!=0?clip.y/clip.w:0;
        std::fprintf(f,"{\"position\":[%.17g,%.17g,%.17g],\"pixel\":[%.9g,%.9g],\"inside\":%s}",px,py,pz,(.5f+.5f*nx)*observedWidth,(.5f-.5f*ny)*observedHeight,inside?"true":"false");
    }
    else std::fputs("null",f);
    std::fputs(",\"corners\":[",f);bool allInside=valid;
    if(valid)for(int i=0;i<8;++i)
    {
        const b3Vec3 point={(i&1)?bounds.upperBound.x:bounds.lowerBound.x,(i&2)?bounds.upperBound.y:bounds.lowerBound.y,(i&4)?bounds.upperBound.z:bounds.lowerBound.z};
        const b3Vec3 relative=b3SubPos(b3ToPos(point),origin);
        const Vec4 clip=MulMV4(observed->Proj(),MulMV4(observed->View(),MakeVec4(relative.x,relative.y,relative.z,1)));
        const bool finite=std::isfinite(clip.x)&&std::isfinite(clip.y)&&std::isfinite(clip.z)&&std::isfinite(clip.w);
        const bool inside=finite&&clip.w>0&&std::fabs(clip.x)<=clip.w+1e-4f&&std::fabs(clip.y)<=clip.w+1e-4f&&clip.z>=-1e-4f&&clip.z<=clip.w+1e-4f;
        allInside&=inside;const bool divide=finite&&clip.w!=0;
        if(i)std::fputc(',',f);
        std::fprintf(f,"{\"point\":[%.9g,%.9g,%.9g],\"ndc\":[%.9g,%.9g,%.9g],\"positive_w\":%s,\"inside\":%s}",point.x,point.y,point.z,divide?clip.x/clip.w:0,divide?clip.y/clip.w:0,divide?clip.z/clip.w:0,clip.w>0?"true":"false",inside?"true":"false");
    }
    std::fprintf(f,"],\"fully_inside\":%s}\n",allInside?"true":"false");std::fclose(f);
}
