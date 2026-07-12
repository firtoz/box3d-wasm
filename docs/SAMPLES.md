# Box3D WASM Sample Port Status

Legend:
- `[x]` = TS sample implemented and matches C++ scene
- `[~]` = partial / needs fix / no direct C++ match
- `[ ]` = not yet implemented
- `🔧` = C API already in WASM bindings
- `🚧` = C API needs WASM bindings first
- `🧩` = uses mesh loading (not trivially portable to web)

---

---

## Easy next ports

Maintained queue for the "what's next" loop in `AGENTS.md`. Keep this list short (about 5–8 items), ordered by preference. After finishing a port, remove it here, mark it `[x]` in the tables below, and refill from remaining `[ ]` + `🔧` rows (skip `🚧` / `🧩` unless the user asks for bindings/mesh work).

**Before adding or recommending a queue item:** open the upstream C++ sample class. Prefer samples that create bodies in `m_worldId` and can dump-compare. Do **not** treat Manifold / pure geometry editors / collide-debug tools as generic-host warm-ups — they need pairwise collide bindings and a custom host, and usually have no dumpable bodies.

1. **Benchmark / Junkyard** — `createRock` + kinematic pusher; ~10k bodies; `shader-instanced-host`.
2. **Benchmark / Falling Trees** — needs `createWaveMesh` binding first, then compound cylinder trees.
3. **Benchmark / Large World** — needs hull `invokeContactCreation` at create; ship debug-scale (not release 1M statics) unless capacity policy changes.
4. **Mesh / Big Box** or other remaining mesh samples — after Grid patterns.

Defer for later sessions: **Joints / Driving** (heightfield `🚧`), **Manifold** (pairwise `b3Collide*` helpers `🚧`), **Long Ray Cast** (wave mesh + heightfield), **Ragdoll / Pose** (pose-control bindings), **Chains** (`createWaveMesh` not wrapped), events (`🚧` callbacks), character movers, and most `🧩` mesh samples.

---

## Bodies (`sample_bodies.cpp`)

| Sample | TS | APIs needed | Notes |
|--------|----|-------------|-------|
| **Body Type** | [x] | `b3Body_SetType`, `b3Body_Enable`, `b3Body_Disable`, `b3Body_IsEnabled`, `b3Body_SetLinearVelocity`, `b3Body_SetAngularVelocity`, `b3CreateRevoluteJoint`, `b3CreatePrismaticJoint`, `b3DefaultPrismaticJointDef`, `b3DefaultRevoluteJointDef`, `b3MakeTransformedBoxHull` | 🔧 All APIs exist. 6 bodies, prismatic + revolute joints, kinematic oscillation. |
| **Spinning Book** | [x] | `b3BodyDef.gravityScale`, `b3BodyDef.angularVelocity`, `b3MakeBoxHull` | 🔧 All exist. Three boxes with gravity disabled and different angular velocities. C++/WASM dump parity verified with the default 5-second-or-sleep window. |
| **Gyroscopic Torque** | [x] | `b3CreateCylinder`, `b3CreateHullShape` (multiple on same body), `b3Body_ApplyMassFromShapes`, `b3Body_GetWorldCenter`, `shapeDef.updateBodyMass = false` | 🔧 All APIs exist. Dzhanibekov effect: cylinder + box on gravityScale=0 body with angular velocity. C++/WASM dump parity verified with the default 5-second comparison window. Render uses generic host `compound` parts; box dimensions are doubled from `b3MakeBoxHull` half-extents and cylinder is locally offset by `0.5 * height`. |
| **Weeble** | [x] | `b3Body_GetMass`, `b3Body_GetLocalRotationalInertia`, `b3Body_SetMassData`, `b3Body_SetTransform`, `b3Body_SetAwake`, `b3Body_GetWorldPoint`, `b3Body_GetLocalPointVelocity`, `b3Body_GetWorldPointVelocity`, `b3World_Explode` | 🔧 All APIs now wrapped. Capsule with shifted COM + Teleport/Explode buttons. Interactive C++/WASM dump parity now covers a scripted teleport; the demo control params were also aligned with upstream while adding parity. |
| **Disable** | [x] | `b3Body_Enable`, `b3Body_Disable`, `b3Body_IsEnabled`, `b3Body_ApplyLinearImpulseToCenter`, `b3CreateWeldJoint` | 🔧 All APIs now wrapped. 4-link chain with weld joints + ball, enable/disable toggles. |
| **Cast** | [ ] | `b3Body_CastRay`, `b3Body_CastShape`, `b3Body_OverlapShape`, `b3Body_CollideMover`, `b3CreateCylinder` | 🚧 Needs body-level cast/overlap/collide APIs. Low-level query APIs not yet wrapped. |
| **Kinematic** | [x] | `b3Body_SetTargetTransform`, `bodyDef.type = kinematic` | 🔧 `setBodyTargetTransform` exists. Kinematic body type exists. C++/WASM dump parity verified at epsilon=1e-6 across all 5 checkpoints (frames 0,50,100,200,300) — stationary for 2s delay then circular motion at radius 4. Uses `makeQuatFromAxisAngle` for rotation quaternion, standard `Math.cos`/`Math.sin` for position (matching C++ `cosf`/`sinf`), and `Math.fround` for float32-equivalent time accumulation. |
| **Lock Mixing** | [x] | `bodyDef.motionLocks.angularX/Y/Z`, `bodyDef.motionLocks.linearX/Y/Z` | 🔧 `setBodyMotionLocks` exists, can set at body creation via `bodyDef.motionLocks`? Actually in TS we use `setBodyMotionLocks` after creation. |
| **Fixed Rotation** | [x] | `bodyDef.motionLocks.angularX/Y/Z`, `bodyDef.gravityScale` | 🔧 All exist. C++/WASM dump parity verified with the default 5-second comparison window after matching the upstream vertical capsule setup exactly. |

