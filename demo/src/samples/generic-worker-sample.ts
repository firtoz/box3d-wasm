import * as THREE from "three";
import type { Box3DRuntime } from "box3d-wasm";
import { wasmBuildVersion } from "virtual:wasm-version";
import type { PhysicsWorkerMessage, PhysicsWorkerReady } from "../physics-worker-protocol";
import { MAX_PROJECTILES, SNAPSHOT_PROJECTILE_COUNT_INDEX, SNAPSHOT_VERSION_INDEX } from "../physics-worker-protocol";
import { createWorkerWorld, type WorkerWorldState } from "../worker-world-bridge";
import { RAGDOLL_RENDER_BONES, ragdollCapsuleMesh } from "../ragdoll-render";
import type { DemoBody, DemoSample } from "./types";
import { capsuleMesh, disposeBodies, getWasmVariant, getWorkerCounts } from "./shared";
import type { GenericSceneId } from "./generic.worker";

const dummy = new THREE.Object3D();

type RenderBodyBase = { position: [number, number, number]; rotation?: [number, number, number, number]; color: number; type?: number };
type RenderBody = RenderBodyBase & ({ kind: "box"; size: [number, number, number] } | { kind: "sphere"; radius: number } | { kind: "cylinder"; radius: number; height: number } | { kind: "capsule"; radius: number; length: number });
type RenderSpec = { groundSize: [number, number, number]; bodies: RenderBody[]; info?: string; camera?: { position: [number, number, number]; target: [number, number, number] }; launchSpeed?: number };

function qy(angle: number): [number, number, number, number] { return [0, Math.sin(angle / 2), 0, Math.cos(angle / 2)]; }
function qx(angle: number): [number, number, number, number] { return [Math.sin(angle / 2), 0, 0, Math.cos(angle / 2)]; }
function qz(angle: number): [number, number, number, number] { return [0, 0, Math.sin(angle / 2), Math.cos(angle / 2)]; }

function renderSpec(id: GenericSceneId): RenderSpec {
  const bodies: RenderBody[] = [];
  switch (id) {
    case "single-box": bodies.push({ kind: "box", size: [1, 1, 1], position: [0, 0.5, 0], color: 0xf59e0b }); return { groundSize: [40, 2, 40], bodies, camera: { position: [0, 4.226, 9.063], target: [0, 0, 0] } };
    case "compound-material-dedup": bodies.push({ kind: "box", size: [2, 2, 2], position: [-2, 4, 0], color: 0x38bdf8 }, { kind: "box", size: [2, 2, 2], position: [2, 4, 0], color: 0xf97316 }); return { groundSize: [24, 1, 24], bodies };
    case "compound-simple": bodies.push({ kind: "box", size: [8, 1, 8], position: [3, -1.5, 0], rotation: qy(Math.PI / 4), color: 0x223047, type: 0 }, { kind: "sphere", radius: 0.25, position: [0, 2, 0], color: 0xf59e0b }); return { groundSize: [40, 2, 40], bodies, launchSpeed: 1, info: "compound-style static slope" };
    case "cylinder": bodies.push({ kind: "cylinder", radius: 0.25, height: 1, position: [0, 2, 0], color: 0x38bdf8 }); return { groundSize: [20, 2, 20], bodies };
    case "sphere-stack": for (let i = 0, y = 0.75; i < 30; i++, y += 1.5) bodies.push({ kind: "sphere", radius: 0.5, position: [0, y, 0], color: 0x38bdf8 }); return { groundSize: [30, 2, 30], bodies };
    case "box-stack": for (let i = 0; i < 40; i++) bodies.push({ kind: "box", size: [1, 1, 1], position: [0, 0.75 + 1.25 * i, 0], color: 0x60a5fa + (i % 10) * 0x010101 }); return { groundSize: [80, 2, 80], bodies };
    case "shapes-inclined-plane": bodies.push({ kind: "box", size: [32, 1, 20], position: [0, 7.5, -5], rotation: qx(40 * Math.PI / 180), color: 0x94a3b8, type: 0 }); for (let i = 0; i < 5; i++) bodies.push({ kind: "box", size: [2, 2, 2], position: [-10 + 5 * i, 15.75, -10.6], color: 0x60a5fa + i * 0x050505 }); return { groundSize: [100, 2, 100], bodies };
    case "card-house-thick": { const alpha = 25 * Math.PI / 180; const ox = 0.5 * 0.98 * Math.sin(alpha) + 0.045; const oy = 0.5 * 0.98 * Math.cos(alpha) + 0.035; const addPair = (x: number, y: number, count: number) => { for (let j = 0; j < count; j++) { for (const s of [-1, 1]) bodies.push({ kind: "box", size: [0.08, 0.98, 0.38], position: [x + s * ox, y, 0], rotation: qz(s * alpha), color: 0xfde68a }); x += 4 * ox; } }; const addRow = (x: number, y: number, c: number) => { for (let i = 0; i < c; i++) bodies.push({ kind: "box", size: [0.08, 0.98, 0.38], position: [x + i * 4 * ox, y, 0], rotation: qz(Math.PI / 2), color: 0xfde68a }); }; addPair(-6 * ox, oy, 4); addRow(-4 * ox, 2 * oy + 0.04, 3); addPair(-4 * ox, 3 * oy + 0.08, 3); addRow(-2 * ox, 4 * oy + 0.12, 2); addPair(-2 * ox, 5 * oy + 0.16, 2); addRow(0, 6 * oy + 0.20, 1); addPair(0, 7 * oy + 0.24, 1); return { groundSize: [20, 2, 20], bodies, camera: { position: [0, 6.226, 9.063], target: [0, 2, 0] } }; }
    case "jenga-stack": for (let i = 0; i < 24; i++) { const even = (i & 1) === 0; const a = even ? 0.5 * Math.PI : 0; const x = even ? 1.75 : 0; const z = even ? 0 : 1.75; bodies.push({ kind: "box", size: [5, 0.5, 0.5], position: [x, 0.5 * i + 0.25, z], rotation: qy(a), color: 0xf59e0b }, { kind: "box", size: [5, 0.5, 0.5], position: [-x, 0.5 * i + 0.25, -z], rotation: qy(a), color: 0xf59e0b }); } return { groundSize: [60, 2, 60], bodies };
    case "pyramid2d": for (let row = 0; row < 12; row++) for (let col = 0; col < 12 - row; col++) bodies.push({ kind: "box", size: [2, 2, 2], position: [-10 + 2 * col + row, 1.5 + 2.5 * row, 0], color: 0x60a5fa + (row % 10) * 0x010101 }); return { groundSize: [80, 2, 80], bodies, info: "12 rows, 2D stacking (Z-locked)" };
    case "capsule-stack": for (let i = 0, y = 0.75; i < 20; i++, y += 1) bodies.push({ kind: "capsule", radius: 0.5, length: 2, position: [0, y, 0], color: 0x38bdf8 }); return { groundSize: [40, 2, 40], bodies, info: "20 capsules, 2D-stacked (Z-locked, rotation locked)" };
  }
}

