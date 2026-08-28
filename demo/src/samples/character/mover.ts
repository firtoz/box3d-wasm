import * as THREE from "three";
import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { capsuleMesh } from "../shared";
import { moverBodies, moverCamera, moverGroundSize, moverStart } from "./mover-scene";
import { moverCapsule as capsule } from "./character-mover";

const half = moverGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  groundKind: "none",
  bodies: moverBodies,
  camera: moverCamera,
  info: "WASD mover + jump",
  drivePad: true,
  hotkeys: [
    { keys: [" "], message: { type: "jump" } },
  ],
  controls: [
    { type: "toggle", label: "clip velocity", message: { type: "clip" }, value: true },
  ],
  overlay: (scene) => {
    const mesh = capsuleMesh(capsule.radius, 1, 0x22c55e, 0.85, "y");
    mesh.position.set(...moverStart);
    scene.add(mesh);
    return {
      update({ workerState }) {
        const buffer = workerState?.extra?.mover;
        if (!(buffer instanceof SharedArrayBuffer)) return;
        const values = new Float32Array(buffer);
        mesh.position.set(values[0]!, values[1]!, values[2]!);
      },
      dispose() {
        scene.remove(mesh);
        mesh.geometry.dispose();
        (mesh.material as THREE.Material).dispose();
      },
    };
  },
};

export const moverSample = createGenericSample(
  "character/mover",
  "Character / Mover",
  spec,
  () => new Worker(new URL("./mover.worker.ts", import.meta.url), { type: "module" }),
);