## Character (`sample_character.cpp`)

| Sample | TS | APIs needed | Notes |
|--------|----|-------------|-------|
| **CapsulePlane** | [ ] | `b3World_CollideMover`, `b3SolvePlanes`, `b3WorldTransform` | 🚧 Low-level mover/collide APIs. Plane solver not in WASM. |
| **MoverOverlap** | [ ] | `b3World_CollideMover`, `b3SolvePlanes`, deep overlap check | 🚧 Same as CapsulePlane. |
| **Mover** | [ ] | Mesh loading, `b3CreateMeshShape`, `b3CreateHeightFieldShape`, `b3CreateWave`, `b3CreateTorusMesh`, full mover system | 🧩🚧 Very complex. Mesh loading + height fields + full character mover. |
| **Rigid Body** | [ ] | Same as Mover + `b3World_CastRayClosest` + rigibody character system | 🧩🚧 Even more complex. |

## Collision (`sample_collision.cpp`)

| Sample | TS | APIs needed | Notes |
|--------|----|-------------|-------|
| **Ray Curtain** | [x] | `b3World_CastRayClosest`, `b3CreateTorusMesh`, `b3CreateMeshShape` | 🔧 4 kinematic targets (sphere/capsule/box/torus); no physics ground; host draws full yellow rays + hit normals + DrawGroundGrid/axes. Dump compares body poses **and** per-ray hit/fraction/point/normal (`rays` checkpoint field). |
| **Cast World** | [ ] | `b3World_CastRayClosest`, `b3World_CastShape`, `b3World_OverlapShape`, custom callbacks | 🚧 World-level cast/overlap with callbacks not wrapped. |
| **Mesh Scale** | [ ] | Mesh scaling + collision queries | 🧩 |
| **Shape Cast** | [ ] | `b3World_CastShape` | 🚧 |
| **Overlap World** | [ ] | `b3World_OverlapShape` | 🚧 |
| **Long Ray Cast** | [ ] | `b3World_CastRayClosest`, `b3CreateRock`, `b3CreateWaveMesh`, `b3CreateWave` / heightfield | 🚧 Not a simple ray demo: static sphere/capsule/hull/mesh/heightfield targets + long-range accuracy trail. `createRock` exists; wave mesh + heightfield not wrapped. |
| **Initial Overlap** | [ ] | Overlap queries | 🚧 |
| **Shape Cast Debug** | [ ] | `b3World_CastShape` + debug draw | 🚧 |
| **Distance Debug** | [ ] | `b3World_ComputeDistance` | 🚧 Not exposed. |
| **Shape Distance** | [ ] | Distance queries | 🚧 |
| **Time of Impact** | [ ] | `b3World_ComputeTOI` | 🚧 Not exposed. |
| **Capsule Cast Ray** | [ ] | Ray cast with capsule proxy | 🚧 |

## Compound (`sample_compound.cpp`)

| Sample | TS | APIs needed | Notes |
|--------|----|-------------|-------|
| **Simple** | [x] | `b3CreateCompound`, `b3CreateCompoundShape`, `b3DestroyCompound` | Recent fix uses `createCompoundFromHulls`. |
| **Spheres** | [x] | `b3CreateCompound` with spheres, `b3CreateCompoundShape`, `b3DestroyCompound` | 🔧 `createCompoundFromSpheres` exists. |
| **Hulls** | [x] | `b3CreateCompound` with multiple hulls | 🔧 `createCompoundFromHulls` exists. |
| **Tile Floor** | [x] | Compound with many hull instances | 🔧 50×50 compound tile floor with `Box3DRng` Y offsets (seed 12345). Worker skips default ground box. C++/WASM dump parity verified. |
| **Mesh Tile** | [ ] | `b3CreateMesh`, `b3CreateCompound` with meshes | 🚧 Mesh compound not wrapped. |
| **Village** | [ ] | Compound with hulls + capsules + spheres + meshes, mesh loading | 🧩🚧 Most complex compound sample. |

