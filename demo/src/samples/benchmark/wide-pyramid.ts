import * as THREE from "three";
import { BodyType, type BodyHandle, type Box3DRuntime } from "box3d-wasm";
import type { DemoBody, DemoSample, SolverParams } from "../types";
import type { PhysicsWorkerMessage, PhysicsWorkerReady } from "../../physics-worker-protocol";
import { MAX_PROJECTILES, SNAPSHOT_PROJECTILE_COUNT_INDEX, SNAPSHOT_VERSION_INDEX } from "../../physics-worker-protocol";
import { createWorkerWorld, type WorkerWorldState } from "../../worker-world-bridge";
import { RAGDOLL_RENDER_BONES, ragdollCapsuleMesh } from "../../ragdoll-render";
import { bindSnapshotTransforms, createShaderBoxMesh, hexToRgb } from "../../shader-instanced-boxes";
import { wasmBuildVersion } from "virtual:wasm-version";
import { getWasmBaseUrl, getWasmVariant, getWorkerCounts } from "../shared";
import {
  forEachWidePyramidBox,
  WIDE_PYRAMID_BOX_COLOR,
  WIDE_PYRAMID_BOX_COUNT,
  WIDE_PYRAMID_BOX_SIZE,
  widePyramidCamera,
  widePyramidGroundSize,
} from "./wide-pyramid-scene";

