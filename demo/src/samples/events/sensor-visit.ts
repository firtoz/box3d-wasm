import * as THREE from "three";
import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import {
  sensorVisitBodies,
  sensorVisitCamera,
  sensorVisitGroundSize,
} from "./sensor-visit-scene";

const half = sensorVisitGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  // Upstream uses DrawGroundGrid(10) only; no physics ground is created.
  groundKind: "none",
  bodies: sensorVisitBodies,
  camera: sensorVisitCamera,
  info: "sensor box destroys the visitor on begin-touch",
  overlay: (scene) => {
    const groundGrid = new THREE.GridHelper(10, 10, 0x4b5563, 0x4b5563);
    scene.add(groundGrid);
    return {
      update() {},
      dispose() {
        scene.remove(groundGrid);
        groundGrid.geometry.dispose();
        const material = groundGrid.material;
        if (Array.isArray(material)) material.forEach((m) => m.dispose());
        else material.dispose();
      },
    };
  },
};

export const sensorVisitSample = createGenericSample(
  "events/sensor-visit",
  "Events / Sensor Visit",
  spec,
  () => new Worker(new URL("./sensor-visit.worker.ts", import.meta.url), { type: "module" }),
);
