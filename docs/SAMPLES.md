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
| **Body Type** | [ ] | `b3Body_SetType`, `b3Body_Enable`, `b3Body_Disable`, `b3Body_IsEnabled`, `b3Body_SetLinearVelocity`, `b3Body_SetAngularVelocity`, `b3Body_GetPosition`, `b3Body_GetLinearVelocity`, `b3CreateRevoluteJoint`, `b3CreatePrismaticJoint`, `b3DefaultPrismaticJointDef`, `b3DefaultRevoluteJointDef`, `b3MakeTransformedBoxHull` | 🔧 Most APIs exist, `b3Body_SetType` exists, `b3Body_Enable`/`Disable`/`IsEnabled` exist. Check `b3CreatePrismaticJoint` — not in WASM. `b3MakeTransformedBoxHull` exists as `createTransformedHullShape`. Need prismatic joint. |
| **Spinning Book** | [x] | `b3BodyDef.gravityScale`, `b3BodyDef.angularVelocity`, `b3MakeBoxHull` | 🔧 All exist. Three boxes with gravity disabled and different angular velocities. |
| **Gyroscopic Torque** | [ ] | `b3CreateCylinder`, `b3CreateHullShape` (multiple on same body), `b3Body_ApplyMassFromShapes`, `b3Body_GetWorldCenter`, `shapeDef.updateBodyMass = false` | 🔧 `createCylinder` exists, `createHullShape` exists, `setBodyAngularVelocity` exists. Need `applyMassFromShapes` — check if exists in WASM. `getWorldCenter` — need to verify. |
| **Weeble** | [ ] | `b3Body_GetMass`, `b3Body_GetLocalRotationalInertia`, `b3Body_SetMassData`, `b3Body_SetTransform`, `b3Body_SetAwake`, `b3Body_GetWorldPoint`, `b3Body_GetLocalPointVelocity`, `b3Body_GetWorldPointVelocity`, `b3World_Explode` | 🔧 `setBodyMassData` exists. Need `getBodyMass` (not exposed), `getLocalRotationalInertia` (not exposed), `bodySetTransform` exists, `setBodyAwake` exists, `worldExplode` — not in WASM. |
| **Disable** | [ ] | `b3Body_Enable`, `b3Body_Disable`, `b3Body_IsEnabled`, `b3Body_ApplyLinearImpulseToCenter`, `b3CreateWeldJoint`, `b3DefaultWeldJointDef` | 🚧 Need `createWeldJoint` + weld joint def. Enable/disable APIs exist. |
| **Cast** | [ ] | `b3Body_CastRay`, `b3Body_CastShape`, `b3Body_OverlapShape`, `b3Body_CollideMover`, `b3CreateCylinder` | 🚧 Needs body-level cast/overlap/collide APIs. Low-level query APIs not yet wrapped. |
| **Kinematic** | [ ] | `b3Body_SetTargetTransform`, `b3BodyDef.type = kinematic` | 🔧 `setBodyTargetTransform` exists. Kinematic body type exists. |
| **Lock Mixing** | [ ] | `bodyDef.motionLocks.angularX/Y/Z`, `bodyDef.motionLocks.linearX/Y/Z` | 🔧 `setBodyMotionLocks` exists, can set at body creation via `bodyDef.motionLocks`? Actually in TS we use `setBodyMotionLocks` after creation. |
| **Fixed Rotation** | [ ] | `bodyDef.motionLocks.angularX/Y/Z`, `bodyDef.gravityScale` | 🔧 All exist. |

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
| **Spheres** | [ ] | `b3CreateCompound` with spheres, `b3CreateCompoundShape`, `b3DestroyCompound` | 🔧 `createCompoundFromSpheres` exists. |
| **Hulls** | [ ] | `b3CreateCompound` with multiple hulls | 🔧 `createCompoundFromHulls` exists. |
| **Tile Floor** | [ ] | Compound with many hull instances | 🔧 Same API, just many entries. |
| **Mesh Tile** | [ ] | `b3CreateMesh`, `b3CreateCompound` with meshes | 🚧 Mesh compound not wrapped. |
| **Village** | [ ] | Compound with hulls + capsules + spheres + meshes, mesh loading | 🧩🚧 Most complex compound sample. |

## Continuous (`sample_continuous.cpp`)

| Sample | TS | APIs needed | Notes |
|--------|----|-------------|-------|
| **Thin Wall** | [ ] | CCD via `bodyDef.isBullet`, fast-moving bodies | 🔧 `isBullet` exists (sets CCD). Simple scene. |
| **Bounce House** | [ ] | CCD + restitution | 🔧 Should work with existing APIs. |
| **Spinning Stick** | [ ] | CCD + thin fast-spinning body | 🔧 Maybe works. |
| **Bullet vs Stack** | [ ] | CCD bullet + stack | 🔧 Projectile system already handles similar. |
| **Needle Mesh** | [ ] | CCD + mesh shape | 🧩 Needs mesh. |
| **Mesh Drop** | [ ] | Mesh + CCD | 🧩 |
| **Mesh Drop Unit Test** | [ ] | Same | 🧩 |
| **Hump Mesh** | [ ] | Mesh + CCD | 🧩 |
| **Is Fast** | [ ] | `b3Body_IsFast` | 🚧 Not exposed. Simple check. |
| **Stall** | [ ] | CCD stall behavior | 🔧 Maybe works. |