function groundDimensions(id: GenericSceneId): [number, number, number] {
  switch (id) {
    case "single-box": return [40, 2, 40];
    case "compound-material-dedup": return [24, 1, 24];
    case "cylinder": return [20, 2, 20];
    case "sphere-stack": return [30, 2, 30];
    case "box-stack": return [80, 2, 80];
    case "shapes-inclined-plane": return [100, 2, 100];
    case "card-house-thick": return [20, 2, 20];
    case "jenga-stack": return [60, 2, 60];
    case "pyramid2d": return [80, 2, 80];
    case "capsule-stack": return [40, 2, 40];
    case "compound-simple": return [40, 2, 40];
  }
}

function meshFor(body: RenderBody): THREE.Mesh {
  const mat = new THREE.MeshStandardMaterial({ color: body.color, roughness: 0.75 });
  const mesh = body.kind === "box" ? new THREE.Mesh(new THREE.BoxGeometry(...body.size), mat) : body.kind === "sphere" ? new THREE.Mesh(new THREE.SphereGeometry(body.radius, 24, 16), mat) : body.kind === "cylinder" ? new THREE.Mesh(new THREE.CylinderGeometry(body.radius, body.radius, body.height, 12), mat) : capsuleMesh(body.radius, body.length, body.color);
  mesh.position.set(body.position[0], body.position[1], body.position[2]);
  if (body.rotation !== undefined) mesh.quaternion.set(body.rotation[0], body.rotation[1], body.rotation[2], body.rotation[3]);
  mesh.castShadow = body.type !== 0;
  mesh.receiveShadow = true;
  return mesh;
}

