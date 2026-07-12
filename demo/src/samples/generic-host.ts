import * as THREE from "three";
import { ConvexGeometry } from "three/examples/jsm/geometries/ConvexGeometry.js";
import { BodyType, type BodyHandle, type Box3DRuntime } from "box3d-wasm";
import { wasmBuildVersion } from "virtual:wasm-version";
import type { PhysicsWorkerMessage, PhysicsWorkerReady } from "../physics-worker-protocol";
import { MAX_PROJECTILES, SNAPSHOT_BODY_COUNT_INDEX, SNAPSHOT_PROJECTILE_COUNT_INDEX, SNAPSHOT_VERSION_INDEX } from "../physics-worker-protocol";
import { createWorkerWorld, type WorkerWorldState } from "../worker-world-bridge";
import { RAGDOLL_RENDER_BONES, ragdollCapsuleMesh } from "../ragdoll-render";
import type { ControlSpec, DemoBody, DemoSample, SolverParams } from "./types";
import { capsuleMesh, disposeBodies, getWasmBaseUrl, getWasmVariant, getWorkerCounts } from "./shared";

const dummy = new THREE.Object3D();
const localOffset = new THREE.Vector3();
const localRotation = new THREE.Quaternion();

export type RenderLocalTransform = { position?: [number, number, number]; rotation?: [number, number, number, number] };
export type RenderWorldTransform = { position: [number, number, number]; rotation?: [number, number, number, number] };
export type RenderBodyBase = RenderWorldTransform & { localPosition?: [number, number, number]; localRotation?: [number, number, number, number]; scale?: [number, number, number]; type?: BodyType };

// These shape descriptors are render-space conventions, not raw Box3D API inputs.
// In particular:
// - capsules declare their render axis explicitly (`x`, `y`, or `z`)
// - cylinders are Y-axis meshes and may need a local yOffset when the physics hull is not centered
export type RenderShape = { color: number } & ({ kind: "box"; size: [number, number, number] } | { kind: "sphere"; radius: number } | { kind: "cylinder"; radius: number; height: number; segments?: number; yOffset?: number } | { kind: "capsule"; radius: number; length: number; axis?: "x" | "y" | "z" } | { kind: "ragdoll-capsule"; a: [number, number, number]; b: [number, number, number]; radius: number } | { kind: "hull"; points: [number, number, number][] });
export type RenderPart = RenderShape & RenderLocalTransform;
export type RenderBody = (RenderBodyBase & RenderShape) | (RenderBodyBase & { kind: "compound"; parts: [RenderPart, ...RenderPart[]] });
export type RenderControlButton = { type: "button"; label: string; message: Record<string, unknown> };
export type RenderControlToggle = { type: "toggle"; label: string; message: Record<string, unknown>; value: boolean };
export type RenderControlRange = { type: "range"; label: string; message: Record<string, unknown>; min: number; max: number; step: number; value: number };
export type RenderControl = RenderControlButton | RenderControlToggle | RenderControlRange;
export type RenderOverlayContext = { bodies: DemoBody[]; workerState: WorkerWorldState | null };
export type RenderOverlay = { update(context: RenderOverlayContext): void; dispose(): void };
export type RenderSpec = {
  groundSize: [number, number, number];
  groundPosition?: [number, number, number];
  groundKind?: "box" | "plane";
  bodies: RenderBody[];
  info?: string;
  getInfo?: (workerState: WorkerWorldState | null) => string | undefined;
  camera?: { position: [number, number, number]; target: [number, number, number] };
  launchSpeed?: number;
  controls?: RenderControl[];
  /** Extra keyboard → worker messages (e.g. `{ keys: ["l","L"], message: { type: "launch" } }`). */
  hotkeys?: { keys: string[]; message: Record<string, unknown> }[];
  overlay?: (scene: THREE.Scene) => RenderOverlay;
};

function meshForShape(shape: RenderShape): THREE.Mesh {
  if (shape.kind === "ragdoll-capsule") return ragdollCapsuleMesh(shape.a, shape.b, shape.radius, shape.color);
  if (shape.kind === "capsule") return capsuleMesh(shape.radius, shape.length, shape.color, 0.75, shape.axis ?? "x");
  const mat = new THREE.MeshStandardMaterial({ color: shape.color, roughness: 0.75 });
  if (shape.kind === "box") return new THREE.Mesh(new THREE.BoxGeometry(...shape.size), mat);
  if (shape.kind === "sphere") return new THREE.Mesh(new THREE.SphereGeometry(shape.radius, 24, 16), mat);
  if (shape.kind === "cylinder") return new THREE.Mesh(new THREE.CylinderGeometry(shape.radius, shape.radius, shape.height, shape.segments ?? 12), mat);
  if (shape.kind === "hull") return new THREE.Mesh(new ConvexGeometry(shape.points.map((point) => new THREE.Vector3(point[0], point[1], point[2]))), mat);
  throw new Error("Unsupported render shape");
}

