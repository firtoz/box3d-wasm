import * as THREE from "three";
import type { Box3DRuntime } from "box3d-wasm";
import { wasmBuildVersion } from "virtual:wasm-version";
import type { PhysicsWorkerMessage, PhysicsWorkerReady } from "../physics-worker-protocol";
import { MAX_PROJECTILES, SNAPSHOT_PROJECTILE_COUNT_INDEX, SNAPSHOT_VERSION_INDEX } from "../physics-worker-protocol";
import { createWorkerWorld, type WorkerWorldState } from "../worker-world-bridge";
import { RAGDOLL_RENDER_BONES, ragdollCapsuleMesh } from "../ragdoll-render";
import type { DemoBody, DemoSample, SolverParams } from "./types";
import { capsuleMesh, disposeBodies, getWasmVariant, getWorkerCounts } from "./shared";

const dummy = new THREE.Object3D();

export type RenderBodyBase = { position: [number, number, number]; rotation?: [number, number, number, number]; color: number; type?: number };
export type RenderBody = RenderBodyBase & ({ kind: "box"; size: [number, number, number] } | { kind: "sphere"; radius: number } | { kind: "cylinder"; radius: number; height: number } | { kind: "capsule"; radius: number; length: number });
export type RenderSpec = { groundSize: [number, number, number]; bodies: RenderBody[]; info?: string; camera?: { position: [number, number, number]; target: [number, number, number] }; launchSpeed?: number };

export function meshFor(body: RenderBody): THREE.Mesh {
  const mat = new THREE.MeshStandardMaterial({ color: body.color, roughness: 0.75 });
  const mesh = body.kind === "box" ? new THREE.Mesh(new THREE.BoxGeometry(...body.size), mat) : body.kind === "sphere" ? new THREE.Mesh(new THREE.SphereGeometry(body.radius, 24, 16), mat) : body.kind === "cylinder" ? new THREE.Mesh(new THREE.CylinderGeometry(body.radius, body.radius, body.height, 12), mat) : capsuleMesh(body.radius, body.length, body.color);
  mesh.position.set(body.position[0], body.position[1], body.position[2]);
  if (body.rotation !== undefined) mesh.quaternion.set(body.rotation[0], body.rotation[1], body.rotation[2], body.rotation[3]);
  mesh.castShadow = body.type !== 0;
  mesh.receiveShadow = true;
  return mesh;
}

