#pragma once

#include <algorithm>
#include <cmath>
#include <cstring>

// These are broad diagnostic screens, not contact/support/energy qualification.
// Keep the raw historical 120m/s and absolute Y=200 flags alongside this result.
struct NativeSceneHealthLimits
{
    float reference_y;
    float speed;
    float max_y;
    bool valid;
};

inline NativeSceneHealthLimits native_scene_health_limits(const char* sample, float world_speed)
{
    enum Rule { GENERIC, BALLISTIC, WORLD_CAP };
    float reference = 0.0f, launch = 120.0f, drop = 0.0f;
    Rule rule = GENERIC;
    if (sample && std::strcmp(sample, "Continuous/Bounce House") == 0) launch = 180.0f;
#define SCENE_HEALTH_CONTRACT(name, y, speed, height, mode) \
    if (sample && std::strcmp(sample, name) == 0) { reference = y; launch = speed; drop = height; rule = mode; }
#include "scene_health_contracts.inc"
#undef SCENE_HEALTH_CONTRACT
    double ceiling = launch;
    double rise = 200.0; // Same gross excursion screen, in the scene's frame.
    bool valid = std::isfinite(world_speed) && world_speed > 0.0f;
    if (rule == BALLISTIC)
    {
        // v^2 <= v_launch^2 + 2*g*drop. The existing Bounce House arithmetic
        // allowance is 0.05m/s; no physical test tolerance is changed here.
        ceiling = std::sqrt(double(launch)*launch + 2.0*10.0*drop) + 0.05;
        rise = std::max(rise, double(launch)*launch/(2.0*10.0));
    }
    else if (rule == WORLD_CAP)
    {
        valid = std::isfinite(world_speed) && world_speed > 0.0f;
        ceiling = double(world_speed) + 0.05;
        rise = std::max(rise, double(world_speed)*world_speed/(2.0*10.0));
    }
    return {reference, float(ceiling), float(double(reference)+rise), valid};
}

inline bool native_scene_health_failed(int nan_count, float speed, float max_y,
                                      NativeSceneHealthLimits limits)
{
    return !limits.valid || nan_count > 0 || !std::isfinite(speed) || !std::isfinite(max_y)
        || speed > limits.speed || max_y > limits.max_y;
}