## Continuous (`sample_continuous.cpp`)

| Sample | TS | APIs needed | Notes |
|--------|----|-------------|-------|
| **Thin Wall** | [x] | CCD via `bodyDef.isBullet`, fast-moving bodies | 🔧 Static thin wall + 3 high-speed bullets (sphere, capsule, box). C++/WASM dump parity verified. |
| **Bounce House** | [x] | CCD + restitution | 🔧 Compound static body (4 walls) + bouncing sphere with `gravityScale=0`, `restitution=1`. C++/WASM dump parity verified. |
| **Spinning Stick** | [x] | CCD + thin fast-spinning body | 🔧 Thin wall + fast stick with hardcoded angular velocity from upstream `RandomVec3` printf. C++/WASM dump parity verified. |
| **Bullet vs Stack** | [x] | CCD bullet + stack | 🔧 Thin wall (local shape offset on static body) + 10-box stack + Launch/L CCD sphere (`density *= 10`). Worker + generic host; dump launches at frame 1. Stack Y uses float32 `0.5f + 1.1f * row`. C++/WASM dump parity verified at epsilon=1e-5. |
| **Needle Mesh** | [ ] | CCD + mesh shape | 🧩 Needs mesh. |
| **Mesh Drop** | [ ] | Mesh + CCD | 🧩 |
| **Mesh Drop Unit Test** | [ ] | Same | 🧩 |
| **Hump Mesh** | [ ] | Mesh + CCD | 🧩 |
| **Is Fast** | [x] | Fast-spinning tall boxes (CCD stress) | 🔧 Three gravity-disabled boxes with different angular velocities. C++/WASM dump parity verified. |
| **Stall** | [x] | CCD stall behavior | Dense 200×200 torus + rock CCD bullet (`isBullet`, vel 600). Stall threshold 0.001s restored on dispose. Launch respawns bullet. Camera matches `SetView(130,15,15)`. Dump parity verified. |

## Determinism (`sample_determinism.cpp`)

| Sample | TS | APIs needed | Notes |
|--------|----|-------------|-------|
| **Falling Ragdolls** | [x] | `b3CreateHuman`, ragdoll creation | Implemented. C++/WASM dump parity verified at epsilon=1e-5 for frames 0–200; frame 300 drifts like other multi-ragdoll piles. Ground tile + human spawn order matches upstream; spawn layout uses float32-safe helpers. |

## Events (`sample_events.cpp`)

| Sample | TS | APIs needed | Notes |
|--------|----|-------------|-------|
| **Sensor Visit** | [ ] | `shapeDef.isSensor`, `shapeDef.enableSensorEvents`, sensor event callbacks | 🚧 Sensor events not wrapped. Shape can be sensor via `isSensor`. |
| **Hit** | [ ] | `shapeDef.enableHitEvents`, hit event callbacks | 🚧 Hit events not wrapped. |
| **Move** | [ ] | Move events | 🚧 |
| **Joint** | [ ] | Joint events | 🚧 |
| **Persistent Contact** | [ ] | Contact tracking | 🚧 |
| **Sensor Hits** | [ ] | Sensor + hit combined | 🚧 |

## Geometry (`sample_geometry.cpp`)

| Sample | TS | APIs needed | Notes |
|--------|----|-------------|-------|
| **Box Hull** | [x] | `b3MakeBoxHull`, hull debug draw | 🔧 Trivial. |
| **Hull** | [x] | `b3CreateHull` from points, hull debug draw | 🔧 `createHullFromPoints` exists. |
| **Hull Reduction** | [x] | `b3CreateHull` with many points → reduction | 🔧 Maybe works. |
| **Hull Transform** | [x] | `b3MakeTransformedBoxHull`, hull transform debug | 🔧 Most APIs exist. |
| **Capsule Mass** | [ ] | `b3Body_GetMass`, `b3Body_GetLocalRotationalInertia`, `b3Body_SetMassData` | 🚧 `getMass`, `getLocalRotationalInertia` not exposed. |

## Issues (`sample_issues.cpp`)

