import * as THREE from "three";
import { BodyType, type BodyHandle, type Box3DRuntime } from "box3d-wasm";
import type { DemoBody, DemoSample, SolverParams } from "../types";
import type { PhysicsWorkerMessage, PhysicsWorkerReady } from "../../physics-worker-protocol";
import {
  MAX_PROJECTILES,
  RAGDOLL_RENDER_BONE_COUNT,
  SNAPSHOT_BODY_COUNT_INDEX,
  SNAPSHOT_PROJECTILE_COUNT_INDEX,
  SNAPSHOT_VERSION_INDEX,
} from "../../physics-worker-protocol";
import { createWorkerWorld, type WorkerWorldState } from "../../worker-world-bridge";
import { RAGDOLL_RENDER_BONES, ragdollCapsuleGeometry, ragdollCapsuleMesh } from "../../ragdoll-render";
import { createShaderInstanceMesh, hexToRgb, type ShaderBoxMesh } from "../../shader-instanced-boxes";
import { wasmBuildVersion } from "virtual:wasm-version";
import { getWasmBaseUrl, getWasmVariant, getWorkerCounts } from "../shared";
import { rainCamera, rainGroundSize, rainMaxHumanCount, rainTileCount } from "./rain-scene";

const TILE_COUNT = rainTileCount();
const MAX_HUMANS = rainMaxHumanCount();

