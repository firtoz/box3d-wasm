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
| **Gyroscopic Torque** | [x] | `b3CreateCylinder`, `b3CreateHullShape` (multiple on same body), `b3Body_ApplyMassFromShapes`, `b3Body_GetWorldCenter`, `shapeDef.updateBodyMass = false` | 🔧 All APIs exist. Dzhanibekov effect: cylinder + box on gravityScale=0 body with angular velocity. Render uses generic host `compound` parts; box dimensions are doubled from `b3MakeBoxHull` half-extents and cylinder is locally offset by `0.5 * height`. |
| **Weeble** | [x] | `b3Body_GetMass`, `b3Body_GetLocalRotationalInertia`, `b3Body_SetMassData`, `b3Body_SetTransform`, `b3Body_SetAwake`, `b3Body_GetWorldPoint`, `b3Body_GetLocalPointVelocity`, `b3Body_GetWorldPointVelocity`, `b3World_Explode` | 🔧 All APIs now wrapped. Capsule with shifted COM + Teleport/Explode buttons. |
| **Disable** | [x] | `b3Body_Enable`, `b3Body_Disable`, `b3Body_IsEnabled`, `b3Body_ApplyLinearImpulseToCenter`, `b3CreateWeldJoint` | 🔧 All APIs now wrapped. 4-link chain with weld joints + ball, enable/disable toggles. |
| **Cast** | [ ] | `b3Body_CastRay`, `b3Body_CastShape`, `b3Body_OverlapShape`, `b3Body_CollideMover`, `b3CreateCylinder` | 🚧 Needs body-level cast/overlap/collide APIs. Low-level query APIs not yet wrapped. |
| **Kinematic** | [x] | `b3Body_SetTargetTransform`, `b3BodyDef.type = kinematic` | 🔧 `setBodyTargetTransform` exists. Kinematic body type exists. |
| **Lock Mixing** | [x] | `bodyDef.motionLocks.angularX/Y/Z`, `bodyDef.motionLocks.linearX/Y/Z` | 🔧 `setBodyMotionLocks` exists, can set at body creation via `bodyDef.motionLocks`? Actually in TS we use `setBodyMotionLocks` after creation. |
| **Fixed Rotation** | [x] | `bodyDef.motionLocks.angularX/Y/Z`, `bodyDef.gravityScale` | 🔧 All exist. |

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
| **Filter** | [ ] | `b3CreateFilterJoint` | 🔧 `createFilterJoint` exists! |
| **Motor Joint** | [ ] | `b3CreateMotorJoint` | 🔧 `createMotorJoint` exists. |
| **Top Down Friction** | [ ] | Motor joint + friction | 🔧 Should work. |
| **Prismatic** | [ ] | `b3CreatePrismaticJoint` | 🔧 Now in WASM. |
| **Spherical** | [ ] | `b3CreateSphericalJoint` | 🔧 `createSphericalJoint` exists. |
| **Parallel Spring** | [ ] | Distance joint with spring params | 🚧 |
| **Revolute** | [ ] | `b3CreateRevoluteJoint` | 🔧 `createRevoluteJoint` exists. |
| **Weld** | [ ] | `b3CreateWeldJoint` | 🔧 Now in WASM. |
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
| **HighMassRatio1** | [x] | High density ratios | 🔧 Three pyramids with heavy top boxes. |
| **Tiny Pyramid** | [x] | Tiny scale pyramid | 🔧 30-base pyramid of 2.5cm boxes. |
| **Overlap Recovery** | [x] | Bodies starting in overlap | 🔧 25% overlap with contact tuning. |
| **Overflow Color Pile** | [x] | Many bodies + color debug | 🔧 Hub + 24 neighbors for graph color overflow. |

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
| **Sphere Stack** | [x] | Sphere stacking | Implemented; C++/WASM dump parity verified with the default 5-second-or-sleep window. |
| **Capsule Stack** | [x] | Capsule stacking with motion locks | Implemented; C++/WASM dump parity verified with the default 5-second-or-sleep window. |
| **Single Box** | [x] | Single dynamic box | Implemented. |
| **Cylinder** | [x] | Cylinder hull | Implemented (offset fix done); C++/WASM dump parity verified with the default 5-second-or-sleep window. |
| **Cylinder Stack** | [ ] | `b3CreateTransformedHullShape` with scaling | 🔧 `createTransformedHullShape` exists. |
| **Box Stack** | [x] | Box stacking | Implemented; C++/WASM dump parity verified with the default 5-second-or-sleep window. |
| **Jenga Stack** | [x] | Jenga tower | Implemented. |
| **Dominoes** | [x] | Domino ring | Implemented. |
| **Wedge** | [ ] | `b3CreateHull` from custom points | 🔧 `createHullFromPoints` exists. |
| **Arch** | [ ] | `b3CreateHull` from custom points, per-body hulls | 🔧 All exist. |
| **Double Domino** | [ ] | Domino row with impulse | 🔧 Simple. |
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
| **Far Ragdolls** | [x] | Ragdolls far from origin | 🔧 20 ragdolls at 1000km offset. |
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
- **TS implemented (matching C++)**: 33
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
  29. Dominoes
  30. Card / House Thick
  31. Jenga / Stack
  32. Pyramid 2D
  33. Capsule / Stack

- **Easy next ports** (all APIs exist, simple scenes):
  1. Stacking / Cylinder Stack
  2. Stacking / Wedge
  3. Stacking / Arch
  4. Stacking / Double Domino
  5. Stacking / Card House
  6. Shapes / Rolling Resistance
  7. Shapes / Restitution
  8. Shapes / Isotropic Friction
  9. Compound / Spheres
  10. Compound / Hulls
