# WASM API Surface

This tracks what `@firtoz/box3d-wasm` exposes to JavaScript/TypeScript today and what still needs binding work. Update this file whenever a new `b3w*` C bridge function and matching TypeScript wrapper are added.

Legend:

- `[x]` exposed through `packages/box3d-wasm/cmake/*` and `packages/box3d-wasm/src/index.ts`
- `[~]` partially exposed or exposed only as a convenience/special-case wrapper
- `[ ]` not exposed yet
- `demo-only` means helper functionality exists for the demo, not as a general upstream Box3D C API binding

Source files:

- C bridge: `packages/box3d-wasm/cmake/box3d_web_*.c`
- TypeScript wrapper: `packages/box3d-wasm/src/index.ts`
- Upstream headers: `box3d/include/box3d/*.h`
- Sample-level binding needs: `docs/SAMPLES.md`

## Current Shape

- Approximate JS-callable surface: ~75 TypeScript methods
- Current binding style: manual `b3w*` C bridge functions plus TypeScript wrapper classes
- Current focus: sample-driven API growth rather than a full 1:1 upstream C API mirror
- Threading: Emscripten pthreads enabled in WASM build, with world worker-count controls exposed
- Usage guide: see [`TYPESCRIPT_API.md`](./TYPESCRIPT_API.md) for public TypeScript examples and conventions

## Maintenance Checklist

When adding an API binding:

- [ ] Add C bridge function in `packages/box3d-wasm/cmake/box3d_web_*.c`
- [ ] Add TypeScript `cwrap` signature and wrapper in `packages/box3d-wasm/src/index.ts`
- [ ] Add or update a demo/sample using the API, if applicable
- [ ] Update this document from `[ ]` or `[~]` to `[x]`
- [ ] Update `docs/SAMPLES.md` if the API unblocks sample ports
- [ ] Rebuild release WASM if the compiled output changes
- [ ] Check gzipped WASM size and update `docs/OTHER_PROJECTS.md` if size changes materially

## World

- [x] Create world: `b3CreateWorld` via `createWorld` (gravity, worker count, optional `b3Capacity` hint)
- [x] Bridge slot limits: `b3wGetSlotLimits` via `runtime.limits`
- [x] Bridge slot usage: `b3wGetSlotUsage` via `runtime.getSlotUsage()`
- [x] Destroy world: `b3DestroyWorld` via `destroyWorld` / `PhysicsWorld.destroy`
- [x] Step world: `b3World_Step` via `step`
- [x] Get counters: `b3World_GetCounters` via `getWorldCounters`
- [x] Get profile: `b3World_GetProfile` via `getWorldProfile`
- [x] Get awake body count: `b3World_GetAwakeBodyCount`
- [x] Set/get worker count: `b3World_SetWorkerCount`, `b3World_GetWorkerCount`
- [x] Threading support check: Emscripten threading support via `checkThreadingSupport`
- [x] Enable sleeping: `b3World_EnableSleeping`
- [x] Enable continuous collision: `b3World_EnableContinuous`
- [x] Enable warm starting: `b3World_EnableWarmStarting`
- [x] Set contact tuning: `b3World_SetContactTuning`
- [x] Set contact recycle distance: `b3World_SetContactRecycleDistance`
- [ ] Get/set gravity after creation: `b3World_SetGravity`, `b3World_GetGravity`
- [ ] World bounds: `b3World_GetBounds`
- [ ] Restitution threshold: `b3World_SetRestitutionThreshold`, `b3World_GetRestitutionThreshold`
- [ ] Hit event threshold: `b3World_SetHitEventThreshold`, `b3World_GetHitEventThreshold`
- [ ] Maximum linear speed: `b3World_SetMaximumLinearSpeed`, `b3World_GetMaximumLinearSpeed`
- [ ] Contact recycle distance getter: `b3World_GetContactRecycleDistance`
- [ ] Static tree rebuild: `b3World_RebuildStaticTree`
- [ ] Speculative collision toggle: `b3World_EnableSpeculative`
- [ ] Max capacity: `b3World_GetMaxCapacity`
- [ ] Shape bounds dump/debug helpers