function setLocalTransform(mesh: THREE.Mesh, basePosition: THREE.Vector3, baseRotation: THREE.Quaternion): void {
  mesh.position.copy(basePosition);
  mesh.quaternion.copy(baseRotation);
  const partPosition = mesh.userData.localPosition as [number, number, number] | undefined;
  if (partPosition !== undefined) {
    localOffset.set(partPosition[0], partPosition[1], partPosition[2]).applyQuaternion(baseRotation);
    mesh.position.add(localOffset);
  }
  const partRotation = mesh.userData.localRotation as [number, number, number, number] | undefined;
  if (partRotation !== undefined) {
    localRotation.set(partRotation[0], partRotation[1], partRotation[2], partRotation[3]);
    mesh.quaternion.multiply(localRotation);
  }
}

function hasLocalRenderTransform(mesh: THREE.Mesh): boolean {
  return mesh.userData.localPosition !== undefined || mesh.userData.localRotation !== undefined;
}

function setMeshLocalRenderTransform(mesh: THREE.Mesh, transform: RenderLocalTransform): void {
  mesh.userData.localPosition = transform.position;
  mesh.userData.localRotation = transform.rotation;
}

export function meshFor(body: RenderBody): THREE.Mesh {
  const mesh = meshForShape(body.kind === "compound" ? body.parts[0] : body);
  mesh.position.set(body.position[0], body.position[1], body.position[2]);
  if (body.localPosition !== undefined || body.localRotation !== undefined) {
    setMeshLocalRenderTransform(mesh, { position: body.localPosition, rotation: body.localRotation });
  }
  if (body.rotation !== undefined) {
    mesh.quaternion.set(body.rotation[0], body.rotation[1], body.rotation[2], body.rotation[3]);
  }
  if (body.scale !== undefined) mesh.scale.set(body.scale[0], body.scale[1], body.scale[2]);
  if (body.kind === "cylinder" && body.yOffset !== undefined) setMeshLocalRenderTransform(mesh, { position: [0, body.yOffset, 0] as const, rotation: mesh.userData.localRotation as [number, number, number, number] | undefined });
  mesh.castShadow = body.type !== BodyType.Static;
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
      const groundGeom = spec.groundKind === "plane"
        ? new THREE.PlaneGeometry(spec.groundSize[0], spec.groundSize[2])
        : new THREE.BoxGeometry(...spec.groundSize);
      const groundMat = new THREE.MeshStandardMaterial({ color: 0x222222 });
      const groundMesh = new THREE.Mesh(groundGeom, groundMat);
      const groundPos = spec.groundPosition ?? [0, -1, 0];
      groundMesh.position.set(groundPos[0], groundPos[1], groundPos[2]);
      if (spec.groundKind === "plane") groundMesh.rotation.x = -0.5 * Math.PI;
      groundMesh.receiveShadow = true;
      scene.add(groundMesh);
      const bodies: DemoBody[] = [];
      const overlay = spec.overlay?.(scene);
      for (let i = 0; i < spec.bodies.length; i++) {
        const bodySpec = spec.bodies[i];
        const mesh = meshFor(bodySpec);
        const bodyPosition = new THREE.Vector3(bodySpec.position[0], bodySpec.position[1], bodySpec.position[2]);
        const bodyRotation = bodySpec.rotation === undefined ? new THREE.Quaternion() : new THREE.Quaternion(bodySpec.rotation[0], bodySpec.rotation[1], bodySpec.rotation[2], bodySpec.rotation[3]);
        if (bodySpec.kind === "compound") {
          setMeshLocalRenderTransform(mesh, bodySpec.parts[0]);
          setLocalTransform(mesh, bodyPosition, bodyRotation);
        } else if (hasLocalRenderTransform(mesh)) {
          setLocalTransform(mesh, bodyPosition, new THREE.Quaternion());
        }
        scene.add(mesh);
        const extraMeshes = bodySpec.kind === "compound" ? bodySpec.parts.slice(1).map((shape) => {
          const extra = meshForShape(shape);
          setMeshLocalRenderTransform(extra, shape);
          setLocalTransform(extra, bodyPosition, bodyRotation);
          extra.castShadow = mesh.castShadow;
          extra.receiveShadow = true;
          scene.add(extra);
          return extra;
        }) : undefined;
        bodies.push({ handle: (i + 1) as BodyHandle, mesh, extraMeshes, type: bodySpec.type ?? BodyType.Dynamic });
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
            extra: ready.extra,
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
        solverParams: initialSolverParams, wasmVersion: wasmBuildVersion, wasmVariant: getWasmVariant(), wasmBaseUrl: getWasmBaseUrl(),
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
        const trackedCount = Atomics.load(state, SNAPSHOT_BODY_COUNT_INDEX);
        const activeCount = trackedCount > 0 ? Math.min(bodies.length, trackedCount) : bodies.length;
        for (let i = 0; i < bodies.length; i++) {
          const body = bodies[i];
          const visible = i < activeCount;
          body.mesh.visible = visible;
          if (body.extraMeshes !== undefined) {
            for (const extra of body.extraMeshes) extra.visible = visible;
          }
          if (!visible) continue;
          if (awake[i] !== 0) {
            const p = i * 3, r = i * 4;
            dummy.position.set(positions[p], positions[p + 1], positions[p + 2]);
            dummy.quaternion.set(rotations[r], rotations[r + 1], rotations[r + 2], rotations[r + 3]);
            if (hasLocalRenderTransform(body.mesh)) {
              setLocalTransform(body.mesh, dummy.position, dummy.quaternion);
            } else {
              body.mesh.position.copy(dummy.position);
              body.mesh.quaternion.copy(dummy.quaternion);
            }
            if (body.extraMeshes !== undefined) {
              for (const extra of body.extraMeshes) {
                setLocalTransform(extra, dummy.position, dummy.quaternion);
              }
            }
          }
          const colorHex = colors[i] & 0xffffff;
          if ((colorCache[i] & 0xffffff) !== colorHex) {
            (body.mesh.material as THREE.MeshStandardMaterial).color.setHex(colorHex);
            if (body.extraMeshes !== undefined) {
              for (const extra of body.extraMeshes) {
                (extra.material as THREE.MeshStandardMaterial).color.setHex(colorHex);
              }
            }
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

        overlay?.update({ bodies, workerState: workerWorldState });
      }

      function dispose(): void {
        worker.postMessage({ type: "dispose" });
        worker.terminate();
        scene.remove(groundMesh);
        groundGeom.dispose();
        groundMat.dispose();
        disposeBodies(scene, bodies);
        overlay?.dispose();
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
        controls: (spec.controls ?? []).map((c): ControlSpec => {
          if (c.type === "button") {
            return { key: c.label.toLowerCase().replace(/\s+/g, "-"), label: c.label, type: "button", onClick: () => worker.postMessage(c.message) };
          }
          if (c.type === "range") {
            return {
              key: c.label.toLowerCase().replace(/\s+/g, "-"),
              label: c.label,
              type: "range",
              min: c.min,
              max: c.max,
              step: c.step,
              value: c.value,
              onChange: (v) => { if (typeof v === "number") worker.postMessage({ ...c.message, value: v }); },
            };
          }
          return { key: c.label.toLowerCase().replace(/\s+/g, "-"), label: c.label, type: "toggle", value: c.value, onChange: (v) => worker.postMessage({ ...c.message, value: v }) };
        }),
        profile: true,
        launchSpeed: spec.launchSpeed,
        camera: spec.camera,
        info: `${spec.info ?? name} | worker simulation | ${wc} workers | ${colorMode} colors`,
        getInfo: spec.getInfo === undefined ? undefined : () => spec.getInfo!(workerWorldState),
        onKey(key: string) {
          if (key === "c" || key === "C") {
            colorMode = colorMode === "full" ? "light" : "full";
            localStorage.setItem("box3d:color-mode", colorMode);
            worker.postMessage({ type: "set-color-mode", mode: colorMode });
            console.log(`[generic] color mode: ${colorMode}`);
          }
          for (const hotkey of spec.hotkeys ?? []) {
            if (hotkey.keys.includes(key)) worker.postMessage(hotkey.message);
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
