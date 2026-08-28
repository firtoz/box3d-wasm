import * as THREE from "three";
import { WorldCastMode, WorldCastType, type Box3DRuntime, type Vec3 } from "box3d-wasm";
import {
  createDebugLine,
  createDebugPoint,
  createWireSphere,
  disposeDebugObject,
  updateDebugLine,
  updateDebugPoint,
} from "../debug-overlay";
import type { DemoSample } from "../types";
import { meshScaleCamera } from "./mesh-scale-scene";
import { MESH_SCALE_CENTER, MESH_SCALE_EXTENT } from "./mesh-scale-scene";

export const meshScaleSample: DemoSample = {
  id: "collision/mesh-scale",
  name: "Collision / Mesh Scale",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -10, 0] });
    const scale: Vec3 = [1, 1, 1];
    const start: Vec3 = [-2, 0, 0];
    let sphereCast = true;
    const mesh = world.createBoxMesh(MESH_SCALE_CENTER, MESH_SCALE_EXTENT, true);
    const body = world.createBody({ position: [0, 0, 0] });
    const shape = world.createMeshShape(body, mesh, { scale });
    const box = new THREE.Mesh(
      new THREE.BoxGeometry(1, 1, 1),
      new THREE.MeshStandardMaterial({ color: 0x60a5fa, roughness: 0.75, wireframe: true }),
    );
    scene.add(box);
    const axes = new THREE.AxesHelper(1);
    scene.add(axes);
    const rayLine = createDebugLine(scene, 0xffffff);
    const originPoint = createDebugPoint(scene, 0x22c55e, 8);
    const endPoint = createDebugPoint(scene, 0xef4444, 8);
    const hitPoint = createDebugPoint(scene, 0xeab308, 5);
    const hitNormal = createDebugLine(scene, 0x22c55e);
    const sphere = createWireSphere(scene, 0.25, 0xeab308);

    const applyScale = () => {
      world.setMesh(shape, mesh, scale);
      box.scale.set(scale[0], scale[1], scale[2]);
    };

    return {
      world,
      bodies: [],
      camera: meshScaleCamera,
      controls: [
        { key: "sx", label: "Scale X", type: "range", min: -2, max: 2, step: 0.1, value: 1, onChange: (v) => { if (typeof v === "number") { scale[0] = v; applyScale(); } } },
        { key: "sy", label: "Scale Y", type: "range", min: -2, max: 2, step: 0.1, value: 1, onChange: (v) => { if (typeof v === "number") { scale[1] = v; applyScale(); } } },
        { key: "sz", label: "Scale Z", type: "range", min: -2, max: 2, step: 0.1, value: 1, onChange: (v) => { if (typeof v === "number") { scale[2] = v; applyScale(); } } },
        { key: "start-y", label: "Start Y", type: "range", min: -2, max: 2, step: 0.1, value: 0, onChange: (v) => { if (typeof v === "number") start[1] = v; } },
        { key: "start-z", label: "Start Z", type: "range", min: -2, max: 2, step: 0.1, value: 0, onChange: (v) => { if (typeof v === "number") start[2] = v; } },
        { key: "sphere-cast", label: "sphere Cast", type: "toggle", value: true, onChange: (v) => { if (typeof v === "boolean") sphereCast = v; } },
      ],
      step() {
        world.step();
        const translation: Vec3 = [4, 0, 0];
        const end: Vec3 = [start[0] + 4, start[1], start[2]];
        updateDebugLine(rayLine, start, end);
        updateDebugPoint(originPoint, start);
        updateDebugPoint(endPoint, end);
        let fraction = 1;
        let hit: { point: Vec3; normal: Vec3 } | null = null;
        if (sphereCast) {
          const hits = world.worldCast({
            mode: WorldCastMode.Closest,
            type: WorldCastType.Sphere,
            origin: start,
            translation,
            radius: 0.25,
          });
          const closest = hits[0];
          if (closest !== undefined) {
            fraction = closest.fraction;
            hit = { point: closest.point, normal: closest.normal };
          }
          sphere.visible = true;
          sphere.position.set(fraction * translation[0], start[1] + fraction * translation[1], start[2] + fraction * translation[2]);
          (sphere.material as THREE.MeshBasicMaterial).color.setHex(hit !== null ? 0xeab308 : 0x9ca3af);
        } else {
          sphere.visible = false;
          const result = world.rayCastClosest(start, translation);
          if (result !== null) hit = { point: result.point, normal: result.normal };
        }
        if (hit !== null) {
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
        scene.remove(box);
        box.geometry.dispose();
        (box.material as THREE.Material).dispose();
        scene.remove(axes);
        axes.dispose();
        disposeDebugObject(scene, rayLine);
        disposeDebugObject(scene, originPoint);
        disposeDebugObject(scene, endPoint);
        disposeDebugObject(scene, hitPoint);
        disposeDebugObject(scene, hitNormal);
        disposeDebugObject(scene, sphere);
        world.destroyMesh(mesh);
        world.destroy();
      },
    };
  },
};
