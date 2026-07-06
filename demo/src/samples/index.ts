import { dominoesSample, createDominoesSample } from "./dominoes";
import { createWasherSample } from "./washer";
import { compoundSimpleSample } from "./compound/simple";
import { compoundSpheresSample } from "./compound/spheres";
import { compoundHullsSample } from "./compound/hulls";
import { compoundMaterialDedupSample } from "./compound/material-dedup";
import { singleBoxSample } from "./single/box";
import { cylinderSample } from "./cylinder";
import { cylinderStackSample } from "./cylinder-stack";
import { wedgeSample } from "./wedge";
import { archSample } from "./arch";
import { doubleDominoSample } from "./double-domino";
import { cardHouseSample } from "./card-house";
import { rollingResistanceSample } from "./rolling-resistance";
import { restitutionSample } from "./restitution";
import { isotropicFrictionSample } from "./isotropic-friction";
import { sphereStackSample } from "./sphere/stack";
import { boxStackSample } from "./box/stack";
import { shapesInclinedPlaneSample } from "./shapes/inclined-plane";
import { cardHouseThickSample } from "./card/house-thick";
import { jengaStackSample } from "./jenga/stack";
import { pyramid2dSample } from "./pyramid2d";
import { capsuleStackSample } from "./capsule/stack";
import { spinningBookSample } from "./bodies/spinning-book";
import { fixedRotationSample } from "./bodies/fixed-rotation";
import { lockMixingSample } from "./bodies/lock-mixing";
import { kinematicSample } from "./bodies/kinematic";
import { gyroscopicTorqueSample } from "./bodies/gyroscopic-torque";
import { bodyTypeSample } from "./bodies/body-type";
import { weebleSample } from "./bodies/weeble";
import { disableSample } from "./bodies/disable";
import { boxHullSample } from "./geometry/box-hull";
import { hullSample } from "./geometry/hull";
import { hullReductionSample } from "./geometry/hull-reduction";
import { hullTransformSample } from "./geometry/hull-transform";
import { hullCrashSample } from "./issues/hull-crash";
import { convexJitterSample } from "./issues/convex-jitter";
import { highMassRatio1Sample } from "./robustness/high-mass-ratio-1";
import { tinyPyramidSample } from "./robustness/tiny-pyramid";
import { overlapRecoverySample } from "./robustness/overlap-recovery";
import { overflowColorPileSample } from "./robustness/overflow-color-pile";
import { farStackSample } from "./world/far-stack";
import { farPyramidSample } from "./world/far-pyramid";
import { farRagdollsSample } from "./world/far-ragdolls";
import { filterJointSample } from "./joints/filter";
import { motorJointSample } from "./joints/motor-joint";
import { prismaticJointSample } from "./joints/prismatic";
import { revoluteJointSample } from "./joints/revolute";
import { weldJointSample } from "./joints/weld";
import { topDownFrictionSample } from "./joints/top-down-friction";
import { sphericalJointSample } from "./joints/spherical";
import { ballAndChainSample } from "./joints/ball-and-chain";
import { doorSample } from "./joints/door";
import { bridgeSample } from "./joints/bridge";
export { type ControlSpec, type DemoBody, type DemoSample, type DemoSampleInstance, type SampleId, type SolverParams } from "./types";

export const samples = [
  spinningBookSample,
  fixedRotationSample,
  lockMixingSample,
  kinematicSample,
  gyroscopicTorqueSample,
  bodyTypeSample,
  weebleSample,
  disableSample,
  boxHullSample,
  hullSample,
  hullReductionSample,
  hullTransformSample,
  hullCrashSample,
  convexJitterSample,
  highMassRatio1Sample,
  tinyPyramidSample,
  overlapRecoverySample,
  overflowColorPileSample,
  filterJointSample,
  motorJointSample,
  prismaticJointSample,
  revoluteJointSample,
  weldJointSample,
  topDownFrictionSample,
  sphericalJointSample,
  ballAndChainSample,
  doorSample,
  bridgeSample,
  farStackSample,
  farPyramidSample,
  farRagdollsSample,
  compoundSimpleSample,
  compoundSpheresSample,
  compoundHullsSample,
  compoundMaterialDedupSample,
  singleBoxSample,
  cylinderSample,
  cylinderStackSample,
  wedgeSample,
  archSample,
  doubleDominoSample,
  cardHouseSample,
  rollingResistanceSample,
  restitutionSample,
  isotropicFrictionSample,
  sphereStackSample,
  boxStackSample,
  shapesInclinedPlaneSample,
  dominoesSample,
  cardHouseThickSample,
  jengaStackSample,
  pyramid2dSample,
  capsuleStackSample,
  createDominoesSample(2),
  createWasherSample(),
];
