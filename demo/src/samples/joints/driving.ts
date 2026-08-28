import * as THREE from "three";
import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import type { DemoSample } from "../types";
import { disposeObject3D } from "../grid-mesh-visual";
import { f32 } from "../f32";
import {
  DRIVING_LOWER_STEERING_DEG,
  DRIVING_LOWER_SUSPENSION,
  DRIVING_MAX_SPIN_TORQUE,
  DRIVING_MAX_STEERING_TORQUE,
  DRIVING_SPIN_SPEED,
  DRIVING_STEERING_DAMPING,
  DRIVING_STEERING_HERTZ,
  DRIVING_SUSPENSION_DAMPING,
  DRIVING_SUSPENSION_HERTZ,
  DRIVING_UPPER_STEERING_DEG,
  DRIVING_UPPER_SUSPENSION,
  drivingBodies,
  drivingCamera,
  drivingGroundSize,
  drivingHeightFieldVisual,
} from "./driving-scene";

function createWaveHeightFieldVisual(scene: THREE.Scene): THREE.Group {
  const { rowCount, columnCount, scale, rowFrequency, columnFrequency, position } = drivingHeightFieldVisual;
  const positions = new Float32Array(rowCount * columnCount * 3);
  let cursor = 0;
  for (let row = 0; row < rowCount; row++) {
    const rowHeight = f32(Math.sin(f32(2 * Math.PI * rowFrequency * row)));
    for (let column = 0; column < columnCount; column++) {
      const columnHeight = f32(Math.sin(f32(2 * Math.PI * columnFrequency * column)));
      positions[cursor++] = f32(column * scale[0]);
      positions[cursor++] = f32(scale[1] * rowHeight * columnHeight);
      positions[cursor++] = f32(row * scale[2]);
    }
  }

  const indices: number[] = [];
  for (let row = 0; row < rowCount - 1; row++) {
    for (let column = 0; column < columnCount - 1; column++) {
      const i1 = row * columnCount + column;
      const i2 = i1 + 1;
      const i3 = i2 + columnCount;
      const i4 = i3 - 1;
      indices.push(i1, i2, i3, i3, i4, i1);
    }
  }

  const geom = new THREE.BufferGeometry();
  geom.setAttribute("position", new THREE.BufferAttribute(positions, 3));
  geom.setIndex(indices);
  geom.computeVertexNormals();

  const fill = new THREE.Mesh(
    geom,
    new THREE.MeshStandardMaterial({
      color: 0x2a2a2a,
      roughness: 0.9,
      side: THREE.DoubleSide,
      flatShading: true,
    }),
  );
  fill.receiveShadow = true;

  const edges = new THREE.LineSegments(
    new THREE.WireframeGeometry(geom),
    new THREE.LineBasicMaterial({ color: 0x64748b }),
  );

  const root = new THREE.Group();
  root.add(fill);
  root.add(edges);
  root.position.set(position[0], position[1], position[2]);
  scene.add(root);
  return root;
}

function hudLine(values: Float32Array | undefined): string | undefined {
  if (values === undefined || values.length < 9) return undefined;
  return [
    `speed = ${values[0]!.toFixed(1)}`,
    `spin speed = ${values[1]!.toFixed(1)}/${values[2]!.toFixed(1)}`,
    `spin torque = ${values[3]!.toFixed(1)}/${values[4]!.toFixed(1)}`,
    `steering degrees = ${values[5]!.toFixed(1)}/${values[6]!.toFixed(1)}`,
    `steering torque = ${values[7]!.toFixed(1)}/${values[8]!.toFixed(1)}`,
  ].join(" | ");
}

const half = drivingGroundSize();

