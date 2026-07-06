// Linkable no-op definitions for gfx/draw.h, gfx/debug_adapter.h, host/camera.h, etc.
// The real headers are included for declarations; these TU provides the symbols.

#include "../../box3d/samples/gfx/draw.h"
#include "../../box3d/samples/gfx/debug_adapter.h"
#include "../../box3d/samples/gfx/text.h"
#include "../../box3d/samples/host/camera.h"
#include "../../box3d/samples/mesh_loader.h"
#include "sokol_app.h"

#include <stdarg.h>

// --- gfx/draw.h ------------------------------------------------------------

void SetDrawOrigin(b3Pos origin) { (void)origin; }
b3Pos GetDrawOrigin(void) { b3Pos p = {0, 0, 0}; return p; }
void ResetGroundShapeId(void) {}
void DrawCube(b3WorldTransform transform, b3Vec3 scale, Vec4 baseColor) { (void)transform; (void)scale; (void)baseColor; }
void DrawCubeEx(b3WorldTransform transform, b3Vec3 scale, Vec4 baseColor, float metallic, float roughness, TransparentShadowCast shadowCast) { (void)transform; (void)scale; (void)baseColor; (void)metallic; (void)roughness; (void)shadowCast; }
void DrawSphere(b3WorldTransform transform, float radius, Vec4 baseColor) { (void)transform; (void)radius; (void)baseColor; }
void DrawSphereEx(b3WorldTransform transform, float radius, Vec4 baseColor, float metallic, float roughness, TransparentShadowCast shadowCast) { (void)transform; (void)radius; (void)baseColor; (void)metallic; (void)roughness; (void)shadowCast; }
void DrawCapsule(b3WorldTransform transform, float halfLength, float radius, Vec4 baseColor) { (void)transform; (void)halfLength; (void)radius; (void)baseColor; }
void DrawCapsuleEx(b3WorldTransform transform, float halfLength, float radius, Vec4 baseColor, float metallic, float roughness, TransparentShadowCast shadowCast) { (void)transform; (void)halfLength; (void)radius; (void)baseColor; (void)metallic; (void)roughness; (void)shadowCast; }
void DrawSolidSphere(b3WorldTransform transform, b3Sphere sphere, Vec4 color) { (void)transform; (void)sphere; (void)color; }
void DrawSolidCapsule(b3WorldTransform transform, b3Capsule capsule, Vec4 color) { (void)transform; (void)capsule; (void)color; }
void DrawHull(b3WorldTransform transform, const b3HullData* hull, Vec4 color) { (void)transform; (void)hull; (void)color; }
void DrawPlane(b3Vec3 normal, b3Pos point, Vec4 color) { (void)normal; (void)point; (void)color; }
void DrawLine(b3Pos a, b3Pos b, Vec4 color) { (void)a; (void)b; (void)color; }
void DrawLineEx(b3Pos a, b3Pos b, Vec4 color, float thickness, OverlayThicknessUnit thicknessUnit, OverlayOcclusionMode occlusionMode) { (void)a; (void)b; (void)color; (void)thickness; (void)thicknessUnit; (void)occlusionMode; }
void DrawPoint(b3Pos p, float size, Vec4 color) { (void)p; (void)size; (void)color; }
void DrawPointEx(b3Pos p, Vec4 color, float size, OverlayThicknessUnit sizeUnit, OverlayOcclusionMode occlusionMode) { (void)p; (void)color; (void)size; (void)sizeUnit; (void)occlusionMode; }
void DrawArrow(b3Pos a, b3Pos b, Vec4 color) { (void)a; (void)b; (void)color; }
void DrawArrowEx(b3Pos a, b3Pos b, Vec4 color, float thickness, OverlayThicknessUnit thicknessUnit, OverlayOcclusionMode occlusionMode, float headLengthFrac) { (void)a; (void)b; (void)color; (void)thickness; (void)thicknessUnit; (void)occlusionMode; (void)headLengthFrac; }
void DrawCross(b3Pos center, float size, Vec4 color) { (void)center; (void)size; (void)color; }
void DrawCrossEx(b3Pos center, float size, Vec4 color, float thickness, OverlayThicknessUnit thicknessUnit, OverlayOcclusionMode occlusionMode) { (void)center; (void)size; (void)color; (void)thickness; (void)thicknessUnit; (void)occlusionMode; }
void DrawAabb(b3Vec3 min, b3Vec3 max, Vec4 color) { (void)min; (void)max; (void)color; }
void DrawAabbEx(b3Vec3 min, b3Vec3 max, Vec4 color, float thickness, OverlayThicknessUnit thicknessUnit, OverlayOcclusionMode occlusionMode) { (void)min; (void)max; (void)color; (void)thickness; (void)thicknessUnit; (void)occlusionMode; }
void DrawBounds(b3AABB bounds, float extension, Vec4 color) { (void)bounds; (void)extension; (void)color; }
void DrawAxes(b3WorldTransform transform, float size) { (void)transform; (void)size; }
void DrawAxesEx(b3WorldTransform transform, float size, float thickness, OverlayThicknessUnit thicknessUnit, OverlayOcclusionMode occlusionMode) { (void)transform; (void)size; (void)thickness; (void)thicknessUnit; (void)occlusionMode; }
void DrawGrid(b3Pos center, b3Vec3 normal, float halfExtent, int divisions, Vec4 color) { (void)center; (void)normal; (void)halfExtent; (void)divisions; (void)color; }
void DrawGroundGrid(int size) { (void)size; }
void DrawTriangle(b3WorldTransform transform, b3Vec3 a, b3Vec3 b, b3Vec3 c, Vec4 color) { (void)transform; (void)a; (void)b; (void)c; (void)color; }
void DrawWireSphere(b3WorldTransform transform, const b3Sphere* sphere, int segments, Vec4 color) { (void)transform; (void)sphere; (void)segments; (void)color; }
void DrawString3D(b3Pos point, Vec4 color, const char* format, ...) { (void)point; (void)color; (void)format; }

