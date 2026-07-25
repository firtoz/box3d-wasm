import type { Box3DRuntime } from "box3d-wasm";
import type { Scene } from "three";
import { createGenericSample } from "./generic-host";
import type { RenderSpec } from "./generic-host";
import type { DemoSample, SolverParams } from "./types";
import {
  createEdgeCrossingBodies,
  edgeCrossingCamera,
  edgeCrossingGroundSize,
} from "./edge-crossing-scene";

const half = edgeCrossingGroundSize();

export const edgeCrossingSample: DemoSample = {
  id: "edge-crossing",
  name: "Stacking / Edge Crossing",
  create(runtime: Box3DRuntime, scene: Scene, solverParams: SolverParams) {
    const spec: RenderSpec = {
      groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
      bodies: createEdgeCrossingBodies(runtime),
      camera: edgeCrossingCamera,
      info: "thin boxes crossing on edges at varied angles",
    };
    return createGenericSample(
      "edge-crossing",
      "Stacking / Edge Crossing",
      spec,
      () => new Worker(new URL("./edge-crossing.worker.ts", import.meta.url), { type: "module" }),
    ).create(runtime, scene, solverParams);
  },
};
