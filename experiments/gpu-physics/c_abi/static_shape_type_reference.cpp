#include "box3d/box3d.h"
#include "box3d/collision.h"
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>
static size_t align8(size_t n) { return (n + 7) & ~size_t(7); }
static b3HeightFieldData *make_height_field(int cells, int mode,
                                            bool clockwise) {
  const int columns = cells + 1;
  const int rows = cells + 1;
  const int heightCount = columns * rows;
  const int cellCount = cells * cells;
  const int triangleCount = 2 * cellCount;
  size_t heightsOffset = align8(sizeof(b3HeightFieldData));
  size_t materialOffset =
      align8(heightsOffset + size_t(heightCount) * sizeof(uint16_t));
  size_t flagsOffset = align8(materialOffset + size_t(cellCount));
  size_t byteCount = align8(flagsOffset + size_t(triangleCount));
  b3HeightFieldData *hf =
      static_cast<b3HeightFieldData *>(calloc(1, byteCount));
  hf->version = B3_HEIGHT_FIELD_VERSION;
  hf->hash = 1;
  hf->byteCount = int32_t(byteCount);
  hf->minHeight = -1.0f;
  hf->maxHeight = 1.0f;
  hf->heightScale = 2.0f / 65535.0f;
  hf->scale = {8.0f / float(cells), 1.0f, 8.0f / float(cells)};
  hf->columnCount = columns;
  hf->rowCount = rows;
  hf->heightsOffset = int32_t(heightsOffset);
  hf->materialOffset = int32_t(materialOffset);
  hf->flagsOffset = int32_t(flagsOffset);
  hf->clockwise = clockwise;
  uint16_t *heights = reinterpret_cast<uint16_t *>(
      reinterpret_cast<char *>(hf) + heightsOffset);
  uint8_t *materials = reinterpret_cast<uint8_t *>(
      reinterpret_cast<char *>(hf) + materialOffset);
  uint8_t *flags =
      reinterpret_cast<uint8_t *>(reinterpret_cast<char *>(hf) + flagsOffset);
  float low = FLT_MAX;
  float high = -FLT_MAX;
  for (int z = 0; z < rows; ++z) {
    for (int x = 0; x < columns; ++x) {
      float height = 0.0f;
      if (mode == 1) {
        height = -0.75f + 1.5f * float(x) / float(cells);
      } else if (mode == 2) {
        height = 0.35f * sinf(0.45f * float(x)) * cosf(0.35f * float(z));
      }
      float quantized =
          fminf(65535.0f, fmaxf(0.0f, (height + 1.0f) / hf->heightScale));
      heights[z * columns + x] = uint16_t(quantized);
      float decoded =
          hf->minHeight + hf->heightScale * float(heights[z * columns + x]);
      low = fminf(low, decoded);
      high = fmaxf(high, decoded);
    }
  }
  for (int i = 0; i < cellCount; ++i) {
    materials[i] = 0;
  }
  if (mode == 3) {
    materials[(cells / 2) * cells + cells / 2] = B3_HEIGHT_FIELD_HOLE;
  }
  for (int i = 0; i < triangleCount; ++i) {
    flags[i] = b3_allFlatEdges;
  }
  hf->aabb = {{0.0f, low, 0.0f}, {8.0f, high, 8.0f}};
  return hf;
}
int main() {
  for (int kind = 0; kind < 3; kind++)
    for (int target = 1; target <= 2; target++)
      for (int enabled = 0; enabled < 2; enabled++) {
        auto wd = b3DefaultWorldDef();
        auto w = b3CreateWorld(&wd);
        auto bd = b3DefaultBodyDef();
        bd.position = {0, 1, 0};
        bd.isEnabled = enabled;
        auto b = b3CreateBody(w, &bd);
        auto sd = b3DefaultShapeDef();
        b3ShapeId shape = {};
        b3CompoundData *compound = nullptr;
        b3HeightFieldData *height = nullptr;
        if (kind == 0) {
          b3CompoundSphereDef sphere = {};
          sphere.sphere = {{0, 0, 0}, .1f};
          sphere.material = b3DefaultSurfaceMaterial();
          b3CompoundDef def = {};
          def.spheres = &sphere;
          def.sphereCount = 1;
          compound = b3CreateCompound(&def);
          shape = b3CreateBakedCompoundShape(b, &sd, compound);
        } else if (kind == 1) {
          height = make_height_field(1, 0, false);
          shape = b3CreateHeightFieldShape(b, &sd, height);
        } else {
          b3Sphere sphere = {{0, 0, 0}, .1f};
          shape = b3CreateSphereShape(b, &sd, &sphere);
        }
        b3Body_SetType(b, (b3BodyType)target);
        b3Body_SetLinearVelocity(b, {0, -200, 0});
        auto pos = b3Body_GetPosition(b);
        auto vel = b3Body_GetLinearVelocity(b);
        printf("%d %d %d %d %d %.9g %.9g\n", kind, target, enabled,
               int(b3Body_GetType(b)), b3Shape_IsValid(shape), float(pos.y),
               vel.y);
        bool liveBody = b3Body_IsValid(b);
        b3DestroyWorld(w);
        if (kind == 2)
          printf("lifetime %d %d %d %d %d %d %d %d\n", kind, target, enabled,
                 liveBody, b3Body_IsValid(b), b3Shape_IsValid(shape),
                 b3Body_IsValid(b3_nullBodyId),
                 b3Shape_IsValid(b3_nullShapeId));
        if (compound)
          b3DestroyCompound(compound);
        free(height);
      }
  // Initial enabled state must not accidentally wake a sleeping body.
  for (int awake = 0; awake < 2; awake++)
    for (int enabled = 0; enabled < 2; enabled++) {
      auto wd = b3DefaultWorldDef();
      auto w = b3CreateWorld(&wd);
      auto bd = b3DefaultBodyDef();
      bd.type = b3_dynamicBody;
      bd.isAwake = awake;
      bd.isEnabled = enabled;
      auto b = b3CreateBody(w, &bd);
      printf("creation %d %d %d %d\n", awake, enabled, b3Body_IsAwake(b),
             b3Body_IsEnabled(b));
      b3DestroyWorld(w);
    }
}