// --- gfx/debug_adapter.h ---------------------------------------------------

void InitAdapter(void) {}
void ResetAdapterPool(void) {}
void AttachToWorldDef(b3WorldDef* def) { (void)def; }
static b3DebugDraw g_guiDraw = {};
void MakeDebugDraw(b3DebugDraw* out) { if (out) *out = {}; }
int GetDebugShapeCount(void) { return 0; }
void SetGroundShape(b3ShapeId shapeId) { (void)shapeId; }
void SetShapeMaterial(b3ShapeId shapeId, Vec4 color, float metallic, float roughness) { (void)shapeId; (void)color; (void)metallic; (void)roughness; }
void SetTransparentDynamic(bool enabled) { (void)enabled; }
bool GetTransparentDynamic(void) { return false; }
void SetViewBounds(b3AABB bounds) { (void)bounds; }
int GetLastCompoundDrawStats(int* outTotal) { if (outTotal) *outTotal = 0; return 0; }
void SetSelectedBody(b3BodyId bodyId) { (void)bodyId; }
void SetSelectedShape(b3ShapeId shapeId) { (void)shapeId; }
void ClearSelection(void) {}
b3BodyId GetSelectedBody(void) { b3BodyId id = {0}; return id; }
bool IsBodySelected(b3BodyId bodyId) { (void)bodyId; return false; }
b3DebugDraw* GetGuiDraw(void) { return &g_guiDraw; }
void ApplyGuiFlags(b3DebugDraw* out) { (void)out; }

// --- gfx/text.h -----------------------------------------------------------

