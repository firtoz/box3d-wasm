import { dominoesSample, createDominoesSample } from "./dominoes";
import { createWasherSample } from "./washer";
import { compoundSimpleSample } from "./compound/simple";
import { compoundMaterialDedupSample } from "./compound/material-dedup";
import { singleBoxSample } from "./single/box";
import { cylinderSample } from "./cylinder";
import { sphereStackSample } from "./sphere/stack";
import { boxStackSample } from "./box/stack";
import { shapesInclinedPlaneSample } from "./shapes/inclined-plane";
import { cardHouseThickSample } from "./card/house-thick";
import { jengaStackSample } from "./jenga/stack";
import { pyramid2dSample } from "./pyramid2d";
import { capsuleStackSample } from "./capsule/stack";
export { type ControlSpec, type DemoBody, type DemoSample, type DemoSampleInstance, type SampleId, type SolverParams } from "./types";

export const samples = [
  compoundSimpleSample,
  compoundMaterialDedupSample,
  singleBoxSample,
  cylinderSample,
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
