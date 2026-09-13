#include "box3d/box3d.h"
#include "box3d/collision.h"
#include <cstdio>
#include <cstdlib>
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

static void ray(b3ShapeId shape, int fixture, int state, int target,
                b3Transform transform, b3Vec3 origin, b3Vec3 direction) {
  auto o = b3TransformPoint(transform, origin);
  auto d = b3RotateVector(transform.q, direction);
  auto r = b3Shape_RayCast(shape, {o.x, o.y, o.z}, d);
  printf("%d %d %d %d %.9g %d %d %d %.9g %.9g %.9g %.9g %.9g %.9g\n", fixture,
         state, target, r.hit, r.fraction, r.childIndex, r.materialIndex,
         r.triangleIndex, r.point.x, r.point.y, r.point.z, r.normal.x,
         r.normal.y, r.normal.z);
}
static bool count_shape(b3ShapeId, void *p) {
  ++*(int *)p;
  return true;
}
int main() {
  for (int fixture = 0; fixture < 3; ++fixture) {
    auto wd = b3DefaultWorldDef();
    auto w = b3CreateWorld(&wd);
    auto bd = b3DefaultBodyDef();
    auto b = b3CreateBody(w, &bd);
    b3CompoundSphereDef spheres[2] = {};
    for (int i = 0; i < 2; ++i) {
      spheres[i].sphere = {{i ? 2.f : -2.f, 0, 0}, .5f};
      spheres[i].material = b3DefaultSurfaceMaterial();
      spheres[i].material.userMaterialId = 100 + i;
    }
    b3SurfaceMaterial materials[2] = {b3DefaultSurfaceMaterial(),
                                      b3DefaultSurfaceMaterial()};
    materials[0].userMaterialId = 200;
    materials[1].userMaterialId = 201;
    auto mesh = make_grid_mesh(1, 1);
    mesh->materialCount = 2;
    auto mi = (unsigned char *)mesh + mesh->materialOffset;
    mi[0] = 0;
    mi[1] = 1;
    b3CompoundMeshDef instance = {};
    instance.meshData = mesh;
    instance.transform = b3Transform_identity;
    instance.scale = b3Vec3_one;
    instance.materials = materials;
    instance.materialCount = 2;
    b3CompoundDef cd = {};
    cd.spheres = spheres;
    cd.sphereCount = 2;
    if (fixture == 1) {
      cd.meshes = &instance;
      cd.meshCount = 1;
    }
    auto c = b3CreateCompound(&cd);
    auto sd = b3DefaultShapeDef();
    auto s = fixture == 2 ? b3CreateSphereShape(b, &sd, &spheres[0].sphere)
                          : b3CreateBakedCompoundShape(b, &sd, c);
    free(mesh);
    for (int state = 0; state < 3; ++state) {
      b3Transform t = b3Transform_identity;
      if (state) {
        t = {{10, 3, -5}, b3MakeQuatFromAxisAngle({0, 0, 1}, .4f)};
        b3Body_SetTransform(b, {10, 3, -5}, t.q);
      }
      if (state == 2)
        b3Body_Disable(b);
      ray(s, fixture, state, 0, t, {-4, 0, 0}, {8, 0, 0});
      ray(s, fixture, state, 1, t, {4, 0, 0}, {-8, 0, 0});
      ray(s, fixture, state, 2, t, {0, 2, 2}, {0, -4, 0});
      ray(s, fixture, state, 6, t, {-2, 0, 0}, {8, 0, 0});
      ray(s, fixture, state, 7, t, {-4, 0, 0}, {1, 0, 0});
      ray(s, fixture, state, 8, t, {-1.8f, 0, 0}, {8, 0, 0});
      ray(s, fixture, state, 9, t, {-2, 0, 0}, {0, 0, 0});
      ray(s, fixture, state, 10, t, {-4, 0, 0}, {0, 0, 0});
      ray(s, fixture, state, 11, t, {-2.5f, 0, 0}, {8, 0, 0});
      ray(s, fixture, state, 12, t, {-2.5f, 0, 0}, {-8, 0, 0});
      int count = 0;
      b3World_OverlapAABB(w, {{-20, -20, -20}, {20, 20, 20}},
                          b3DefaultQueryFilter(), count_shape, &count);
      printf("scope %d %d %d %.9g\n", fixture, state, count,
             b3Shape_GetDensity(s));
      if (fixture == 1) {
        ray(s, fixture, state, 3, t, {.5f, 2, -.5f}, {0, -4, 0});
        ray(s, fixture, state, 4, t, {-.5f, 2, .5f}, {0, -4, 0});
        ray(s, fixture, state, 5, t, {.5f, -2, -.5f}, {0, 4, 0});
      }
    }
    b3DestroyWorld(w);
    b3DestroyCompound(c);
  }
}