void DrawString(b3Vec3 worldPos, Vec4 color, const char* text) { (void)worldPos; (void)color; (void)text; }
void DrawScreenString(int x, int y, Vec4 color, const char* text) { (void)x; (void)y; (void)color; (void)text; }
void DrawScreenStringFormat(int x, int y, Vec4 color, const char* fmt, ...) { (void)x; (void)y; (void)color; (void)fmt; }
int GetTextCount(void) { return 0; }
const TextEntry* GetTextAt(int i) { (void)i; return NULL; }
void ResetTextArena(void) {}

// --- mesh_loader.h ---------------------------------------------------------

b3MeshData* CreateMeshData(const char* path, float scale, bool zUp, bool useMedianSplit, bool identifyConvexEdges, bool weldVertices)
{
  (void)path;
  (void)scale;
  (void)zUp;
  (void)useMedianSplit;
  (void)identifyConvexEdges;
  (void)weldVertices;
  return NULL;
}

void LoadTempMesh(const char* path, TempMesh* tempMesh, float scale, bool zUp)
{
  (void)path;
  (void)tempMesh;
  (void)scale;
  (void)zUp;
}

// --- sokol_app.h -----------------------------------------------------------

extern "C" {
int sapp_width(void) { return 1920; }
int sapp_height(void) { return 1080; }
void sapp_quit(void) {}
double sapp_frame_duration(void) { return 1.0 / 60.0; }
uint64_t sapp_frame_count(void) { return 0; }
void sapp_lock_mouse(bool lock) { (void)lock; }
void sapp_request_quit(void) {}
}

// --- host/camera.h ---------------------------------------------------------

Camera::Camera()
{
  m_pivot = b3Vec3{0.0f, 0.0f, 0.0f};
  m_yaw = 0.0f;
  m_pitch = 0.0f;
  m_radius = 10.0f;
  m_fov = 0.25f * B3_PI;
  m_near = 0.1f;
  m_far = kViewDistance;
  m_drawDistance = kViewDistance;
  m_speed = 10.0f;
  m_width = 1920;
  m_height = 1080;
  m_thirdPerson = false;
  m_position = b3Vec3{0.0f, 0.0f, 0.0f};
  m_worldEye = b3Vec3{0.0f, 0.0f, -10.0f};
  m_right = b3Vec3{1.0f, 0.0f, 0.0f};
  m_up = b3Vec3{0.0f, 1.0f, 0.0f};
  m_forward = b3Vec3{0.0f, 0.0f, 1.0f};
  m_view = MakeIdentity();
  m_viewInv = MakeIdentity();
  m_proj = MakeIdentity();
  m_projInv = MakeIdentity();
  m_renderXform = MakeIdentity();
  m_renderXformInv = MakeIdentity();
  m_lengthUnitsPerMeter = 1.0f;
  m_zUp = false;
}

void Camera::OnEvent(const sapp_event* e) { (void)e; }
void Camera::Update(float dt, int width, int height) { (void)dt; m_width = width; m_height = height; }
b3Vec3 Camera::Position() const { return m_worldEye; }
void Camera::SetOrbit(float yawRadians, float pitchRadians, float radius) { m_yaw = yawRadians; m_pitch = pitchRadians; m_radius = radius; }
void Camera::SetView(float yawDegrees, float pitchDegrees, float radius, b3Pos pivot) { (void)yawDegrees; (void)pitchDegrees; m_radius = radius; m_pivot = pivot; }
void Camera::Frame(b3AABB aabb, float aspect, float padding) { (void)aabb; (void)aspect; (void)padding; }
void Camera::RebuildProj(float aspect) { (void)aspect; }
void Camera::UpdateTransform() {}
PickRay Camera::BuildPickRay(float x, float y) const { (void)x; (void)y; PickRay ray = {m_worldEye, b3Vec3{0.0f, 0.0f, 1.0f}}; return ray; }
void Camera::SetRenderTransform(float lengthUnitsPerMeter, bool zUp) { m_lengthUnitsPerMeter = lengthUnitsPerMeter; m_zUp = zUp; }