| Sample | TS | APIs needed | Notes |
|--------|----|-------------|-------|
| **Dump Loader** | [ ] | World dump/load | 🚧 |
| **Crash** | [x] | `b3CreateGridMesh`, `b3CreateMeshShape`, weld | 🔧 Mesh floor visual (`createGridMesh(20,20,2)` at y=-1) + two boxes; **Add Joint** welds them. Dump variants: baseline (no joint), `issues/crash-joint-awake` (weld @ frame 10), `issues/crash-joint-asleep` (weld @ frame 200 while sleeping — no yank). |
| **Multiple Prismatic** | [x] | `b3CreatePrismaticJoint` (`constraintHertz`) | 🔧 Empty static anchor (no ground hull/visual); 6 stacked boxes; prismatic limits ±6, `constraintHertz=240`; mouse force 1e6. C++/WASM dump parity verified. |
| **Hull Crash** | [x] | Hull creation | 🔧 Hull from 5 regression points. |
| **Convex Jitter** | [x] | Hull creation + stacking | 🔧 Two custom hulls from point clouds. |
| **s&box mover** | [ ] | Mover/character system | 🚧 |
| **Capsule Mesh** | [ ] | Capsule + mesh collision | 🧩 |

## Joints (`sample_joint.cpp`)

| Sample | TS | APIs needed | Notes |
|--------|----|-------------|-------|
| **Distance Joint** | [ ] | `b3CreateDistanceJoint`, `b3DefaultDistanceJointDef` | 🚧 Not in WASM. |
| **Filter** | [x] | `b3CreateFilterJoint` | 🔧 `createFilterJoint` exists. Passive C++/WASM dump parity verified with the default 5-second-or-sleep window. |
| **Motor Joint** | [x] | `b3CreateMotorJoint` | 🔧 `createMotorJoint` exists. Interactive C++/WASM dump parity now covers scripted target motion via a deterministic frame schedule. |
| **Top Down Friction** | [x] | Motor joint + friction | 🔧 Interactive C++/WASM dump parity now covers the sample's scripted explosion. |
| **Prismatic** | [x] | `b3CreatePrismaticJoint` | 🔧 Passive C++/WASM dump parity verified with the default 5-second-or-sleep window. |
| **Spherical** | [x] | `b3CreateSphericalJoint` | 🔧 Passive C++/WASM dump parity verified with the default 5-second-or-sleep window. |
| **Parallel Spring** | [ ] | Distance joint with spring params | 🚧 |
| **Revolute** | [x] | `b3CreateRevoluteJoint` | 🔧 Passive C++/WASM dump parity verified with the default 5-second-or-sleep window. Preserving upstream revolute joint base constraint defaults (`60 Hz`, `2.0`) in the WASM wrapper was required for a clean match. |
| **Weld** | [x] | `b3CreateWeldJoint` | 🔧 Passive C++/WASM dump parity verified with the default 5-second-or-sleep window. |
| **Wheel** | [ ] | `b3CreateWheelJoint` | 🚧 Not in WASM. |
| **Ball and Chain** | [x] | Spherical joint chain | 🔧 Passive C++/WASM dump parity verified with the default 5-second comparison window. |
| **Door** | [x] | Revolute joint with limit | 🔧 Interactive C++/WASM dump parity now covers the door impulse. Matching upstream required fixing the revolute joint frame rotation in the scene and exposing revolute creation-time `constraintHertz` / `constraintDampingRatio` in WASM. |
| **Bridge** | [x] | Revolute joint chain | 🔧 Passive C++/WASM dump parity verified with the default 5-second comparison window. |
| **Motion Locks** | [x] | `b3Body_SetMotionLocks` | Implemented. C++/WASM dump parity verified at epsilon=1e-5. Uses new `createDistanceJoint` binding plus joint force/torque thresholds on prismatic/revolute/weld. |
| **Driving** | [ ] | Wheel/suspension joints + `b3CreateWave` heightfield ground | 🚧 Heightfield not wrapped. Not a joints-only sample despite the category. |
| **Gear Lift** | [ ] | Gear joint or equivalent | 🚧 |

## Manifold (`sample_manifold.cpp`)

These are **not** physics-world body scenes. Upstream samples inherit a Manifold base that stores two world transforms, calls pairwise `b3Collide*` each step, draws contact points, and lets the user drag/rotate shape B. There is no stack of bodies for `generic-host` / dump-compare. Pairwise collide helpers are still unwrapped (`docs/WASM_API_SURFACE.md` → Collision, GJK, And Mass Utilities). Porting them means bindings + a custom interactive host; do not queue them as easy ports.

| Sample | TS | APIs needed | Notes |
|--------|----|-------------|-------|
| **Sphere vs Sphere** | [ ] | `b3CollideSpheres` | 🚧 Interactive collide demo (two spheres at transforms A/B). |
| **Capsule vs Sphere** | [ ] | `b3CollideCapsuleAndSphere` | 🚧 Same pattern; capsule endpoints along X. |
| **Hull vs Sphere** | [ ] | `b3CollideHullAndSphere` | 🚧 Same pattern; box hull + sphere. |
| **Triangle vs Sphere** | [ ] | `b3CollideSphereAndTriangle` | 🚧🧩 Triangle + sphere collide. |
| **Capsule vs Capsule** | [ ] | `b3CollideCapsules` | 🚧 Same pattern; two capsules. |
| **Capsule vs Hull** | [ ] | `b3CollideHullAndCapsule` | 🚧 Same pattern. |
| **Triangle vs Capsule** | [ ] | `b3CollideCapsuleAndTriangle` | 🚧🧩 |
| **Hull vs Hull** | [ ] | `b3CollideHulls` | 🚧 Same pattern; uses SAT cache. |
| **Triangle vs Hull** | [ ] | `b3CollideHullAndTriangle` | 🚧🧩 |

