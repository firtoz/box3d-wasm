import * as THREE from "three";
import { B3_AXIS_X, B3_AXIS_Y, B3_AXIS_Z, B3_DEG_TO_RAD, type Box3DRuntime, type HullHandle, type Quat, type Vec3 } from "box3d-wasm";
import type { DemoSample } from "../types";
import { boxHullCamera } from "./box-hull-scene";
import { disposeHullMesh, hullMeshFromHandle } from "./hull-draw";

const SIGNS: ReadonlyArray<Vec3> = [
  [1, 1, 1], [1, 1, -1], [1, -1, 1], [1, -1, -1],
  [-1, 1, 1], [-1, 1, -1], [-1, -1, 1], [-1, -1, -1],
];

export const boxHullSample: DemoSample = {
  id: "geometry/box-hull",
  name: "Geometry / Box Hull",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -10, 0] });
    const half: Vec3 = [1, 0.5, 0.25];
    const center: Vec3 = [0, 0, 0];
    const rotationDeg: Vec3 = [0, 0, 0];
    const postScale: Vec3 = [1, 1, 1];
    let pointHull: HullHandle | null = null;
    let boxHull: HullHandle | null = null;
    let yellow: THREE.Object3D | null = null;
    let cyan: THREE.Object3D | null = null;
    const axes = new THREE.AxesHelper(1);
    scene.add(axes);

    const rotationQuat = (): Quat => {
      const qx = runtime.makeQuatFromAxisAngle(B3_AXIS_X, B3_DEG_TO_RAD * rotationDeg[0]);
      const qy = runtime.makeQuatFromAxisAngle(B3_AXIS_Y, B3_DEG_TO_RAD * rotationDeg[1]);
      const qz = runtime.makeQuatFromAxisAngle(B3_AXIS_Z, B3_DEG_TO_RAD * rotationDeg[2]);
      return runtime.mulQuat(qz, runtime.mulQuat(qy, qx));
    };

    const rebuild = () => {
      disposeHullMesh(scene, yellow);
      disposeHullMesh(scene, cyan);
      yellow = null;
      cyan = null;
      if (pointHull !== null) runtime.destroyHull(pointHull);
      if (boxHull !== null) runtime.destroyHull(boxHull);
      const q = rotationQuat();
      const points: number[] = [];
      for (const sign of SIGNS) {
        const local: Vec3 = [sign[0] * half[0], sign[1] * half[1], sign[2] * half[2]];
        const rotated = runtime.rotateVector(q, local);
        points.push(postScale[0] * (rotated[0] + center[0]), postScale[1] * (rotated[1] + center[1]), postScale[2] * (rotated[2] + center[2]));
      }
      pointHull = runtime.createHullFromPoints(points);
      boxHull = runtime.makeScaledBoxHull(half, { position: center, rotation: q }, postScale);
      yellow = hullMeshFromHandle(runtime, pointHull, 0xeab308);
      cyan = hullMeshFromHandle(runtime, boxHull, 0x22d3ee);
      scene.add(yellow);
      scene.add(cyan);
    };
    rebuild();

    return {
      world,
      bodies: [],
      camera: boxHullCamera,
      info: "yellow point hull vs cyan scaled box hull",
      controls: [
        { key: "hx", label: "h x", type: "range", min: 0.1, max: 2, step: 0.1, value: half[0], onChange: (v) => { if (typeof v === "number") { half[0] = v; rebuild(); } } },
        { key: "hy", label: "h y", type: "range", min: 0.1, max: 2, step: 0.1, value: half[1], onChange: (v) => { if (typeof v === "number") { half[1] = v; rebuild(); } } },
        { key: "hz", label: "h z", type: "range", min: 0.1, max: 2, step: 0.1, value: half[2], onChange: (v) => { if (typeof v === "number") { half[2] = v; rebuild(); } } },
        { key: "cx", label: "c x", type: "range", min: -2, max: 2, step: 0.1, value: center[0], onChange: (v) => { if (typeof v === "number") { center[0] = v; rebuild(); } } },
        { key: "cy", label: "c y", type: "range", min: -2, max: 2, step: 0.1, value: center[1], onChange: (v) => { if (typeof v === "number") { center[1] = v; rebuild(); } } },
        { key: "cz", label: "c z", type: "range", min: -2, max: 2, step: 0.1, value: center[2], onChange: (v) => { if (typeof v === "number") { center[2] = v; rebuild(); } } },
        { key: "rx", label: "r x", type: "range", min: -180, max: 180, step: 1, value: 0, onChange: (v) => { if (typeof v === "number") { rotationDeg[0] = v; rebuild(); } } },
        { key: "ry", label: "r y", type: "range", min: -180, max: 180, step: 1, value: 0, onChange: (v) => { if (typeof v === "number") { rotationDeg[1] = v; rebuild(); } } },
        { key: "rz", label: "r z", type: "range", min: -180, max: 180, step: 1, value: 0, onChange: (v) => { if (typeof v === "number") { rotationDeg[2] = v; rebuild(); } } },
        { key: "sx", label: "s x", type: "range", min: -2, max: 2, step: 0.1, value: 1, onChange: (v) => { if (typeof v === "number") { postScale[0] = v; rebuild(); } } },
        { key: "sy", label: "s y", type: "range", min: -2, max: 2, step: 0.1, value: 1, onChange: (v) => { if (typeof v === "number") { postScale[1] = v; rebuild(); } } },
        { key: "sz", label: "s z", type: "range", min: -2, max: 2, step: 0.1, value: 1, onChange: (v) => { if (typeof v === "number") { postScale[2] = v; rebuild(); } } },
        { key: "refresh", label: "Refresh", type: "button", onClick: () => rebuild() },
      ],
      step() {},
      dispose() {
        disposeHullMesh(scene, yellow);
        disposeHullMesh(scene, cyan);
        if (pointHull !== null) runtime.destroyHull(pointHull);
        if (boxHull !== null) runtime.destroyHull(boxHull);
        scene.remove(axes);
        axes.dispose();
        world.destroy();
      },
    };
  },
};
