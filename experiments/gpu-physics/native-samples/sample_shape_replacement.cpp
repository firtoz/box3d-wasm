#include "../c_abi/shape_replacement_schedule.h"
#include "sample.h"

class GpuShapeReplacement : public Sample {
  b3BodyId m_body;
  b3ShapeId m_shape;
  int m_lastPhase = -1;

public:
  explicit GpuShapeReplacement(SampleContext *context) : Sample(context) {
    if (!context->restart)
      m_camera->SetView(20.0f, 25.0f, 9.0f, {0, 1, 0});
    auto bd = b3DefaultBodyDef();
    bd.position = {0, -0.5, 0};
    auto ground = b3CreateBody(m_worldId, &bd);
    auto sd = b3DefaultShapeDef();
    auto floor = b3MakeBoxHull(5, 0.5f, 5);
    b3CreateHullShape(ground, &sd, &floor.base);
    bd.type = b3_dynamicBody;
    bd.position = {0, 1.5, 0};
    m_body = b3CreateBody(m_worldId, &bd);
    auto box = b3MakeBoxHull(0.5f, 0.5f, 0.5f);
    m_shape = b3CreateHullShape(m_body, &sd, &box.base);
    b3Shape_SetName(m_shape, "Replacing geometry");
  }
  void Step() override {
    Sample::Step();
    if (!m_didStep)
      return;
    int phase = m_stepCount / 180;
    if (phase != m_lastPhase) {
      replace_sample_shape(m_shape, m_body, phase);
      m_lastPhase = phase;
    }
  }
  static Sample *Create(SampleContext *context) {
    return new GpuShapeReplacement(context);
  }
};
static int replacement =
    RegisterSample("GPU API", "Shape Replacement", GpuShapeReplacement::Create);