## Bodies

- [x] Create body: `b3CreateBody`
- [x] Destroy body: `b3DestroyBody`
- [x] Set body transform: `b3Body_SetTransform`
- [x] Read body transform: `b3Body_GetPosition` + `b3Body_GetRotation` via `readBodyTransform`
- [x] Set linear velocity: `b3Body_SetLinearVelocity`
- [x] Set angular velocity: `b3Body_SetAngularVelocity`
- [x] Apply linear impulse: `b3Body_ApplyLinearImpulse`
- [x] Apply linear impulse to center: `b3Body_ApplyLinearImpulseToCenter`
- [x] Set awake: `b3Body_SetAwake`
- [x] Is awake: `b3Body_IsAwake`
- [x] Set linear/angular damping: `b3Body_SetLinearDamping`, `b3Body_SetAngularDamping`
- [x] Get body type: `b3Body_GetType`
- [x] Set body type: `b3Body_SetType`
- [x] Set body name: `b3Body_SetName`
- [x] Set gravity scale: `b3Body_SetGravityScale`
- [x] Set sleep threshold: `b3Body_SetSleepThreshold`
- [x] Enable sleep: `b3Body_EnableSleep`
- [x] Set bullet: `b3Body_SetBullet`
- [x] Enable contact recycling: `b3Body_EnableContactRecycling`
- [x] Enable hit events: `b3Body_EnableHitEvents`
- [x] Set motion locks: `b3Body_SetMotionLocks`
- [x] Set mass data: `b3Body_SetMassData`
- [x] Get mass data: `b3Body_GetMassData`
- [x] Apply mass from shapes: `b3Body_ApplyMassFromShapes`
- [x] Set target transform: `b3Body_SetTargetTransform`
- [x] Get local point: `b3Body_GetLocalPoint`
- [~] Get debug color: demo helper, not a general Box3D public C API item
- [~] Batch body transform read: demo helper for render sync
- [ ] Body validity: `b3Body_IsValid`
- [ ] Get position: `b3Body_GetPosition` as direct wrapper
- [ ] Get rotation: `b3Body_GetRotation` as direct wrapper
- [x] Get linear velocity: `b3Body_GetLinearVelocity`
- [x] Get angular velocity: `b3Body_GetAngularVelocity`
- [ ] Apply force: `b3Body_ApplyForce`
- [ ] Apply force to center: `b3Body_ApplyForceToCenter`
- [ ] Apply torque: `b3Body_ApplyTorque`
- [ ] Apply angular impulse: `b3Body_ApplyAngularImpulse`
- [x] Get mass: `b3Body_GetMass`
- [ ] Get inverse mass: `b3Body_GetInverseMass`
- [x] Get world center of mass: `b3Body_GetWorldCenter`
- [x] Get local rotational inertia: `b3Body_GetLocalRotationalInertia`
- [ ] Get world inverse rotational inertia
- [ ] Get/set linear damping as separate getter/setter pair
- [ ] Get/set angular damping as separate getter/setter pair
- [ ] Get gravity scale
- [ ] Get sleep threshold
- [ ] Is sleep enabled
- [x] Enable/disable body: `b3Body_Enable`, `b3Body_Disable`, `b3Body_IsEnabled`
- [ ] Is bullet
- [ ] Get motion locks
- [ ] Is contact recycling enabled
- [ ] Local/world vector transforms
- [x] Local/world point velocity: `b3Body_GetLocalPointVelocity`, `b3Body_GetWorldPointVelocity`
- [ ] Compute body AABB
- [x] Get body shapes: `b3Body_GetShapes`
- [ ] Get body joints: `b3Body_GetJoints`
- [ ] Get body world: `b3Body_GetWorld`
- [ ] Get body name

