#ifdef GPU_REFERENCE_BOTH
extern "C" {
#include "both_ids.h"
}
#endif
#include "box3d/box3d.h"
#include "box3d/collision.h"
#include <cstdio>
#include <cstdlib>
struct QueryCheck {
  int hits = 0;
  bool identity = true;
  float fraction = 1;
  b3ShapeId expected = {};
};
static bool Overlap(b3ShapeId id, void *context) {
  auto *c = (QueryCheck *)context;
  ++c->hits;
  c->identity &= B3_ID_EQUALS(id, c->expected);
  return true;
}
static float Ray(b3ShapeId id, b3Pos, b3Vec3, float fraction, uint64_t, int,
                 int childIndex, void *context) {
  auto *c = (QueryCheck *)context;
  ++c->hits;
  c->identity &= B3_ID_EQUALS(id, c->expected);
  c->identity &= childIndex == 0;
  c->fraction = fraction;
  return fraction;
}
static bool Planes(b3ShapeId id, const b3PlaneResult *, int count,
                   void *context) {
  auto *c = (QueryCheck *)context;
  c->hits += count;
  c->identity &= B3_ID_EQUALS(id, c->expected);
  return true;
}
static bool AllowMover(b3ShapeId, void *) { return true; }
static bool oblique = false;
static size_t align8(size_t value) { return (value + 7u) & ~size_t(7u); }

static b3MeshData *make_grid_mesh(int cells, float halfWidth) {
  const int side = cells + 1;
  const int vertexCount = side * side;
  const int triangleCount = 2 * cells * cells;
  if (triangleCount > 8) {
    return nullptr;
  }
  size_t nodeOffset = align8(sizeof(b3MeshData));
  size_t vertexOffset = align8(nodeOffset + sizeof(b3MeshNode));
  size_t triangleOffset =
      align8(vertexOffset + size_t(vertexCount) * sizeof(b3Vec3));
  size_t materialOffset =
      align8(triangleOffset + size_t(triangleCount) * sizeof(b3MeshTriangle));
  size_t flagsOffset = align8(materialOffset + size_t(triangleCount));
  size_t byteCount = align8(flagsOffset + size_t(triangleCount));
  b3MeshData *mesh = static_cast<b3MeshData *>(calloc(1, byteCount));
  mesh->version = B3_MESH_VERSION;
  mesh->hash = 1;
  mesh->byteCount = int32_t(byteCount);
  mesh->bounds = {{-halfWidth, oblique ? -.5f * halfWidth : 0, -halfWidth},
                  {halfWidth, oblique ? .5f * halfWidth : 0, halfWidth}};
  mesh->nodeOffset = int32_t(nodeOffset);
  mesh->nodeCount = 1;
  mesh->vertexOffset = int32_t(vertexOffset);
  mesh->vertexCount = vertexCount;
  mesh->triangleOffset = int32_t(triangleOffset);
  mesh->triangleCount = triangleCount;
  mesh->materialOffset = int32_t(materialOffset);
  mesh->materialCount = 1;
  mesh->flagsOffset = int32_t(flagsOffset);
  b3MeshNode *node = reinterpret_cast<b3MeshNode *>(
      reinterpret_cast<char *>(mesh) + nodeOffset);
  node->lowerBound = mesh->bounds.lowerBound;
  node->upperBound = mesh->bounds.upperBound;
  node->data.asLeaf.type = 3;
  node->data.asLeaf.triangleCount = uint32_t(triangleCount);
  node->triangleOffset = 0;
  b3Vec3 *vertices =
      reinterpret_cast<b3Vec3 *>(reinterpret_cast<char *>(mesh) + vertexOffset);
  b3MeshTriangle *triangles = reinterpret_cast<b3MeshTriangle *>(
      reinterpret_cast<char *>(mesh) + triangleOffset);
  uint8_t *flags =
      reinterpret_cast<uint8_t *>(reinterpret_cast<char *>(mesh) + flagsOffset);
  int vertex = 0;
  for (int z = 0; z < side; ++z) {
    for (int x = 0; x < side; ++x) {
      vertices[vertex++] = {
          -halfWidth + 2.0f * halfWidth * float(x) / float(cells), 0.0f,
          -halfWidth + 2.0f * halfWidth * float(z) / float(cells)};
    }
  }
  int triangle = 0;
  for (int z = 0; z < cells; ++z) {
    for (int x = 0; x < cells; ++x) {
      int a = z * side + x;
      int b = a + 1;
      int c = a + side;
      int d = c + 1;
      triangles[triangle] = {a, d, b};
      flags[triangle++] = b3_allFlatEdges;
      triangles[triangle] = {a, c, d};
      flags[triangle++] = b3_allFlatEdges;
    }
  }
  if (oblique)
    for (int v = 0; v < vertexCount; ++v)
      vertices[v].y = .2f * vertices[v].x + .3f * vertices[v].z;
  return mesh;
}

