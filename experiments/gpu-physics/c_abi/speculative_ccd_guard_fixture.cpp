// Reuse the frozen public fixture geometry and linkage checks. This additional
// requirement verifies both shape switches and the CCD guard's limited scope.
#define main original_speculative_main
#include "speculative_fixture.cpp"
#undef main

int main() {
    // The CCD guard does not restore the wider experimental speculative range,
    // does not activate for a resting gap, and requires continuous collision.
    for(bool continuous: {false,true}) for(bool moving: {false,true}) for(float height: {.505f,.51f}) {
        #ifdef CPU_REFERENCE
        auto f=create(0,false,true,true,height);
#else
        auto f=create(0,true,true,false,height);
#endif
        b3World_EnableContinuous(f.world,continuous);
        if(moving) b3Body_SetLinearVelocity(f.body,{60,0,0});
        step(f);step(f);
        const auto p=b3Body_GetPosition(f.body);
        const int expected=0;
        const int n=contacts(f);
        std::printf("guard continuous=%d moving=%d height=%.9g contacts=%d y=%.9g\n",
                    int(continuous),int(moving),double(height),n,double(p.y));std::fflush(stdout);
        assert(n==expected && std::fabs(p.y-height)<1e-5f);
        destroy(f);
    }
    // The native experimental shape-off CCD failure is retained separately.
#ifndef CPU_REFERENCE
    for(int disabled: {0,1,2}) {
        auto f=create(0,disabled!=1,disabled!=2,disabled!=0,2);
        b3World_SetGravity(f.world,{0,-10,0});
        b3Body_SetLinearVelocity(f.body,{0,-150,0});
        for(int frame=0;frame<30;++frame) {
            step(f);auto p=b3Body_GetPosition(f.body);auto v=b3Body_GetLinearVelocity(f.body);
            std::printf("guard-ccd disabled=%d frame=%d y=%.9g vy=%.9g\n",disabled,frame,double(p.y),double(v.y));std::fflush(stdout);
            assert(std::isfinite(p.y)&&std::isfinite(v.y)&&p.y>=.495f);
        }
        destroy(f);
    }

#endif
    puts("speculative-ccd-guard=pass");
}