## Shapes

- [x] Create convenience box body + hull shape: `createBox`
- [x] Create convenience sphere body + shape: `createSphere`
- [x] Create hull shape from box half-widths: `b3CreateHullShape` with `b3MakeBoxHull`
- [x] Create transformed hull shape: `b3CreateTransformedHullShape`
- [x] Create sphere shape: `b3CreateSphereShape`
- [x] Create capsule shape: `b3CreateCapsuleShape`
- [x] Create shape from existing hull: `b3CreateHullShape`
- [x] Create compound shape: `b3CreateCompoundShape`
- [x] Set density: `b3Shape_SetDensity`
- [x] Set friction: `b3Shape_SetFriction`
- [x] Set restitution: `b3Shape_SetRestitution`
- [x] Set surface material: `b3Shape_SetSurfaceMaterial` (with `tangentVelocity` support)
- [x] Set filter: `b3Shape_SetFilter`
- [x] Get shape body handle: `b3Shape_GetBody`
- [x] Enable sensor events: `b3Shape_EnableSensorEvents`
- [x] Enable contact events: `b3Shape_EnableContactEvents`
- [x] Enable pre-solve events: `b3Shape_EnablePreSolveEvents`
- [x] Enable hit events: `b3Shape_EnableHitEvents`
- [x] Set sphere geometry: `b3Shape_SetSphere`
- [x] Set capsule geometry: `b3Shape_SetCapsule`
- [x] Apply wind: `b3Shape_ApplyWind`
- [ ] Shape validity: `b3Shape_IsValid`
- [x] Destroy shape: `b3DestroyShape`
- [ ] Shape type: `b3Shape_GetType`
- [ ] Shape world: `b3Shape_GetWorld`
- [ ] Sensor status: `b3Shape_IsSensor`
- [ ] Event enabled getters: sensor/contact/pre-solve/hit
- [ ] Shape AABB: `b3Shape_GetAABB`
- [ ] Closest point: `b3Shape_GetClosestPoint`
- [ ] Get sphere geometry: `b3Shape_GetSphere`
- [ ] Get capsule geometry: `b3Shape_GetCapsule`
- [ ] Get filter: `b3Shape_GetFilter`
- [ ] Get surface material: `b3Shape_GetSurfaceMaterial`
- [ ] Get density/friction/restitution
- [ ] Shape raycast: `b3Shape_RayCast`
- [ ] Compute shape mass data: `b3Shape_ComputeMassData`
- [ ] Sensor data: `b3Shape_GetSensorData`
- [ ] Shape hull vertices: `b3Shape_GetHullVertices`
- [x] Mesh shape creation: `b3CreateMeshShape`
- [ ] Heightfield shape creation: `b3CreateHeightFieldShape`

## Hulls

- [x] Create cylinder hull: `b3CreateCylinder`
- [x] Create hull from points: `b3CreateHull`
- [x] Destroy hull: `b3DestroyHull`
- [x] Box hull creation for shapes: `b3MakeBoxHull` via `createHullShape`
- [x] Transformed/scaled box hull creation for shapes: `b3CreateTransformedHullShape` / `b3MakeScaledBoxHull`
- [ ] Create cone: `b3CreateCone`
- [x] Create rock: `b3CreateRock`
- [ ] Clone hull: `b3CloneHull`
- [ ] Clone and transform hull: `b3CloneAndTransformHull`
- [ ] Cube hull helper: `b3MakeCubeHull`
- [ ] Offset box hull helper: `b3MakeOffsetBoxHull`
- [ ] Hull vertex/point/edge/face/plane accessors

## Compounds