## Determinism (`sample_determinism.cpp`)

| Sample | TS | APIs needed | Notes |
|--------|----|-------------|-------|
| **Falling Ragdolls** | [ ] | `b3CreateHuman`, ragdoll creation | 🔧 `createHuman` exists. |

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
| **Box Hull** | [ ] | `b3MakeBoxHull`, hull debug draw | 🔧 Trivial. |
| **Hull** | [ ] | `b3CreateHull` from points, hull debug draw | 🔧 `createHullFromPoints` exists. |
| **Hull Reduction** | [ ] | `b3CreateHull` with many points → reduction | 🔧 Maybe works. |
| **Hull Transform** | [ ] | `b3MakeTransformedBoxHull`, hull transform debug | 🔧 Most APIs exist. |
| **Capsule Mass** | [ ] | `b3Body_GetMass`, `b3Body_GetLocalRotationalInertia`, `b3Body_SetMassData` | 🚧 `getMass`, `getLocalRotationalInertia` not exposed. |

## Issues (`sample_issues.cpp`)

| Sample | TS | APIs needed | Notes |
|--------|----|-------------|-------|
| **Dump Loader** | [ ] | World dump/load | 🚧 |
| **Crash** | [ ] | Regression test scene | 🔧 Simple bodies. |
| **Multiple Prismatic** | [ ] | `b3CreatePrismaticJoint` | 🚧 Prismatic joint not wrapped. |
| **Hull Crash** | [ ] | Hull creation | 🔧 Should work. |
| **Convex Jitter** | [ ] | Hull creation + stacking | 🔧 Should work. |
| **s&box mover** | [ ] | Mover/character system | 🚧 |
| **Capsule Mesh** | [ ] | Capsule + mesh collision | 🧩 |

## Joints (`sample_joint.cpp`)

| Sample | TS | APIs needed | Notes |
|--------|----|-------------|-------|
| **Distance Joint** | [ ] | `b3CreateDistanceJoint`, `b3DefaultDistanceJointDef` | 🚧 Not in WASM. |
| **Filter** | [ ] | `b3CreateFilterJoint` | 🔧 `createFilterJoint` exists! |
| **Motor Joint** | [ ] | `b3CreateMotorJoint` | 🔧 `createMotorJoint` exists. |
| **Top Down Friction** | [ ] | Motor joint + friction | 🔧 Should work. |
| **Prismatic** | [ ] | `b3CreatePrismaticJoint` | 🚧 Not in WASM. |
| **Spherical** | [ ] | `b3CreateSphericalJoint` | 🔧 `createSphericalJoint` exists. |
| **Parallel Spring** | [ ] | Distance joint with spring params | 🚧 |
| **Revolute** | [ ] | `b3CreateRevoluteJoint` | 🔧 `createRevoluteJoint` exists. |
| **Weld** | [ ] | `b3CreateWeldJoint` | 🚧 Not in WASM. |
| **Wheel** | [ ] | `b3CreateWheelJoint` | 🚧 Not in WASM. |
| **Ball and Chain** | [ ] | Spherical joint chain | 🔧 Should work with spherical joint. |
| **Door** | [ ] | Revolute joint with limit | 🔧 Should work. |
| **Bridge** | [ ] | Revolute joint chain | 🔧 Should work. |
| **Motion Locks** | [ ] | `b3Body_SetMotionLocks` | 🔧 Exists. |
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
| **Box** | [ ] | `b3CreateHuman` + drop on box | 🔧 `createHuman` exists. |
| **Mesh** | [ ] | Human + mesh floor | 🧩 |
| **Pile** | [ ] | Multiple humans piling | 🔧 Should work. |
| **Incline** | [ ] | Human + inclined ramp | 🔧 Should work. |
| **Pose** | [ ] | Human posing | 🔧 |

## Robustness (`sample_robustness.cpp`)

| Sample | TS | APIs needed | Notes |
|--------|----|-------------|-------|
| **HighMassRatio1** | [ ] | High density ratios | 🔧 Simple stacking. |
| **Tiny Pyramid** | [ ] | Tiny scale pyramid | 🔧 Simple scene. |
| **Overlap Recovery** | [ ] | Bodies starting in overlap | 🔧 Should work. |
| **Overflow Color Pile** | [ ] | Many bodies + color debug | 🔧 May hit buffer limits. |

## Shapes (`sample_shapes.cpp`)

