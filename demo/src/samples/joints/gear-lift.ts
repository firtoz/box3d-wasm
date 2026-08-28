import * as THREE from "three";
import type { Box3DRuntime } from "box3d-wasm";
import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import type { DemoSample } from "../types";
import { disposeObject3D } from "../grid-mesh-visual";
import {
  createGearLiftBodies,
  GEAR_LIFT_MOTOR_SPEED,
  GEAR_LIFT_MOTOR_TORQUE,
  gearLiftCamera,
  gearLiftGroundSize,
  gearLiftStairMesh,
} from "./gear-lift-scene";

const half = gearLiftGroundSize();

function createStairVisual(scene: THREE.Scene): THREE.Mesh {
  const { vertices, indices } = gearLiftStairMesh();
  const geom = new THREE.BufferGeometry();
  geom.setAttribute("position", new THREE.BufferAttribute(new Float32Array(vertices), 3));
  geom.setIndex(indices);
  geom.computeVertexNormals();
  const mesh = new THREE.Mesh(
    geom,
    new THREE.MeshStandardMaterial({
      color: 0x8fbc8f,
      roughness: 0.85,
      side: THREE.DoubleSide,
      flatShading: true,
    }),
  );
  mesh.receiveShadow = true;
  scene.add(mesh);
  return mesh;
}

export const gearLiftSample: DemoSample = {
  id: "joints/gear-lift",
  name: "Joints / Gear Lift",
  create(runtime: Box3DRuntime, scene, solverParams) {
    let stairVisual: THREE.Mesh | null = null;

    const spec: RenderSpec = {
      groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
      bodies: createGearLiftBodies(runtime),
      camera: gearLiftCamera,
      info: "meshing gears lift a gate on two capsule chains",
      controls: [
        { type: "toggle", label: "Motor", message: { type: "enable-motor" }, value: true },
        { type: "range", label: "Max Torque", message: { type: "set-max-torque" }, min: 0, max: 100000, step: 100, value: GEAR_LIFT_MOTOR_TORQUE },
        { type: "range", label: "Speed", message: { type: "set-motor-speed" }, min: -0.3, max: 0.3, step: 0.01, value: GEAR_LIFT_MOTOR_SPEED },
      ],
      overlay: (overlayScene) => {
        stairVisual = createStairVisual(overlayScene);
        return {
          update() {},
          dispose() {
            if (stairVisual !== null) disposeObject3D(overlayScene, stairVisual);
            stairVisual = null;
          },
        };
      },
    };

    return createGenericSample(
      "joints/gear-lift",
      "Joints / Gear Lift",
      spec,
      () => new Worker(new URL("./gear-lift.worker.ts", import.meta.url), { type: "module" }),
    ).create(runtime, scene, solverParams);
  },
};
