// Latest-step event semantics: skipping a read must not replay an older end.
#include "box3d/box3d.h"
#include "box3d/collision.h"
#include <cstdio>
#include <cstring>
int main(int argc, char** argv) {
  const bool unreadBegin = argc == 2 && std::strcmp(argv[1], "unread-begin") == 0;
  const bool latestEnd = argc == 2 && std::strcmp(argv[1], "latest-end") == 0;
  const bool retouch = argc == 2 && std::strcmp(argv[1], "retouch") == 0;
  if (argc > 2 || (argc == 2 && !latestEnd && !retouch && !unreadBegin)) return 64;
  auto wd = b3DefaultWorldDef();
  wd.gravity = b3Vec3_zero;
  wd.enableSleep = false;
  auto world = b3CreateWorld(&wd);
  auto gd = b3DefaultBodyDef();
  auto ground = b3CreateBody(world, &gd);
  auto sd = b3DefaultShapeDef();
  sd.enableContactEvents = true;
  auto floor = b3MakeBoxHull(4.0f, 0.5f, 4.0f);
  b3CreateHullShape(ground, &sd, &floor.base);
  auto bd = b3DefaultBodyDef();
  bd.type = b3_dynamicBody;
  bd.position = {0.0f, 1.0f, 0.0f};
  auto body = b3CreateBody(world, &bd);
  auto box = b3MakeBoxHull(0.5f, 0.5f, 0.5f);
  b3CreateHullShape(body, &sd, &box.base);
  b3World_Step(world, 1.0f / 60.0f, 4);
  auto first = unreadBegin ? b3ContactEvents{} : b3World_GetContactEvents(world);
  int firstBegin = first.beginCount, firstEnd = first.endCount;
  if (!latestEnd && !unreadBegin)
    b3Body_SetTransform(body, {10.0f, 1.0f, 0.0f}, b3Quat_identity);
  b3World_Step(world, 1.0f / 60.0f,
               4); // This step is intentionally unread in every scenario.
  if (latestEnd || unreadBegin)
    b3Body_SetTransform(body, {10.0f, 1.0f, 0.0f}, b3Quat_identity);
  if (retouch)
    b3Body_SetTransform(body, {0.0f, 1.0f, 0.0f}, b3Quat_identity);
  b3World_Step(world, 1.0f / 60.0f, 4);
  auto latest = b3World_GetContactEvents(world);
  int begin = latest.beginCount, end = latest.endCount;
  std::printf("{\"first_begin\":%d,\"first_end\":%d,\"latest_begin\":%d,"
              "\"latest_end\":%d}\n",
              firstBegin, firstEnd, begin, end);
  b3DestroyWorld(world);
  return firstBegin == int(!unreadBegin) && firstEnd == 0 && begin == int(retouch) && end == int(latestEnd || unreadBegin) ? 0 : 2;
}