## Mesh (`sample_mesh.cpp`)

| Sample | TS | APIs needed | Notes |
|--------|----|-------------|-------|
| **Grid** | [x] | `b3CreateGridMesh`, `b3CreateMeshShape`, `b3Shape_SetMesh` | 🔧 20×20 grid mesh floor (wireframe visual) scale `[2,2,2]`; Scale X/Z `[-2,2]` via `setMesh` (host mesh scales too); Sphere/Capsule/Box/Cylinder radios; DrawAxes. C++/WASM dump parity verified at default cylinder. |
| **Big Box** | [ ] | `b3CreateBoxMesh`, `b3CreateMeshShape` | 🧩 |
| **Box** | [ ] | Same | 🧩 |
| **Reflection** | [ ] | Mesh with negative scale | 🧩 |
| **Height Field** | [ ] | `b3CreateWave`, `b3CreateHeightFieldShape` | 🚧 Not in WASM. |
| **Viewer** | [ ] | Mesh file loader | 🧩 |
| **Creation Benchmark** | [ ] | Mesh creation perf | 🧩 |
| **Voxel** | [ ] | Voxel mesh | 🧩 |
| **Hollow Box** | [ ] | Hollow mesh box | 🧩 |

## Ragdoll (`sample_ragdoll.cpp`)

| Sample | TS | APIs needed | Notes |
|--------|----|-------------|-------|
| **Box** | [x] | `b3CreateHuman` + drop on box | 🔧 Single human on box ground. C++ reference dump uses `Ragdoll/Box` (disambiguates Mesh/Box). C++/WASM dump parity verified. |
| **Mesh** | [ ] | Human + mesh floor | 🧩 |
| **Pile** | [x] | Multiple humans piling | 🔧 20 humans on mesh floor (release build count). Spawn positions use float32-safe `0.1f * i` arithmetic (`f32Mul`/`f32Add`). Matches through frame ~46 at 1e-5; later divergence is native-vs-WASM solver FP drift in the multi-contact ragdoll pile (single-human `ragdoll/box` matches fully). |
| **Incline** | [x] | Human + inclined ramp | Implemented. C++/WASM dump parity verified at epsilon=1e-5. Motor demotion at 2s via `dumpStep`. |
| **Pose** | [ ] | `b3CreateHuman` + pose/motor control + grid mesh floor | 🚧 Human spawn exists; pose-control / motor-adjust bindings not wrapped. Uses `createGridMesh` floor (API exists). |

## Robustness (`sample_robustness.cpp`)

| Sample | TS | APIs needed | Notes |
|--------|----|-------------|-------|
| **HighMassRatio1** | [x] | High density ratios | 🔧 Three pyramids with heavy top boxes. Ground half-extent must be 50 (matches `AddGroundBox(50)`). C++/WASM dump parity verified at epsilon=1e-5. |
| **Tiny Pyramid** | [x] | Tiny scale pyramid | 🔧 30-base pyramid of 2.5cm boxes. C++/WASM dump parity verified at epsilon=0. Uses `Math.fround()` for float32 intermediate rounding to match upstream position arithmetic. Render bodies added (465 boxes). |
| **Overlap Recovery** | [x] | Bodies starting in overlap | 🔧 25% overlap with contact tuning. Ground half-extent must be 20 (matches `AddGroundBox(20)`). C++/WASM dump parity verified at epsilon=1e-5. |
| **Overflow Color Pile** | [x] | Many bodies + color debug | 🔧 Hub + 24 neighbors for graph color overflow. C++/WASM dump parity verified at epsilon=1e-7. Uses `b3wCosf`/`b3wSinf` (float32 `<math.h>`) for initial positions to match upstream `cosf`/`sinf`. |

## Shapes (`sample_shapes.cpp`)