export const rainSample: DemoSample = {
  id: "benchmark/rain",
  name: "Benchmark / Rain",
  create(_runtime: Box3DRuntime, scene: THREE.Scene, initialSolverParams?: SolverParams) {
    const { defaultWorkerCount, maxWorkerCount: maxWorkers, poolSize } = getWorkerCounts();
    let wc = Math.min(maxWorkers, Math.max(1, initialSolverParams?.workerCount ?? defaultWorkerCount));

    let workerWorldState: WorkerWorldState | null = null;
    let activeBodyCount = TILE_COUNT;
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

    const worker = new Worker(new URL("./rain.worker.ts", import.meta.url), { type: "module" });
    const world = createWorkerWorld(worker, () => workerWorldState, () => wc);

    const half = rainGroundSize();
    const groundGeom = new THREE.PlaneGeometry(2 * half[0], 2 * half[2]);
    const groundMat = new THREE.MeshStandardMaterial({ color: 0x222222 });
    const groundMesh = new THREE.Mesh(groundGeom, groundMat);
    groundMesh.rotation.x = -0.5 * Math.PI;
    groundMesh.position.set(0, 0, 0);
    groundMesh.receiveShadow = true;
    scene.add(groundMesh);
    bodies.push({ handle: 0 as BodyHandle, mesh: groundMesh, type: BodyType.Static });

    const boneMeshes: ShaderBoxMesh[] = [];
    for (const bone of RAGDOLL_RENDER_BONES) {
      const geom = ragdollCapsuleGeometry(bone.a, bone.b, bone.radius);
      const shaderMesh = createShaderInstanceMesh(geom, MAX_HUMANS, { shadows: false });
      geom.dispose();
      for (let i = 0; i < MAX_HUMANS; i++) {
        const pOff = i * 3;
        const qOff = i * 4;
        // Park unused instances underground until humans spawn.
        shaderMesh.positionArray[pOff] = 0;
        shaderMesh.positionArray[pOff + 1] = -1000;
        shaderMesh.positionArray[pOff + 2] = 0;
        shaderMesh.quaternionArray[qOff] = 0;
        shaderMesh.quaternionArray[qOff + 1] = 0;
        shaderMesh.quaternionArray[qOff + 2] = 0;
        shaderMesh.quaternionArray[qOff + 3] = 1;
        hexToRgb(bone.color, shaderMesh.colorArray, pOff);
      }
      shaderMesh.positionAttribute.needsUpdate = true;
      shaderMesh.quaternionAttribute.needsUpdate = true;
      shaderMesh.colorAttribute.needsUpdate = true;
      (shaderMesh.mesh.geometry as THREE.InstancedBufferGeometry).instanceCount = 0;
      scene.add(shaderMesh.mesh);
      boneMeshes.push(shaderMesh);
    }

    worker.addEventListener("message", (event: MessageEvent<PhysicsWorkerMessage>) => {
      const message = event.data;
      if (message.type === "ready") {
        const ready = message as PhysicsWorkerReady;
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
        activeBodyCount = Atomics.load(state, SNAPSHOT_BODY_COUNT_INDEX) || TILE_COUNT;
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

    function spawnProjectile(origin: [number, number, number], velocity: [number, number, number], spin: boolean, ragdoll: boolean): void {
      if (projectileMeshes.length >= MAX_PROJECTILES) return;
      if (ragdoll) {
        worker.postMessage({ type: "spawn-ragdoll", origin, velocity });
        for (const bone of RAGDOLL_RENDER_BONES) {
          if (projectileMeshes.length >= MAX_PROJECTILES) break;
          const mesh = ragdollCapsuleMesh(bone.a, bone.b, bone.radius, bone.color);
          scene.add(mesh);
          projectileMeshes.push(mesh);
        }
        return;
      }
      const mesh = new THREE.Mesh(
        new THREE.SphereGeometry(0.25, 12, 8),
        new THREE.MeshStandardMaterial({ color: 0xffa500, roughness: 0.75 }),
      );
      mesh.castShadow = true;
      mesh.position.set(origin[0], origin[1], origin[2]);
      scene.add(mesh);
      projectileMeshes.push(mesh);
      worker.postMessage({ type: "spawn-projectile", origin, velocity });
      void spin;
    }

    function syncBoneInstances(): void {
      if (positions === null || rotations === null) return;
      const humanCount = Math.max(0, Math.min(MAX_HUMANS, Math.floor((activeBodyCount - TILE_COUNT) / RAGDOLL_RENDER_BONE_COUNT)));
      for (let bone = 0; bone < boneMeshes.length; bone++) {
        const shaderMesh = boneMeshes[bone];
        const geom = shaderMesh.mesh.geometry as THREE.InstancedBufferGeometry;
        geom.instanceCount = humanCount;
        for (let human = 0; human < humanCount; human++) {
          const bodyIndex = TILE_COUNT + human * RAGDOLL_RENDER_BONE_COUNT + bone;
          const pOff = human * 3;
          const qOff = human * 4;
          const srcP = bodyIndex * 3;
          const srcQ = bodyIndex * 4;
          shaderMesh.positionArray[pOff] = positions[srcP];
          shaderMesh.positionArray[pOff + 1] = positions[srcP + 1];
          shaderMesh.positionArray[pOff + 2] = positions[srcP + 2];
          shaderMesh.quaternionArray[qOff] = rotations[srcQ];
          shaderMesh.quaternionArray[qOff + 1] = rotations[srcQ + 1];
          shaderMesh.quaternionArray[qOff + 2] = rotations[srcQ + 2];
          shaderMesh.quaternionArray[qOff + 3] = rotations[srcQ + 3];
        }
        shaderMesh.positionAttribute.needsUpdate = true;
        shaderMesh.quaternionAttribute.needsUpdate = true;
      }
    }

    return {
      world,
      bodies,
      controls: [],
      camera: rainCamera,
      profile: true,
      info: "ragdoll rain with instanced capsule shaders",
      getInfo: () => {
        const humans = Math.max(0, Math.floor((activeBodyCount - TILE_COUNT) / RAGDOLL_RENDER_BONE_COUNT));
        return `Rain | ${humans}/${MAX_HUMANS} humans | ${wc} workers`;
      },
      spawnProjectile,
      setPaused: (paused: boolean) => worker.postMessage({ type: "set-paused", paused }),
      stepOnce: () => worker.postMessage({ type: "step-once" }),
      sendSolverParams: (params: SolverParams) => {
        if (params.workerCount !== undefined) wc = Math.min(maxWorkers, Math.max(1, params.workerCount));
        worker.postMessage({ type: "set-solver-params", params });
      },
      step() {
        if (state === null || positions === null || rotations === null) return;
        const version = Atomics.load(state, SNAPSHOT_VERSION_INDEX);
        if (version === lastVersion) return;
        lastVersion = version;
        activeBodyCount = Atomics.load(state, SNAPSHOT_BODY_COUNT_INDEX);
        syncBoneInstances();

        if (projectilePositions !== null && projectileRotations !== null) {
          const projectileCount = Math.min(Atomics.load(state, SNAPSHOT_PROJECTILE_COUNT_INDEX), projectileMeshes.length);
          for (let i = 0; i < projectileCount; i++) {
            const mesh = projectileMeshes[i];
            mesh.position.set(projectilePositions[i * 3], projectilePositions[i * 3 + 1], projectilePositions[i * 3 + 2]);
            mesh.quaternion.set(
              projectileRotations[i * 4],
              projectileRotations[i * 4 + 1],
              projectileRotations[i * 4 + 2],
              projectileRotations[i * 4 + 3],
            );
            if (projectileDebugColors !== null) {
              const packed = projectileDebugColors[i];
              if (projectileColorCache[i] !== packed) {
                projectileColorCache[i] = packed;
                (mesh.material as THREE.MeshStandardMaterial).color.setRGB(
                  ((packed >> 16) & 0xff) / 255,
                  ((packed >> 8) & 0xff) / 255,
                  (packed & 0xff) / 255,
                );
              }
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
        for (const shaderMesh of boneMeshes) {
          scene.remove(shaderMesh.mesh);
          shaderMesh.dispose();
        }
        for (const mesh of projectileMeshes) {
          scene.remove(mesh);
          mesh.geometry.dispose();
          (mesh.material as THREE.Material).dispose();
        }
      },
    };
  },
};
