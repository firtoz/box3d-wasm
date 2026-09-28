// CPU-only diagnostic bridge, linked explicitly into reference fixtures.
// Keep upstream sources clean; this is not part of the public GPU ABI.
#include "joint.h"
#include "box3d/box3d.h"
#include <math.h>
#include <string.h>

bool reference_set_spherical_cache(b3JointId id, const float values[12])
{
    if (!b3Joint_IsValid(id) || b3Joint_GetType(id) != b3_sphericalJoint) return false;
    for (int i = 0; i < 12; ++i) if (!isfinite(values[i])) return false;
    b3SphericalJoint* j = &b3GetJointSimCheckType(id, b3_sphericalJoint)->sphericalJoint;
    j->linearImpulse = (b3Vec3){values[0], values[1], values[2]};
    j->springImpulse = (b3Vec3){values[3], values[4], values[5]};
    j->motorImpulse = (b3Vec3){values[6], values[7], values[8]};
    j->lowerTwistImpulse = values[9];
    j->upperTwistImpulse = values[10];
    j->swingImpulse = values[11];
    const float actual[12] = {j->linearImpulse.x, j->linearImpulse.y, j->linearImpulse.z,
        j->springImpulse.x, j->springImpulse.y, j->springImpulse.z,
        j->motorImpulse.x, j->motorImpulse.y, j->motorImpulse.z,
        j->lowerTwistImpulse, j->upperTwistImpulse, j->swingImpulse};
    return memcmp(actual, values, sizeof actual) == 0;
}

bool reference_set_revolute_cache(b3JointId id, const float values[9])
{
    if (!b3Joint_IsValid(id) || b3Joint_GetType(id) != b3_revoluteJoint) return false;
    for (int i = 0; i < 9; ++i) if (!isfinite(values[i])) return false;
    b3RevoluteJoint* j = &b3GetJointSimCheckType(id, b3_revoluteJoint)->revoluteJoint;
    j->linearImpulse = (b3Vec3){values[0], values[1], values[2]};
    j->perpImpulse = (b3Vec2){values[3], values[4]};
    j->springImpulse = values[5];j->motorImpulse = values[6];
    j->lowerImpulse = values[7];j->upperImpulse = values[8];
    const float actual[9] = {j->linearImpulse.x,j->linearImpulse.y,j->linearImpulse.z,
        j->perpImpulse.x,j->perpImpulse.y,j->springImpulse,j->motorImpulse,j->lowerImpulse,j->upperImpulse};
    return memcmp(actual, values, sizeof actual) == 0;
}
