import * as THREE from "three";
import { B3_AXIS_X, B3_AXIS_Y, B3_AXIS_Z, B3_PI, WorldCastMode, WorldCastType, type Box3DRuntime, type Vec3 } from "box3d-wasm";
import {
  createDebugLine,
  createDebugPoint,
  disposeDebugObject,
  updateDebugLine,
  updateDebugPoint,
} from "../debug-overlay";
import { meshFor } from "../generic-host";
import { capsuleMesh } from "../shared";
import type { DemoSample } from "../types";
import { buildShapeCastScene, createShapeCastBodies, shapeCastCamera } from "./shape-cast-scene";

const TRANSLATION: Vec3 = [0, 0, 10];
const SPHERE_RADIUS = 0.3;
const CAPSULE_RADIUS = 0.2;
const HULL_SIZE = 0.6;

function add(a: Vec3, b: Vec3): Vec3 {
  return [a[0] + b[0], a[1] + b[1], a[2] + b[2]];
}

function along(origin: Vec3, fraction: number): Vec3 {
  return [
    origin[0] + fraction * TRANSLATION[0],
    origin[1] + fraction * TRANSLATION[1],
    origin[2] + fraction * TRANSLATION[2],
  ];
}

export const shapeCastSample: DemoSample = {
  id: "collision/shape-cast",
  name: "Collision / Shape Cast",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, 0, 0], workerCount: 1 });
    const mesh = world.createTorusMesh(10, 12, 0.65, 0.35);
    buildShapeCastScene(world, runtime, mesh);
    const targets: THREE.Object3D[] = [];
    for (const body of createShapeCastBodies(runtime)) {
      const visual = meshFor(body);
      scene.add(visual);
      targets.push(visual);
    }
    const grid = new THREE.GridHelper(10, 10, 0x4b5563, 0x4b5563);
    scene.add(grid);
    const axes = new THREE.AxesHelper(1);
    scene.add(axes);

    const offset: Vec3 = [0, 0, 0];
    let initialOverlap = false;
    const qx = runtime.makeQuatFromAxisAngle(B3_AXIS_X, 0.25 * B3_PI);
    const qy = runtime.makeQuatFromAxisAngle(B3_AXIS_Y, 0.25 * B3_PI);
    const qz = runtime.makeQuatFromAxisAngle(B3_AXIS_Z, 0.25 * B3_PI);
    const hullQuat = runtime.mulQuat(qx, runtime.mulQuat(qy, qz));

    const spheresStart = Array.from({ length: 4 }, () => {
      const sphere = new THREE.Mesh(
        new THREE.SphereGeometry(SPHERE_RADIUS, 16, 12),
        new THREE.MeshStandardMaterial({ color: 0x22c55e, roughness: 0.6, transparent: true, opacity: 0.7 }),
      );
      scene.add(sphere);
      return sphere;
    });
    const spheresEnd = Array.from({ length: 4 }, () => {
      const sphere = new THREE.Mesh(
        new THREE.SphereGeometry(SPHERE_RADIUS, 16, 12),
        new THREE.MeshStandardMaterial({ color: 0x9ca3af, roughness: 0.6, transparent: true, opacity: 0.7 }),
      );
      scene.add(sphere);
      return sphere;
    });
    const capsulesStart = Array.from({ length: 4 }, () => {
      const capsule = capsuleMesh(CAPSULE_RADIUS, 1, 0x22c55e, 0.6, "y");
      scene.add(capsule);
      return capsule;
    });
    const capsulesEnd = Array.from({ length: 4 }, () => {
      const capsule = capsuleMesh(CAPSULE_RADIUS, 1, 0x9ca3af, 0.6, "y");
      scene.add(capsule);
      return capsule;
    });
    const hullsStart = Array.from({ length: 4 }, () => {
      const hull = new THREE.Mesh(
        new THREE.BoxGeometry(HULL_SIZE, HULL_SIZE, HULL_SIZE),
        new THREE.MeshStandardMaterial({ color: 0x22c55e, roughness: 0.6, transparent: true, opacity: 0.45 }),
      );
      hull.quaternion.set(hullQuat[0], hullQuat[1], hullQuat[2], hullQuat[3]);
      scene.add(hull);
      return hull;
    });
    const hullsEnd = Array.from({ length: 4 }, () => {
      const hull = new THREE.Mesh(
        new THREE.BoxGeometry(HULL_SIZE, HULL_SIZE, HULL_SIZE),
        new THREE.MeshStandardMaterial({ color: 0x9ca3af, roughness: 0.6, transparent: true, opacity: 0.45 }),
      );
      hull.quaternion.set(hullQuat[0], hullQuat[1], hullQuat[2], hullQuat[3]);
      scene.add(hull);
      return hull;
    });
    const hitPoints = Array.from({ length: 12 }, () => createDebugPoint(scene, 0xef4444, 6));
    const hitNormals = Array.from({ length: 12 }, () => createDebugLine(scene, 0xeab308));

    const setPair = (
      start: THREE.Object3D,
      end: THREE.Object3D,
      origin: Vec3,
      hitIndex: number,
      type: typeof WorldCastType.Sphere | typeof WorldCastType.Capsule | typeof WorldCastType.Box,
      radius: number,
      startOffset: Vec3,
    ) => {
      start.position.set(origin[0] + startOffset[0], origin[1] + startOffset[1], origin[2] + startOffset[2]);
      const hits = world.worldCast({
        mode: WorldCastMode.Closest,
        type,
        origin,
        translation: TRANSLATION,
        radius,
        initialOverlap,
      });
      const hit = hits[0];
      const fraction = hit?.fraction ?? 1;
      const endPos = along(origin, fraction);
      end.position.set(endPos[0] + startOffset[0], endPos[1] + startOffset[1], endPos[2] + startOffset[2]);
      const endMat = (end as THREE.Mesh).material as THREE.MeshStandardMaterial;
      endMat.color.setHex(hit !== undefined ? 0xef4444 : 0x9ca3af);
      const marker = hitPoints[hitIndex]!;
      const normal = hitNormals[hitIndex]!;
      if (hit !== undefined) {
        marker.visible = true;
        normal.visible = true;
        updateDebugPoint(marker, hit.point);
        updateDebugLine(normal, hit.point, [
          hit.point[0] + 0.2 * hit.normal[0],
          hit.point[1] + 0.2 * hit.normal[1],
          hit.point[2] + 0.2 * hit.normal[2],
        ]);
      } else {
        marker.visible = false;
        normal.visible = false;
      }
    };

    return {
      world,
      bodies: [],
      camera: shapeCastCamera,
      info: "Offset sliders move the cast start (C++ Shift/Ctrl+LMB drag)",
      controls: [
        { key: "initial-overlap", label: "Initial Overlap", type: "toggle", value: false, onChange: (v) => { if (typeof v === "boolean") initialOverlap = v; } },
        { key: "offset-y", label: "Offset Y", type: "range", min: -8, max: 8, step: 0.05, value: 0, onChange: (v) => { if (typeof v === "number") offset[1] = v; } },
        { key: "offset-z", label: "Offset Z", type: "range", min: -8, max: 8, step: 0.05, value: 0, onChange: (v) => { if (typeof v === "number") offset[2] = v; } },
      ],
      step() {
        world.step();
        for (let i = 0; i < 4; i++) {
          const x = -6 + 4 * i;
          setPair(spheresStart[i]!, spheresEnd[i]!, add([x, 3, -5], offset), i, WorldCastType.Sphere, SPHERE_RADIUS, [0, 0, 0]);
          setPair(capsulesStart[i]!, capsulesEnd[i]!, add([x, 5, -5], offset), 4 + i, WorldCastType.Capsule, CAPSULE_RADIUS, [0, 0.5, 0]);
          setPair(hullsStart[i]!, hullsEnd[i]!, add([x, 7, -5], offset), 8 + i, WorldCastType.Box, 0.3, [0, 0, 0]);
        }
      },
      dispose() {
        for (const visual of targets) {
          scene.remove(visual);
          visual.traverse((child) => {
            const mesh = child as THREE.Mesh;
            mesh.geometry?.dispose();
            const material = mesh.material;
            if (material === undefined) return;
            if (Array.isArray(material)) material.forEach((entry) => entry.dispose());
            else material.dispose();
          });
        }
        scene.remove(grid);
        grid.geometry.dispose();
        const gridMat = grid.material;
        if (Array.isArray(gridMat)) gridMat.forEach((entry) => entry.dispose());
        else gridMat.dispose();
        scene.remove(axes);
        axes.dispose();
        for (const mesh of [...spheresStart, ...spheresEnd, ...capsulesStart, ...capsulesEnd, ...hullsStart, ...hullsEnd]) {
          scene.remove(mesh);
          mesh.geometry.dispose();
          (mesh.material as THREE.Material).dispose();
        }
        for (const object of [...hitPoints, ...hitNormals]) disposeDebugObject(scene, object);
        world.destroyMesh(mesh);
        world.destroy();
      },
    };
  },
};
