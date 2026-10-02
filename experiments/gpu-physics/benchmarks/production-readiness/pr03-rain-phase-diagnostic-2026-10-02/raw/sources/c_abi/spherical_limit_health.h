#pragma once
#include "box3d/box3d.h"
#include <algorithm>

struct SphericalLimitHealth {
    float swing, twist, coneExcess, lowerTwistExcess, upperTwistExcess;
};

// Measure violation, not allowed angular motion. The Z-axis twist and swing
// decomposition matches Box3D's spherical joint convention and is q/-q invariant.
inline SphericalLimitHealth measureSphericalLimits(b3Quat relative,
    bool coneEnabled, float coneLimit, bool twistEnabled, float lower, float upper)
{
    const float swing = b3GetSwingAngle(relative);
    const float twist = b3GetTwistAngle(relative);
    return {swing, twist, coneEnabled ? std::max(0.0f, swing-coneLimit) : 0.0f,
        twistEnabled ? std::max(0.0f, lower-twist) : 0.0f,
        twistEnabled ? std::max(0.0f, twist-upper) : 0.0f};
}