| Sample | TS | APIs needed | Notes |
|--------|----|-------------|-------|
| **Inclined Plane** | [x] | Standard APIs | Implemented. C++/WASM dump parity verified at epsilon=1e-5. Float32 friction constant `0.04f` was the root cause of earlier mismatch. |
| **Rolling Resistance** | [x] | `shapeDef.baseMaterial.rollingResistance`, spheres + capsules on plane | 🔧 All exist. |
| **High Resistance** | [x] | High rolling resistance capsules | 🔧 10 capsules with rolling resistance 0–1.8. C++/WASM dump parity verified. |
| **Isotropic Friction** | [x] | Friction sweep with boxes on circle | 🔧 All exist. |
| **Slide Twist** | [x] | Friction + twisting | 🔧 Static plane + dynamic box with angular velocity. Uses `b3wRotateVector` for exact angular velocity match. C++/WASM dump parity verified. |
| **Restitution** | [x] | Bounciness sweep | 🔧 All exist. |
| **Static Invoke** | [x] | `shapeDef.invokeContactCreation` | 🔧 Dynamic sphere + Invoke/Passive + Create/Destroy (Create recreates); auto-create at step 20. Sphere create takes `invokeContactCreation`. C++/WASM dump parity verified. |
| **Conveyor Belt** | [x] | `shapeDef.baseMaterial.tangentVelocity` | 🔧 Platform with `tangentVelocity` + 5 boxes. Required adding `tangentVelocity` params to `b3wShapeSetSurfaceMaterial` bridge. C++/WASM dump parity verified. |
| **Conveyor Mesh** | [ ] | Mesh + tangent velocities per-material | 🧩🚧 Mesh + material per triangle. |
| **Wind** | [x] | `b3Shape_ApplyWind`, spherical joint chain | 🔧 Ten box shapes on a spherical joint chain (not all anchored to ground). `dumpPostStep` uses WASM `randomVec3`, `lerpVec3`, and `getLengthAndNormalize` for bit-exact upstream noise/wind math. C++/WASM dump parity verified at epsilon=1e-5. |
| **Wind Drop** | [x] | `b3Shape_ApplyWind` on single shape | 🔧 Thin plate with post-step wind (`wake=true`). Hull half-extents use float32 `4.0f * radius` via `f32Mul`. C++/WASM dump parity verified at epsilon=1e-5 across default checkpoints. |
| **Wind Flap** | [x] | Wind + revolute joints + spring | 🔧 Flapping wings driven by `setRevoluteJointTargetAngle` + `b3wSin` with float32-safe angle/time updates. Frames 0–200 match at 1e-5; frame 300 wing ω/v drift ~1–3e-5 (cross-target solver FP, not a setup bug; SIMD on/off identical). |

## Stacking (`sample_stacking.cpp`)

| Sample | TS | APIs needed | Notes |
|--------|----|-------------|-------|
| **Card House Thick** | [x] | Standard stacking | Implemented; C++/WASM dump parity verified at epsilon=1e-5 (chaotic stacking, 27 bodies, 3 checkpoints). |
| **Card House** | [x] | Thin card stacking (tiny thickness hull) | 🔧 All exist. Exact C++ layout with thin card hulls. C++/WASM dump parity verified at epsilon=1e-5. |
| **Sphere Stack** | [x] | Sphere stacking | Implemented; C++/WASM dump parity verified with the default 5-second-or-sleep window. |
| **Capsule Stack** | [x] | Capsule stacking with motion locks | Implemented; C++/WASM dump parity verified with the default 5-second-or-sleep window. |
| **Single Box** | [x] | Single dynamic box | Implemented. |
| **Cylinder** | [x] | Cylinder hull | Implemented (offset fix done); C++/WASM dump parity verified with the default 5-second-or-sleep window. |
| **Cylinder Stack** | [x] | `b3CreateTransformedHullShape` with scaling | 🔧 `createTransformedHullShape` exists. Cylinder stack parity verified at epsilon=1e-5. |
| **Box Stack** | [x] | Box stacking | Implemented; C++/WASM dump parity verified with the default 5-second-or-sleep window. |
| **Jenga Stack** | [x] | Jenga tower | Implemented. |
| **Dominoes** | [x] | Domino ring | Implemented. |
| **Wedge** | [x] | `b3CreateHull` from custom points | 🔧 `createHullFromPoints` exists. Custom hull render matches the physics wedge. C++/WASM dump parity verified with the default 5-second-or-sleep window. |
| **Arch** | [x] | `b3CreateHull` from custom points, per-body hulls | 🔧 All exist. Custom hull render matches the physics arch. C++/WASM dump parity verified with the default 5-second-or-sleep window. |
| **Double Domino** | [x] | Domino row with impulse | 🔧 Simple. Initial impulse applied at creation matches the C++ sample. C++/WASM dump parity verified with the default 5-second comparison window. |
| **Pyramid2D** | [x] | 2D pyramid stacking | Implemented; C++/WASM dump parity verified with the default 5-second-or-sleep window. |

## Tree (`sample_tree.cpp`)

| Sample | TS | APIs needed | Notes |
|--------|----|-------------|-------|
| **Benchmark** | [ ] | Dynamic tree internals debug | 🚧 Not applicable to web. |

## World (`sample_world.cpp`)

