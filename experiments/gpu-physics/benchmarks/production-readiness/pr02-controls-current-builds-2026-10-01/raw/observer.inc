// Diagnostic-only observer in generated build copies, never production/Box3D.
#if defined(GPU_PHYSICS_SAMPLES)
extern "C" uint64_t gpu_b3_world_physics_step(b3WorldId);
extern "C" uint64_t gpu_b3_world_completed_step(b3WorldId);
extern "C" int gpu_b3_world_completed_known(b3WorldId);
#endif
#if defined(BOTH_SAMPLES)
extern "C" b3WorldId both_cpu_world(b3WorldId);
extern "C" bool cpu_b3World_IsSleepingEnabled(b3WorldId);
extern "C" bool cpu_b3World_IsWarmStartingEnabled(b3WorldId);
extern "C" bool cpu_b3World_IsContinuousEnabled(b3WorldId);
#endif
static void controls_observe(const SampleContext& c, int frame)
{
    static FILE* output = nullptr;
    if (!output) {
        const char* path = getenv("GPU_VIEWER_CONTROL_OBSERVER");
        if (!path) return;
        output = fopen(path, "w");
        if (!output) { fprintf(stderr, "controls observer open failure\n"); abort(); }
    }
    if (!c.sample) return;
    b3WorldId w = c.sample->m_worldId;
    b3BodyEvents events = b3World_GetBodyEvents(w);
    b3Counters counters = b3World_GetCounters(w);
    int submitted = c.sample->m_stepCount, completed = submitted, known = 1;
#if defined(GPU_PHYSICS_SAMPLES)
    submitted = (int)gpu_b3_world_physics_step(w);
    completed = (int)gpu_b3_world_completed_step(w);
    known = gpu_b3_world_completed_known(w);
#endif
    fprintf(output, "{\"frame\":%d,\"world\":[%u,%u],\"sample_step\":%d,\"submitted\":%d,\"completed\":%d,\"completed_known\":%d,\"pause\":%d,\"single_remaining\":%d,\"context_flags\":[%d,%d,%d],\"world_flags\":[%d,%d,%d],\"hertz\":%.9g,\"substeps\":%d,\"show_metrics\":%d,\"body_count\":%d,\"shape_count\":%d,\"move_count\":%d",
        frame, w.index1, w.generation, c.sample->m_stepCount, submitted, completed, known,
        c.pause, c.singleStep, c.enableSleep, c.enableWarmStarting, c.enableContinuous,
        b3World_IsSleepingEnabled(w), b3World_IsWarmStartingEnabled(w), b3World_IsContinuousEnabled(w),
        c.hertz, c.subStepCount, c.showMetrics, counters.bodyCount, counters.shapeCount, events.moveCount);
    if (events.moveCount > 0) {
        const b3BodyMoveEvent& e = events.moveEvents[0];
        fprintf(output, ",\"first_body\":[%d,%u,%u],\"first_position\":[%.9g,%.9g,%.9g],\"fell_asleep\":%d",
            e.bodyId.index1, e.bodyId.world0, e.bodyId.generation,
            e.transform.p.x, e.transform.p.y, e.transform.p.z, e.fellAsleep);
    }
#if defined(BOTH_SAMPLES)
    b3WorldId cpu = both_cpu_world(w);
    fprintf(output, ",\"cpu_world\":[%u,%u],\"cpu_flags\":[%d,%d,%d]", cpu.index1, cpu.generation,
        cpu_b3World_IsSleepingEnabled(cpu), cpu_b3World_IsWarmStartingEnabled(cpu), cpu_b3World_IsContinuousEnabled(cpu));
#endif
    fputs("}\n", output);
    fflush(output);
}
