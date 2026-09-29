// Analytical steady cone-limit compliance under constant torque. Unit isotropic
// inertia and coincident COM anchors isolate K = I * (2*pi*effectiveHertz)^2.
// The final 120 of 600 steps must match torque/K within 1e-4 rad and settle below
// 1e-3 rad/s for both loads. This does not qualify contact-loaded Rain behavior.
#include "box3d/box3d.h"
#include <cmath>
#include <cstdio>
#include <initializer_list>
extern "C" void b3_world_gpu_wait_with_mirror(b3WorldId) __attribute__((weak));
int main() {
    bool pass=true;
    for(float expected:{0.01f,0.1f}) {
        auto wd=b3DefaultWorldDef();wd.gravity={0,0,0};wd.enableSleep=false;
        auto world=b3CreateWorld(&wd);
        auto bd=b3DefaultBodyDef();auto ground=b3CreateBody(world,&bd);
        bd.type=b3_dynamicBody;bd.rotation=b3MakeQuatFromAxisAngle({1,0,0},10.0f*B3_DEG_TO_RAD+expected);
        auto body=b3CreateBody(world,&bd);
        auto sd=b3DefaultShapeDef();sd.filter.maskBits=0;
        b3Sphere sphere={{0,0,0},0.1f};b3CreateSphereShape(body,&sd,&sphere);
        b3MassData md={1.0f,{0,0,0},b3Mat3_identity};b3Body_SetMassData(body,md);
        auto jd=b3DefaultSphericalJointDef();jd.base.bodyIdA=ground;jd.base.bodyIdB=body;
        jd.enableConeLimit=true;jd.coneAngle=10.0f*B3_DEG_TO_RAD;
        if(jd.base.constraintHertz!=60 || jd.base.constraintDampingRatio!=2 || jd.enableSpring || jd.enableMotor)return 3;
        auto joint=b3CreateSphericalJoint(world,&jd);
        const float h=1.0f/240.0f;
        const double hz=std::fmin(jd.base.constraintHertz,0.25/double(h));
        const double stiffness=std::pow(2.0*3.14159265358979323846*hz,2);
        const float torque=float(stiffness*expected);
        const double predicted=torque/stiffness;
        std::fprintf(stderr,"load %.9g torque %.9g predicted %.12g hertz %.9g damping %.9g\n",expected,torque,predicted,jd.base.constraintHertz,jd.base.constraintDampingRatio);
        double worst=0,speedMax=0;
        for(int i=0;i<600;++i) {
            b3Body_ApplyTorque(body,{torque,0,0},true);
            b3World_Step(world,1.0f/60.0f,4);
            if(b3_world_gpu_wait_with_mirror)b3_world_gpu_wait_with_mirror(world);
            auto q=b3Body_GetRotation(body);auto w=b3Body_GetAngularVelocity(body);
            double swing=2*std::atan2(std::hypot(double(q.v.x),double(q.v.y)),std::hypot(double(q.v.z),double(q.s)));
            double residual=swing-jd.coneAngle;
            double speed=std::sqrt(double(w.x)*w.x+double(w.y)*w.y+double(w.z)*w.z);
            if(!std::isfinite(residual)||!std::isfinite(speed))return 4;
            std::printf("%.9g %d %.12g %.12g %.12g\n",expected,i,residual,predicted,speed);
            if(i>=480){worst=std::fmax(worst,std::fabs(residual-predicted));speedMax=std::fmax(speedMax,speed);}
        }
        std::fprintf(stderr,"result %.9g max_error %.12g max_speed %.12g\n",expected,worst,speedMax);
        pass=pass && worst<1e-4 && speedMax<1e-3;
        b3DestroyWorld(world);
    }
    return pass?0:1;
}
