import { dominoesSample, createDominoesSample } from "./dominoes";
import { createGenericWorkerSample } from "./generic-worker-sample";
import { createWasherSample } from "./washer";
export { type ControlSpec, type DemoBody, type DemoSample, type DemoSampleInstance, type SampleId, type SolverParams } from "./types";

const compoundSimpleSample = createGenericWorkerSample("compound-simple", "Compound / Simple");
const compoundMaterialDedupSample = createGenericWorkerSample("compound-material-dedup", "Compound Material Dedup");
const singleBoxSample = createGenericWorkerSample("single-box", "Stacking / Single Box");
const cylinderSample = createGenericWorkerSample("cylinder", "Stacking / Cylinder");
const sphereStackSample = createGenericWorkerSample("sphere-stack", "Stacking / Sphere Stack");
const boxStackSample = createGenericWorkerSample("box-stack", "Stacking / Box Stack");
const shapesInclinedPlaneSample = createGenericWorkerSample("shapes-inclined-plane", "Shapes / Inclined Plane");
const cardHouseThickSample = createGenericWorkerSample("card-house-thick", "Stacking / Card House Thick");
const jengaStackSample = createGenericWorkerSample("jenga-stack", "Stacking / Jenga Stack");
const pyramid2dSample = createGenericWorkerSample("pyramid2d", "Stacking / Pyramid2D");
const capsuleStackSample = createGenericWorkerSample("capsule-stack", "Stacking / Capsule Stack");

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
