#include "height_field_compat.h"
#include <assert.h>
#include <float.h>
#include <stdio.h>
#include <string.h>
static void unchanged(b3HeightFieldData field)
{
 GpuHeightFieldEncoding actual=gpu_height_field_encoding(&field);
 assert(memcmp(&actual.minHeight,&field.minHeight,sizeof(float))==0);
 assert(memcmp(&actual.heightScale,&field.heightScale,sizeof(float))==0);
 assert(memcmp(&actual.scale,&field.scale,sizeof(b3Vec3))==0);
}
int main(void)
{
 b3HeightFieldData f={0};f.minHeight=-256;f.heightScale=512.0f/65535.0f;f.scale=(b3Vec3){2,0,2};
 for(int sign=0;sign<2;++sign){f.scale.y=sign?-0.0f:0.0f;GpuHeightFieldEncoding e=gpu_height_field_encoding(&f);assert(e.minHeight==0&&e.heightScale==0&&e.scale.y==1&&e.scale.x==2&&e.scale.z==2);for(int h=0;h<=UINT16_MAX;++h)assert(f.scale.y*(f.minHeight+f.heightScale*h)==e.scale.y*(e.minHeight+e.heightScale*h));}
 f.scale.y=1.5f;unchanged(f);f.scale.y=-1;unchanged(f);f.scale.y=NAN;unchanged(f);
 f.scale.y=0;f.scale.x=0;unchanged(f);f.scale.x=-1;unchanged(f);f.scale.x=INFINITY;unchanged(f);f.scale.x=2;
 f.scale.z=0;unchanged(f);f.scale.z=NAN;unchanged(f);f.scale.z=2;
 f.minHeight=NAN;unchanged(f);f.minHeight=-256;f.heightScale=-1;unchanged(f);f.heightScale=INFINITY;unchanged(f);f.heightScale=FLT_MAX;unchanged(f);
 puts("PASS: 131072 flat decoded vertices; positive/negative/nonfinite/invalid/overflow metadata preserve original encoding");
}
