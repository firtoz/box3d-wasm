import { boxStackSample } from "./box-stack";
import { capsuleStackSample } from "./capsule-stack";
import { cardHouseThickSample } from "./card-house-thick";
import { compoundSimpleSample } from "./compound-simple";
import { compoundMaterialDedupSample } from "./compound-material-dedup";
import { cylinderSample } from "./cylinder-sample";
import { dominoesSample, createDominoesSample } from "./dominoes";
import { createWasherSample } from "./washer";
import { jengaStackSample } from "./jenga-stack";
import { pyramid2dSample } from "./pyramid2d";
import { shapesInclinedPlaneSample } from "./shapes-inclined-plane";
import { singleBoxSample } from "./stacking-single-box";
import { sphereStackSample } from "./sphere-stack";
export { type ControlSpec, type DemoBody, type DemoSample, type DemoSampleInstance, type SampleId } from "./types";

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
