import * as THREE from "three";
import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import type { DemoSample } from "../types";
import {
  CONTACT_DEFAULT_TORQUE,
  contactCamera,
  contactGroundSize,
  createContactBodies,
} from "./contact-scene";

const half = contactGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createContactBodies(),
  camera: contactCamera,
  info: "move using WASD",
  drivePad: true,
  getInfo: (workerState) => {
    const buffer = workerState?.extra?.contactHud;
    if (!(buffer instanceof SharedArrayBuffer)) return "move using WASD";
    const count = new Int32Array(buffer)[0] ?? 1;
    return `move using WASD\nplayer shape count = ${count}`;
  },
  controls: [
    {
      type: "range",
      label: "torque",
      message: { type: "set-torque" },
      min: 0,
      max: 60000,
      step: 100,
      value: CONTACT_DEFAULT_TORQUE,
    },
  ],
};

export const contactEventSample: DemoSample = {
  id: "events/contact",
  name: "Events / Contact",
  create(runtime, scene, solverParams) {
    const instance = createGenericSample(
      "events/contact",
      "Events / Contact",
      spec,
      () => new Worker(new URL("./contact.worker.ts", import.meta.url), { type: "module" }),
    ).create(runtime, scene, solverParams);
    const origStep = instance.step.bind(instance);
    instance.step = (dt?: number, subSteps?: number) => {
      origStep(dt, subSteps);
      for (let i = 2; i < instance.bodies.length; i++) {
        const mesh = instance.bodies[i]!.mesh;
        mesh.visible = (mesh.material as THREE.MeshStandardMaterial).color.getHex() !== 0;
      }
    };
    return instance;
  },
};