export const drivingSample: DemoSample = {
  id: "joints/driving",
  name: "Joints / Driving",
  create(runtime, scene, solverParams) {
    let waveVisual: THREE.Group | null = null;
    let thirdPerson = false;

    const spec: RenderSpec = {
      groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
      groundKind: "none",
      bodies: drivingBodies,
      camera: drivingCamera,
      drivePad: true,
      info: "WASD / arrows / on-screen pad to drive · T third person",
      getInfo: (workerState) => {
        const buffer = workerState?.extra?.driveHud;
        if (!(buffer instanceof SharedArrayBuffer)) return undefined;
        return hudLine(new Float32Array(buffer));
      },
      overlay: (overlayScene) => {
        waveVisual = createWaveHeightFieldVisual(overlayScene);
        return {
          update() {},
          dispose() {
            if (waveVisual !== null) disposeObject3D(overlayScene, waveVisual);
            waveVisual = null;
          },
        };
      },
      controls: [
        { type: "range", label: "Susp Min", message: { type: "set-suspension-min" }, min: -10, max: 10, step: 0.1, value: DRIVING_LOWER_SUSPENSION },
        { type: "range", label: "Susp Max", message: { type: "set-suspension-max" }, min: -10, max: 10, step: 0.1, value: DRIVING_UPPER_SUSPENSION },
        { type: "range", label: "Susp Hertz", message: { type: "set-suspension-hertz" }, min: 0, max: 10, step: 0.1, value: DRIVING_SUSPENSION_HERTZ },
        { type: "range", label: "Susp Damping", message: { type: "set-suspension-damping" }, min: 0, max: 2, step: 0.1, value: DRIVING_SUSPENSION_DAMPING },
        { type: "range", label: "Max Spin Torque", message: { type: "set-max-spin-torque" }, min: 0, max: 100, step: 1, value: DRIVING_MAX_SPIN_TORQUE },
        { type: "range", label: "Spin Speed", message: { type: "set-spin-speed" }, min: 0, max: 100, step: 1, value: DRIVING_SPIN_SPEED },
        { type: "range", label: "Steer Hertz", message: { type: "set-steering-hertz" }, min: 0, max: 20, step: 0.1, value: DRIVING_STEERING_HERTZ },
        { type: "range", label: "Steer Damping", message: { type: "set-steering-damping" }, min: 0, max: 2, step: 0.1, value: DRIVING_STEERING_DAMPING },
        { type: "range", label: "Steer Torque", message: { type: "set-steering-torque" }, min: 0, max: 20, step: 0.1, value: DRIVING_MAX_STEERING_TORQUE },
        { type: "range", label: "Steer Min Deg", message: { type: "set-steering-min-deg" }, min: -90, max: 0, step: 1, value: DRIVING_LOWER_STEERING_DEG },
        { type: "range", label: "Steer Max Deg", message: { type: "set-steering-max-deg" }, min: 0, max: 90, step: 1, value: DRIVING_UPPER_STEERING_DEG },
        {
          type: "toggle",
          label: "Third Person (T)",
          message: { type: "noop" },
          value: false,
          onHostChange: (value) => {
            thirdPerson = value;
          },
        },
      ],
    };

    const instance = createGenericSample(
      "joints/driving",
      "Joints / Driving",
      spec,
      () => new Worker(new URL("./driving.worker.ts", import.meta.url), { type: "module" }),
    ).create(runtime, scene, solverParams);

    const groundMesh = instance.bodies[0]?.mesh;
    if (groundMesh !== undefined) groundMesh.visible = false;
    const chassis = instance.bodies[1];

    const thirdPersonToggle = instance.controls.find((c) => c.key === "third-person-(t)");
    const innerOnKey = instance.onKey;
    return {
      ...instance,
      onKey(key: string) {
        if (key === "t" || key === "T") {
          thirdPerson = !thirdPerson;
          if (thirdPersonToggle !== undefined) thirdPersonToggle.value = thirdPerson;
        }
        innerOnKey?.(key);
      },
      followTarget: () => {
        if (!thirdPerson || chassis === undefined) return null;
        const p = chassis.mesh.position;
        return [p.x, p.y, p.z];
      },
    };
  },
};
