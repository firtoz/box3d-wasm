// Exercise the production dual adapter and inspect its mapped CPU endpoint.
// Only viewer bookkeeping hooks are omitted; both physics worlds are real.
#include "both_dual.c"
#include <assert.h>

extern bool cpu_b3Body_IsContactRecyclingEnabled(b3BodyId body);
void gpu_samples_on_world_created(b3WorldId world, const b3WorldDef* def) { (void)world; (void)def; }
void gpu_samples_on_world_destroyed(b3WorldId world) { (void)world; }

int main(void)
{
    b3WorldDef wd = b3DefaultWorldDef();
    b3WorldId world = b3CreateWorld(&wd);
    b3BodyDef bd = b3DefaultBodyDef();
    bd.type = b3_dynamicBody;
    b3BodyId body = b3CreateBody(world, &bd);
    b3BodyId cpu = both_cpu_body(body);
    assert(cpu.index1 != 0);
    const bool states[] = {true, false, true, false};
    for (unsigned i = 0; i < sizeof(states) / sizeof(states[0]); ++i)
    {
        b3Body_EnableContactRecycling(body, states[i]);
        assert(b3Body_IsContactRecyclingEnabled(body) == states[i]);
        assert(cpu_b3Body_IsContactRecyclingEnabled(cpu) == states[i]);
    }
    b3DestroyWorld(world);
    puts("dual runtime recycling: GPU and CPU flags agree");
    return 0;
}