- [x] Create raw compound handle: `b3CreateCompound`
- [x] Create compound from hull entries: `createCompoundFromHulls`
- [x] Create compound from sphere entries: `createCompoundFromSpheres`
- [x] Destroy compound: `b3DestroyCompound`
- [x] Get compound tree height: internal/demo helper
- [x] Create compound shape: `b3CreateCompoundShape`
- [ ] Compound capsules in high-level TS wrapper
- [ ] Compound meshes in high-level TS wrapper
- [ ] General compound spec mirroring upstream `b3CompoundDef`

## Meshes

- [ ] Create mesh from definition: `b3CreateMesh`
- [x] Destroy mesh: `b3DestroyMesh`
- [x] Grid mesh: `b3CreateGridMesh`
- [ ] Wave mesh: `b3CreateWaveMesh`
- [x] Torus mesh: `b3CreateTorusMesh`
- [ ] Box mesh: `b3CreateBoxMesh`
- [ ] Hollow box mesh: `b3CreateHollowBoxMesh`
- [ ] Platform mesh: `b3CreatePlatformMesh`
- [ ] Mesh vertices/triangles/material indices/flags accessors
- [ ] Mesh height/tree height helpers

## Heightfields

- [ ] Create heightfield: `b3CreateHeightField`
- [ ] Destroy heightfield: `b3DestroyHeightField`
- [ ] Grid heightfield helper
- [ ] Wave heightfield helper
- [ ] Heightfield compressed heights/material/flags accessors

## Joints

- [x] Destroy joint: `b3DestroyJoint`
- [x] Create motor joint: `b3CreateMotorJoint`
- [x] Create filter joint: `b3CreateFilterJoint`
- [x] Create revolute joint: `b3CreateRevoluteJoint` (including creation-time local frames, base constraint tuning, and optional force/torque thresholds)
- [x] Create spherical joint: `b3CreateSphericalJoint`
- [x] Create distance joint: `b3CreateDistanceJoint`
- [x] Create prismatic joint: `b3CreatePrismaticJoint` (optional force/torque thresholds and `collideConnected`)
- [ ] Create wheel joint: `b3CreateWheelJoint`
- [x] Create weld joint: `b3CreateWeldJoint` (optional force/torque thresholds and `collideConnected`)
- [ ] Create parallel joint: `b3CreateParallelJoint`
- [~] Common joint validity/type/body/world/frame/collide/force/torque accessors (`constraint force`, `constraint torque`, and `linear separation` now exposed)
- [ ] Joint wake bodies: `b3Joint_WakeBodies`
- [ ] Constraint tuning: `b3Joint_SetConstraintTuning`
- [~] Force/torque thresholds and joint break support (creation-time thresholds on distance/prismatic/revolute/weld joints)
- [ ] Distance joint runtime controls
- [ ] Revolute joint runtime controls after creation
- [x] Revolute joint target angle: `b3RevoluteJoint_SetTargetAngle` (`setRevoluteJointTargetAngle`)
- [ ] Prismatic joint runtime controls
- [ ] Wheel joint runtime controls
- [ ] Weld joint runtime controls
- [ ] Spherical joint runtime controls after creation
- [ ] Motor joint runtime controls after creation
- [ ] Parallel joint runtime controls

## Queries

- [x] World raycast closest: `b3World_CastRayClosest`
- [ ] World raycast all hits: `b3World_CastRay`
- [ ] World shapecast: `b3World_CastShape`
- [ ] World AABB overlap: `b3World_OverlapAABB`
- [ ] World shape overlap: `b3World_OverlapShape`
- [ ] Body raycast: `b3Body_CastRay`
- [ ] Body shapecast: `b3Body_CastShape`
- [ ] Body shape overlap: `b3Body_OverlapShape`
- [ ] Cast mover: `b3World_CastMover`
- [ ] Collide mover: `b3World_CollideMover`

## Events And Callbacks