export const widePyramidSample: DemoSample = {
  id: "benchmark/wide-pyramid",
  name: "Benchmark / Wide Pyramid",
  create(_runtime: Box3DRuntime, scene: THREE.Scene, initialSolverParams?: SolverParams) {
    const { defaultWorkerCount, maxWorkerCount: maxWorkers, poolSize } = getWorkerCounts();
    let wc = Math.min(maxWorkers, Math.max(1, initialSolverParams?.workerCount ?? defaultWorkerCount));

    let workerWorldState: WorkerWorldState | null = null;
    let bodyCount = WIDE_PYRAMID_BOX_COUNT;
    let positions: Float32Array | null = null;
    let rotations: Float32Array | null = null;
    let state: Int32Array | null = null;
    let projectilePositions: Float32Array | null = null;
    let projectileRotations: Float32Array | null = null;
    let projectileDebugColors: Uint32Array | null = null;
    let lastVersion = -1;
    const projectileMeshes: THREE.Mesh[] = [];
    const projectileColorCache = new Uint32Array(MAX_PROJECTILES);
    const bodies: DemoBody[] = [];

    const worker = new Worker(new URL("./wide-pyramid.worker.ts", import.meta.url), { type: "module" });
    const world = createWorkerWorld(worker, () => workerWorldState, () => wc);

    const half = widePyramidGroundSize();
    const groundGeom = new THREE.BoxGeometry(2 * half[0], 2 * half[1], 2 * half[2]);
    const groundMat = new THREE.MeshStandardMaterial({ color: 0x222222 });
    const groundMesh = new THREE.Mesh(groundGeom, groundMat);
    groundMesh.position.set(0, -1, 0);
    groundMesh.receiveShadow = true;
    scene.add(groundMesh);
    bodies.push({ handle: 0 as BodyHandle, mesh: groundMesh, type: BodyType.Static });

    const shaderMesh = createShaderBoxMesh(WIDE_PYRAMID_BOX_COUNT, WIDE_PYRAMID_BOX_SIZE, { shadows: false });
    let idx = 0;
    forEachWidePyramidBox(([x, y, z]) => {
      const pOff = idx * 3;
      const qOff = idx * 4;
      shaderMesh.positionArray[pOff] = x;
      shaderMesh.positionArray[pOff + 1] = y;
      shaderMesh.positionArray[pOff + 2] = z;
      shaderMesh.quaternionArray[qOff] = 0;
      shaderMesh.quaternionArray[qOff + 1] = 0;
      shaderMesh.quaternionArray[qOff + 2] = 0;
      shaderMesh.quaternionArray[qOff + 3] = 1;
      hexToRgb(WIDE_PYRAMID_BOX_COLOR, shaderMesh.colorArray, pOff);
      idx++;
    });
    shaderMesh.positionAttribute.needsUpdate = true;
    shaderMesh.quaternionAttribute.needsUpdate = true;
    shaderMesh.colorAttribute.needsUpdate = true;
    scene.add(shaderMesh.mesh);

    worker.addEventListener("message", (event: MessageEvent<PhysicsWorkerMessage>) => {
      const message = event.data;
      if (message.type === "ready") {
        const ready = message as PhysicsWorkerReady;
        bodyCount = ready.count;
        wc = ready.workerCount;
        workerWorldState = {
          count: ready.count,
          workerCount: ready.workerCount,
          positions: new Float32Array(ready.positions),
          rotations: new Float32Array(ready.rotations),
          awake: new Uint8Array(ready.awake),
          colors: new Uint32Array(ready.colors),
          projectilePositions: new Float32Array(ready.projectilePositions),
          projectileRotations: new Float32Array(ready.projectileRotations),
          projectileAwake: new Uint8Array(ready.projectileAwake),
          projectileColors: new Uint32Array(ready.projectileColors),
          state: new Int32Array(ready.state),
        };
        positions = workerWorldState.positions;
        rotations = workerWorldState.rotations;
        projectilePositions = workerWorldState.projectilePositions;
        projectileRotations = workerWorldState.projectileRotations;
        projectileDebugColors = workerWorldState.projectileColors;
        state = workerWorldState.state;
        bindSnapshotTransforms(shaderMesh, positions, rotations, Math.min(WIDE_PYRAMID_BOX_COUNT, bodyCount));
        shaderMesh.positionAttribute.needsUpdate = true;
        shaderMesh.quaternionAttribute.needsUpdate = true;
      } else if (message.type === "error") {
        console.error(`Physics worker error: ${message.message}`);
      }
    });

    worker.postMessage({
      type: "init",
      data: {},
      workerCount: wc,
      maxWorkers,
      poolSize,
      solverParams: initialSolverParams,
      wasmVersion: wasmBuildVersion,
      wasmVariant: getWasmVariant(),
      wasmBaseUrl: getWasmBaseUrl(),
    });
    worker.postMessage({ type: "set-color-mode", mode: "light" });

    function spawnProjectile(origin: [number, number, number], velocity: [number, number, number], spin: boolean, ragdoll: boolean): void {
      if (projectileMeshes.length >= MAX_PROJECTILES) return;
      if (ragdoll) {
        const ragdollCount = Math.min(RAGDOLL_RENDER_BONES.length, MAX_PROJECTILES - projectileMeshes.length);
        for (let i = 0; i < ragdollCount; i++) {
          const bone = RAGDOLL_RENDER_BONES[i];
          const ragdollMesh = ragdollCapsuleMesh(bone.a, bone.b, bone.radius, bone.color);
          ragdollMesh.position.set(origin[0] + bone.position[0], origin[1] + bone.position[1], origin[2] + bone.position[2]);
          ragdollMesh.quaternion.set(bone.rotation[0], bone.rotation[1], bone.rotation[2], bone.rotation[3]);
          scene.add(ragdollMesh);
          projectileMeshes.push(ragdollMesh);
          projectileColorCache[projectileMeshes.length - 1] = bone.color;
        }
        worker.postMessage({ type: "spawn-ragdoll", origin, velocity });
        return;
      }
      const projectileMesh = new THREE.Mesh(
        new THREE.SphereGeometry(0.3, 12, 8),
        new THREE.MeshStandardMaterial({ color: spin ? 0x8b5cf6 : 0xf59e0b }),
      );
      projectileMesh.castShadow = true;
      projectileMesh.position.set(origin[0], origin[1], origin[2]);
      scene.add(projectileMesh);
      projectileMeshes.push(projectileMesh);
      projectileColorCache[projectileMeshes.length - 1] = spin ? 0x8b5cf6 : 0xf59e0b;
      worker.postMessage({ type: "spawn-projectile", origin, velocity });
    }

    return {
      world,
      bodies,
      controls: [],
      profile: true,
      info: `15-layer pyramid (${WIDE_PYRAMID_BOX_COUNT} boxes) | shader render | ${wc} workers`,
      camera: widePyramidCamera,
      spawnProjectile,
      startMouseDragRay: (origin, translation) => {
        worker.postMessage({ type: "drag-start", origin, translation });
        return true;
      },
      updateMouseDragRay: (origin, translation) => worker.postMessage({ type: "drag-update", origin, translation }),
      stopMouseDrag: () => worker.postMessage({ type: "drag-end" }),
      setPaused: (paused) => worker.postMessage({ type: "set-paused", paused }),
      stepOnce: () => worker.postMessage({ type: "step-once" }),
      sendSolverParams: (params) => worker.postMessage({ type: "set-solver-params", params }),
      step() {
        if (positions === null || rotations === null || state === null) return;
        const version = Atomics.load(state, SNAPSHOT_VERSION_INDEX);
        if (version === lastVersion) return;
        lastVersion = version;
        shaderMesh.positionAttribute.needsUpdate = true;
        shaderMesh.quaternionAttribute.needsUpdate = true;

        if (projectilePositions !== null && projectileRotations !== null && projectileDebugColors !== null) {
          const projectileCount = Math.min(Atomics.load(state, SNAPSHOT_PROJECTILE_COUNT_INDEX), projectileMeshes.length);
          for (let i = 0; i < projectileCount; i++) {
            const pOff = i * 3;
            const rOff = i * 4;
            projectileMeshes[i].position.set(projectilePositions[pOff], projectilePositions[pOff + 1], projectilePositions[pOff + 2]);
            projectileMeshes[i].quaternion.set(
              projectileRotations[rOff],
              projectileRotations[rOff + 1],
              projectileRotations[rOff + 2],
              projectileRotations[rOff + 3],
            );
            const colorHex = projectileDebugColors[i] & 0xffffff;
            if ((projectileColorCache[i] & 0xffffff) !== colorHex) {
              (projectileMeshes[i].material as THREE.MeshStandardMaterial).color.setHex(colorHex);
              projectileColorCache[i] = colorHex;
            }
          }
        }
      },
      dispose() {
        worker.postMessage({ type: "dispose" });
        worker.terminate();
        scene.remove(groundMesh);
        groundGeom.dispose();
        groundMat.dispose();
        scene.remove(shaderMesh.mesh);
        shaderMesh.dispose();
        for (const projectileMesh of projectileMeshes) {
          scene.remove(projectileMesh);
          projectileMesh.geometry.dispose();
          const projectileMaterial = projectileMesh.material;
          if (Array.isArray(projectileMaterial)) projectileMaterial.forEach((entry) => entry.dispose());
          else projectileMaterial.dispose();
        }
      },
    };
  },
};
