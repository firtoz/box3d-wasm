import type { Box3DRuntime } from "box3d-wasm";
import type { Scene } from "three";
import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import type { DemoSample, SolverParams } from "../types";
import {
  createSlideTwistOffCenterBodies,
  slideTwistOffCenterCamera,
  slideTwistOffCenterGroundSize,
} from "./slide-twist-off-center-scene";

const half = slideTwistOffCenterGroundSize();

export const slideTwistOffCenterSample: DemoSample = {
  id: "issues/slide-twist-off-center",
  name: "Issues / Slide Twist Off Center Shape",
  create(runtime: Box3DRuntime, scene: Scene, solverParams: SolverParams) {
    const spec: RenderSpec = {
      groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
      bodies: createSlideTwistOffCenterBodies(runtime),
      camera: slideTwistOffCenterCamera,
      info: "offset hull on inclined plane with spin",
    };
    return createGenericSample(
      "issues/slide-twist-off-center",
      "Issues / Slide Twist Off Center Shape",
      spec,
      () => new Worker(new URL("./slide-twist-off-center.worker.ts", import.meta.url), { type: "module" }),
    ).create(runtime, scene, solverParams);
  },
};