- [~] Shape event toggles are exposed: sensor/contact/pre-solve/hit enable flags
- [ ] Body move events: `b3World_GetBodyEvents`
- [ ] Contact events: `b3World_GetContactEvents`
- [ ] Sensor events: `b3World_GetSensorEvents`
- [ ] Joint events: `b3World_GetJointEvents`
- [ ] Zero-allocation event buffers for JS reads
- [ ] Contact/manifold buffer for current contacts
- [ ] Custom filter callback: `b3World_SetCustomFilterCallback`
- [ ] Pre-solve callback: `b3World_SetPreSolveCallback`

## Collision, GJK, And Mass Utilities

- [ ] Shape distance: `b3ShapeDistance`
- [ ] Shape cast: `b3ShapeCast`
- [ ] Time of impact: `b3TimeOfImpact`
- [ ] Pairwise collision helpers for sphere/capsule/hull/triangle combinations
- [ ] Compute sphere/capsule/hull mass without creating a body
- [ ] Compute sphere/capsule/hull AABB without creating a body

## Character And Mover Helpers

- [ ] Solve planes: `b3SolvePlanes`
- [ ] Clip vector: `b3ClipVector`
- [ ] Plane result buffer helpers for JS callbacks

## Dynamic Tree

- [ ] Create/destroy dynamic tree
- [ ] Create/destroy/move/enlarge proxy
- [ ] Proxy category bits get/set
- [ ] Query
- [ ] Query closest
- [ ] Raycast
- [ ] Box cast
- [ ] Rebuild
- [ ] Height/area/root bounds/proxy count/byte count/stats
- [ ] Validate helpers
- [ ] Save/load helpers, if wanted for web builds

## Recording And Replay

- [ ] Create/destroy recording
- [ ] Start/stop world recording
- [ ] Recording byte size/access helpers
- [ ] Replay player create/destroy
- [ ] Replay step/restart/seek
- [ ] Replay world/body accessors
- [ ] Divergence checks

## Explosion

- [ ] Default explosion definition
- [x] World explode: `b3World_Explode`

## Debug Draw

- [ ] `b3World_Draw` wrapper
- [ ] JS callback/handler shape for segments, points, spheres, capsules, boxes, bounds, transforms
- [ ] Debug draw flag plumbing

## Global/System Utilities

- [ ] Version: `b3GetVersion`
- [ ] Double precision check: `b3IsDoublePrecision`
- [ ] Byte count: `b3GetByteCount`
- [ ] Length units: `b3SetLengthUnitsPerMeter`, `b3GetLengthUnitsPerMeter`
- [x] Stall threshold: `b3SetStallThreshold`, `b3GetStallThreshold`
- [ ] Timing helpers: ticks, milliseconds, reset
- [ ] Log/assert callback hooks, if useful in web builds
- [ ] Binary file helpers are intentionally low priority for browser builds

## Math Utilities

- [ ] Vector distance/distance squared
- [ ] Cross product
- [ ] Rotate/inverse rotate vector
- [ ] Transform point
- [ ] Transform multiply/inverse multiply
- [x] Quaternion from axis-angle: `b3MakeQuatFromAxisAngle`
- [ ] AABB union/area/center/extents/closest point
- [ ] Plane validation helpers
- [x] Cos/sin helper: `b3Sin`, `b3Cos` (Box3D Bhāskara I approximation)
- [x] Float32 cos/sin from `<math.h>`: `b3wCosf`, `b3wSinf` (match upstream C++ `cosf`/`sinf`)
- [x] Shared RNG + lerp helpers: `randomVec3`, `lerpVec3` (wrap `RandomVec3` / `b3Lerp`, seed 12345)
- [x] Length-and-normalize helper: `getLengthAndNormalize` (`b3GetLengthAndNormalize`)

## Demo-Specific Helpers

- [x] Human/ragdoll helper: `createHuman`
- [x] Human bone access: `getHumanBoneBody`, `getHumanBoneCount`
- [x] Human velocity: `setHumanVelocity`
- [x] Human bullet toggle: `setHumanBullet`
- [x] Human joint friction/spring/damping tuning
- [x] Batched render transform reads
- [x] Debug color read for render visualization
