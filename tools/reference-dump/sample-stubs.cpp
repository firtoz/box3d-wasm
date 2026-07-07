#include "sample.h"

#include "box3d/box3d.h"
#include "gfx/debug_adapter.h"
#include "benchmarks.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

bool IsKeyDown(int key) { (void)key; return false; }
void SetKeyDown(int key, bool down) { (void)key; (void)down; }

SampleEntry g_sampleEntries[MAX_SAMPLES];
int g_sampleCount = 0;
int g_replayIndex = -1;

void SampleContext::Save() {}
void SampleContext::Load() {}

int RegisterSample(const char* category, const char* name, SampleCreateFcn* fcn)
{
  for (int i = 0; i < g_sampleCount; i++)
  {
    if (strcmp(g_sampleEntries[i].Category, category) == 0 &&
        strcmp(g_sampleEntries[i].Name, name) == 0)
    {
      return i;
    }
  }

  if (g_sampleCount >= MAX_SAMPLES) return -1;
  g_sampleEntries[g_sampleCount].Category = category;
  g_sampleEntries[g_sampleCount].Name = name;
  g_sampleEntries[g_sampleCount].CreateFcn = fcn;
  g_sampleCount++;
  return g_sampleCount - 1;
}

int RegisterReplay(const char* category, const char* name, SampleCreateFcn* fcn)
{
  int index = RegisterSample(category, name, fcn);
  g_replayIndex = index;
  return index;
}

Sample::Sample(SampleContext* context)
  : m_context(context),
    m_camera(&context->camera),
    m_worldId(b3_nullWorldId),
    m_mousePoint(),
    m_mouseBodyId(),
    m_mouseJointId(),
    m_mouseFraction(0.0f),
    m_mouseForceScale(100.0f),
    m_launchSpeedScale(5.0f),
    m_stepCount(0),
    m_textLine(0),
    m_textIncrement(22),
    m_triangleIndex(0),
    m_userMaterialId(0),
    m_recording(nullptr),
    m_recordStartStep(0),
    m_currentProfileIndex(0),
    m_profileReadIndex(0),
    m_profileWriteIndex(0),
    m_mouseLast(),
    m_mouseDelta(),
    m_didStep(false),
    m_stepWhilePaused(true),
    m_haveMouseLast(false)
{
  memset(m_profiles, 0, sizeof(m_profiles));
  g_randomSeed = RAND_SEED;
  b3Capacity capacity = {};
  CreateWorld(&capacity);
}

Sample::~Sample()
{
  ResetAdapterPool();
  ResetGroundShapeId();
  if (b3World_IsValid(m_worldId))
  {
    FinishRecording();
    b3DestroyWorld(m_worldId);
  }
}

void Sample::CreateWorld(b3Capacity* capacity)
{
  if (b3World_IsValid(m_worldId))
  {
    FinishRecording();
    b3DestroyWorld(m_worldId);
  }

  m_mouseBodyId = {};
  m_mouseJointId = {};
  m_mousePoint = {};

  b3WorldDef worldDef = b3DefaultWorldDef();
  worldDef.workerCount = m_context->workerCount;
  worldDef.enableSleep = m_context->enableSleep;
  AttachToWorldDef(&worldDef);
  if (capacity)
  {
    worldDef.capacity = *capacity;
  }
  m_worldId = b3CreateWorld(&worldDef);
  b3World_SetContactRecycleDistance(m_worldId, m_context->recycleDistance);
  B3_ASSERT(b3World_IsValid(m_worldId));
}

void Sample::Step()
{
  m_didStep = false;

  if (b3World_IsValid(m_worldId))
  {
    float timeStep = 0.0f;
    if (m_context->pause == false || m_context->singleStep > 0)
    {
      timeStep = m_context->hertz > 0.0f ? 1.0f / m_context->hertz : 0.0f;
      m_context->singleStep = m_context->singleStep > 0 ? m_context->singleStep - 1 : 0;
    }

    int subStepCount = m_context->subStepCount;
    b3World_EnableSleeping(m_worldId, m_context->enableSleep);
    b3World_EnableWarmStarting(m_worldId, m_context->enableWarmStarting);
    b3World_EnableContinuous(m_worldId, m_context->enableContinuous);

    if (timeStep > 0.0f || m_stepWhilePaused)
    {
      b3World_Step(m_worldId, timeStep, subStepCount);
    }

    if (timeStep > 0.0f)
    {
      m_stepCount += 1;
      m_didStep = true;

      if (m_profileWriteIndex - m_profileReadIndex == m_profileCapacity)
      {
        m_profileReadIndex += 1;
      }

      m_currentProfileIndex = m_profileWriteIndex & (m_profileCapacity - 1);
      m_profiles[m_currentProfileIndex] = b3World_GetProfile(m_worldId);
      m_profileWriteIndex += 1;
    }
  }

  m_triangleIndex = -1;
  m_userMaterialId = 0;
  m_textLine = 0;
}

void Sample::ResetText() { m_textLine = 0; }

void Sample::DrawTextLine(const char* text, ...) { (void)text; }

b3BodyId Sample::AddGroundBox(float extent)
{
  b3BodyDef bodyDef = b3DefaultBodyDef();
  bodyDef.position = {0.0f, -1.0f, 0.0f};
  b3BodyId bodyId = b3CreateBody(m_worldId, &bodyDef);

  b3ShapeDef shapeDef = b3DefaultShapeDef();
  b3BoxHull hull = b3MakeBoxHull(extent, 1.0f, extent);
  b3ShapeId shapeId = b3CreateHullShape(bodyId, &shapeDef, &hull.base);
  (void)shapeId;

  return bodyId;
}

float Sample::InfoPanelWidthEm() const { return 20.0f; }
bool Sample::FocusBounds(b3AABB* bounds) { (void)bounds; return false; }
b3BodyId Sample::FocusBody() const { b3BodyId id = {0}; return id; }
void Sample::FocusHome() {}
void Sample::StartRecording() {}
void Sample::FinishRecording() {}
void Sample::DrawMetrics() {}
void Sample::MouseDown(b3Vec2 p, int button, int modifiers) { (void)p; (void)button; (void)modifiers; }
void Sample::MouseUp(b3Vec2 p, int button) { (void)p; (void)button; }
void Sample::MouseMove(b3Vec2 p) { (void)p; }
void Sample::ToggleThirdPerson() {}
void Sample::ResetProfile() {}

void SelectSample(SampleContext* context, int selection, bool restart) { (void)context; (void)selection; (void)restart; }
void OpenReplayFileDialog(SampleContext* context) { (void)context; }
void DrawUI(SampleContext* context) { (void)context; }

float CastClosestCallback(b3ShapeId shapeId, b3Pos point, b3Vec3 normal, float fraction, uint64_t materialId, int triangleIndex, int childIndex, void* context)
{
  (void)shapeId; (void)point; (void)normal; (void)fraction; (void)materialId; (void)triangleIndex; (void)childIndex; (void)context;
  return fraction;
}

void CharacterMover::Initialize(Sample* sample, b3Pos position) { (void)sample; (void)position; }
void CharacterMover::SolveMove(float timeStep, b3Vec3 forward, b3Vec3 right, b3Vec2 throttle, bool clipVelocity) { (void)timeStep; (void)forward; (void)right; (void)throttle; (void)clipVelocity; }
void CharacterMover::Step(b3ShapeId* ignoreShapes, int ignoreCount, bool clipVelocity) { (void)ignoreShapes; (void)ignoreCount; (void)clipVelocity; }
