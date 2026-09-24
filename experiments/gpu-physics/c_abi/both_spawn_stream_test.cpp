#include "box3d/box3d.h"
extern "C" {
#include "both_ids.h"
b3Vec3 cpu_b3Body_GetLinearVelocity(b3BodyId);
b3WorldTransform cpu_b3Body_GetTransform(b3BodyId);
bool cpu_b3Body_IsValid(b3BodyId);
void SetSelectedBody(b3BodyId) {}
void SetComparisonSelectedBody(b3BodyId) {}
}
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <vector>
static int checks;
static void near(float a,float b) {
    if (!(std::isfinite(a)&&std::isfinite(b)&&fabsf(a-b)<=1e-5f*fmaxf(1.f,fmaxf(fabsf(a),fabsf(b))))) {
        fprintf(stderr,"spawn mismatch GPU=%.9g CPU=%.9g\n",a,b); abort();
    }
    ++checks;
}
static void inspect(b3BodyId body) {
    auto cpu=both_cpu_body(body);
    assert(b3Body_IsValid(body)&&cpu_b3Body_IsValid(cpu));
    auto p=b3Body_GetTransform(body).p, q=cpu_b3Body_GetTransform(cpu).p;
    near(p.x,q.x);near(p.y,q.y);near(p.z,q.z);
    auto v=b3Body_GetLinearVelocity(body),w=cpu_b3Body_GetLinearVelocity(cpu);
    near(v.x,w.x);near(v.y,w.y);near(v.z,w.z);
}
int main(int argc, char** argv) {
    const int peak=argc>1?atoi(argv[1]):1024;
    if(peak<32 || peak>8192 || peak%32) return 2;
    const int batches=4*peak/32;
    for(int reserve: {0,2*peak}) {
        auto def=b3DefaultWorldDef();
        def.capacity.staticBodyCount=def.capacity.staticShapeCount=0;
        def.capacity.dynamicBodyCount=def.capacity.dynamicShapeCount=reserve;
        auto world=b3CreateWorld(&def);
        std::deque<b3BodyId> bodies;
        std::vector<double> times;
        // Start small, cross several allocation boundaries, then recycle expired
        // projectiles for four lifetimes of the requested peak (default 1024).
        for(int batch=0;batch<batches;++batch) {
            auto start=b3GetTicks();
            for(int i=0;i<32;++i) {
                if(bodies.size()==size_t(peak)) {
                    auto old=bodies.front(), cpu=both_cpu_body(old);bodies.pop_front();
                    b3DestroyBody(old);
                    assert(!b3Body_IsValid(old)&&!cpu_b3Body_IsValid(cpu));
                }
                auto bd=b3DefaultBodyDef();bd.type=b3_dynamicBody;
                bd.position={float(i)*3.f,5.f,-float(batch)*3.f};
                bd.linearVelocity={0.f,0.f,-20.f};bd.gravityScale=0.f;bd.isBullet=true;
                auto body=b3CreateBody(world,&bd);
                auto sd=b3DefaultShapeDef();sd.density*=4.f;
                b3Sphere sphere={b3Vec3_zero,0.25f};b3CreateSphereShape(body,&sd,&sphere);
                bodies.push_back(body);
            }
            b3World_Step(world,1.f/60.f,4);
            for(auto body:bodies) inspect(body);
            times.push_back(b3GetMilliseconds(start));
        }
        // Pending poses and the next step must remain correct after emptying.
        for(auto body:bodies) b3DestroyBody(body);
        b3World_Step(world,1.f/60.f,4);
        b3DestroyWorld(world);
        std::sort(times.begin()+1,times.end());
        printf("spawn-stream reserve=%d launches=%d peak=%d first_ms=%.3f p50_ms=%.3f p95_ms=%.3f max_ms=%.3f\n",
            reserve,4*peak,peak,times[0],times[times.size()/2],times[times.size()*95/100],times.back());
    }
    printf("spawn-stream: %d independent scalar comparisons passed\n",checks);
}
