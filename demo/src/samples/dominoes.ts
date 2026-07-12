import * as THREE from "three";
import { B3_AXIS_Y, B3_DEG_TO_RAD, BodyType, quatFromAxisAngle, type BodyHandle, type Box3DRuntime } from "box3d-wasm";
import type { DemoBody, DemoSample, SolverParams } from "./types";
import type { PhysicsWorkerMessage, PhysicsWorkerReady } from "../physics-worker-protocol";
import { MAX_PROJECTILES, SNAPSHOT_PROJECTILE_COUNT_INDEX, SNAPSHOT_VERSION_INDEX } from "../physics-worker-protocol";
import { createWorkerSnapshotViews, releasePublishLock, tryAcquirePublishLock } from "../snapshot-views";
import { createWorkerWorld, type WorkerWorldState } from "../worker-world-bridge";
import { RAGDOLL_RENDER_BONES, ragdollCapsuleMesh } from "../ragdoll-render";
import { wasmBuildVersion } from "virtual:wasm-version";
import { getWasmBaseUrl, getWasmVariant, getWorkerCounts } from "./shared";

const dummy = new THREE.Object3D();
const awakeColor = new THREE.Color(0xd2b48c);
const sleepColor = new THREE.Color(0x778899);

export function createDominoesSample(multiplier: number): DemoSample {
  const ringCount = 30 * multiplier;
  const dominoTotal = ringCount * 180;

  return {
    id: multiplier === 1 ? "dominoes" : `dominoes-${multiplier}x`,
    name: multiplier === 1 ? `Stacking / Dominoes` : `Bench / Dominoes ${multiplier}\u00d7`,
    create(_runtime: Box3DRuntime, scene: THREE.Scene, initialSolverParams?: SolverParams) {
      const bodies: DemoBody[] = [];
      let workerWorldState: WorkerWorldState | null = null;
      let positions: Float32Array | null = null;
      let rotations: Float32Array | null = null;
      let awake: Uint8Array | null = null;
      let projectilePositions: Float32Array | null = null;
      let projectileRotations: Float32Array | null = null;
      let projectileAwake: Uint8Array | null = null;
      let state: Int32Array | null = null;
      let publishLock: Int32Array | null = null;
      let lastVersion = -1;
      const projectileMeshes: THREE.Mesh[] = [];
      const projectileColors: THREE.Color[] = [];
      const projectileAwakeCache = new Uint8Array(MAX_PROJECTILES);
      let awCache: Uint8Array | null = null;
      let count = dominoTotal;
      const { defaultWorkerCount, maxWorkerCount: maxWorkers, poolSize } = getWorkerCounts();
      let wc = Math.min(maxWorkers, Math.max(1, initialSolverParams?.workerCount ?? defaultWorkerCount));

      const worker = new Worker(new URL("./dominoes.worker.ts", import.meta.url), { type: "module" });
      const world = createWorkerWorld(worker, () => workerWorldState, () => wc);

      const groundGeom = new THREE.BoxGeometry(320, 2, 320);
      const groundMat = new THREE.MeshStandardMaterial({ color: 0x222222 });
      const groundMesh = new THREE.Mesh(groundGeom, groundMat);
      groundMesh.position.set(0, -1, 0);
      groundMesh.receiveShadow = true;
      scene.add(groundMesh);
      bodies.push({ handle: 0 as BodyHandle, mesh: groundMesh, type: BodyType.Static });

      const geometry = new THREE.BoxGeometry(0.4, 1.6, 0.1);
      const material = new THREE.MeshStandardMaterial({ color: 0xfbbf24, roughness: 0.75 });
      const mesh = new THREE.InstancedMesh(geometry, material, dominoTotal);
      mesh.castShadow = true;
      mesh.receiveShadow = true;
      scene.add(mesh);

      let idx = 0;
      for (let ring = 0; ring < ringCount; ring++) {
        const s = 0.5 + ring * 0.0585;
        const radius = 7.0 + (1.5 + ring * 0.015) * ring;
        const n = 1.515 + ring * 0.03;
        for (let deg = 0; deg < 360; deg += 2) {
          const rad = deg * B3_DEG_TO_RAD;
          const cs = Math.cos(rad);
          const sn = Math.sin(rad);
          const px = radius * cs + (deg * n / 716) * cs;
          const pz = radius * sn + (deg * n / 716) * sn;
          dummy.position.set(px, 0.8 * s, pz);
          dummy.scale.set(s, s, s);
          dummy.quaternion.set(...quatFromAxisAngle(B3_AXIS_Y, -rad));
          dummy.updateMatrix();
          mesh.setMatrixAt(idx, dummy.matrix);
          mesh.setColorAt(idx, awakeColor);
          idx++;
        }
      }
      mesh.instanceMatrix.needsUpdate = true;
      mesh.instanceColor!.needsUpdate = true;

      worker.addEventListener("message", (event: MessageEvent<PhysicsWorkerMessage>) => {
        const message = event.data;
        if (message.type === "ready") {
          const ready = message as PhysicsWorkerReady;
          count = ready.count;
          wc = ready.workerCount;
          const views = createWorkerSnapshotViews(ready);
          workerWorldState = {
            count: ready.count,
            workerCount: ready.workerCount,
            positions: views.positions,
            rotations: views.rotations,
            awake: views.awake,
            colors: views.colors,
            projectilePositions: views.projectilePositions,
            projectileRotations: views.projectileRotations,
            projectileAwake: views.projectileAwake,
            projectileColors: views.projectileColors,
            state: views.state,
            publishLock: views.publishLock,
            snapshotBacking: views.snapshotBacking,
          };
          positions = views.positions;
          rotations = views.rotations;
          awake = views.awake;
          projectilePositions = views.projectilePositions;
          projectileRotations = views.projectileRotations;
          projectileAwake = views.projectileAwake;
          state = views.state;
          publishLock = views.publishLock;
          awCache = new Uint8Array(count);
          awCache.fill(1);
        } else if (message.type === "error") {
          console.error(`Physics worker error: ${message.message}`);
        }
      });
      let colorMode = localStorage.getItem("box3d:color-mode") === "light" ? "light" : "full";
      worker.postMessage({ type: "init", data: { multiplier }, workerCount: wc, maxWorkers, poolSize, solverParams: initialSolverParams, wasmVersion: wasmBuildVersion, wasmVariant: getWasmVariant(), wasmBaseUrl: getWasmBaseUrl() });
      worker.postMessage({ type: "set-color-mode", mode: colorMode });

      return {
        world,
        bodies,
        controls: [],
        profile: true,
        info: `${ringCount} rings of dominoes | worker simulation | ${wc} workers | dynamic cap ${MAX_PROJECTILES} | ${colorMode} colors`,
        camera: { position: [0, 55, 75], target: [0, 0, 0] },
        onKey(key: string) {
          if (key === "t" || key === "T") {
            worker.postMessage({ type: "toggle-worker-count" });
          } else if (key === "c" || key === "C") {
            colorMode = localStorage.getItem("box3d:color-mode") === "light" ? "light" : "full";
            worker.postMessage({ type: "set-color-mode", mode: colorMode });
          }
        },
        spawnProjectile(origin, velocity, spin, ragdoll) {
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
              projectileColors.push(new THREE.Color(bone.color));
              projectileAwakeCache[projectileMeshes.length - 1] = 1;
            }
            worker.postMessage({ type: "spawn-ragdoll", origin, velocity });
            return;
          }
          const projectileMesh = new THREE.Mesh(
            new THREE.SphereGeometry(0.25, 16, 12),
            new THREE.MeshStandardMaterial({ color: spin ? 0x8b5cf6 : 0xf59e0b, roughness: 0.6 }),
          );
          projectileMesh.castShadow = true;
          projectileMesh.position.set(origin[0], origin[1], origin[2]);
          scene.add(projectileMesh);
          projectileMeshes.push(projectileMesh);
          projectileColors.push(new THREE.Color(spin ? 0x8b5cf6 : 0xf59e0b));
          projectileAwakeCache[projectileMeshes.length - 1] = 1;
          worker.postMessage({ type: "spawn-projectile", origin, velocity });
        },
        startMouseDragRay(origin, translation) {
          worker.postMessage({ type: "drag-start", origin, translation });
          return true;
        },
        updateMouseDragRay(origin, translation) {
          worker.postMessage({ type: "drag-update", origin, translation });
        },
        stopMouseDrag() {
          worker.postMessage({ type: "drag-end" });
        },
        setPaused(paused) {
          worker.postMessage({ type: "set-paused", paused });
        },
        stepOnce() {
          worker.postMessage({ type: "step-once" });
        },
        sendSolverParams(params) {
          worker.postMessage({ type: "set-solver-params", params });
        },
        step(_dt?, _subSteps?) {
          if (positions === null || rotations === null || awake === null || state === null || publishLock === null || awCache === null) return;
          const version = Atomics.load(state, SNAPSHOT_VERSION_INDEX);
          if (version === lastVersion) return;
          if (!tryAcquirePublishLock(publishLock)) return;
          lastVersion = version;
          try {
            let needsMatrixUpdate = false;
            let needsColorUpdate = false;
            for (let i = 0; i < count; i++) {
              const isAwake = awake[i] !== 0;
              const wasAwake = awCache[i] !== 0;
              if (isAwake) {
                const pOff = i * 3;
                const rOff = i * 4;
                const s = 0.5 + Math.floor(i / 180) * 0.0585;
                dummy.scale.set(s, s, s);
                dummy.position.set(positions[pOff], positions[pOff + 1], positions[pOff + 2]);
                dummy.quaternion.set(rotations[rOff], rotations[rOff + 1], rotations[rOff + 2], rotations[rOff + 3]);
                dummy.updateMatrix();
                mesh.setMatrixAt(i, dummy.matrix);
                needsMatrixUpdate = true;
              }
              if (isAwake !== wasAwake) {
                mesh.setColorAt(i, isAwake ? awakeColor : sleepColor);
                awCache[i] = isAwake ? 1 : 0;
                needsColorUpdate = true;
              }
            }
            if (needsMatrixUpdate) mesh.instanceMatrix.needsUpdate = true;
            if (needsColorUpdate) mesh.instanceColor!.needsUpdate = true;

            if (projectilePositions !== null && projectileRotations !== null && projectileAwake !== null) {
              const projectileCount = Math.min(Atomics.load(state, SNAPSHOT_PROJECTILE_COUNT_INDEX), projectileMeshes.length);
              for (let i = 0; i < projectileCount; i++) {
                const pOff = i * 3;
                const rOff = i * 4;
                projectileMeshes[i].position.set(projectilePositions[pOff], projectilePositions[pOff + 1], projectilePositions[pOff + 2]);
                projectileMeshes[i].quaternion.set(projectileRotations[rOff], projectileRotations[rOff + 1], projectileRotations[rOff + 2], projectileRotations[rOff + 3]);
                const isAwake = projectileAwake[i] !== 0;
                const wasAwake = projectileAwakeCache[i] !== 0;
                if (isAwake !== wasAwake) {
                  const material = projectileMeshes[i].material as THREE.MeshStandardMaterial;
                  material.color.copy(isAwake ? projectileColors[i] : sleepColor);
                  projectileAwakeCache[i] = isAwake ? 1 : 0;
                }
              }
            }
          } finally {
            releasePublishLock(publishLock);
          }
        },
        dispose() {
          worker.postMessage({ type: "dispose" });
          worker.terminate();
          scene.remove(mesh);
          geometry.dispose();
          material.dispose();
          for (const projectileMesh of projectileMeshes) {
            scene.remove(projectileMesh);
            projectileMesh.geometry.dispose();
            const projectileMaterial = projectileMesh.material;
            if (Array.isArray(projectileMaterial)) projectileMaterial.forEach((entry) => entry.dispose());
            else projectileMaterial.dispose();
          }
          scene.remove(groundMesh);
          groundGeom.dispose();
          groundMat.dispose();
        },
      };
    },
  };
}

export const dominoesSample = createDominoesSample(1);
