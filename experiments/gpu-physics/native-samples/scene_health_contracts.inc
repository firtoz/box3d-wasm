// Audited native setup, not measured CPU/GPU output. Shared with the validator.
// name, reference Y, launch speed, available gravity drop, speed rule
// Convex Pile: benchmarks.c CreateConvexPile, 80 layers at 4*layer+2.
SCENE_HEALTH_CONTRACT("Benchmark/Convex Pile", 318.0f, 120.0f, 0.0f, GENERIC)
// BenchmarkSensor CreateRow: 10 + 40*5, staggered by at most 1.
SCENE_HEALTH_CONTRACT("Benchmark/Sensor", 211.0f, 120.0f, 0.0f, GENERIC)
// VoxelMesh: ground origin is (5000,3500,-7000); retain local 200m screen.
SCENE_HEALTH_CONTRACT("Mesh/Voxel", 3500.0f, 120.0f, 0.0f, GENERIC)
// Native launches under gravity -10; static walls/floor supply no launch work.
SCENE_HEALTH_CONTRACT("Continuous/Thin Wall", 10.0f, 180.0f, 10.0f, BALLISTIC)
SCENE_HEALTH_CONTRACT("Events/Sensor Hits", 6.0f, 300.0f, 6.0f, BALLISTIC)
// Stall requests 600m/s; native integration clamps to the actual world limit.
// Capacity loss and stalled progress remain separate, mandatory failures.
SCENE_HEALTH_CONTRACT("Continuous/Stall", 1.0f, 600.0f, 0.0f, WORLD_CAP)
