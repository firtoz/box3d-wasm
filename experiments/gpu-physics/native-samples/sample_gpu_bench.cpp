// Native counterpart of scenes::create_mixed_stacks(world, count).
// Compiled unchanged into CPU, GPU and combined viewers.
#include "sample.h"
#include "box3d/box3d.h"

template<int Count>
class GpuBenchMixedStacks : public Sample
{
public:
    explicit GpuBenchMixedStacks(SampleContext* context) : Sample(context)
    {
        if (!context->restart) m_camera->SetView(30.0f, 35.0f, 85.0f, {28.5, 0.0, 21.0});
        b3ShapeDef shape = b3DefaultShapeDef();
        constexpr int perLayer = (Count + 1) / 2;
        constexpr float extent = 3.0f * ((perLayer - 1) / 20) + 3.0f;
        constexpr float groundHalf = extent > 100.0f ? extent : 100.0f;
        b3BoxHull groundHull = b3MakeBoxHull(groundHalf, 1.0f, groundHalf);
        // Deliberate overlapping supports exercise multiple static constraints.
        for (int i = 0; i < 2; ++i)
        {
            b3BodyDef ground = b3DefaultBodyDef();
            ground.position = {0.0, -1.0, 0.0};
            b3CreateHullShape(b3CreateBody(m_worldId, &ground), &shape, &groundHull.base);
        }
        b3BoxHull cube = b3MakeBoxHull(0.5f, 0.5f, 0.5f);
        b3BodyDef body = b3DefaultBodyDef();
        body.type = b3_dynamicBody;
        for (int i = 0; i < Count; ++i)
        {
            body.position = {3.0f * ((i % perLayer) % 20), 0.5f + (i >= perLayer ? 1.0f : 0.0f),
                             3.0f * ((i % perLayer) / 20)};
            b3CreateHullShape(b3CreateBody(m_worldId, &body), &shape, &cube.base);
        }
    }
    static Sample* Create(SampleContext* context) { return new GpuBenchMixedStacks(context); }
};
static int mixedStacks = RegisterSample("GPU Bench", "Mixed Stacks 600", GpuBenchMixedStacks<600>::Create);

static int denseMixedStacks = RegisterSample("GPU Bench", "Mixed Stacks 4096", GpuBenchMixedStacks<4096>::Create);
