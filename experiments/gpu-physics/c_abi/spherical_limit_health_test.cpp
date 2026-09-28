#include "spherical_limit_health.h"
#include <cstdlib>
#include <cstdio>
#include <cmath>
int main()
{
    auto rotation=[](float x,float y,float z,float angle) {
        const float s=std::sin(0.5f*angle);
        return b3Quat{{x*s,y*s,z*s},std::cos(0.5f*angle)};
    };
    auto near=[](float a,float b) { if (!(std::fabs(a-b)<1e-4f)) std::abort(); };
    auto inside=measureSphericalLimits(rotation(0,0,1,0.2f),true,0.3f,true,-0.4f,0.5f);
    near(inside.swing,0); near(inside.twist,0.2f);
    near(inside.coneExcess,0);near(inside.lowerTwistExcess,0);near(inside.upperTwistExcess,0);
    auto cone=measureSphericalLimits(rotation(1,0,0,0.7f),true,0.3f,true,-0.4f,0.5f);
    near(cone.coneExcess,0.4f);near(cone.twist,0);
    auto low=measureSphericalLimits(rotation(0,0,1,-0.7f),true,0.3f,true,-0.4f,0.5f);
    near(low.lowerTwistExcess,0.3f);near(low.upperTwistExcess,0);
    auto q=rotation(0,0,1,0.9f);
    auto high=measureSphericalLimits(q,true,0.3f,true,-0.4f,0.5f);
    near(high.upperTwistExcess,0.4f);near(high.lowerTwistExcess,0);
    auto opposite=measureSphericalLimits(b3NegateQuat(q),true,0.3f,true,-0.4f,0.5f);
    near(opposite.twist,high.twist);near(opposite.upperTwistExcess,high.upperTwistExcess);
    auto combined=measureSphericalLimits(b3MulQuat(rotation(0,1,0,0.7f),rotation(0,0,1,0.9f)),
        true,0.3f,true,-0.4f,0.5f);
    near(combined.swing,0.7f);near(combined.twist,0.9f);
    near(combined.coneExcess,0.4f);near(combined.upperTwistExcess,0.4f);
    auto onlyTwist=measureSphericalLimits(rotation(0,0,1,0.9f),false,0.0f,true,-0.4f,0.5f);
    near(onlyTwist.coneExcess,0);near(onlyTwist.upperTwistExcess,0.4f);
    auto disabled=measureSphericalLimits(rotation(1,0,0,0.7f),false,0.3f,false,0.1f,0.2f);
    near(disabled.coneExcess,0);near(disabled.lowerTwistExcess,0);near(disabled.upperTwistExcess,0);
    std::puts("spherical limit analytical controls passed");
}
