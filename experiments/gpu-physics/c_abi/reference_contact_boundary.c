// CPU-only replay diagnostic, explicitly linked with -Wl,--wrap=b3Solve.
// Observe contacts after normal collision update, without an extra world step.
#include "physics_world.h"
#include "contact.h"
#include "solver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static void cache_failure(const char* reason)
{
    fprintf(stderr, "contact-cache rejected: %s\n", reason);
    exit(8);
}

static float cache_float(FILE* input)
{
    float value;
    if (fscanf(input, "%f", &value) != 1 || !isfinite(value)) cache_failure("invalid float");
    return value;
}

// A deliberately narrow fixture payload: the two feet and one self-contact
// identified in the Rain onset audit. Validate every record before any mutation.
static void restore_contact_caches(b3World* world, const char* path)
{
    FILE* input = fopen(path, "r");
    if (!input) cache_failure("cannot open payload");
    int version, count;
    if (fscanf(input, "%d %d", &version, &count) != 2 || version != 1 || count != 3)
        cache_failure("expected version1 with three audited contacts");
    b3Manifold* targets[3] = {0};
    b3Manifold values[3];
    for (int i = 0; i < count; ++i)
    {
        int a, b, sa, sb, points;
        if (fscanf(input, "%d %d %d %d %d", &a, &b, &sa, &sb, &points) != 5 || points < 1 || points > 4)
            cache_failure("invalid contact identity/count");
        float normal[3];
        for (int k = 0; k < 3; ++k) normal[k] = cache_float(input);
        for (int c = 0; c < world->contacts.count; ++c)
        {
            b3Contact* contact = world->contacts.data + c;
            if (contact->setIndex == -1 || contact->edges[0].bodyId != a || contact->edges[1].bodyId != b ||
                contact->shapeIdA != sa || contact->shapeIdB != sb) continue;
            for (int m = 0; m < contact->manifoldCount; ++m)
            {
                b3Manifold* candidate = contact->manifolds + m;
                if (candidate->pointCount != points || memcmp(&candidate->normal, normal, sizeof normal) != 0) continue;
                if (targets[i]) cache_failure("ambiguous manifold");
                targets[i] = candidate;
            }
        }
        if (!targets[i]) cache_failure("missing matching manifold");
        for (int j = 0; j < i; ++j) if (targets[j] == targets[i]) cache_failure("duplicate manifold");
        values[i] = *targets[i];
        float impulse[7];
        for (int k = 0; k < 7; ++k) impulse[k] = cache_float(input);
        values[i].frictionImpulse = (b3Vec3){impulse[0], impulse[1], impulse[2]};
        values[i].twistImpulse = impulse[3];
        values[i].rollingImpulse = (b3Vec3){impulse[4], impulse[5], impulse[6]};
        bool used[4] = {false};
        for (int j = 0; j < points; ++j)
        {
            unsigned feature;
            int triangle;
            if (fscanf(input, "%u %d", &feature, &triangle) != 2) cache_failure("missing point identity");
            float geometry[7];
            for (int k = 0; k < 7; ++k) geometry[k] = cache_float(input);
            float normalImpulse = cache_float(input);
            if (normalImpulse < 0.0f) cache_failure("negative normal impulse");
            int found = -1;
            for (int k = 0; k < points; ++k)
            {
                b3ManifoldPoint* p = values[i].points + k;
                if (p->featureId != feature || p->triangleIndex != triangle) continue;
                if (found != -1 || used[k]) cache_failure("ambiguous or duplicate point");
                found = k;
                const float actual[7] = {p->anchorA.x,p->anchorA.y,p->anchorA.z,
                    p->anchorB.x,p->anchorB.y,p->anchorB.z,p->separation};
                // Diagnostic identity guard, much smaller than contact slop.
                for (int n = 0; n < 7; ++n)
                    if (fabsf(actual[n] - geometry[n]) > 1.0e-6f) cache_failure("contact geometry mismatch");
            }
            if (found == -1) cache_failure("missing feature/triangle");
            used[found] = true;
            values[i].points[found].normalImpulse = normalImpulse;
        }
    }
    char extra;
    if (fscanf(input, " %c", &extra) != EOF || ferror(input)) cache_failure("trailing data or read error");
    fclose(input);
    for (int i = 0; i < count; ++i) *targets[i] = values[i];
    fprintf(stderr, "contact-cache restored %d manifolds\n", count);
}

void __real_b3Solve(b3World* world, b3StepContext* context);

void __wrap_b3Solve(b3World* world, b3StepContext* context)
{
    // This fixture owns one world and invokes its steps serially.
    static unsigned step = 0;
    ++step;
    const char* cache = getenv("RAIN_REPLAY_CONTACT_CACHE");
    if (step == 1 && cache) restore_contact_caches(world, cache);
    unsigned patches = 0;
    for (int i = 0; i < world->contacts.count; ++i)
    {
        const b3Contact* c = world->contacts.data + i;
        if (c->setIndex == -1) continue;
        for (int j = 0; j < c->manifoldCount; ++j)
        {
            const b3Manifold* m = c->manifolds + j;
            if (m->pointCount == 0) continue;
            ++patches;
            fprintf(stderr, "solve-boundary-manifold %u %d %d %d %d %d %d %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",
                step, c->edges[0].bodyId, c->edges[1].bodyId, c->shapeIdA, c->shapeIdB, j, m->pointCount,
                m->normal.x, m->normal.y, m->normal.z,
                m->frictionImpulse.x, m->frictionImpulse.y, m->frictionImpulse.z,
                m->twistImpulse, m->rollingImpulse.x, m->rollingImpulse.y, m->rollingImpulse.z);
            for (int k = 0; k < m->pointCount; ++k)
            {
                const b3ManifoldPoint* p = m->points + k;
                fprintf(stderr, "solve-boundary-point %u %d %d %d %u %d %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",
                    step, c->edges[0].bodyId, c->edges[1].bodyId, j, p->featureId, p->triangleIndex,
                    p->anchorA.x, p->anchorA.y, p->anchorA.z,
                    p->anchorB.x, p->anchorB.y, p->anchorB.z, p->separation, p->normalImpulse);
            }
        }
    }
    fprintf(stderr, "solve-boundary-step %u %u\n", step, patches);
    __real_b3Solve(world, context);
}