export function createGenericWorkerSample(id: GenericSceneId, name: string): DemoSample {
  return {
    id,
    name,
    create(_runtime: Box3DRuntime, scene: THREE.Scene) {
      const { defaultWorkerCount, maxWorkerCount: maxWorkers } = getWorkerCounts();
      let wc = defaultWorkerCount;
      const spec = renderSpec(id);
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
      const worker = new Worker(new URL("./generic.worker.ts", import.meta.url), { type: "module" });
      const world = createWorkerWorld(worker, () => workerWorldState, () => wc);
      const groundGeom = new THREE.BoxGeometry(...groundDimensions(id));
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
          workerWorldState = { count: ready.count, workerCount: ready.workerCount, positions: new Float32Array(ready.positions), rotations: new Float32Array(ready.rotations), awake: new Uint8Array(ready.awake), colors: new Uint32Array(ready.colors), projectilePositions: new Float32Array(ready.projectilePositions), projectileRotations: new Float32Array(ready.projectileRotations), projectileAwake: new Uint8Array(ready.projectileAwake), projectileColors: new Uint32Array(ready.projectileColors), state: new Int32Array(ready.state) };
          positions = workerWorldState.positions; rotations = workerWorldState.rotations; awake = workerWorldState.awake; colors = workerWorldState.colors; projectilePositions = workerWorldState.projectilePositions; projectileRotations = workerWorldState.projectileRotations; projectileAwake = workerWorldState.projectileAwake; projectileDebugColors = workerWorldState.projectileColors; state = workerWorldState.state; colorCache = new Uint32Array(ready.count);
        } else if (message.type === "error") console.error(`Physics worker error: ${message.message}`);
      });
      worker.postMessage({ type: "init", data: { sceneId: id }, workerCount: wc, maxWorkers, wasmVersion: wasmBuildVersion, wasmVariant: getWasmVariant() });
      return { world, bodies, controls: [], profile: true, launchSpeed: spec.launchSpeed, camera: spec.camera, info: `${spec.info ?? name} | worker simulation | ${wc} workers`, spawnProjectile(origin, velocity, spin, ragdoll) { if (projectileMeshes.length >= MAX_PROJECTILES) return; if (ragdoll) { const count = Math.min(RAGDOLL_RENDER_BONES.length, MAX_PROJECTILES - projectileMeshes.length); for (let i = 0; i < count; i++) { const bone = RAGDOLL_RENDER_BONES[i]; const mesh = ragdollCapsuleMesh(bone.a, bone.b, bone.radius, bone.color); mesh.position.set(origin[0] + i * 0.5, origin[1], origin[2]); scene.add(mesh); projectileMeshes.push(mesh); projectileColorCache[projectileMeshes.length - 1] = bone.color; } worker.postMessage({ type: "spawn-ragdoll", origin, velocity }); return; } const mesh = new THREE.Mesh(new THREE.SphereGeometry(0.3, 12, 8), new THREE.MeshStandardMaterial({ color: spin ? 0x8b5cf6 : 0xf59e0b })); mesh.castShadow = true; mesh.position.set(origin[0], origin[1], origin[2]); scene.add(mesh); projectileMeshes.push(mesh); projectileColorCache[projectileMeshes.length - 1] = spin ? 0x8b5cf6 : 0xf59e0b; worker.postMessage({ type: "spawn-projectile", origin, velocity }); }, startMouseDragRay(origin, translation) { worker.postMessage({ type: "drag-start", origin, translation }); return true; }, updateMouseDragRay(origin, translation) { worker.postMessage({ type: "drag-update", origin, translation }); }, stopMouseDrag() { worker.postMessage({ type: "drag-end" }); }, setPaused(paused) { worker.postMessage({ type: "set-paused", paused }); }, stepOnce() { worker.postMessage({ type: "step-once" }); }, sendSolverParams(params) { worker.postMessage({ type: "set-solver-params", params }); }, step() { if (positions === null || rotations === null || awake === null || colors === null || state === null || colorCache === null) return; const version = Atomics.load(state, SNAPSHOT_VERSION_INDEX); if (version === lastVersion) return; lastVersion = version; let needsColor = false; for (let i = 0; i < bodies.length; i++) { if (awake[i] !== 0) { const p = i * 3, r = i * 4; dummy.position.set(positions[p], positions[p + 1], positions[p + 2]); dummy.quaternion.set(rotations[r], rotations[r + 1], rotations[r + 2], rotations[r + 3]); dummy.scale.set(1, 1, 1); dummy.updateMatrix(); bodies[i].mesh.matrix.copy(dummy.matrix); bodies[i].mesh.matrix.decompose(bodies[i].mesh.position, bodies[i].mesh.quaternion, bodies[i].mesh.scale); } const colorHex = colors[i] & 0xffffff; if ((colorCache[i] & 0xffffff) !== colorHex) { (bodies[i].mesh.material as THREE.MeshStandardMaterial).color.setHex(colorHex); colorCache[i] = colorHex; needsColor = true; } } void needsColor; if (projectilePositions !== null && projectileRotations !== null && projectileAwake !== null && projectileDebugColors !== null) { const count = Math.min(Atomics.load(state, SNAPSHOT_PROJECTILE_COUNT_INDEX), projectileMeshes.length); for (let i = 0; i < count; i++) { const p = i * 3, r = i * 4; projectileMeshes[i].position.set(projectilePositions[p], projectilePositions[p + 1], projectilePositions[p + 2]); projectileMeshes[i].quaternion.set(projectileRotations[r], projectileRotations[r + 1], projectileRotations[r + 2], projectileRotations[r + 3]); const colorHex = projectileDebugColors[i] & 0xffffff; if ((projectileColorCache[i] & 0xffffff) !== colorHex) { (projectileMeshes[i].material as THREE.MeshStandardMaterial).color.setHex(colorHex); projectileColorCache[i] = colorHex; } } } }, dispose() { worker.postMessage({ type: "dispose" }); worker.terminate(); scene.remove(groundMesh); groundGeom.dispose(); groundMat.dispose(); disposeBodies(scene, bodies); for (const mesh of projectileMeshes) { scene.remove(mesh); mesh.geometry.dispose(); const mat = mesh.material; if (Array.isArray(mat)) mat.forEach((m) => m.dispose()); else mat.dispose(); } } };
    },
  };
}
