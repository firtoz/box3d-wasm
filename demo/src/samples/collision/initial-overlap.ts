import * as THREE from "three";
import { WorldCastMode, WorldCastType, type Box3DRuntime, type Vec3 } from "box3d-wasm";
import {
  createDebugLine,
  createDebugPoint,
  disposeDebugObject,
  updateDebugLine,
  updateDebugPoint,
} from "../debug-overlay";
import { capsuleMesh } from "../shared";
import type { DemoSample } from "../types";
import { buildInitialOverlapScene, initialOverlapBodies, initialOverlapCamera } from "./initial-overlap-scene";
import { meshFor } from "../generic-host";

export const initialOverlapSample: DemoSample = {
  id: "collision/initial-overlap",
  name: "Collision / Initial Overlap",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -10, 0] });
    const { mesh } = buildInitialOverlapScene(world, runtime);
    const plate = meshFor(initialOverlapBodies[0]!);
    scene.add(plate);
    const axes = new THREE.AxesHelper(1);
    scene.add(axes);
    let initialOverlap = true;
    const offset: Vec3 = [-2.1, -0.8, 0.95];
    const capsuleA = capsuleMesh(0.25, 1, 0x22c55e, 0.75, "y");
    capsuleA.position.set(offset[0], offset[1] + 0.5, offset[2]);
    scene.add(capsuleA);
    const capsuleB = capsuleMesh(0.25, 1, 0xef4444, 0.75, "y");
    capsuleB.position.copy(capsuleA.position);
    scene.add(capsuleB);
    const hitPoint = createDebugPoint(scene, 0xf0f8ff, 8);
    const hitNormal = createDebugLine(scene, 0xf0f8ff);

    return {
      world,
      bodies: [],
      camera: initialOverlapCamera,
      controls: [
        { key: "initial-overlap", label: "initial overlap", type: "toggle", value: true, onChange: (v) => { if (typeof v === "boolean") initialOverlap = v; } },
      ],
      step() {
        world.step();
        const hits = world.worldCast({
          mode: WorldCastMode.Closest,
          type: WorldCastType.Capsule,
          origin: offset,
          translation: [0, 0, 0],
          radius: 0.25,
          initialOverlap,
        });
        const hit = hits[0];
        capsuleB.visible = true;
        (capsuleB.material as THREE.MeshStandardMaterial).color.setHex(hit !== undefined ? 0xef4444 : 0x22c55e);
        if (hit !== undefined) {
          hitPoint.visible = true;
          hitNormal.visible = true;
          updateDebugPoint(hitPoint, hit.point);
          updateDebugLine(hitNormal, hit.point, [
            hit.point[0] + 0.5 * hit.normal[0],
            hit.point[1] + 0.5 * hit.normal[1],
            hit.point[2] + 0.5 * hit.normal[2],
          ]);
        } else {
          hitPoint.visible = false;
          hitNormal.visible = false;
        }
      },
      dispose() {
        scene.remove(plate);
        plate.geometry.dispose();
        (plate.material as THREE.Material).dispose();
        scene.remove(capsuleA);
        capsuleA.geometry.dispose();
        (capsuleA.material as THREE.Material).dispose();
        scene.remove(capsuleB);
        capsuleB.geometry.dispose();
        (capsuleB.material as THREE.Material).dispose();
        scene.remove(axes);
        axes.dispose();
        disposeDebugObject(scene, hitPoint);
        disposeDebugObject(scene, hitNormal);
        world.destroyMesh(mesh);
        world.destroy();
      },
    };
  },
};