| Sample | TS | APIs needed | Notes |
|--------|----|-------------|-------|
| **Far Stack** | [x] | Stack far from origin | 🔧 10000km offset box column. |
| **Far Pyramid** | [x] | Pyramid far from origin | 🔧 10000km offset 40-base pyramid. |
| **Far Ragdolls** | [x] | Ragdolls far from origin | 🔧 20 ragdolls at 1000km offset. Uses `createGridMesh` + `createMeshShape` for the upstream mesh floor. Spawn offsets use float32-safe helpers. Matches through ~frame 29 at 1e-5; later divergence is native-vs-WASM solver FP in the chaotic ragdoll pile (SIMD on/off identical on both targets; single-human samples match). |
| **Far Mesh Drop** | [ ] | Mesh + far origin | 🧩 |

## Benchmark (`sample_benchmark.cpp`)

| Sample | TS | APIs needed | Notes |
|--------|----|-------------|-------|
| **Large Pyramid** | [x] | Many box pyramid | 🔧 90-base pyramid (4095 boxes); continuous off (upstream). Shared multi-layer `shader-instanced-host` applies awake/sleep debug colors (`C` toggles light/full; Sleep toggle enables sleeping). Dump create still disables sleep. C++/WASM dump parity verified. |
| **Wide Pyramid** | [x] | Wide pyramid | 🔧 15-layer 3D pyramid (~1190 boxes), shared `shader-instanced-host`. C++/WASM dump parity verified. |
| **Many Pyramids** | [x] | Multiple pyramids | 🔧 14×14 pyramids (base 10 → 10780 boxes), `Math.fround` positions. Sleeping disabled. Shared `shader-instanced-host`. C++/WASM dump parity verified. |
| **Rain** | [x] | Many falling ragdolls | Implemented. Host uses multi-layer `shader-instanced-host` gather draws (14 bone capsules × up to 300 humans). Worker pre-sizes transform snapshots and grows the tracked body set as columns spawn. C++/WASM dump parity verified at epsilon=1e-5 for frames 0–64; chaotic drift afterward. `B3W_MAX_HUMANS` raised to 512 for full spawn count. |
| **Large World** | [ ] | Large world / rain-scale helpers | 🔧 Larger session; upstream uses shared benchmark create/step helpers. |
| **Joint Grid** | [x] | Grid of joints | 🔧 100×100 spherical joint grid (10000 spheres) with filter bits. Sleeping disabled. Shared `shader-instanced-host` (spheres). C++/WASM dump parity verified. |
| **Falling Boxes** | [x] | Many boxes | 🔧 50×8×8 = 3200 boxes, sleeping enabled. Shared `shader-instanced-host`. C++/WASM dump parity verified. |
| **Candy Cups** | [x] | `b3CreateHull` cup profiles + many dynamics | 🔧 16×16×16 cups (4096) from `CreateConvex(0.6,0,0.95,1)` via float32 `b3ComputeCosSin` port; shared hull + `createShapeFromHull`. `shader-instanced-host` with `ConvexGeometry`. Ground `AddGroundBox(60)`. Dump scripted mid-pile `explode` at frame 200 (same schedule in TS + `reference-dump`). Frames 0–200 match at 1e-5 including the post-blast checkpoint; frame 300 drifts like other multi-contact piles. |
| **Explosion** | [x] | `b3World_Explode`, `shapeDef.explosionScale`, grid mesh + walls | 🔧 33×33 cylinders (`explosionScale=2`) in mesh arena; **Explode** button + Magnitude slider (and `E`) match upstream ImGui controls. Dump `explode` at frame 1 matching upstream button defaults. `createShapeFromHull` accepts `explosionScale`. Frames 0–20 match at 1e-5 including post-blast; later multi-contact drift. |
| **Height Field** | [ ] | Height field mesh | 🧩 |
| **Falling Trees** | [ ] | Tree/chain create helpers | 🔧 Larger session; port shared `CreateTrees*` helpers. |
| **Sensor** | [ ] | Sensors + sensor events + custom filter callback | 🚧 Sensor event callbacks / custom filter not wrapped. |
| **Washer** | [x] | Custom convex hull | Implemented. Shader path uses shared multi-layer `shader-instanced-host` (`bodyOffset: 1` + drum `setupScene`); matrix A/B path keeps `InstancedMesh` via shared `createWorkerSampleShell`. |
| **Hull** | [ ] | Hull create/clone timing (no bodies) | 🚧 Not a body scene — microbench drawing hulls + `b3CloneAndTransformHull` (not wrapped). Vacuous dump only. |
| **Chains** | [ ] | Capsule chains + `b3CreateWaveMesh` ground + wind | 🚧 Wave mesh not wrapped (`createGridMesh`/`createTorusMesh` exist; wave mesh does not). |
| **Destruction** | [x] | Body grid + `createGridMesh` + explode | 🔧 20³ spawn grid (~½ filled via Box3D RNG skip), mesh floor, explode-at-create, live destroy/respawn every 140 steps. Shared `shader-instanced-host`. Dump uses `DumpDestruction` (no mid-run respawn — pool reuse would scramble C++/WASM body order). Default checkpoints match at 1e-5 (bodies asleep by frame 300). |
| **Junkyard** | [ ] | Mixed scene via `CreateJunkyard` | 🔧 Larger session; port shared junkyard helpers. |

