// Actual GPU sample draw loop, frozen inputs from real CPU boundary traces.
// Pose/AABB endpoints are isolated; full captures exercise actual GPU providers.
#include API_SOURCE
#include <assert.h>

typedef struct Record {
    uint32_t shape, shape_generation, body, body_generation, visible;
    float p[3], q[4], lower[3], upper[3];
} Record;
_Static_assert(sizeof(Record) == 72, "frozen record layout");
typedef struct Token { b3ShapeId id; } Token;
static Record** lookup;
static uint32_t maximum, created, destroyed, submitted, aabb_calls;
static uint32_t* seen;
static Record* resolve(uint32_t id) { assert(id <= maximum && lookup[id]); return lookup[id]; }
GpuPoseExport gpu_b3_world_pose_export(b3WorldId world) { (void)world; return (GpuPoseExport){0}; }
bool gpu_b3_world_pose_export_uuid(b3WorldId world, uint8_t* uuid) { (void)world; (void)uuid; return false; }
void gpu_gl_import_pose_fd(int fd, uint64_t size, const uint8_t* uuid) { (void)fd; (void)size; (void)uuid; assert(false); }
bool gpu_gl_poses_imported(void) { return true; }
void gpu_b3_world_prepare_pose_snapshot(b3WorldId world) { assert(world.index1 == 1); }
bool gpu_b3_body_is_static(b3BodyId body) { (void)body; return false; }
void gpu_b3_body_get_transform(b3BodyId body, float* p, float* q)
{
    assert(body.world0 == 1); Record* r=resolve(body.index1);
    assert(body.generation == r->body_generation);
    memcpy(p,r->p,sizeof(r->p)); memcpy(q,r->q,sizeof(r->q));
}
b3AABB gpu_b3_shape_get_aabb(b3ShapeId id)
{
    Record* r=resolve(id.index1); assert(id.world0==1 && id.generation==r->shape_generation); ++aabb_calls;
    return (b3AABB){{r->lower[0],r->lower[1],r->lower[2]}, {r->upper[0],r->upper[1],r->upper[2]}};
}
static void* capture(const b3DebugShape* shape, void* context)
{
    (void)context; Record* r=resolve(shape->shapeId.index1);
    assert(shape->type==b3_hullShape && shape->hull && shape->shapeId.generation==r->shape_generation);
    Token* t=malloc(sizeof(Token)); assert(t); t->id=shape->shapeId; ++created; return t;
}
static void release(void* user, void* context) { (void)context; assert(user); ++destroyed; free(user); }
static void submit(void* user, b3WorldTransform xf, b3HexColor color, void* context)
{
    (void)color;(void)context;Token* t=user;Record* r=resolve(t->id.index1);
    assert(t->id.generation==r->shape_generation && xf.p.x==r->p[0] && xf.p.y==r->p[1] && xf.p.z==r->p[2]);
    assert(xf.q.v.x==r->q[0] && xf.q.v.y==r->q[1] && xf.q.v.z==r->q[2] && xf.q.s==r->q[3]);
    ++seen[t->id.index1];++submitted;
}
static void run(uint32_t scene,uint32_t frame,Record* rows,uint32_t count,uint32_t expected,const float bounds[6])
{
    maximum=0;for(uint32_t i=0;i<count;++i)if(rows[i].shape>maximum)maximum=rows[i].shape;
    lookup=calloc(maximum+1,sizeof(*lookup));seen=calloc(maximum+1,sizeof(*seen));assert(lookup&&seen);
    created=destroyed=submitted=aabb_calls=0;
    b3WorldId world={1,1};b3WorldDef def={0};def.createDebugShape=capture;def.destroyDebugShape=release;
    gpu_samples_on_world_created(world,&def);
    for(uint32_t i=0;i<count;++i) {
        Record* r=rows+i;assert(r->shape==r->body && !lookup[r->shape]);lookup[r->shape]=r;
        b3ShapeId id={(int32_t)r->shape,1,(uint16_t)r->shape_generation};
        b3BodyId body={(int32_t)r->body,1,(uint16_t)r->body_generation};
        bool ground=scene<1000 && r->body<=(scene==75?2:1);
        float half=ground?(scene==14?77:scene==75?309:100):(scene==139?1:.5f);
        b3BoxHull box=b3MakeBoxHull(half,ground?1:half,half);
        gpu_samples_on_shape_created(id,body,b3_hullShape,NULL,NULL,&box.base);
        gpu_samples_shape_set_name(id,"frozen-input");
    }
    b3DebugDraw draw={0};draw.drawShapes=true;draw.DrawShapeFcn=submit;
    draw.drawingBounds=(b3AABB){{bounds[0],bounds[1],bounds[2]},{bounds[3],bounds[4],bounds[5]}};
    for(int repeat=0;repeat<2;++repeat) {
        memset(seen,0,(maximum+1)*sizeof(*seen));submitted=0;
        gpu_samples_world_draw(world,&draw,UINT64_MAX);
#ifdef EXPECT_FIXED
        assert(submitted==expected);
#else
        assert(submitted==count);
#endif
        for(uint32_t i=0;i<count;++i) {
            Record* r=rows+i; b3ShapeId id={(int32_t)r->shape,1,(uint16_t)r->shape_generation};ShapeVis* s=shape_vis(id,false);
            assert(s && s->live && s->body.index1==(int32_t)r->body && s->body.generation==r->body_generation);
            assert(!strcmp(s->name,"frozen-input"));
#ifdef EXPECT_FIXED
            assert(seen[r->shape]==r->visible);
#else
            assert(seen[r->shape]==1);
#endif
        }
        assert(destroyed==0);
    }
    printf("scene=%u stage=%u input_shapes=%u CPU_expected=%u submitted=%u AABB_calls=%u\n",scene,frame,count,expected,submitted,aabb_calls);
#ifdef EXPECT_FIXED
    assert(aabb_calls==2*count && created==expected);
#else
    assert(aabb_calls==0 && created==count);
#endif
    // Reenter under wide bounds: no culled body/shape was deleted or renumbered.
    draw.drawingBounds=(b3AABB){{-1000000,-1000000,-1000000},{1000000,1000000,1000000}};
    submitted=0;gpu_samples_world_draw(world,&draw,UINT64_MAX);assert(submitted==count && created==count);
    gpu_samples_on_world_destroyed(world);assert(destroyed==created && !g_shapes[1].chunks);
    free(seen);free(lookup);lookup=NULL;
}
int main(int argc,char** argv)
{
    assert(argc==2);FILE* f=fopen(argv[1],"rb");assert(f);uint32_t cases;
    assert(fread(&cases,sizeof(cases),1,f)==1 && cases==9);
    for(uint32_t c=0;c<cases;++c) {
        uint32_t header[4];float bounds[6];assert(fread(header,sizeof(header),1,f)==1 && fread(bounds,sizeof(bounds),1,f)==1);
        Record* rows=malloc(header[2]*sizeof(*rows));assert(rows&&fread(rows,sizeof(*rows),header[2],f)==header[2]);
        run(header[0],header[1],rows,header[2],header[3],bounds);free(rows);
    }
    assert(fgetc(f)==EOF);fclose(f);
    const float bounds[6]={-1,-1,-1,1,1,1};
    for(int axis=0;axis<3;++axis)for(int side=-1;side<=1;side+=2)for(int touching=0;touching<2;++touching) {
        Record r={.shape=1,.shape_generation=7,.body=1,.body_generation=11,.visible=(uint32_t)touching,.q={0,0,0,1},.lower={-.5f,-.5f,-.5f},.upper={.5f,.5f,.5f}};
        float offset=side*(touching?1.5f:2.0f);r.p[axis]=offset;r.lower[axis]+=offset;r.upper[axis]+=offset;
        run(1000+axis,side<0?1:2,&r,1,r.visible,bounds);
    }
    puts("21 actual draw-loop cases passed; repeated cache/identity, inclusive boundaries and wide reentry preserved");
}
