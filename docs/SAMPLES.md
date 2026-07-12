# Box3D WASM Sample Port Status

Legend:
- `[x]` = TS sample implemented and matches C++ scene
- `[~]` = partial / needs fix / no direct C++ match
- `[ ]` = not yet implemented
- `🔧` = C API already in WASM bindings
- `🚧` = C API needs WASM bindings first
- `🧩` = uses mesh loading (not trivially portable to web)

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
| **Ray Curtain** | [ ] | `b3World_CastRayClosest`, `b3CreateTorusMesh`, `b3CreateMeshShape` | 🔧 `rayCastClosest` exists. Need `createTorusMesh` helper and `createMeshShape` — check if mesh API is in WASM. |
| **Cast World** | [ ] | `b3World_CastRayClosest`, `b3World_CastShape`, `b3World_OverlapShape`, custom callbacks | 🚧 World-level cast/overlap with callbacks not wrapped. |
| **Mesh Scale** | [ ] | Mesh scaling + collision queries | 🧩 |
| **Shape Cast** | [ ] | `b3World_CastShape` | 🚧 |
| **Overlap World** | [ ] | `b3World_OverlapShape` | 🚧 |
| **Long Ray Cast** | [ ] | `b3World_CastRayClosest` | 🔧 Simple ray cast demo. |
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
| **Bullet vs Stack** | [x] | CCD bullet + stack | 🔧 Wall + 10-box stack + mouse-launched bullet. Scripted launch at frame 1 for dump compare. Stack Y uses float32 `0.5f + 1.1f * row` via `f32Add`/`f32Mul`. C++/WASM dump parity verified at epsilon=1e-5 across default checkpoints. |
| **Needle Mesh** | [ ] | CCD + mesh shape | 🧩 Needs mesh. |
| **Mesh Drop** | [ ] | Mesh + CCD | 🧩 |
| **Mesh Drop Unit Test** | [ ] | Same | 🧩 |
| **Hump Mesh** | [ ] | Mesh + CCD | 🧩 |
| **Is Fast** | [x] | Fast-spinning tall boxes (CCD stress) | 🔧 Three gravity-disabled boxes with different angular velocities. C++/WASM dump parity verified. |
| **Stall** | [x] | CCD stall behavior | Implemented. C++/WASM dump parity verified at epsilon=1e-5. Uses `createTorusMesh`, `createRock`, stall threshold bindings. |

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
| **Crash** | [ ] | `b3CreateGridMesh`, `b3CreateMeshShape` | 🚧 Mesh APIs not wrapped. Uses mesh ground — not simple bodies despite 🔧 note. |
| **Multiple Prismatic** | [ ] | `b3CreatePrismaticJoint` | 🚧 Prismatic joint not wrapped. |
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
| **Driving** | [ ] | Multiple joints + vehicle | 🔧 Complex but uses existing joints. |
| **Gear Lift** | [ ] | Gear joint or equivalent | 🚧 |

## Manifold (`sample_manifold.cpp`)

| Sample | TS | APIs needed | Notes |
|--------|----|-------------|-------|
| **Sphere vs Sphere** | [ ] | Simple scene with two shapes | 🔧 Trivial. |
| **Capsule vs Sphere** | [ ] | Same | 🔧 |
| **Hull vs Sphere** | [ ] | Same | 🔧 |
| **Triangle vs Sphere** | [ ] | Mesh triangle | 🧩 |
| **Capsule vs Capsule** | [ ] | Two capsules | 🔧 |
| **Capsule vs Hull** | [ ] | Capsule + hull | 🔧 |
| **Triangle vs Capsule** | [ ] | Mesh triangle + capsule | 🧩 |
| **Hull vs Hull** | [ ] | Two hulls | 🔧 |
| **Triangle vs Hull** | [ ] | Mesh triangle + hull | 🧩 |

## Mesh (`sample_mesh.cpp`)

| Sample | TS | APIs needed | Notes |
|--------|----|-------------|-------|
| **Grid** | [ ] | `b3CreateGridMesh`, `b3CreateMeshShape` | 🚧 Not in WASM. |
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
| **Pose** | [ ] | Human posing | 🔧 |

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
| **Static Invoke** | [ ] | `shapeDef.invokeContactCreation` | 🔧 `invokeContactCreation` not exposed. |
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
| **Large Pyramid** | [x] | Many box pyramid | 🔧 90-base pyramid (4095 boxes), sleeping disabled. C++/WASM dump parity verified. |
| **Wide Pyramid** | [x] | Wide pyramid | 🔧 15-layer 3D pyramid (~1190 boxes). C++/WASM dump parity verified. |
| **Many Pyramids** | [x] | Multiple pyramids | 🔧 20 offset pyramids with `Box3DRng` layout (`Math.fround` on positions). Sleeping disabled. C++/WASM dump parity verified. |
| **Rain** | [x] | Many falling ragdolls | Implemented. C++/WASM dump parity verified at epsilon=1e-5 for frames 0–64; chaotic drift afterward. `B3W_MAX_HUMANS` raised to 512 for full spawn count. |
| **Large World** | [ ] | Large world scale | 🔧 |
| **Joint Grid** | [x] | Grid of joints | 🔧 10×10 revolute joint grid with filter bits. Sleeping disabled. C++/WASM dump parity verified. |
| **Falling Boxes** | [x] | Many boxes | 🔧 50×8×8 = 3200 boxes, sleeping enabled. C++/WASM dump parity verified. |
| **Candy Cups** | [ ] | Small convex shapes | 🔧 |
| **Explosion** | [ ] | `b3World_Explode` | 🚧 `explode` not wrapped. |
| **Height Field** | [ ] | Height field mesh | 🧩 |
| **Falling Trees** | [ ] | Chains + trees | 🔧 |
| **Sensor** | [ ] | Sensor shapes | 🔧 Maybe works. |
| **Washer** | [x] | Custom convex hull | Implemented. |
| **Hull** | [ ] | Custom hulls | 🔧 |
| **Chains** | [ ] | Joint chains | 🔧 Uses joints. |
| **Destruction** | [ ] | Body destruction | 🔧 |
| **Junkyard** | [ ] | Mixed scene | 🔧 |

---

## Summary

- **Total C++ samples**: ~136
- **TS implemented (matching C++)**: 62
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

- **Dump-match status**: Nearly all dump-enabled samples match at epsilon=1e-5 on default checkpoints (frames 0,50,100,200,300). Recent fixes closed former setup bugs in `continuous/bullet-vs-stack` (stack Y float32) and `shapes/wind-drop` (hull half-extent float32). Remaining soft exceptions are multi-contact / long-horizon FP drift, not missed scene parameters: `ragdoll/pile` (matches through ~frame 46), `world/far-ragdolls` (matches through ~frame 29), `determinism/falling-ragdolls` (matches through frame 200; frame 300 drifts), `shapes/wind-flap` (frames 0–200 exact; ~1–3e-5 drift at frame 300). Native SIMD vs scalar dumps are bit-identical; WASM SIMD vs scalar dumps are bit-identical — residual drift is cross-target codegen/libm, not SSE vs wasm SIMD128. `geometry/hull` uses `dumpNoPhysics` (upstream sample has no bodies).
- **New sample dump checklist**: When porting the next sample, follow `AGENTS.md` → Dump-match readiness (and `docs/reference-dump-plan.md` → New sample dump checklist) so gravity, float32 setup math, step/post-step order, and worker step cadence are dump-ready from day one.