int main(int argc, char **argv) {
  oblique = argc > 1 && argv[1][0] == '1';
  for (int bits = 0; bits < 8; ++bits) {
    auto wd = b3DefaultWorldDef();
    auto w = b3CreateWorld(&wd);
    auto bd = b3DefaultBodyDef();
    bd.position = {1, .25f, -2};
    bd.rotation = b3MakeQuatFromAxisAngle({0, 0, 1}, .2f);
    auto body = b3CreateBody(w, &bd);
    auto mesh = make_grid_mesh(1, 4);
    auto material = b3DefaultSurfaceMaterial();
    b3CompoundMeshDef instance = {};
    instance.meshData = mesh;
    instance.transform = {{.5f, 8, .25f},
                          b3MakeQuatFromAxisAngle({1, 0, 0}, .3f)};
    instance.scale = {(bits & 1) ? -2.f : 2.f, (bits & 2) ? -1.5f : 1.5f,
                      (bits & 4) ? -.5f : .5f};
    instance.materials = &material;
    instance.materialCount = 1;
    b3CompoundDef cd = {};
    // Repeated native geometry must survive temporary source-shape cleanup.
    // Keep the second instance away from the measured contact/ray corridor.
    b3CompoundMeshDef instances[2] = {instance, instance};
    instances[1].transform.p.x += 40;
    cd.meshes = instances;
    cd.meshCount = 2;
    auto compound = b3CreateCompound(&cd);
    auto sd = b3DefaultShapeDef();
    auto shape = b3CreateBakedCompoundShape(body, &sd, compound);
#ifdef GPU_REFERENCE_BOTH
    // Prove this executable selected the real dual-world bridge, not shim.c.
    if (!both_has_cpu_world(w) || !both_has_cpu_shape(shape)) {
      fprintf(stderr, "combined reference did not create mapped CPU/GPU objects\n");
      return 93;
    }
#endif
    free(mesh);
    auto n = b3Normalize(b3Vec3{oblique ? -.2f / instance.scale.x : 0,
                                1 / instance.scale.y,
                                oblique ? -.3f / instance.scale.z : 0});
    auto normal =
        b3RotateVector(bd.rotation, b3RotateVector(instance.transform.q, n));
    auto local =
        b3Vec3{.1f * instance.scale.x, oblique ? .05f * instance.scale.y : 0,
               .1f * instance.scale.z};
    auto center = b3TransformPoint({{1, .25f, -2}, bd.rotation},
                                   b3TransformPoint(instance.transform, local));
    for (int side = 0; side < 2; ++side) {
      auto direction = b3MulSV(side ? -1.f : 1.f, normal);
      auto origin = b3Add(center, b3MulSV(2, direction));
      auto ray = b3World_CastRayClosest(w, {origin.x, origin.y, origin.z},
                                        b3MulSV(-4, direction),
                                        b3DefaultQueryFilter());
      printf("%d %d %d %.9g %.9g %.9g %.9g %d %d\n", bits, side, ray.hit,
             ray.fraction, ray.normal.x, ray.normal.y, ray.normal.z,
             ray.hit && ray.shapeId.index1 == shape.index1, ray.childIndex);
    }
    QueryCheck overlap, ray, mover;
    overlap.expected = ray.expected = mover.expected = shape;
    auto close = b3Add(center, b3MulSV(.1f, normal));
    b3ShapeProxy proxy = {&close, 1, .25f};
    b3World_OverlapShape(w, b3Pos_zero, &proxy, b3DefaultQueryFilter(), Overlap,
                         &overlap);
    auto rayStart = b3Add(center, b3MulSV(.2f, normal));
    b3World_CastRay(w, {rayStart.x, rayStart.y, rayStart.z},
                    b3MulSV(-.4f, normal), b3DefaultQueryFilter(), Ray, &ray);
    b3Capsule capsule = {b3Vec3_zero, b3Vec3_zero, .25f};
    b3World_CollideMover(w, {close.x, close.y, close.z}, &capsule,
                         b3DefaultQueryFilter(), Planes, &mover);
    printf("query %d %d %d %d %d %.9g\n", bits, overlap.hits > 0, ray.hits > 0,
           mover.hits > 0, overlap.identity && ray.identity && mover.identity,
           ray.fraction);
    // Refit the existing query index after a host teleport. Internal collider
    // slots must remain distinct from the public compound ID used in callback
    // results.
    auto shift = b3Vec3{3, -2, 4};
    auto moved = b3Add(center, shift);
    b3Body_SetTransform(body,
                        {bd.position.x + shift.x, bd.position.y + shift.y,
                         bd.position.z + shift.z},
                        bd.rotation);
    overlap = QueryCheck{};
    ray = QueryCheck{};
    mover = QueryCheck{};
    overlap.expected = ray.expected = mover.expected = shape;
    close = b3Add(moved, b3MulSV(.1f, normal));
    // Preserve the same origin-relative overlap coordinates through the
    // teleport.
    auto relativeClose = b3Sub(close, shift);
    proxy.points = &relativeClose;
    b3World_OverlapShape(w, {shift.x, shift.y, shift.z}, &proxy,
                         b3DefaultQueryFilter(), Overlap, &overlap);
    rayStart = b3Add(moved, b3MulSV(.2f, normal));
    b3World_CastRay(w, {rayStart.x, rayStart.y, rayStart.z},
                    b3MulSV(-.4f, normal), b3DefaultQueryFilter(), Ray, &ray);
    b3World_CollideMover(w, {close.x, close.y, close.z}, &capsule,
                         b3DefaultQueryFilter(), Planes, &mover);
    printf("refit %d %d %d %d %d %.9g\n", bits, overlap.hits > 0, ray.hits > 0,
           mover.hits > 0, overlap.identity && ray.identity && mover.identity,
           ray.fraction);
    b3Body_SetTransform(body, bd.position, bd.rotation);
    auto castStart = b3Add(center, b3MulSV(2, normal));
    auto castFraction = b3World_CastMover(
        w, {castStart.x, castStart.y, castStart.z}, &capsule,
        b3MulSV(-4, normal), b3DefaultQueryFilter(), AllowMover, nullptr);
    printf("mover-cast %d %.9g\n", bits, castFraction);
    b3World_SetGravity(w, b3MulSV(-10, normal));
    auto dynamicDef = b3DefaultBodyDef();
    dynamicDef.type = b3_dynamicBody;
    dynamicDef.enableSleep = false;
    auto start = b3Add(center, b3MulSV(.3f, normal));
    dynamicDef.position = {start.x, start.y, start.z};
    auto ball = b3CreateBody(w, &dynamicDef);
    auto sphereDef = b3DefaultShapeDef();
    b3Sphere sphere = {{0, 0, 0}, .25f};
    b3CreateSphereShape(ball, &sphereDef, &sphere);
    for (int i = 0; i < 120; ++i)
      b3World_Step(w, 1.f / 60, 4);
    auto velocity = b3Body_GetLinearVelocity(ball);
    auto position = b3Body_GetPosition(ball);
    auto delta =
        b3Vec3{float(position.x - center.x), float(position.y - center.y),
               float(position.z - center.z)};
    printf("support %d %.9g %.9g\n", bits, b3Dot(delta, normal),
           b3Length(velocity));
    b3DestroyWorld(w);
    b3DestroyCompound(compound);
  }
}
