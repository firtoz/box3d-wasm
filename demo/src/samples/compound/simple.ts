import { B3_AXIS_Y, BodyType, quatFromAxisAngle } from "box3d-wasm";
import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";

const spec: RenderSpec = {
  groundSize: [40, 2, 40],
  bodies: [
    { kind: "box", size: [8, 1, 8], position: [3, -1.5, 0], rotation: quatFromAxisAngle(B3_AXIS_Y, Math.PI / 4), color: 0x223047, type: BodyType.Static },
    { kind: "sphere", radius: 0.25, position: [0, 2, 0], color: 0xf59e0b },
  ],
  camera: { position: [27.55, 22.5, 27.55], target: [0, 0, 0] },
  launchSpeed: 1,
  info: "compound with hull shape (C++ SimpleCompound)",
};

export const compoundSimpleSample = createGenericSample("compound-simple", "Compound / Simple", spec, () => new Worker(new URL("./simple.worker.ts", import.meta.url), { type: "module" }));
