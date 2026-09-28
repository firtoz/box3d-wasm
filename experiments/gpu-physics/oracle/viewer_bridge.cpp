// Reuse the real oracle fixtures, including their creation order and defaults.
#define GPU_PHYSICS_ORACLE_LIBRARY
#include "oracle.cpp"

// Mirrors Rust BodyStateGpu. Positions are COM, not body origins.
struct ViewerState {
    float pos[3], inv_mass;
    float vel[3]; uint32_t flags;
    float rot[4];
    float omega[3], sleep_velocity;
    float dp[3], sleep_time;
    float dq[4];
    float origin[3]; uint32_t origin_valid;
};
static_assert(sizeof(ViewerState) == 112);

extern "C" uint32_t viewer_cpu_abi() { return 2; }
extern "C" void* viewer_cpu_create(const char* scene, uint32_t count) {
    if (!scene || (std::strcmp(scene, "dominoes") && std::strcmp(scene, "mixed-stacks") && std::strcmp(scene, "falling-cubes"))) return nullptr;
    return new SceneState(build_scene(scene, count));
}
extern "C" void* viewer_cpu_create_workers(const char* scene, uint32_t count, int workers) {
    if (!scene || workers < 1 || workers > 64 || (std::strcmp(scene, "dominoes") && std::strcmp(scene, "mixed-stacks") && std::strcmp(scene, "falling-cubes"))) return nullptr;
    return new SceneState(build_scene(scene, count, workers));
}
extern "C" bool viewer_cpu_set_sleeping(void* ptr, bool enabled) {
    auto* st = static_cast<SceneState*>(ptr);
    b3World_EnableSleeping(st->world, enabled);
    return b3World_IsSleepingEnabled(st->world);
}
extern "C" void viewer_cpu_destroy(void* ptr) {
    if (!ptr) return;
    auto* st = static_cast<SceneState*>(ptr);
    destroy_scene(*st);
    delete st;
}
extern "C" uint32_t viewer_cpu_count(void* ptr) {
    return ptr ? static_cast<uint32_t>(static_cast<SceneState*>(ptr)->bodies.size()) : 0;
}
extern "C" void viewer_cpu_step(void* ptr, float dt, int substeps) {
    if (ptr && std::isfinite(dt) && dt > 0 && substeps > 0)
        b3World_Step(static_cast<SceneState*>(ptr)->world, dt, substeps);
}
extern "C" uint32_t viewer_cpu_states(void* ptr, ViewerState* out, uint32_t capacity) {
    if (!ptr || !out) return 0;
    const auto& st = *static_cast<SceneState*>(ptr);
    if (capacity < st.bodies.size()) return 0; // Never truncate a successful snapshot.
    for (size_t i = 0; i < st.bodies.size(); ++i) {
        const auto& v = st.bodies[i];
        auto& g = out[i]; g = {};
        auto p = b3Body_GetWorldCenter(v.id);
        g.pos[0] = float(p.x); g.pos[1] = float(p.y); g.pos[2] = float(p.z);
        float mass = b3Body_GetMass(v.id); g.inv_mass = mass > 0 ? 1.0f / mass : 0;
        auto vel = b3Body_GetLinearVelocity(v.id);
        g.vel[0] = vel.x; g.vel[1] = vel.y; g.vel[2] = vel.z;
        auto q = b3Body_GetRotation(v.id);
        g.rot[0] = q.v.x; g.rot[1] = q.v.y; g.rot[2] = q.v.z; g.rot[3] = q.s;
        auto w = b3Body_GetAngularVelocity(v.id);
        g.omega[0] = w.x; g.omega[1] = w.y; g.omega[2] = w.z;
        g.flags = v.flags | (b3Body_IsAwake(v.id) ? 0u : 4u);
        g.dq[3] = 1.0f;
    }
    return static_cast<uint32_t>(st.bodies.size());
}
