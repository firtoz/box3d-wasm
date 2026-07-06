#include "dump-core.h"

#include "box3d/box3d.h"
#include "body.h"
#include "physics_world.h"
#include "solver_set.h"

#include <stdio.h>

#ifndef B3_NULL_INDEX
#define B3_NULL_INDEX (-1)
#endif

static int is_pool_index_free(const b3IdPool* pool, int index)
{
  for (int i = 0; i < pool->freeArray.count; i++)
  {
    if (pool->freeArray.data[i] == index)
      return 1;
  }
  return 0;
}

static void dump_body(FILE* out, const b3World* world, int bodyIndex, int first)
{
  b3BodyId bodyId = b3MakeBodyId((b3World*)world, bodyIndex);
  b3Body* body = b3GetBodyFullId((b3World*)world, bodyId);
  if (!body) return;

  b3BodySim* sim = b3GetBodySim((b3World*)world, body);
  if (!sim) return;

  b3Vec3 pos = sim->transform.p;
  b3Quat rot = sim->transform.q;

  b3Vec3 vel = {0.0f, 0.0f, 0.0f};
  b3Vec3 angVel = {0.0f, 0.0f, 0.0f};
  b3BodyState* state = b3GetBodyState((b3World*)world, body);
  if (state)
  {
    vel = state->linearVelocity;
    angVel = state->angularVelocity;
  }

  int isAwake = body->setIndex == b3_awakeSet;
  int bodyType = (int)body->type;

  fprintf(out, "%s{\"p\":[%.9g,%.9g,%.9g],\"q\":[%.9g,%.9g,%.9g,%.9g],\"v\":[%.9g,%.9g,%.9g],\"w\":[%.9g,%.9g,%.9g],\"t\":%d,\"a\":%s}",
    first ? "" : ",",
    (double)pos.x, (double)pos.y, (double)pos.z,
    (double)rot.v.x, (double)rot.v.y, (double)rot.v.z, (double)rot.s,
    (double)vel.x, (double)vel.y, (double)vel.z,
    (double)angVel.x, (double)angVel.y, (double)angVel.z,
    bodyType,
    isAwake ? "true" : "false");
}

int dump_bodies(FILE* out, b3WorldId worldId, int frame, int emitComma)
{
  if (!b3World_IsValid(worldId))
    return 0;

  b3World* world = b3GetWorldFromId(worldId);

  fprintf(out, "%s{\"frame\":%d,\"bodies\":[", emitComma ? "," : "", frame);

  int firstBody = 1;
  for (int i = 0; i < world->bodyIdPool.nextIndex; i++)
  {
    if (is_pool_index_free(&world->bodyIdPool, i))
      continue;
    dump_body(out, world, i, firstBody);
    firstBody = 0;
  }

  fprintf(out, "]}");
  return 1;
}

int all_bodies_asleep(b3WorldId worldId)
{
  if (!b3World_IsValid(worldId))
    return 1;
  b3World* world = b3GetWorldFromId(worldId);
  return world->solverSets.data[b3_awakeSet].bodyStates.count == 0;
}