export function createGenericSample(id: string, name: string, spec: RenderSpec, createWorker: () => Worker): DemoSample {
  return {
    id,
    name,
    create(_runtime: Box3DRuntime, scene: THREE.Scene, initialSolverParams?: SolverParams) {
      const { defaultWorkerCount, maxWorkerCount: maxWorkers, poolSize } = getWorkerCounts();
      let wc = Math.min(maxWorkers, Math.max(1, initialSolverParams?.workerCount ?? defaultWorkerCount));
      let workerWorldState: WorkerWorldState | null = null;
      let positions: Float32Array | null = null;
      let rotations: Float32Array | null = null;
      let awake: Uint8Array | null = null;
      let colors: Uint32Array | null = null;
      let state: Int32Array | null = null;
      let lastVersion = -1;
      let colorCache: Uint32Array | null = null;
      let projectilePositions: Float32Array | null = null;
      let projectileRotations: Float32Array | null = null;
      let projectileAwake: Uint8Array | null = null;
      let projectileDebugColors: Uint32Array | null = null;
      const projectileMeshes: THREE.Mesh[] = [];
      const projectileColorCache = new Uint32Array(MAX_PROJECTILES);
      const worker = createWorker();
      const world = createWorkerWorld(worker, () => workerWorldState, () => wc);
      const groundGeom = new THREE.BoxGeometry(...spec.groundSize);
      const groundMat = new THREE.MeshStandardMaterial({ color: 0x222222 });
      const groundMesh = new THREE.Mesh(groundGeom, groundMat);
      groundMesh.position.set(0, -1, 0);
      groundMesh.receiveShadow = true;
      scene.add(groundMesh);
      const bodies: DemoBody[] = [];
      for (let i = 0; i < spec.bodies.length; i++) {
        const mesh = meshFor(spec.bodies[i]);
        scene.add(mesh);
        bodies.push({ handle: i + 1, mesh, type: spec.bodies[i].type ?? 2 });
      }

      worker.addEventListener("message", (event: MessageEvent<PhysicsWorkerMessage>) => {
        const message = event.data;
        if (message.type === "ready") {
          const ready = message as PhysicsWorkerReady;
          wc = ready.workerCount;
          workerWorldState = {
            count: ready.count, workerCount: ready.workerCount,
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
          awake = workerWorldState.awake;
          colors = workerWorldState.colors;
          projectilePositions = workerWorldState.projectilePositions;
          projectileRotations = workerWorldState.projectileRotations;
          projectileAwake = workerWorldState.projectileAwake;
          projectileDebugColors = workerWorldState.projectileColors;
          state = workerWorldState.state;
          colorCache = new Uint32Array(ready.count);
        } else if (message.type === "error") {
          console.error(`Physics worker error: ${message.message}`);
        }
      });
      let colorMode = localStorage.getItem("box3d:color-mode") === "light" ? "light" : "full";
      worker.postMessage({
        type: "init", data: {}, workerCount: wc, maxWorkers, poolSize,
        solverParams: initialSolverParams, wasmVersion: wasmBuildVersion, wasmVariant: getWasmVariant(),
      });
      worker.postMessage({ type: "set-color-mode", mode: colorMode });

      function spawnProjectile(origin: [number, number, number], velocity: [number, number, number], spin: boolean, ragdoll: boolean): void {
        if (projectileMeshes.length >= MAX_PROJECTILES) return;
        if (ragdoll) {
          const count = Math.min(RAGDOLL_RENDER_BONES.length, MAX_PROJECTILES - projectileMeshes.length);
          for (let i = 0; i < count; i++) {
            const bone = RAGDOLL_RENDER_BONES[i];
            const mesh = ragdollCapsuleMesh(bone.a, bone.b, bone.radius, bone.color);
            mesh.position.set(origin[0] + bone.position[0], origin[1] + bone.position[1], origin[2] + bone.position[2]);
            mesh.quaternion.set(bone.rotation[0], bone.rotation[1], bone.rotation[2], bone.rotation[3]);
            scene.add(mesh);
            projectileMeshes.push(mesh);
            projectileColorCache[projectileMeshes.length - 1] = bone.color;
          }
          worker.postMessage({ type: "spawn-ragdoll", origin, velocity });
          return;
        }
        const mesh = new THREE.Mesh(
          new THREE.SphereGeometry(0.3, 12, 8),
          new THREE.MeshStandardMaterial({ color: spin ? 0x8b5cf6 : 0xf59e0b }),
        );
        mesh.castShadow = true;
        mesh.position.set(origin[0], origin[1], origin[2]);
        scene.add(mesh);
        projectileMeshes.push(mesh);
        projectileColorCache[projectileMeshes.length - 1] = spin ? 0x8b5cf6 : 0xf59e0b;
        worker.postMessage({ type: "spawn-projectile", origin, velocity });
      }

      function startMouseDragRay(origin: [number, number, number], translation: [number, number, number]): boolean {
        worker.postMessage({ type: "drag-start", origin, translation });
        return true;
      }

      function updateMouseDragRay(origin: [number, number, number], translation: [number, number, number]): void {
        worker.postMessage({ type: "drag-update", origin, translation });
      }

      function stopMouseDrag(): void {
        worker.postMessage({ type: "drag-end" });
      }

      function setPaused(paused: boolean): void {
        worker.postMessage({ type: "set-paused", paused });
      }

      function stepOnce(): void {
        worker.postMessage({ type: "step-once" });
      }

      function sendSolverParams(params: SolverParams): void {
        worker.postMessage({ type: "set-solver-params", params });
      }

      function step(): void {
        if (positions === null || rotations === null || awake === null || colors === null || state === null || colorCache === null) return;
        const version = Atomics.load(state, SNAPSHOT_VERSION_INDEX);
        if (version === lastVersion) return;
        lastVersion = version;
        for (let i = 0; i < bodies.length; i++) {
          if (awake[i] !== 0) {
            const p = i * 3, r = i * 4;
            dummy.position.set(positions[p], positions[p + 1], positions[p + 2]);
            dummy.quaternion.set(rotations[r], rotations[r + 1], rotations[r + 2], rotations[r + 3]);
            dummy.scale.set(1, 1, 1);
            dummy.updateMatrix();
            bodies[i].mesh.matrix.copy(dummy.matrix);
            bodies[i].mesh.matrix.decompose(bodies[i].mesh.position, bodies[i].mesh.quaternion, bodies[i].mesh.scale);
          }
          const colorHex = colors[i] & 0xffffff;
          if ((colorCache[i] & 0xffffff) !== colorHex) {
            (bodies[i].mesh.material as THREE.MeshStandardMaterial).color.setHex(colorHex);
            colorCache[i] = colorHex;
          }
        }

        if (projectilePositions !== null && projectileRotations !== null && projectileAwake !== null && projectileDebugColors !== null) {
          const count = Math.min(Atomics.load(state, SNAPSHOT_PROJECTILE_COUNT_INDEX), projectileMeshes.length);
          for (let i = 0; i < count; i++) {
            const p = i * 3, r = i * 4;
            projectileMeshes[i].position.set(projectilePositions[p], projectilePositions[p + 1], projectilePositions[p + 2]);
            projectileMeshes[i].quaternion.set(projectileRotations[r], projectileRotations[r + 1], projectileRotations[r + 2], projectileRotations[r + 3]);
            const colorHex = projectileDebugColors[i] & 0xffffff;
            if ((projectileColorCache[i] & 0xffffff) !== colorHex) {
              (projectileMeshes[i].material as THREE.MeshStandardMaterial).color.setHex(colorHex);
              projectileColorCache[i] = colorHex;
            }
          }
        }
      }

      function dispose(): void {
        worker.postMessage({ type: "dispose" });
        worker.terminate();
        scene.remove(groundMesh);
        groundGeom.dispose();
        groundMat.dispose();
        disposeBodies(scene, bodies);
        for (const mesh of projectileMeshes) {
          scene.remove(mesh);
          mesh.geometry.dispose();
          const mat = mesh.material;
          if (Array.isArray(mat)) mat.forEach((m) => m.dispose());
          else mat.dispose();
        }
      }

      return {
        world,
        bodies,
        controls: [],
        profile: true,
        launchSpeed: spec.launchSpeed,
        camera: spec.camera,
        info: `${spec.info ?? name} | worker simulation | ${wc} workers | ${colorMode} colors`,
        onKey(key: string) {
          if (key === "c" || key === "C") {
            colorMode = colorMode === "full" ? "light" : "full";
            localStorage.setItem("box3d:color-mode", colorMode);
            worker.postMessage({ type: "set-color-mode", mode: colorMode });
            console.log(`[generic] color mode: ${colorMode}`);
          }
        },
        spawnProjectile,
        startMouseDragRay,
        updateMouseDragRay,
        stopMouseDrag,
        setPaused,
        stepOnce,
        sendSolverParams,
        step,
        dispose,
      };
    },
  };
}