---

## Summary

- **Total C++ samples**: ~136
- **TS implemented (matching C++)**: 73
- **TS implemented (TS-only)**: 2 (dominoes variant, washer variant, material-dedup)
- **Implemented samples**:
  1. Bodies / Spinning Book
  2. Bodies / Fixed Rotation
  3. Bodies / Lock Mixing
  4. Bodies / Kinematic
  5. Bodies / Gyroscopic Torque
  6. Bodies / Body Type
  7. Bodies / Weeble
  8. Bodies / Disable
  9. Geometry / Box Hull
  10. Geometry / Hull
  11. Geometry / Hull Reduction
  12. Geometry / Hull Transform
  13. Issues / Hull Crash
  14. Issues / Convex Jitter
  15. Robustness / High Mass Ratio 1
  16. Robustness / Tiny Pyramid
  17. Robustness / Overlap Recovery
  18. Robustness / Overflow Color Pile
  19. World / Far Stack
  20. World / Far Pyramid
  21. World / Far Ragdolls
  22. Compound / Simple
  23. Compound / Material Dedup
  24. Single / Box
  25. Cylinder
  26. Sphere / Stack
  27. Box / Stack
  28. Shapes / Inclined Plane
  29. Shapes / Rolling Resistance
  30. Shapes / High Resistance
  31. Shapes / Slide Twist
  32. Shapes / Conveyor Belt
  33. Shapes / Wind Drop
  34. Shapes / Restitution
  35. Shapes / Isotropic Friction
  36. Continuous / Thin Wall
  37. Continuous / Bounce House
  38. Benchmark / Large Pyramid
  39. Benchmark / Wide Pyramid
  40. Benchmark / Falling Boxes
  41. Dominoes
  42. Card / House Thick
  43. Jenga / Stack
  44. Pyramid 2D
  45. Capsule / Stack
  46. Joints / Filter
  47. Joints / Motor Joint
  48. Joints / Prismatic
  49. Joints / Revolute
  50. Joints / Weld
  51. Joints / Top Down Friction
  52. Joints / Spherical
  53. Joints / Ball and Chain
  54. Joints / Door
  55. Joints / Bridge
  56. Continuous / Spinning Stick
  57. Continuous / Is Fast
  58. Continuous / Bullet vs Stack
  59. Benchmark / Many Pyramids
  60. Benchmark / Joint Grid
  61. Compound / Tile Floor
  62. Ragdoll / Box
  63. Ragdoll / Pile
  64. Shapes / Wind
  65. Shapes / Wind Flap
  66. Benchmark / Candy Cups
  67. Benchmark / Explosion
  68. Benchmark / Destruction
  69. Issues / Multiple Prismatic
  70. Issues / Crash
  71. Mesh / Grid
  72. Shapes / Static Invoke
  73. Collision / Ray Curtain

- **Dump-match status**: Nearly all dump-enabled samples match at epsilon=1e-5 on default checkpoints (frames 0,50,100,200,300). Recent fixes closed former setup bugs in `continuous/bullet-vs-stack` (stack Y float32) and `shapes/wind-drop` (hull half-extent float32). Remaining soft exceptions are multi-contact / long-horizon FP drift, not missed scene parameters: `ragdoll/pile` (matches through ~frame 46), `world/far-ragdolls` (matches through ~frame 29), `determinism/falling-ragdolls` (matches through frame 200; frame 300 drifts), `shapes/wind-flap` (frames 0–200 exact; ~1–3e-5 drift at frame 300), `benchmark/candy-cups` (scripted explode at frame 200; frames 0–200 match including post-blast; frame 300 drifts), `benchmark/explosion` (scripted explode at frame 1; frames 0–20 match; drift from ~frame 30). `benchmark/destruction` matches default checkpoints (dump skips mid-run respawn). Native SIMD vs scalar dumps are bit-identical; WASM SIMD vs scalar dumps are bit-identical — residual drift is cross-target codegen/libm, not SSE vs wasm SIMD128. `geometry/hull` uses `dumpNoPhysics` (upstream sample has no bodies).
- **New sample dump checklist**: When porting the next sample, follow `AGENTS.md` → Dump-match readiness (and `docs/reference-dump-plan.md` → New sample dump checklist) so gravity, float32 setup math, step/post-step order, and worker step cadence are dump-ready from day one.