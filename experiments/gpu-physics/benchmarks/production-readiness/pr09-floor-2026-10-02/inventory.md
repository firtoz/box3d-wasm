# PR09 full sample inventory (2026-10-02)

165 active native CPU registrations and shared GPU counterparts, 174 registered browser CPU entries, and 20 CPU oracle fixtures. Disabled `#if0` registrations are excluded; active browser ports of disabled samples are explicit gaps. The native source list comes from the viewer CMake registration inputs, including local GPU Bench/API scenes and Replay. Registration proves availability, not correct behavior.

Five native scenes have a bounded **floor-only** review. All other samples and the entire browser/oracle catalogs are **UNREVIEWED** in this milestone. No full PR09 scene equivalence or physical/performance pass is claimed. See [evidence and limitations](README.md); [machine-readable inventory](inventory.json) retains source hashes, registration lines, scene API/floor hints, settings, interaction hints, dispositions and separate backend status.

## Native CPU/GPU viewer inventory

Shared CPU scene geometry/defaults and GPU registration use the same named scene source. Ordinary and native review columns below apply to standalone viewers; combined covers both routes. Except the five sampled rows, defaults, interactions, behavior and performance remain unreviewed.

| CPU identity → GPU counterpart | Scene source | Ordinary | Native | Combined | Finding / disposition |
| --- | --- | --- | --- | --- | --- |
| Benchmark/Candy Cups → same scene | [CandyCups](../../../../box3d/samples/sample_benchmark.cpp#L373) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Benchmark/Chains → same scene | [BenchmarkChains](../../../../box3d/samples/sample_benchmark.cpp#L1223) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Benchmark/Convex Pile → same scene | [BenchmarkConvexPile](../../../../box3d/samples/sample_benchmark.cpp#L1488) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Benchmark/Destruction → same scene | [BenchmarkDestruction](../../../../box3d/samples/sample_benchmark.cpp#L1417) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Benchmark/Explosion → same scene | [BenchmarkExplosion](../../../../box3d/samples/sample_benchmark.cpp#L490) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Benchmark/Falling Boxes → same scene | [FallingBoxes](../../../../box3d/samples/sample_benchmark.cpp#L295) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Benchmark/Falling Trees → same scene | [BenchmarkFallingTrees](../../../../box3d/samples/sample_benchmark.cpp#L732) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Benchmark/Height Field → same scene | [BenchmarkHeightField](../../../../box3d/samples/sample_benchmark.cpp#L660) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Benchmark/Hull → same scene | [BenchmarkHull](../../../../box3d/samples/sample_benchmark.cpp#L1111) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Benchmark/Joint Grid → same scene | [BenchmarkJointGrid](../../../../box3d/samples/sample_benchmark.cpp#L250) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Benchmark/Junkyard → same scene | [BenchmarkJunkyard](../../../../box3d/samples/sample_benchmark.cpp#L1456) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Benchmark/Large Pyramid → same scene | [BenchmarkLargePyramid](../../../../box3d/samples/sample_benchmark.cpp#L46) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Benchmark/Large World → same scene | [BenchmarkLargeWorld](../../../../box3d/samples/sample_benchmark.cpp#L1024) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Benchmark/Many Pyramids → same scene | [BenchmarkManyPyramids](../../../../box3d/samples/sample_benchmark.cpp#L103) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Benchmark/Rain → same scene | [BenchmarkRain](../../../../box3d/samples/sample_benchmark.cpp#L156) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR04 physical investigation; no closed experiment resumed |
| Benchmark/Sensor → same scene | [BenchmarkSensor](../../../../box3d/samples/sample_benchmark.cpp#L965) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Benchmark/Washer → same scene | [BenchmarkWasher](../../../../box3d/samples/sample_benchmark.cpp#L992) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Benchmark/Wide Pyramid → same scene | [BenchmarkWidePyramid](../../../../box3d/samples/sample_benchmark.cpp#L70) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Bodies/Body Type → same scene | [BodyType](../../../../box3d/samples/sample_bodies.cpp#L269) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Bodies/Cast → same scene | [BodyCast](../../../../box3d/samples/sample_bodies.cpp#L981) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Bodies/Class Ring → same scene | [ClassRing](../../../../box3d/samples/sample_bodies.cpp#L1284) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Bodies/Disable → same scene | [DisableBody](../../../../box3d/samples/sample_bodies.cpp#L794) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Bodies/Fixed Rotation → same scene | [FixedRotation](../../../../box3d/samples/sample_bodies.cpp#L1188) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Bodies/Gyroscopic Precession → same scene | [GyroscopicPrecession](../../../../box3d/samples/sample_bodies.cpp#L584) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Bodies/Gyroscopic Torque → same scene | [GyroscopicTorque](../../../../box3d/samples/sample_bodies.cpp#L370) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Bodies/Kinematic → same scene | [Kinematic](../../../../box3d/samples/sample_bodies.cpp#L1057) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Bodies/Lock Mixing → same scene | [LockMixing](../../../../box3d/samples/sample_bodies.cpp#L1139) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Bodies/Offset Kinematic → same scene | [OffsetKinematic](../../../../box3d/samples/sample_bodies.cpp#L1335) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Bodies/Spinning Book → same scene | [SpinningBooks](../../../../box3d/samples/sample_bodies.cpp#L317) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Bodies/Weeble → same scene | [Weeble](../../../../box3d/samples/sample_bodies.cpp#L685) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Character/CapsulePlane → same scene | [CapsulePlane](../../../../box3d/samples/sample_character.cpp#L146) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Character/Mover → same scene | [BasicMover](../../../../box3d/samples/sample_character.cpp#L584) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Character/MoverOverlap → same scene | [MoverOverlap](../../../../box3d/samples/sample_character.cpp#L313) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Character/Rigid Body → same scene | [RigidBodyCharacter](../../../../box3d/samples/sample_character.cpp#L1667) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Collision/Capsule Cast Ray → same scene | [CapsuleCastRay](../../../../box3d/samples/sample_collision.cpp#L2798) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Collision/Cast World → same scene | [CastWorld](../../../../box3d/samples/sample_collision.cpp#L772) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Collision/Distance Debug → same scene | [DistanceDebug](../../../../box3d/samples/sample_collision.cpp#L2048) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Collision/Initial Overlap → same scene | [InitialOverlap](../../../../box3d/samples/sample_collision.cpp#L1680) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Collision/Long Ray Cast → same scene | [LongRayCast](../../../../box3d/samples/sample_collision.cpp#L1573) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Collision/Mesh Scale → same scene | [MeshScale](../../../../box3d/samples/sample_collision.cpp#L888) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Collision/Overlap World → same scene | [OverlapWorld](../../../../box3d/samples/sample_collision.cpp#L1329) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Collision/Ray Curtain → same scene | [RayCurtain](../../../../box3d/samples/sample_collision.cpp#L118) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Collision/Shape Cast → same scene | [ShapeCast](../../../../box3d/samples/sample_collision.cpp#L1144) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Collision/Shape Cast Debug → same scene | [ShapeCastDebug](../../../../box3d/samples/sample_collision.cpp#L1791) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Collision/Shape Distance → same scene | [ShapeDistance](../../../../box3d/samples/sample_collision.cpp#L2496) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Collision/Time of Impact → same scene | [TimeOfImpact](../../../../box3d/samples/sample_collision.cpp#L2724) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Compound/Hulls → same scene | [CompoundHulls](../../../../box3d/samples/sample_compound.cpp#L243) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Compound/Mesh Tile → same scene | [MeshTile](../../../../box3d/samples/sample_compound.cpp#L477) | floor-only reviewed | floor-only reviewed | floor-only reviewed | Four compound mesh platforms visible; no missing-platform defect in sampled stages. |
| Compound/Simple → same scene | [SimpleCompound](../../../../box3d/samples/sample_compound.cpp#L108) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Compound/Spheres → same scene | [CompoundSpheres](../../../../box3d/samples/sample_compound.cpp#L169) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Compound/Tile Floor → same scene | [TileFloor](../../../../box3d/samples/sample_compound.cpp#L361) | floor-only reviewed | floor-only reviewed | floor-only reviewed | Tiled compound floor visible; no missing-floor defect in two sampled stages. |
| Compound/Village → same scene | [Village](../../../../box3d/samples/sample_compound.cpp#L806) | floor-only reviewed | floor-only reviewed | floor-only reviewed | Confirmed missing GPU ground/buildings in BOTH before combined viewers; repaired stable metadata allocation restores them. Single viewers retain floor. |
| Continuous/Bounce House → same scene | [BounceHouse](../../../../box3d/samples/sample_continuous.cpp#L145) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Continuous/Bullet vs Stack → same scene | [BulletVersusStack](../../../../box3d/samples/sample_continuous.cpp#L277) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Continuous/Hump Mesh → same scene | [HumpMesh](../../../../box3d/samples/sample_continuous.cpp#L861) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Continuous/Is Fast → same scene | [IsFast](../../../../box3d/samples/sample_continuous.cpp#L945) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Continuous/Mesh Drop → same scene | [MeshDrop](../../../../box3d/samples/sample_continuous.cpp#L748) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Continuous/Needle Mesh → same scene | [NeedleMesh](../../../../box3d/samples/sample_continuous.cpp#L388) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Continuous/Spinning Stick → same scene | [SpinningStick](../../../../box3d/samples/sample_continuous.cpp#L187) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Continuous/Stall → same scene | [Stall](../../../../box3d/samples/sample_continuous.cpp#L1029) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Continuous/Thin Wall → same scene | [ThinWall](../../../../box3d/samples/sample_continuous.cpp#L71) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Determinism/Falling Ragdolls → same scene | [FallingRagdolls](../../../../box3d/samples/sample_determinism.cpp#L62) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR04 physical investigation; no closed experiment resumed |
| Determinism/Mesh Drop → same scene | [MeshDropDeterminism](../../../../box3d/samples/sample_determinism.cpp#L266) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Determinism/Query Spawn → same scene | [QuerySpawn](../../../../box3d/samples/sample_determinism.cpp#L213) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Determinism/Wave Pile → same scene | [WavePile](../../../../box3d/samples/sample_determinism.cpp#L115) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Events/Contact → same scene | [ContactEvent](../../../../box3d/samples/sample_events.cpp#L934) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Events/Hit → same scene | [HitEvent](../../../../box3d/samples/sample_events.cpp#L246) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Events/Joint → same scene | [JointEvent](../../../../box3d/samples/sample_events.cpp#L572) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Events/Move → same scene | [MoveEvent](../../../../box3d/samples/sample_events.cpp#L329) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Events/Persistent Contact → same scene | [PersistentContact](../../../../box3d/samples/sample_events.cpp#L1033) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Events/Sensor Hits → same scene | [SensorHits](../../../../box3d/samples/sample_events.cpp#L1264) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Events/Sensor Visit → same scene | [SensorVisit](../../../../box3d/samples/sample_events.cpp#L81) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| GPU API/Shape Replacement → same scene | [GpuShapeReplacement](../../../../experiments/gpu-physics/native-samples/sample_shape_replacement.cpp#L41) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| GPU Bench/Falling Cubes → same scene | [GpuBenchFallingCubes](../../../../experiments/gpu-physics/native-samples/sample_gpu_bench.cpp#L69) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| GPU Bench/Mixed Stacks 4096 → same scene | [GpuBenchMixedStacks<4096>](../../../../experiments/gpu-physics/native-samples/sample_gpu_bench.cpp#L39) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| GPU Bench/Mixed Stacks 600 → same scene | [GpuBenchMixedStacks<600>](../../../../experiments/gpu-physics/native-samples/sample_gpu_bench.cpp#L37) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Geometry/Box Hull → same scene | [BoxHull](../../../../box3d/samples/sample_geometry.cpp#L136) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Geometry/Capsule Mass → same scene | [CapsuleMass](../../../../box3d/samples/sample_geometry.cpp#L648) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Geometry/Hull → same scene | [Hull](../../../../box3d/samples/sample_geometry.cpp#L231) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Geometry/Hull Reduction → same scene | [HullReduction](../../../../box3d/samples/sample_geometry.cpp#L360) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Geometry/Hull Transform → same scene | [HullTransform](../../../../box3d/samples/sample_geometry.cpp#L488) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Issues/Capsule Mesh → same scene | [CapsuleMeshBug](../../../../box3d/samples/sample_issues.cpp#L493) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Issues/Convex Jitter → same scene | [ConvexJitter](../../../../box3d/samples/sample_issues.cpp#L343) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Issues/Crash → same scene | [Crash](../../../../box3d/samples/sample_issues.cpp#L77) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Issues/GMod Wheel Stack → same scene | [WheelStack](../../../../box3d/samples/sample_issues.cpp#L1147) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Issues/Hull Crash → same scene | [HullCrash](../../../../box3d/samples/sample_issues.cpp#L234) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Issues/Multiple Prismatic → same scene | [MultiplePrismatic](../../../../box3d/samples/sample_issues.cpp#L133) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Issues/Restitution Overshoot → same scene | [RestitutionOvershoot](../../../../box3d/samples/sample_issues.cpp#L1254) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Issues/Slide Twist Off Center Shape → same scene | [SlideTwistOffCenterShape](../../../../box3d/samples/sample_issues.cpp#L1304) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Issues/s&box Ghost Collisions → same scene | [SBoxGhostCollisions](../../../../box3d/samples/sample_issues.cpp#L925) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Issues/s&box mover → same scene | [SBoxMover](../../../../box3d/samples/sample_issues.cpp#L421) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Joints/Ball and Chain → same scene | [BallAndChain](../../../../box3d/samples/sample_joint.cpp#L1602) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Joints/Bridge → same scene | [Bridge](../../../../box3d/samples/sample_joint.cpp#L1954) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Joints/Distance Joint → same scene | [DistanceJoint](../../../../box3d/samples/sample_joint.cpp#L236) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Joints/Door → same scene | [Door](../../../../box3d/samples/sample_joint.cpp#L1831) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Joints/Driving → same scene | [Driving](../../../../box3d/samples/sample_joint.cpp#L2624) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Joints/Filter → same scene | [FilterJoint](../../../../box3d/samples/sample_joint.cpp#L276) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Joints/Gear Lift → same scene | [GearLift](../../../../box3d/samples/sample_joint.cpp#L3100) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Joints/Motion Locks → same scene | [MotionLocks](../../../../box3d/samples/sample_joint.cpp#L2235) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Joints/Motor Joint → same scene | [MotorJoint](../../../../box3d/samples/sample_joint.cpp#L445) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Joints/Parallel Spring → same scene | [ParallelJoint](../../../../box3d/samples/sample_joint.cpp#L1013) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Joints/Prismatic → same scene | [PrismaticJoint](../../../../box3d/samples/sample_joint.cpp#L718) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Joints/Revolute → same scene | [RevoluteJoint](../../../../box3d/samples/sample_joint.cpp#L1191) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Joints/Spherical → same scene | [SphericalJoint](../../../../box3d/samples/sample_joint.cpp#L897) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Joints/Top Down Friction → same scene | [TopDownFriction](../../../../box3d/samples/sample_joint.cpp#L550) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Joints/Weld → same scene | [WeldJoint](../../../../box3d/samples/sample_joint.cpp#L1290) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Joints/Wheel → same scene | [WheelJoint](../../../../box3d/samples/sample_joint.cpp#L1536) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Manifold/Capsule vs Capsule → same scene | [CapsuleAndCapsule](../../../../box3d/samples/sample_manifold.cpp#L562) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Manifold/Capsule vs Hull → same scene | [CapsuleAndHull](../../../../box3d/samples/sample_manifold.cpp#L621) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Manifold/Capsule vs Sphere → same scene | [CapsuleAndSphere](../../../../box3d/samples/sample_manifold.cpp#L430) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Manifold/Hull vs Hull → same scene | [HullAndHull](../../../../box3d/samples/sample_manifold.cpp#L810) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Manifold/Hull vs Sphere → same scene | [HullAndSphere](../../../../box3d/samples/sample_manifold.cpp#L473) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Manifold/Sphere vs Sphere → same scene | [SphereAndSphere](../../../../box3d/samples/sample_manifold.cpp#L392) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Manifold/Triangle vs Capsule → same scene | [TriangleAndCapsule](../../../../box3d/samples/sample_manifold.cpp#L677) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Manifold/Triangle vs Hull → same scene | [TriangleAndHull](../../../../box3d/samples/sample_manifold.cpp#L926) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Manifold/Triangle vs Sphere → same scene | [TriangleAndSphere](../../../../box3d/samples/sample_manifold.cpp#L526) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Mesh/Big Box → same scene | [BigBoxMesh](../../../../box3d/samples/sample_mesh.cpp#L382) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Mesh/Box → same scene | [BoxMesh](../../../../box3d/samples/sample_mesh.cpp#L551) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Mesh/Creation Benchmark → same scene | [MeshCreationBenchmark](../../../../box3d/samples/sample_mesh.cpp#L1413) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Mesh/Grid → same scene | [GridMesh](../../../../box3d/samples/sample_mesh.cpp#L208) | floor-only reviewed | floor-only reviewed | floor-only reviewed | Triangle mesh ground and dynamic cylinder visible; no missing-floor defect in sampled stages. |
| Mesh/Height Field → same scene | [HeightField](../../../../box3d/samples/sample_mesh.cpp#L1003) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Mesh/Hollow Box → same scene | [HollowBox](../../../../box3d/samples/sample_mesh.cpp#L1623) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Mesh/Reflection → same scene | [MeshReflection](../../../../box3d/samples/sample_mesh.cpp#L748) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Mesh/Viewer → same scene | [MeshViewer](../../../../box3d/samples/sample_mesh.cpp#L1327) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Mesh/Voxel → same scene | [VoxelMesh](../../../../box3d/samples/sample_mesh.cpp#L1532) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Ragdoll/Box → same scene | [RagdollOnBox](../../../../box3d/samples/sample_ragdoll.cpp#L80) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Ragdoll/Incline → same scene | [RagdollIncline](../../../../box3d/samples/sample_ragdoll.cpp#L335) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Ragdoll/Mesh → same scene | [RagdollOnMesh](../../../../box3d/samples/sample_ragdoll.cpp#L206) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Ragdoll/Pile → same scene | [RagdollPile](../../../../box3d/samples/sample_ragdoll.cpp#L260) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Replay/Viewer → same scene | [ReplayViewer](../../../../box3d/samples/sample_replay.cpp#L1843) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR02 excluded native recording/player/static-tree operations; explicit unavailable gap, review UI at PR10 |
| Robustness/HighMassRatio1 → same scene | [HighMassRatio1](../../../../box3d/samples/sample_robustness.cpp#L70) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Robustness/Overflow Color Pile → same scene | [OverflowColorPile](../../../../box3d/samples/sample_robustness.cpp#L281) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Robustness/Overlap Recovery → same scene | [OverlapRecovery](../../../../box3d/samples/sample_robustness.cpp#L241) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Robustness/Tiny Pyramid → same scene | [TinyPyramid](../../../../box3d/samples/sample_robustness.cpp#L129) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Shapes/Conveyor Belt → same scene | [ConveyorBelt](../../../../box3d/samples/sample_shapes.cpp#L486) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Shapes/Conveyor Mesh → same scene | [ConveyorMesh](../../../../box3d/samples/sample_shapes.cpp#L681) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Shapes/High Resistance → same scene | [HighResistance](../../../../box3d/samples/sample_shapes.cpp#L149) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Shapes/Inclined Plane → same scene | [InclinedPlane](../../../../box3d/samples/sample_shapes.cpp#L54) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Shapes/Isotropic Friction → same scene | [IsotropicFriction](../../../../box3d/samples/sample_shapes.cpp#L193) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Shapes/Restitution → same scene | [Restitution](../../../../box3d/samples/sample_shapes.cpp#L337) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Shapes/Rolling Resistance → same scene | [RollingResistance](../../../../box3d/samples/sample_shapes.cpp#L110) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Shapes/Slide Twist → same scene | [SlideTwist](../../../../box3d/samples/sample_shapes.cpp#L239) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Shapes/Static Invoke → same scene | [StaticInvoke](../../../../box3d/samples/sample_shapes.cpp#L436) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Shapes/Wind → same scene | [Wind](../../../../box3d/samples/sample_shapes.cpp#L848) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Shapes/Wind Drop → same scene | [WindDrop](../../../../box3d/samples/sample_shapes.cpp#L909) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Shapes/Wind Flap → same scene | [WindFlap](../../../../box3d/samples/sample_shapes.cpp#L1025) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Stacking/Arch → same scene | [Arch](../../../../box3d/samples/sample_stacking.cpp#L758) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Stacking/Box Stack → same scene | [BoxStack](../../../../box3d/samples/sample_stacking.cpp#L402) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Stacking/Capsule Stack → same scene | [CapsuleStack](../../../../box3d/samples/sample_stacking.cpp#L192) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Stacking/Card House → same scene | [CardHouse](../../../../box3d/samples/sample_stacking.cpp#L87) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Stacking/Cylinder → same scene | [Cylinder](../../../../box3d/samples/sample_stacking.cpp#L288) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Stacking/Cylinder Stack → same scene | [CylinderStack](../../../../box3d/samples/sample_stacking.cpp#L346) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Stacking/Dominoes → same scene | [Dominoes](../../../../box3d/samples/sample_stacking.cpp#L557) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Stacking/Double Domino → same scene | [DoubleDomino](../../../../box3d/samples/sample_stacking.cpp#L804) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Stacking/Edge Crossing → same scene | [EdgeCrossing](../../../../box3d/samples/sample_stacking.cpp#L939) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Stacking/Jenga Stack → same scene | [JengaStack](../../../../box3d/samples/sample_stacking.cpp#L491) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Stacking/Pyramid2D → same scene | [Pyramid2D](../../../../box3d/samples/sample_stacking.cpp#L849) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Stacking/Single Box → same scene | [SingleBox](../../../../box3d/samples/sample_stacking.cpp#L237) | floor-only reviewed | floor-only reviewed | floor-only reviewed | Static floor/body visible. CPU grid decoration missing in combined view remains PR09 rendering finding; GPU body sleep color differs, physical sleep qualification remains PR04. |
| Stacking/Sphere Stack → same scene | [SphereStack](../../../../box3d/samples/sample_stacking.cpp#L148) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Stacking/Wedge → same scene | [Wedge](../../../../box3d/samples/sample_stacking.cpp#L602) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| Tree/Benchmark → same scene | [TreeBenchmark](../../../../box3d/samples/sample_tree.cpp#L663) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| World/Far Mesh Drop → same scene | [FarMeshDrop](../../../../box3d/samples/sample_world.cpp#L308) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| World/Far Pyramid → same scene | [FarPyramid](../../../../box3d/samples/sample_world.cpp#L182) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| World/Far Ragdolls → same scene | [FarRagdolls](../../../../box3d/samples/sample_world.cpp#L245) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |
| World/Far Stack → same scene | [FarStack](../../../../box3d/samples/sample_world.cpp#L122) | UNREVIEWED | UNREVIEWED | UNREVIEWED | PR09 runtime review pending |

## Browser CPU catalog reconciliation

Every entry is listed, including factories, custom extras and geometry/manifold tools. Native mappings are family mappings, not a claim that browser geometry/defaults match native or that a native GPU browser port exists. Browser/WASM/WebGPU is outside the initial native release. A no-counterpart entry stays a named PR09 gap.

| Browser CPU entry | Native CPU/GPU scene family | Current status / disposition |
| --- | --- | --- |
| [bodies/spinning-book — Bodies / Spinning Book](../../../../demo/src/samples/bodies/spinning-book.ts) | Bodies/Spinning Book | UNREVIEWED. shared native scene available, runtime unqualified |
| [bodies/fixed-rotation — Bodies / Fixed Rotation](../../../../demo/src/samples/bodies/fixed-rotation.ts) | Bodies/Fixed Rotation | UNREVIEWED. shared native scene available, runtime unqualified |
| [bodies/lock-mixing — Bodies / Lock Mixing](../../../../demo/src/samples/bodies/lock-mixing.ts) | Bodies/Lock Mixing | UNREVIEWED. shared native scene available, runtime unqualified |
| [bodies/kinematic — Bodies / Kinematic](../../../../demo/src/samples/bodies/kinematic.ts) | Bodies/Kinematic | UNREVIEWED. shared native scene available, runtime unqualified |
| [bodies/offset-kinematic — Bodies / Offset Kinematic](../../../../demo/src/samples/bodies/offset-kinematic.ts) | Bodies/Offset Kinematic | UNREVIEWED. shared native scene available, runtime unqualified |
| [bodies/gyroscopic-torque — Bodies / Gyroscopic Torque](../../../../demo/src/samples/bodies/gyroscopic-torque.ts) | Bodies/Gyroscopic Torque | UNREVIEWED. shared native scene available, runtime unqualified |
| [bodies/gyroscopic-precession — Bodies / Gyroscopic Precession](../../../../demo/src/samples/bodies/gyroscopic-precession.ts) | Bodies/Gyroscopic Precession | UNREVIEWED. shared native scene available, runtime unqualified |
| [bodies/class-ring — Bodies / Class Ring](../../../../demo/src/samples/bodies/class-ring.ts) | Bodies/Class Ring | UNREVIEWED. shared native scene available, runtime unqualified |
| [bodies/body-type — Bodies / Body Type](../../../../demo/src/samples/bodies/body-type.ts) | Bodies/Body Type | UNREVIEWED. shared native scene available, runtime unqualified |
| [bodies/weeble — Bodies / Weeble](../../../../demo/src/samples/bodies/weeble.ts) | Bodies/Weeble | UNREVIEWED. shared native scene available, runtime unqualified |
| [bodies/disable — Bodies / Disable](../../../../demo/src/samples/bodies/disable.ts) | Bodies/Disable | UNREVIEWED. shared native scene available, runtime unqualified |
| [events/move — Events / Move](../../../../demo/src/samples/events/move.ts) | Events/Move | UNREVIEWED. shared native scene available, runtime unqualified |
| [events/sensor-visit — Events / Sensor Visit](../../../../demo/src/samples/events/sensor-visit.ts) | Events/Sensor Visit | UNREVIEWED. shared native scene available, runtime unqualified |
| [events/sensor-hits — Events / Sensor Hits](../../../../demo/src/samples/events/sensor-hits.ts) | Events/Sensor Hits | UNREVIEWED. shared native scene available, runtime unqualified |
| [events/hit — Events / Hit](../../../../demo/src/samples/events/hit.ts) | Events/Hit | UNREVIEWED. shared native scene available, runtime unqualified |
| [events/persistent-contact — Events / Persistent Contact](../../../../demo/src/samples/events/persistent-contact.ts) | Events/Persistent Contact | UNREVIEWED. shared native scene available, runtime unqualified |
| [events/joint — Events / Joint](../../../../demo/src/samples/events/joint.ts) | Events/Joint | UNREVIEWED. shared native scene available, runtime unqualified |
| [events/contact — Events / Contact](../../../../demo/src/samples/events/contact.ts) | Events/Contact | UNREVIEWED. shared native scene available, runtime unqualified |
| [geometry/box-hull — Geometry / Box Hull](../../../../demo/src/samples/geometry/box-hull.ts) | Geometry/Box Hull | UNREVIEWED. shared native scene available, runtime unqualified |
| [geometry/hull — Geometry / Hull](../../../../demo/src/samples/geometry/hull.ts) | Geometry/Hull | UNREVIEWED. shared native scene available, runtime unqualified |
| [geometry/hull-reduction — Geometry / Hull Reduction](../../../../demo/src/samples/geometry/hull-reduction.ts) | Geometry/Hull Reduction | UNREVIEWED. shared native scene available, runtime unqualified |
| [geometry/hull-transform — Geometry / Hull Transform](../../../../demo/src/samples/geometry/hull-transform.ts) | Geometry/Hull Transform | UNREVIEWED. shared native scene available, runtime unqualified |
| [geometry/capsule-mass — Geometry / Capsule Mass](../../../../demo/src/samples/geometry/capsule-mass.ts) | Geometry/Capsule Mass | UNREVIEWED. shared native scene available, runtime unqualified |
| [issues/hull-crash — Issues / Hull Crash](../../../../demo/src/samples/issues/hull-crash.ts) | Issues/Hull Crash | UNREVIEWED. shared native scene available, runtime unqualified |
| [issues/multiple-prismatic — Issues / Multiple Prismatic](../../../../demo/src/samples/issues/multiple-prismatic.ts) | Issues/Multiple Prismatic | UNREVIEWED. shared native scene available, runtime unqualified |
| [issues/crash — Issues / Crash](../../../../demo/src/samples/issues/crash.ts) | Issues/Crash | UNREVIEWED. shared native scene available, runtime unqualified |
| [issues/convex-jitter — Issues / Convex Jitter](../../../../demo/src/samples/issues/convex-jitter.ts) | Issues/Convex Jitter | UNREVIEWED. shared native scene available, runtime unqualified |
| [issues/restitution-overshoot — Issues / Restitution Overshoot](../../../../demo/src/samples/issues/restitution-overshoot.ts) | Issues/Restitution Overshoot | UNREVIEWED. shared native scene available, runtime unqualified |
| [issues/gmod-wheel-stack — Issues / GMod Wheel Stack](../../../../demo/src/samples/issues/gmod-wheel-stack.ts) | Issues/GMod Wheel Stack | UNREVIEWED. shared native scene available, runtime unqualified |
| [issues/slide-twist-off-center — Issues / Slide Twist Off Center Shape](../../../../demo/src/samples/issues/slide-twist-off-center.ts) | Issues/Slide Twist Off Center Shape | UNREVIEWED. shared native scene available, runtime unqualified |
| [robustness/high-mass-ratio-1 — Robustness / HighMassRatio1](../../../../demo/src/samples/robustness/high-mass-ratio-1.ts) | Robustness/HighMassRatio1 | UNREVIEWED. shared native scene available, runtime unqualified |
| [robustness/tiny-pyramid — Robustness / Tiny Pyramid](../../../../demo/src/samples/robustness/tiny-pyramid.ts) | Robustness/Tiny Pyramid | UNREVIEWED. shared native scene available, runtime unqualified |
| [robustness/overlap-recovery — Robustness / Overlap Recovery](../../../../demo/src/samples/robustness/overlap-recovery.ts) | Robustness/Overlap Recovery | UNREVIEWED. shared native scene available, runtime unqualified |
| [robustness/overflow-color-pile — Robustness / Overflow Color Pile](../../../../demo/src/samples/robustness/overflow-color-pile.ts) | Robustness/Overflow Color Pile | UNREVIEWED. shared native scene available, runtime unqualified |
| [joints/filter — Joints / Filter](../../../../demo/src/samples/joints/filter.ts) | Joints/Filter | UNREVIEWED. shared native scene available, runtime unqualified |
| [joints/motor-joint — Joints / Motor Joint](../../../../demo/src/samples/joints/motor-joint.ts) | Joints/Motor Joint | UNREVIEWED. shared native scene available, runtime unqualified |
| [joints/prismatic — Joints / Prismatic](../../../../demo/src/samples/joints/prismatic.ts) | Joints/Prismatic | UNREVIEWED. shared native scene available, runtime unqualified |
| [joints/revolute — Joints / Revolute](../../../../demo/src/samples/joints/revolute.ts) | Joints/Revolute | UNREVIEWED. shared native scene available, runtime unqualified |
| [joints/weld — Joints / Weld](../../../../demo/src/samples/joints/weld.ts) | Joints/Weld | UNREVIEWED. shared native scene available, runtime unqualified |
| [joints/top-down-friction — Joints / Top Down Friction](../../../../demo/src/samples/joints/top-down-friction.ts) | Joints/Top Down Friction | UNREVIEWED. shared native scene available, runtime unqualified |
| [joints/spherical — Joints / Spherical](../../../../demo/src/samples/joints/spherical.ts) | Joints/Spherical | UNREVIEWED. shared native scene available, runtime unqualified |
| [joints/ball-and-chain — Joints / Ball and Chain](../../../../demo/src/samples/joints/ball-and-chain.ts) | Joints/Ball and Chain | UNREVIEWED. shared native scene available, runtime unqualified |
| [joints/door — Joints / Door](../../../../demo/src/samples/joints/door.ts) | Joints/Door | UNREVIEWED. shared native scene available, runtime unqualified |
| [joints/bridge — Joints / Bridge](../../../../demo/src/samples/joints/bridge.ts) | Joints/Bridge | UNREVIEWED. shared native scene available, runtime unqualified |
| [world/far-stack — World / Far Stack](../../../../demo/src/samples/world/far-stack.ts) | World/Far Stack | UNREVIEWED. shared native scene available, runtime unqualified |
| [world/far-pyramid — World / Far Pyramid](../../../../demo/src/samples/world/far-pyramid.ts) | World/Far Pyramid | UNREVIEWED. shared native scene available, runtime unqualified |
| [world/far-ragdolls — World / Far Ragdolls](../../../../demo/src/samples/world/far-ragdolls.ts) | World/Far Ragdolls | UNREVIEWED. shared native scene available, runtime unqualified |
| [world/far-mesh-drop — World / Far Mesh Drop](../../../../demo/src/samples/world/far-mesh-drop.ts) | World/Far Mesh Drop | UNREVIEWED. shared native scene available, runtime unqualified |
| [compound/simple — Compound / Simple](../../../../demo/src/samples/compound/simple.ts) | Compound/Simple | UNREVIEWED. shared native scene available, runtime unqualified |
| [compound/spheres — Compound / Spheres](../../../../demo/src/samples/compound/spheres.ts) | Compound/Spheres | UNREVIEWED. shared native scene available, runtime unqualified |
| [compound/hulls — Compound / Hulls](../../../../demo/src/samples/compound/hulls.ts) | Compound/Hulls | UNREVIEWED. shared native scene available, runtime unqualified |
| [compound-material-dedup — Compound Material Dedup](../../../../demo/src/samples/compound/material-dedup.ts) | **NO ACTIVE COUNTERPART** | UNREVIEWED. browser-only custom scene; no native CPU/GPU viewer registration. Native port gap at PR09; browser/WASM outside initial native release |
| [compound/tile-floor — Compound / Tile Floor](../../../../demo/src/samples/compound/tile-floor.ts) | Compound/Tile Floor | UNREVIEWED. shared native scene available, runtime unqualified |
| [compound/mesh-tile — Compound / Mesh Tile](../../../../demo/src/samples/compound/mesh-tile.ts) | Compound/Mesh Tile | UNREVIEWED. shared native scene available, runtime unqualified |
| [compound/village — Compound / Village](../../../../demo/src/samples/compound/village.ts) | Compound/Village | UNREVIEWED. shared native scene available, runtime unqualified |
| [ragdoll/box — Ragdoll / Box](../../../../demo/src/samples/ragdoll/box.ts) | Ragdoll/Box | UNREVIEWED. shared native scene available, runtime unqualified |
| [ragdoll/mesh — Ragdoll / Mesh](../../../../demo/src/samples/ragdoll/mesh.ts) | Ragdoll/Mesh | UNREVIEWED. shared native scene available, runtime unqualified |
| [ragdoll/pile — Ragdoll / Pile](../../../../demo/src/samples/ragdoll/pile.ts) | Ragdoll/Pile | UNREVIEWED. shared native scene available, runtime unqualified |
| [ragdoll/incline — Ragdoll / Incline](../../../../demo/src/samples/ragdoll/incline.ts) | Ragdoll/Incline | UNREVIEWED. shared native scene available, runtime unqualified |
| [ragdoll/pose — Ragdoll / Pose](../../../../demo/src/samples/ragdoll/pose.ts) | **NO ACTIVE COUNTERPART** | UNREVIEWED. Upstream registration disabled by #if0; active browser port has no active native CPU/GPU counterpart. PR09 native availability gap. |
| [determinism/falling-ragdolls — Determinism / Falling Ragdolls](../../../../demo/src/samples/determinism/falling-ragdolls.ts) | Determinism/Falling Ragdolls | UNREVIEWED. shared native scene available, runtime unqualified |
| [determinism/wave-pile — Determinism / Wave Pile](../../../../demo/src/samples/determinism/wave-pile.ts) | Determinism/Wave Pile | UNREVIEWED. shared native scene available, runtime unqualified |
| [determinism/query-spawn — Determinism / Query Spawn](../../../../demo/src/samples/determinism/query-spawn.ts) | Determinism/Query Spawn | UNREVIEWED. shared native scene available, runtime unqualified |
| [determinism/mesh-drop — Determinism / Mesh Drop](../../../../demo/src/samples/determinism/mesh-drop.ts) | Determinism/Mesh Drop | UNREVIEWED. shared native scene available, runtime unqualified |
| [continuous/stall — Continuous / Stall](../../../../demo/src/samples/continuous/stall.ts) | Continuous/Stall | UNREVIEWED. shared native scene available, runtime unqualified |
| [benchmark/rain — Benchmark / Rain](../../../../demo/src/samples/benchmark/rain.ts) | Benchmark/Rain | UNREVIEWED. shared native scene available, runtime unqualified |
| [benchmark/candy-cups — Benchmark / Candy Cups](../../../../demo/src/samples/benchmark/candy-cups.ts) | Benchmark/Candy Cups | UNREVIEWED. shared native scene available, runtime unqualified |
| [benchmark/explosion — Benchmark / Explosion](../../../../demo/src/samples/benchmark/explosion.ts) | Benchmark/Explosion | UNREVIEWED. shared native scene available, runtime unqualified |
| [benchmark/destruction — Benchmark / Destruction](../../../../demo/src/samples/benchmark/destruction.ts) | Benchmark/Destruction | UNREVIEWED. shared native scene available, runtime unqualified |
| [joints/motion-locks — Joints / Motion Locks](../../../../demo/src/samples/joints/motion-locks.ts) | Joints/Motion Locks | UNREVIEWED. shared native scene available, runtime unqualified |
| [joints/distance-joint — Joints / Distance Joint](../../../../demo/src/samples/joints/distance-joint.ts) | Joints/Distance Joint | UNREVIEWED. shared native scene available, runtime unqualified |
| [joints/parallel-spring — Joints / Parallel Spring](../../../../demo/src/samples/joints/parallel-spring.ts) | Joints/Parallel Spring | UNREVIEWED. shared native scene available, runtime unqualified |
| [joints/wheel — Joints / Wheel](../../../../demo/src/samples/joints/wheel.ts) | Joints/Wheel | UNREVIEWED. shared native scene available, runtime unqualified |
| [single-box — Stacking / Single Box](../../../../demo/src/samples/single/box.ts) | Stacking/Single Box | UNREVIEWED. shared native scene available, runtime unqualified |
| [cylinder — Stacking / Cylinder](../../../../demo/src/samples/cylinder.ts) | Stacking/Cylinder | UNREVIEWED. shared native scene available, runtime unqualified |
| [cylinder-stack — Stacking / Cylinder Stack](../../../../demo/src/samples/cylinder-stack.ts) | Stacking/Cylinder Stack | UNREVIEWED. shared native scene available, runtime unqualified |
| [wedge — Stacking / Wedge](../../../../demo/src/samples/wedge.ts) | Stacking/Wedge | UNREVIEWED. shared native scene available, runtime unqualified |
| [arch — Stacking / Arch](../../../../demo/src/samples/arch.ts) | Stacking/Arch | UNREVIEWED. shared native scene available, runtime unqualified |
| [double-domino — Stacking / Double Domino](../../../../demo/src/samples/double-domino.ts) | Stacking/Double Domino | UNREVIEWED. shared native scene available, runtime unqualified |
| [card-house — Stacking / Card House](../../../../demo/src/samples/card-house.ts) | Stacking/Card House | UNREVIEWED. shared native scene available, runtime unqualified |
| [rolling-resistance — Shapes / Rolling Resistance](../../../../demo/src/samples/rolling-resistance.ts) | Shapes/Rolling Resistance | UNREVIEWED. shared native scene available, runtime unqualified |
| [restitution — Shapes / Restitution](../../../../demo/src/samples/restitution.ts) | Shapes/Restitution | UNREVIEWED. shared native scene available, runtime unqualified |
| [isotropic-friction — Shapes / Isotropic Friction](../../../../demo/src/samples/isotropic-friction.ts) | Shapes/Isotropic Friction | UNREVIEWED. shared native scene available, runtime unqualified |
| [sphere-stack — Stacking / Sphere Stack](../../../../demo/src/samples/sphere/stack.ts) | Stacking/Sphere Stack | UNREVIEWED. shared native scene available, runtime unqualified |
| [box-stack — Stacking / Box Stack](../../../../demo/src/samples/box/stack.ts) | Stacking/Box Stack | UNREVIEWED. shared native scene available, runtime unqualified |
| [shapes/inclined-plane — Shapes / Inclined Plane](../../../../demo/src/samples/shapes/inclined-plane.ts) | Shapes/Inclined Plane | UNREVIEWED. shared native scene available, runtime unqualified |
| [dominoes — Stacking / Dominoes](../../../../demo/src/samples/dominoes.ts) | Stacking/Dominoes | UNREVIEWED. Native family mapping; browser Dominoes variation is intentional under AGENTS.md. Both remain unreviewed here. |
| [card-house-thick — Stacking / Card House Thick](../../../../demo/src/samples/card/house-thick.ts) | **NO ACTIVE COUNTERPART** | UNREVIEWED. Upstream registration disabled by #if0; active browser port has no active native CPU/GPU counterpart. PR09 native availability gap. |
| [jenga-stack — Stacking / Jenga Stack](../../../../demo/src/samples/jenga/stack.ts) | Stacking/Jenga Stack | UNREVIEWED. shared native scene available, runtime unqualified |
| [pyramid2d — Stacking / Pyramid2D](../../../../demo/src/samples/pyramid2d.ts) | Stacking/Pyramid2D | UNREVIEWED. shared native scene available, runtime unqualified |
| [edge-crossing — Stacking / Edge Crossing](../../../../demo/src/samples/edge-crossing.ts) | Stacking/Edge Crossing | UNREVIEWED. shared native scene available, runtime unqualified |
| [capsule-stack — Stacking / Capsule Stack](../../../../demo/src/samples/capsule/stack.ts) | Stacking/Capsule Stack | UNREVIEWED. shared native scene available, runtime unqualified |
| [dominoes-2x — Bench / Dominoes 2×](../../../../demo/src/samples/dominoes.ts) | Stacking/Dominoes | UNREVIEWED. Browser doubled-count variant maps to Dominoes family, no exact registered native variant. PR09 gap; geometry intentionally differs. |
| [washer — Benchmark / Washer](../../../../demo/src/samples/washer.ts) | Benchmark/Washer | UNREVIEWED. shared native scene available, runtime unqualified |
| [shapes/high-resistance — Shapes / High Resistance](../../../../demo/src/samples/shapes/high-resistance.ts) | Shapes/High Resistance | UNREVIEWED. shared native scene available, runtime unqualified |
| [shapes/slide-twist — Shapes / Slide Twist](../../../../demo/src/samples/shapes/slide-twist.ts) | Shapes/Slide Twist | UNREVIEWED. shared native scene available, runtime unqualified |
| [shapes/conveyor-belt — Shapes / Conveyor Belt](../../../../demo/src/samples/shapes/conveyor-belt.ts) | Shapes/Conveyor Belt | UNREVIEWED. shared native scene available, runtime unqualified |
| [shapes/conveyor-mesh — Shapes / Conveyor Mesh](../../../../demo/src/samples/shapes/conveyor-mesh.ts) | Shapes/Conveyor Mesh | UNREVIEWED. shared native scene available, runtime unqualified |
| [shapes/wind-drop — Shapes / Wind Drop](../../../../demo/src/samples/shapes/wind-drop.ts) | Shapes/Wind Drop | UNREVIEWED. shared native scene available, runtime unqualified |
| [shapes/wind — Shapes / Wind](../../../../demo/src/samples/shapes/wind.ts) | Shapes/Wind | UNREVIEWED. shared native scene available, runtime unqualified |
| [shapes/wind-flap — Shapes / Wind Flap](../../../../demo/src/samples/shapes/wind-flap.ts) | Shapes/Wind Flap | UNREVIEWED. shared native scene available, runtime unqualified |
| [shapes/static-invoke — Shapes / Static Invoke](../../../../demo/src/samples/shapes/static-invoke.ts) | Shapes/Static Invoke | UNREVIEWED. shared native scene available, runtime unqualified |
| [mesh/grid — Mesh / Grid](../../../../demo/src/samples/mesh/grid.ts) | Mesh/Grid | UNREVIEWED. shared native scene available, runtime unqualified |
| [mesh/big-box — Mesh / Big Box](../../../../demo/src/samples/mesh/big-box.ts) | Mesh/Big Box | UNREVIEWED. shared native scene available, runtime unqualified |
| [mesh/box — Mesh / Box](../../../../demo/src/samples/mesh/box.ts) | Mesh/Box | UNREVIEWED. shared native scene available, runtime unqualified |
| [mesh/height-field — Mesh / Height Field](../../../../demo/src/samples/mesh/height-field.ts) | Mesh/Height Field | UNREVIEWED. shared native scene available, runtime unqualified |
| [mesh/hollow-box — Mesh / Hollow Box](../../../../demo/src/samples/mesh/hollow-box.ts) | Mesh/Hollow Box | UNREVIEWED. shared native scene available, runtime unqualified |
| [mesh/reflection — Mesh / Reflection](../../../../demo/src/samples/mesh/reflection.ts) | Mesh/Reflection | UNREVIEWED. shared native scene available, runtime unqualified |
| [mesh/voxel — Mesh / Voxel](../../../../demo/src/samples/mesh/voxel.ts) | Mesh/Voxel | UNREVIEWED. shared native scene available, runtime unqualified |
| [mesh/viewer — Mesh / Viewer](../../../../demo/src/samples/mesh/viewer.ts) | Mesh/Viewer, Replay/Viewer | UNREVIEWED. shared native scene available, runtime unqualified |
| [mesh/creation-benchmark — Mesh / Creation Benchmark](../../../../demo/src/samples/mesh/creation-benchmark.ts) | Mesh/Creation Benchmark | UNREVIEWED. shared native scene available, runtime unqualified |
| [collision/ray-curtain — Collision / Ray Curtain](../../../../demo/src/samples/collision/ray-curtain.ts) | Collision/Ray Curtain | UNREVIEWED. shared native scene available, runtime unqualified |
| [collision/capsule-cast-ray — Collision / Capsule Cast Ray](../../../../demo/src/samples/collision/capsule-cast-ray.ts) | Collision/Capsule Cast Ray | UNREVIEWED. shared native scene available, runtime unqualified |
| [collision/cast-world — Collision / Cast World](../../../../demo/src/samples/collision/cast-world.ts) | Collision/Cast World | UNREVIEWED. shared native scene available, runtime unqualified |
| [character/capsule-plane — Character / CapsulePlane](../../../../demo/src/samples/character/capsule-plane.ts) | Character/CapsulePlane | UNREVIEWED. shared native scene available, runtime unqualified |
| [character/mover-overlap — Character / MoverOverlap](../../../../demo/src/samples/character/mover-overlap.ts) | Character/MoverOverlap | UNREVIEWED. shared native scene available, runtime unqualified |
| [character/mover — Character / Mover](../../../../demo/src/samples/character/mover.ts) | Character/Mover | UNREVIEWED. shared native scene available, runtime unqualified |
| [character/rigid-body — Character / Rigid Body](../../../../demo/src/samples/character/rigid-body.ts) | Character/Rigid Body | UNREVIEWED. shared native scene available, runtime unqualified |
| [collision/long-ray-cast — Collision / Long Ray Cast](../../../../demo/src/samples/collision/long-ray-cast.ts) | Collision/Long Ray Cast | UNREVIEWED. shared native scene available, runtime unqualified |
| [continuous/thin-wall — Continuous / Thin Wall](../../../../demo/src/samples/continuous/thin-wall.ts) | Continuous/Thin Wall | UNREVIEWED. shared native scene available, runtime unqualified |
| [continuous/bounce-house — Continuous / Bounce House](../../../../demo/src/samples/continuous/bounce-house.ts) | Continuous/Bounce House | UNREVIEWED. shared native scene available, runtime unqualified |
| [continuous/spinning-stick — Continuous / Spinning Stick](../../../../demo/src/samples/continuous/spinning-stick.ts) | Continuous/Spinning Stick | UNREVIEWED. shared native scene available, runtime unqualified |
| [continuous/is-fast — Continuous / Is Fast](../../../../demo/src/samples/continuous/is-fast.ts) | Continuous/Is Fast | UNREVIEWED. shared native scene available, runtime unqualified |
| [continuous/mesh-drop — Continuous / Mesh Drop](../../../../demo/src/samples/continuous/mesh-drop.ts) | Continuous/Mesh Drop | UNREVIEWED. shared native scene available, runtime unqualified |
| [continuous/hump-mesh — Continuous / Hump Mesh](../../../../demo/src/samples/continuous/hump-mesh.ts) | Continuous/Hump Mesh | UNREVIEWED. shared native scene available, runtime unqualified |
| [continuous/needle-mesh — Continuous / Needle Mesh](../../../../demo/src/samples/continuous/needle-mesh.ts) | Continuous/Needle Mesh | UNREVIEWED. shared native scene available, runtime unqualified |
| [continuous/bullet-vs-stack — Continuous / Bullet vs Stack](../../../../demo/src/samples/continuous/bullet-vs-stack.ts) | Continuous/Bullet vs Stack | UNREVIEWED. shared native scene available, runtime unqualified |
| [benchmark/large-pyramid — Benchmark / Large Pyramid](../../../../demo/src/samples/benchmark/large-pyramid.ts) | Benchmark/Large Pyramid | UNREVIEWED. shared native scene available, runtime unqualified |
| [benchmark/wide-pyramid — Benchmark / Wide Pyramid](../../../../demo/src/samples/benchmark/wide-pyramid.ts) | Benchmark/Wide Pyramid | UNREVIEWED. shared native scene available, runtime unqualified |
| [benchmark/falling-boxes — Benchmark / Falling Boxes](../../../../demo/src/samples/benchmark/falling-boxes.ts) | Benchmark/Falling Boxes | UNREVIEWED. shared native scene available, runtime unqualified |
| [benchmark/many-pyramids — Benchmark / Many Pyramids](../../../../demo/src/samples/benchmark/many-pyramids.ts) | Benchmark/Many Pyramids | UNREVIEWED. shared native scene available, runtime unqualified |
| [benchmark/joint-grid — Benchmark / Joint Grid](../../../../demo/src/samples/benchmark/joint-grid.ts) | Benchmark/Joint Grid | UNREVIEWED. shared native scene available, runtime unqualified |
| [benchmark/junkyard — Benchmark / Junkyard](../../../../demo/src/samples/benchmark/junkyard.ts) | Benchmark/Junkyard | UNREVIEWED. shared native scene available, runtime unqualified |
| [benchmark/falling-trees — Benchmark / Falling Trees](../../../../demo/src/samples/benchmark/falling-trees.ts) | Benchmark/Falling Trees | UNREVIEWED. shared native scene available, runtime unqualified |
| [benchmark/chains — Benchmark / Chains](../../../../demo/src/samples/benchmark/chains.ts) | Benchmark/Chains | UNREVIEWED. shared native scene available, runtime unqualified |
| [benchmark/convex-pile — Benchmark / Convex Pile](../../../../demo/src/samples/benchmark/convex-pile.ts) | Benchmark/Convex Pile | UNREVIEWED. shared native scene available, runtime unqualified |
| [benchmark/large-world — Benchmark / Large World](../../../../demo/src/samples/benchmark/large-world.ts) | Benchmark/Large World | UNREVIEWED. shared native scene available, runtime unqualified |
| [benchmark/height-field — Benchmark / Height Field](../../../../demo/src/samples/benchmark/height-field-benchmark.ts) | Benchmark/Height Field | UNREVIEWED. shared native scene available, runtime unqualified |
| [bodies/cast — Bodies / Cast](../../../../demo/src/samples/bodies/cast.ts) | Bodies/Cast | UNREVIEWED. shared native scene available, runtime unqualified |
| [collision/shape-cast — Collision / Shape Cast](../../../../demo/src/samples/collision/shape-cast.ts) | Collision/Shape Cast | UNREVIEWED. shared native scene available, runtime unqualified |
| [collision/overlap-world — Collision / Overlap World](../../../../demo/src/samples/collision/overlap-world.ts) | Collision/Overlap World | UNREVIEWED. shared native scene available, runtime unqualified |
| [collision/initial-overlap — Collision / Initial Overlap](../../../../demo/src/samples/collision/initial-overlap.ts) | Collision/Initial Overlap | UNREVIEWED. shared native scene available, runtime unqualified |
| [collision/mesh-scale — Collision / Mesh Scale](../../../../demo/src/samples/collision/mesh-scale.ts) | Collision/Mesh Scale | UNREVIEWED. shared native scene available, runtime unqualified |
| [joints/driving — Joints / Driving](../../../../demo/src/samples/joints/driving.ts) | Joints/Driving | UNREVIEWED. shared native scene available, runtime unqualified |
| [joints/gear-lift — Joints / Gear Lift](../../../../demo/src/samples/joints/gear-lift.ts) | Joints/Gear Lift | UNREVIEWED. shared native scene available, runtime unqualified |
| [issues/capsule-mesh — Issues / Capsule Mesh](../../../../demo/src/samples/issues/capsule-mesh.ts) | Issues/Capsule Mesh | UNREVIEWED. shared native scene available, runtime unqualified |
| [issues/sbox-mover — Issues / s&box mover](../../../../demo/src/samples/issues/sbox-mover.ts) | Issues/s&box mover | UNREVIEWED. shared native scene available, runtime unqualified |
| [issues/sbox-ghost-collisions — Issues / s&box Ghost Collisions](../../../../demo/src/samples/issues/sbox-ghost-collisions.ts) | Issues/s&box Ghost Collisions | UNREVIEWED. shared native scene available, runtime unqualified |
| [manifold/sphere-vs-sphere — Manifold / Sphere vs Sphere](../../../../demo/src/samples/manifold/sphere-vs-sphere.ts) | Manifold/Sphere vs Sphere | UNREVIEWED. shared native scene available, runtime unqualified |
| [manifold/capsule-vs-sphere — Manifold / Capsule vs Sphere](../../../../demo/src/samples/manifold/capsule-vs-sphere.ts) | Manifold/Capsule vs Sphere | UNREVIEWED. shared native scene available, runtime unqualified |
| [manifold/hull-vs-sphere — Manifold / Hull vs Sphere](../../../../demo/src/samples/manifold/hull-vs-sphere.ts) | Manifold/Hull vs Sphere | UNREVIEWED. shared native scene available, runtime unqualified |
| [manifold/triangle-vs-sphere — Manifold / Triangle vs Sphere](../../../../demo/src/samples/manifold/triangle-vs-sphere.ts) | Manifold/Triangle vs Sphere | UNREVIEWED. shared native scene available, runtime unqualified |
| [manifold/capsule-vs-capsule — Manifold / Capsule vs Capsule](../../../../demo/src/samples/manifold/capsule-vs-capsule.ts) | Manifold/Capsule vs Capsule | UNREVIEWED. shared native scene available, runtime unqualified |
| [manifold/capsule-vs-hull — Manifold / Capsule vs Hull](../../../../demo/src/samples/manifold/capsule-vs-hull.ts) | Manifold/Capsule vs Hull | UNREVIEWED. shared native scene available, runtime unqualified |
| [manifold/triangle-vs-capsule — Manifold / Triangle vs Capsule](../../../../demo/src/samples/manifold/triangle-vs-capsule.ts) | Manifold/Triangle vs Capsule | UNREVIEWED. shared native scene available, runtime unqualified |
| [manifold/hull-vs-hull — Manifold / Hull vs Hull](../../../../demo/src/samples/manifold/hull-vs-hull.ts) | Manifold/Hull vs Hull | UNREVIEWED. shared native scene available, runtime unqualified |
| [manifold/triangle-vs-hull — Manifold / Triangle vs Hull](../../../../demo/src/samples/manifold/triangle-vs-hull.ts) | Manifold/Triangle vs Hull | UNREVIEWED. shared native scene available, runtime unqualified |
| [collision/shape-cast-debug — Collision / Shape Cast Debug](../../../../demo/src/samples/collision/shape-cast-debug.ts) | Collision/Shape Cast Debug | UNREVIEWED. shared native scene available, runtime unqualified |
| [collision/distance-debug — Collision / Distance Debug](../../../../demo/src/samples/collision/distance-debug.ts) | Collision/Distance Debug | UNREVIEWED. shared native scene available, runtime unqualified |
| [collision/shape-distance — Collision / Shape Distance](../../../../demo/src/samples/collision/shape-distance.ts) | Collision/Shape Distance | UNREVIEWED. shared native scene available, runtime unqualified |
| [collision/time-of-impact — Collision / Time of Impact](../../../../demo/src/samples/collision/time-of-impact.ts) | Collision/Time of Impact | UNREVIEWED. shared native scene available, runtime unqualified |
| [benchmark/hull — Benchmark / Hull](../../../../demo/src/samples/benchmark/hull.ts) | Benchmark/Hull | UNREVIEWED. shared native scene available, runtime unqualified |
| [benchmark/sensor — Benchmark / Sensor](../../../../demo/src/samples/benchmark/sensor.ts) | Benchmark/Sensor | UNREVIEWED. shared native scene available, runtime unqualified |
| [tree/benchmark — Tree / Benchmark](../../../../demo/src/samples/tree/benchmark.ts) | Tree/Benchmark | UNREVIEWED. shared native scene available, runtime unqualified |
| [extra/object-asserts-bench — Extra / Object Asserts Bench](../../../../demo/src/samples/object-asserts-bench.ts) | **NO ACTIVE COUNTERPART** | UNREVIEWED. browser-only custom scene; no native CPU/GPU viewer registration. Native port gap at PR09; browser/WASM outside initial native release |
| [extra/forces — Extra / Forces](../../../../demo/src/samples/extra/forces.ts) | **NO ACTIVE COUNTERPART** | UNREVIEWED. browser-only custom scene; no native CPU/GPU viewer registration. Native port gap at PR09; browser/WASM outside initial native release |
| [extra/world-knobs — Extra / World Knobs](../../../../demo/src/samples/extra/world-knobs.ts) | **NO ACTIVE COUNTERPART** | UNREVIEWED. browser-only custom scene; no native CPU/GPU viewer registration. Native port gap at PR09; browser/WASM outside initial native release |
| [extra/cone-mass — Extra / Cone Mass](../../../../demo/src/samples/extra/cone-mass.ts) | **NO ACTIVE COUNTERPART** | UNREVIEWED. browser-only custom scene; no native CPU/GPU viewer registration. Native port gap at PR09; browser/WASM outside initial native release |
| [extra/event-buffer — Extra / Event Buffer](../../../../demo/src/samples/extra/event-buffer.ts) | **NO ACTIVE COUNTERPART** | UNREVIEWED. browser-only custom scene; no native CPU/GPU viewer registration. Native port gap at PR09; browser/WASM outside initial native release |
| [extra/one-way — Extra / One-Way Platforms](../../../../demo/src/samples/extra/one-way.ts) | **NO ACTIVE COUNTERPART** | UNREVIEWED. browser-only custom scene; no native CPU/GPU viewer registration. Native port gap at PR09; browser/WASM outside initial native release |
| [extra/sensor-filter — Extra / Sensor Filter](../../../../demo/src/samples/extra/sensor-filter.ts) | **NO ACTIVE COUNTERPART** | UNREVIEWED. browser-only custom scene; no native CPU/GPU viewer registration. Native port gap at PR09; browser/WASM outside initial native release |
| [extra/clone-hull — Extra / Clone Hull](../../../../demo/src/samples/extra/clone-hull.ts) | **NO ACTIVE COUNTERPART** | UNREVIEWED. browser-only custom scene; no native CPU/GPU viewer registration. Native port gap at PR09; browser/WASM outside initial native release |
| [extra/parallel-anchors — Extra / Parallel Anchors](../../../../demo/src/samples/extra/parallel-anchors.ts) | **NO ACTIVE COUNTERPART** | UNREVIEWED. browser-only custom scene; no native CPU/GPU viewer registration. Native port gap at PR09; browser/WASM outside initial native release |
| [extra/slot-exhaustion — Extra / Slot Exhaustion](../../../../demo/src/samples/extra/slot-exhaustion.ts) | **NO ACTIVE COUNTERPART** | UNREVIEWED. browser-only custom scene; no native CPU/GPU viewer registration. Native port gap at PR09; browser/WASM outside initial native release |

## CPU oracle / Rust demo fixtures

These 20 independent CPU fixtures are also inventoried; they do not define the full native sample set. Their Rust demo counterparts are available but UNREVIEWED in this milestone.

| CPU fixture | GPU counterpart | Status |
| --- | --- | --- |
| oracle/single-box | Rust demo --scene single-box | UNREVIEWED |
| oracle/box-stack | Rust demo --scene box-stack | UNREVIEWED |
| oracle/sphere-stack | Rust demo --scene sphere-stack | UNREVIEWED |
| oracle/capsule-stack | Rust demo --scene capsule-stack | UNREVIEWED |
| oracle/revolute | Rust demo --scene revolute | UNREVIEWED |
| oracle/weld | Rust demo --scene weld | UNREVIEWED |
| oracle/anchored-mechanisms | Rust demo --scene anchored-mechanisms | UNREVIEWED |
| oracle/joint-chain | Rust demo --scene joint-chain | UNREVIEWED |
| oracle/stack | Rust demo --scene stack | UNREVIEWED |
| oracle/pyramid | Rust demo --scene pyramid | UNREVIEWED |
| oracle/bounce | Rust demo --scene bounce | UNREVIEWED |
| oracle/mixed | Rust demo --scene mixed | UNREVIEWED |
| oracle/spinner | Rust demo --scene spinner | UNREVIEWED |
| oracle/ramp | Rust demo --scene ramp | UNREVIEWED |
| oracle/spheres | Rust demo --scene spheres | UNREVIEWED |
| oracle/dominoes | Rust demo --scene dominoes | UNREVIEWED |
| oracle/high-resistance | Rust demo --scene high-resistance | UNREVIEWED |
| oracle/mixed-stacks | Rust demo --scene mixed-stacks | UNREVIEWED |
| oracle/falling-cubes | Rust demo --scene falling-cubes | UNREVIEWED |
| oracle/mixed-topology | Rust demo --scene mixed-topology | UNREVIEWED |