| Sample | TS | APIs needed | Notes |
|--------|----|-------------|-------|
| **Inclined Plane** | [x] | Standard APIs | Implemented. |
| **Rolling Resistance** | [ ] | `shapeDef.baseMaterial.rollingResistance`, spheres + capsules on plane | 🔧 All exist. |
| **High Resistance** | [ ] | High rolling resistance capsules | 🔧 All exist. |
| **Isotropic Friction** | [ ] | Friction sweep with boxes on circle | 🔧 All exist. |
| **Slide Twist** | [ ] | Friction + twisting | 🔧 All exist. |
| **Restitution** | [ ] | Bounciness sweep | 🔧 All exist. |
| **Static Invoke** | [ ] | `shapeDef.invokeContactCreation` | 🔧 `invokeContactCreation` not exposed. |
| **Conveyor Belt** | [ ] | `shapeDef.baseMaterial.tangentVelocity` | 🔧 `tangentVelocity` exists in `ShapeDef`. |
| **Conveyor Mesh** | [ ] | Mesh + tangent velocities per-material | 🧩🚧 Mesh + material per triangle. |
| **Wind** | [ ] | `b3Shape_ApplyWind`, joints | 🔧 `applyShapeWind` exists. Joints exist. |
| **Wind Drop** | [ ] | `b3Shape_ApplyWind` on single shape | 🔧 Simple. |
| **Wind Flap** | [ ] | Wind + revolute joints + spring | 🔧 All exist. |

## Stacking (`sample_stacking.cpp`)

| Sample | TS | APIs needed | Notes |
|--------|----|-------------|-------|
| **Card House Thick** | [x] | Standard stacking | Implemented. |
| **Card House** | [ ] | Thin card stacking (tiny thickness hull) | 🔧 All exist. |
| **Sphere Stack** | [x] | Sphere stacking | Implemented. |
| **Capsule Stack** | [x] | Capsule stacking with motion locks | Implemented. |
| **Single Box** | [x] | Single dynamic box | Implemented. |
| **Cylinder** | [x] | Cylinder hull | Implemented (offset fix done). |
| **Cylinder Stack** | [ ] | `b3CreateTransformedHullShape` with scaling | 🔧 `createTransformedHullShape` exists. |
| **Box Stack** | [x] | Box stacking | Implemented. |
| **Jenga Stack** | [x] | Jenga tower | Implemented. |
| **Dominoes** | [x] | Domino ring | Implemented. |
| **Wedge** | [ ] | `b3CreateHull` from custom points | 🔧 `createHullFromPoints` exists. |
| **Arch** | [ ] | `b3CreateHull` from custom points, per-body hulls | 🔧 All exist. |
| **Double Domino** | [ ] | Domino row with impulse | 🔧 Simple. |
| **Pyramid2D** | [x] | 2D pyramid stacking | Implemented. |

## Tree (`sample_tree.cpp`)

| Sample | TS | APIs needed | Notes |
|--------|----|-------------|-------|
| **Benchmark** | [ ] | Dynamic tree internals debug | 🚧 Not applicable to web. |

## World (`sample_world.cpp`)

| Sample | TS | APIs needed | Notes |
|--------|----|-------------|-------|
| **Far Stack** | [ ] | Stack far from origin | 🔧 Simple — just offset coordinates. |
| **Far Pyramid** | [ ] | Pyramid far from origin | 🔧 Simple. |
| **Far Ragdolls** | [ ] | Ragdolls far from origin | 🔧 Simple. |
| **Far Mesh Drop** | [ ] | Mesh + far origin | 🧩 |

## Benchmark (`sample_benchmark.cpp`)

| Sample | TS | APIs needed | Notes |
|--------|----|-------------|-------|
| **Large Pyramid** | [ ] | Many box pyramid | 🔧 Simple but heavy. |
| **Wide Pyramid** | [ ] | Wide pyramid | 🔧 |
| **Many Pyramids** | [ ] | Multiple pyramids | 🔧 |
| **Rain** | [ ] | Many falling spheres | 🔧 |
| **Large World** | [ ] | Large world scale | 🔧 |
| **Joint Grid** | [ ] | Grid of joints | 🔧 Uses joints. |
| **Falling Boxes** | [ ] | Many boxes | 🔧 |
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
- **TS implemented (matching C++)**: 13
- **TS implemented (TS-only)**: 2 (dominoes variant, washer variant, material-dedup)
- **Easy next ports** (all APIs exist, simple scenes):
  1. Bodies / Spinning Book
  2. Bodies / Fixed Rotation
  3. Bodies / Lock Mixing
  4. Bodies / Kinematic
  5. Stacking / Cylinder Stack
  6. Stacking / Wedge
  7. Stacking / Arch
  8. Stacking / Double Domino
  9. Stacking / Card House
  10. Shapes / Rolling Resistance
  11. Shapes / Restitution
  12. Shapes / Isotropic Friction
  13. Compound / Spheres
  14. Compound / Hulls
