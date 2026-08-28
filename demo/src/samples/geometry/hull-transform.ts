import * as THREE from "three";
import { B3_AXIS_X, B3_AXIS_Y, B3_AXIS_Z, B3_PI, type Box3DRuntime, type HullHandle, type Vec3 } from "box3d-wasm";
import type { DemoSample } from "../types";
import { hullTransformCamera } from "./hull-transform-scene";
import { disposeHullMesh, hullMeshFromHandle } from "./hull-draw";

export const hullTransformSample: DemoSample = {
  id: "geometry/hull-transform",
  name: "Geometry / Hull Transform",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -10, 0] });
    const original = runtime.createCylinder(1, 0.5, 0, 9);
    let hull: HullHandle | null = null;
    let originalMesh: THREE.Object3D | null = null;
    let transformedMesh: THREE.Object3D | null = null;
    const scale: Vec3 = [1, 1, 1];
    const angles: Vec3 = [0, 0, 0];
    const offset: Vec3 = [0, 0, 0];
    let info = "";
    const axes = new THREE.AxesHelper(1);
    scene.add(axes);

    const updateHull = () => {
      disposeHullMesh(scene, originalMesh);
      disposeHullMesh(scene, transformedMesh);
      if (hull !== null) runtime.destroyHull(hull);
      const qx = runtime.makeQuatFromAxisAngle(B3_AXIS_X, angles[0] * B3_PI / 180);
      const qy = runtime.makeQuatFromAxisAngle(B3_AXIS_Y, angles[1] * B3_PI / 180);
      const qz = runtime.makeQuatFromAxisAngle(B3_AXIS_Z, angles[2] * B3_PI / 180);
      const q = runtime.mulQuat(qz, runtime.mulQuat(qy, qx));
      hull = runtime.cloneAndTransformHull(original, { position: offset, rotation: q }, scale);
      originalMesh = hullMeshFromHandle(runtime, original, 0x22c55e);
      originalMesh.position.set(-2, 0, 0);
      transformedMesh = hullMeshFromHandle(runtime, hull, 0xeab308);
      transformedMesh.position.set(2, 0, 0);
      scene.add(originalMesh);
      scene.add(transformedMesh);
      const a = runtime.getHullInfo(original);
      const b = runtime.getHullInfo(hull);
      info = `hull 1: area = ${a.surfaceArea.toPrecision(6)}, volume = ${a.volume.toPrecision(6)}, radius = ${a.innerRadius.toPrecision(6)} | hull 2: area = ${b.surfaceArea.toPrecision(6)}, volume = ${b.volume.toPrecision(6)}, radius = ${b.innerRadius.toPrecision(6)}`;
    };
    updateHull();

    return {
      world,
      bodies: [],
      camera: hullTransformCamera,
      getInfo: () => info,
      controls: [
        { key: "sx", label: "sx", type: "range", min: -2, max: 2, step: 0.1, value: 1, onChange: (v) => { if (typeof v === "number") { scale[0] = v; updateHull(); } } },
        { key: "sy", label: "sy", type: "range", min: -2, max: 2, step: 0.1, value: 1, onChange: (v) => { if (typeof v === "number") { scale[1] = v; updateHull(); } } },
        { key: "sz", label: "sz", type: "range", min: -2, max: 2, step: 0.1, value: 1, onChange: (v) => { if (typeof v === "number") { scale[2] = v; updateHull(); } } },
        { key: "rx", label: "rx", type: "range", min: -180, max: 180, step: 1, value: 0, onChange: (v) => { if (typeof v === "number") { angles[0] = v; updateHull(); } } },
        { key: "ry", label: "ry", type: "range", min: -180, max: 180, step: 1, value: 0, onChange: (v) => { if (typeof v === "number") { angles[1] = v; updateHull(); } } },
        { key: "rz", label: "rz", type: "range", min: -180, max: 180, step: 1, value: 0, onChange: (v) => { if (typeof v === "number") { angles[2] = v; updateHull(); } } },
        { key: "px", label: "px", type: "range", min: -1, max: 1, step: 0.1, value: 0, onChange: (v) => { if (typeof v === "number") { offset[0] = v; updateHull(); } } },
        { key: "py", label: "py", type: "range", min: -1, max: 1, step: 0.1, value: 0, onChange: (v) => { if (typeof v === "number") { offset[1] = v; updateHull(); } } },
        { key: "pz", label: "pz", type: "range", min: -1, max: 1, step: 0.1, value: 0, onChange: (v) => { if (typeof v === "number") { offset[2] = v; updateHull(); } } },
      ],
      step() {},
      dispose() {
        disposeHullMesh(scene, originalMesh);
        disposeHullMesh(scene, transformedMesh);
        if (hull !== null) runtime.destroyHull(hull);
        runtime.destroyHull(original);
        scene.remove(axes);
        axes.dispose();
        world.destroy();
      },
    };
  },
};
