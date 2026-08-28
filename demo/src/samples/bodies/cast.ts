import * as THREE from "three";
import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import {
  createDebugLine,
  createDebugPoint,
  createWireSphere,
  disposeDebugObject,
  updateDebugLine,
  updateDebugPoint,
} from "../debug-overlay";
import {
  bodyCastBodies,
  bodyCastCamera,
  bodyCastGroundSize,
  bodyCastOverlapCapsule,
  bodyCastOverlapOrigin,
  bodyCastRayOrigin,
  bodyCastRayTranslation,
  bodyCastSphereOrigin,
  bodyCastSphereRadius,
  bodyCastSphereTranslation,
} from "./cast-scene";

const half = bodyCastGroundSize();

const spec: RenderSpec = {
  groundKind: "none",
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: bodyCastBodies,
  camera: bodyCastCamera,
  info: "Body_CastRay / CastShape / OverlapShape against a query transform (C++ BodyCast HUD)",
  overlay: (scene) => {
    const ray = createDebugLine(scene, 0x22d3ee);
    const rayOrigin = createDebugPoint(scene, 0x22c55e);
    const rayEnd = createDebugPoint(scene, 0xef4444);
    const rayHit = createDebugPoint(scene, 0xeab308);
    rayHit.visible = false;
    const sphereLine = createDebugLine(scene, 0xf8fafc);
    const sphereOrigin = createDebugPoint(scene, 0x22c55e);
    const sphereEnd = createDebugPoint(scene, 0xef4444);
    const sphereHit = createDebugPoint(scene, 0xeab308);
    sphereHit.visible = false;
    const sphereProxy = createWireSphere(scene, bodyCastSphereRadius, 0xffffff);
    const overlapLine = createDebugLine(scene, 0x9ca3af);
    const overlapA = createDebugPoint(scene, 0x9ca3af, 8);
    const overlapB = createDebugPoint(scene, 0x9ca3af, 8);
    const overlapAPos: [number, number, number] = [
      bodyCastOverlapOrigin[0] + bodyCastOverlapCapsule.a[0],
      bodyCastOverlapOrigin[1] + bodyCastOverlapCapsule.a[1],
      bodyCastOverlapOrigin[2] + bodyCastOverlapCapsule.a[2],
    ];
    const overlapBPos: [number, number, number] = [
      bodyCastOverlapOrigin[0] + bodyCastOverlapCapsule.b[0],
      bodyCastOverlapOrigin[1] + bodyCastOverlapCapsule.b[1],
      bodyCastOverlapOrigin[2] + bodyCastOverlapCapsule.b[2],
    ];
    updateDebugLine(overlapLine, overlapAPos, overlapBPos);
    updateDebugPoint(overlapA, overlapAPos);
    updateDebugPoint(overlapB, overlapBPos);
    const grid = new THREE.GridHelper(10, 10, 0x4b5563, 0x4b5563);
    scene.add(grid);
    const axes = new THREE.AxesHelper(4);
    axes.position.set(0, 0.1, 0);
    scene.add(axes);

    const rayTip: [number, number, number] = [
      bodyCastRayOrigin[0] + bodyCastRayTranslation[0],
      bodyCastRayOrigin[1] + bodyCastRayTranslation[1],
      bodyCastRayOrigin[2] + bodyCastRayTranslation[2],
    ];
    updateDebugLine(ray, bodyCastRayOrigin, rayTip);
    updateDebugPoint(rayOrigin, bodyCastRayOrigin);
    updateDebugPoint(rayEnd, rayTip);
    const sphereTip: [number, number, number] = [
      bodyCastSphereOrigin[0] + bodyCastSphereTranslation[0],
      bodyCastSphereOrigin[1] + bodyCastSphereTranslation[1],
      bodyCastSphereOrigin[2] + bodyCastSphereTranslation[2],
    ];
    updateDebugLine(sphereLine, bodyCastSphereOrigin, sphereTip);
    updateDebugPoint(sphereOrigin, bodyCastSphereOrigin);
    updateDebugPoint(sphereEnd, sphereTip);
    sphereProxy.position.set(sphereTip[0], sphereTip[1], sphereTip[2]);

    return {
      update({ workerState }) {
        const buffer = workerState?.extra?.bodyCastHud;
        if (!(buffer instanceof SharedArrayBuffer)) return;
        const v = new Float32Array(buffer);
        if (v[0] === 1) {
          updateDebugPoint(rayHit, [v[2]!, v[3]!, v[4]!]);
          rayHit.visible = true;
        } else {
          rayHit.visible = false;
        }
        if (v[8] === 1) {
          const f = v[9]!;
          sphereProxy.position.set(
            bodyCastSphereOrigin[0] + f * bodyCastSphereTranslation[0],
            bodyCastSphereOrigin[1] + f * bodyCastSphereTranslation[1],
            bodyCastSphereOrigin[2] + f * bodyCastSphereTranslation[2],
          );
          (sphereProxy.material as THREE.MeshBasicMaterial).color.setHex(0x22c55e);
          updateDebugPoint(sphereHit, [v[10]!, v[11]!, v[12]!]);
          sphereHit.visible = true;
        } else {
          sphereProxy.position.set(sphereTip[0], sphereTip[1], sphereTip[2]);
          (sphereProxy.material as THREE.MeshBasicMaterial).color.setHex(0xffffff);
          sphereHit.visible = false;
        }
        const overlapColor = v[16] === 1 ? 0x22c55e : 0x9ca3af;
        (overlapLine.material as THREE.LineBasicMaterial).color.setHex(overlapColor);
        (overlapA.material as THREE.PointsMaterial).color.setHex(overlapColor);
        (overlapB.material as THREE.PointsMaterial).color.setHex(overlapColor);
      },
      dispose() {
        disposeDebugObject(scene, ray);
        disposeDebugObject(scene, rayOrigin);
        disposeDebugObject(scene, rayEnd);
        disposeDebugObject(scene, rayHit);
        disposeDebugObject(scene, sphereLine);
        disposeDebugObject(scene, sphereOrigin);
        disposeDebugObject(scene, sphereEnd);
        disposeDebugObject(scene, sphereHit);
        disposeDebugObject(scene, sphereProxy);
        disposeDebugObject(scene, overlapLine);
        disposeDebugObject(scene, overlapA);
        disposeDebugObject(scene, overlapB);
        scene.remove(grid);
        grid.geometry.dispose();
        const gridMaterial = grid.material;
        if (Array.isArray(gridMaterial)) gridMaterial.forEach((material) => material.dispose());
        else gridMaterial.dispose();
        scene.remove(axes);
        axes.dispose();
      },
    };
  },
};

export const bodyCastSample = createGenericSample(
  "bodies/cast",
  "Bodies / Cast",
  spec,
  () => new Worker(new URL("./cast.worker.ts", import.meta.url), { type: "module" }),
);
