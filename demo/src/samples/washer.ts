import * as THREE from "three";
import type { Box3DRuntime } from "box3d-wasm";
import type { DemoBody, DemoSample } from "./types";
import type { PhysicsWorkerMessage, PhysicsWorkerReady } from "../physics-worker-protocol";
import { MAX_PROJECTILES, SNAPSHOT_CUMULATIVE_STEPS_INDEX, SNAPSHOT_PROJECTILE_COUNT_INDEX, SNAPSHOT_VERSION_INDEX } from "../physics-worker-protocol";
import { createWorkerWorld, type WorkerWorldState } from "../worker-world-bridge";
import { RAGDOLL_RENDER_BONES, ragdollCapsuleMesh } from "../ragdoll-render";

const dummy = new THREE.Object3D();
const awakeColor = new THREE.Color(0xd2b48c);
const sleepColor = new THREE.Color(0x778899);

function rotZ2(v: [number, number], angle: number): [number, number] {
  const c = Math.cos(angle), s = Math.sin(angle);
  return [v[0] * c - v[1] * s, v[0] * s + v[1] * c];
}

const ANGLE_STEP = Math.PI / 18;
const QO_ANGLE = 0.1 * ANGLE_STEP;
const R0 = 14, R1 = 16, R2 = 18;

function computeHullPoints(
  rInner: number, rOuter: number,
  e1: [number, number], e2: [number, number]
): [number, number, number][] {
  return [
    [rInner * e1[0], rInner * e1[1], -10],
    [rOuter * e1[0], rOuter * e1[1], -10],
    [rInner * e2[0], rInner * e2[1], -10],
    [rOuter * e2[0], rOuter * e2[1], -10],
    [rInner * e1[0], rInner * e1[1], 10],
    [rOuter * e1[0], rOuter * e1[1], 10],
    [rInner * e2[0], rInner * e2[1], 10],
    [rOuter * e2[0], rOuter * e2[1], 10],
  ];
}

function hullFromPoints(points: [number, number, number][]): THREE.BufferGeometry {
  const positions = points.flat();
  const indices = [
    0, 3, 1,   0, 2, 3,
    4, 5, 7,   4, 7, 6,
    0, 4, 6,   0, 6, 2,
    1, 3, 7,   1, 7, 5,
    0, 1, 5,   0, 5, 4,
    2, 6, 7,   2, 7, 3,
  ];
  const geo = new THREE.BufferGeometry();
  geo.setAttribute("position", new THREE.Float32BufferAttribute(positions, 3));
  geo.setIndex(indices);
  geo.computeVertexNormals();
  return geo;
}

const WASHER_CUBE_COUNT = 8000;

export function createWasherSample(): DemoSample {
  return {
    id: "washer",
    name: "Benchmark / Washer",
    create(_runtime: Box3DRuntime, scene: THREE.Scene) {
      const maxWorkers = Math.min(127, Math.max(1, (navigator.hardwareConcurrency || 8) - 1));
      const url = new URL(window.location.href);
      let wc = Number(url.searchParams.get("workers")) || maxWorkers;

      let workerWorldState: WorkerWorldState | null = null;
      let count = WASHER_CUBE_COUNT;
      let positions: Float32Array | null = null;
      let rotations: Float32Array | null = null;
      let awake: Uint8Array | null = null;
      let state: Int32Array | null = null;
      let projectilePositions: Float32Array | null = null;
      let projectileRotations: Float32Array | null = null;
      let projectileAwake: Uint8Array | null = null;
      let lastVersion = -1;
      let awCache: Uint8Array | null = null;

      const projectileMeshes: THREE.Mesh[] = [];
      const projectileColors: THREE.Color[] = [];
      const projectileAwakeCache = new Uint8Array(MAX_PROJECTILES);

      const bodies: DemoBody[] = [];

      const worker = new Worker(new URL("./washer.worker.ts", import.meta.url), { type: "module" });
      const world = createWorkerWorld(worker, () => workerWorldState, () => wc);

      const groundGeom = new THREE.BoxGeometry(120, 2, 120);
      const groundMat = new THREE.MeshStandardMaterial({ color: 0x222222 });
      const groundMesh = new THREE.Mesh(groundGeom, groundMat);
      groundMesh.position.set(0, -1, 0);
      groundMesh.receiveShadow = true;
      scene.add(groundMesh);
      bodies.push({ handle: 0, mesh: groundMesh, type: 0 });

      const drumGroup = new THREE.Group();
      drumGroup.position.set(0, 21, 0);

      const wallMat = new THREE.MeshStandardMaterial({ color: 0x3a3a3a, metalness: 0.4, roughness: 0.5, side: THREE.DoubleSide, transparent: true, opacity: 0.8 });
      const wallGeo = new THREE.CylinderGeometry(18, 18, 20, 48, 1, true);
      const wallMesh = new THREE.Mesh(wallGeo, wallMat);
      wallMesh.rotation.x = Math.PI / 2;
      drumGroup.add(wallMesh);

      const u1_0: [number, number] = [1, 0];
      const u2_0: [number, number] = [Math.cos(ANGLE_STEP), Math.sin(ANGLE_STEP)];
      const a1_0 = rotZ2(u1_0, -QO_ANGLE);
      const a2_0 = rotZ2(u2_0, QO_ANGLE);

      const vanePts = computeHullPoints(R1, R2, a1_0, a2_0);
      const vaneGeom = hullFromPoints(vanePts);

      const postPts = computeHullPoints(R0, R1, u1_0, u2_0);
      const postGeom = hullFromPoints(postPts);

      const vaneMat = new THREE.MeshStandardMaterial({ color: 0x555555, metalness: 0.5, roughness: 0.4 });
      const vaneMesh = new THREE.InstancedMesh(vaneGeom, vaneMat, 36);
      vaneMesh.castShadow = true;

      const postMat = new THREE.MeshStandardMaterial({ color: 0x4a4a4a, metalness: 0.5, roughness: 0.4 });
      const postMesh = new THREE.InstancedMesh(postGeom, postMat, 4);
      postMesh.castShadow = true;

      const vi = new THREE.Object3D();
      let pi = 0;
      for (let i = 0; i < 36; i++) {
        const theta = i * ANGLE_STEP;
        vi.quaternion.set(0, 0, Math.sin(theta / 2), Math.cos(theta / 2));
        vi.position.set(0, 0, 0);
        vi.scale.set(1, 1, 1);
        vi.updateMatrix();
        vaneMesh.setMatrixAt(i, vi.matrix);
        if (i % 9 === 0) {
          postMesh.setMatrixAt(pi, vi.matrix);
          pi++;
        }
      }
      vaneMesh.instanceMatrix.needsUpdate = true;
      postMesh.instanceMatrix.needsUpdate = true;
      drumGroup.add(vaneMesh);
      drumGroup.add(postMesh);

      const endCapMat = new THREE.MeshStandardMaterial({ color: 0x333333, metalness: 0.3, roughness: 0.6, side: THREE.DoubleSide });
      const backCapGeo = new THREE.RingGeometry(14, 18, 48);
      const backCap = new THREE.Mesh(backCapGeo, endCapMat);
      backCap.position.set(0, 0, -10);
      drumGroup.add(backCap);
      const frontCap = new THREE.Mesh(new THREE.RingGeometry(14, 18, 48), endCapMat);
      frontCap.position.set(0, 0, 10);
      drumGroup.add(frontCap);

      scene.add(drumGroup);
      bodies.push({ handle: 1, mesh: drumGroup as unknown as THREE.Mesh, type: 2 });

      const cubeGeom = new THREE.BoxGeometry(0.4, 0.4, 0.4);
      const cubeMat = new THREE.MeshStandardMaterial({ color: 0xd2b48c });
      const mesh = new THREE.InstancedMesh(cubeGeom, cubeMat, WASHER_CUBE_COUNT);
      mesh.instanceMatrix.setUsage(THREE.DynamicDrawUsage);
      mesh.castShadow = true;
      mesh.receiveShadow = true;

      const a = 0.2;
      const gridCount = 20;
      let idx = 0;
      for (let i = 0; i < gridCount; i++) {
        const x = -2 * a * gridCount + i * 4 * a;
        for (let j = 0; j < gridCount; j++) {
          const y = -2 * a * gridCount + 21 + j * 4 * a;
          for (let k = 0; k < gridCount; k++) {
            const z = -2 * a * gridCount + k * 4 * a;
            dummy.position.set(x, y, z);
            dummy.scale.set(1, 1, 1);
            dummy.quaternion.set(0, 0, 0, 1);
            dummy.updateMatrix();
            mesh.setMatrixAt(idx, dummy.matrix);
            mesh.setColorAt(idx, awakeColor);
            idx++;
          }
        }
      }
      mesh.instanceMatrix.needsUpdate = true;
      mesh.instanceColor!.needsUpdate = true;
      scene.add(mesh);

      worker.addEventListener("message", (event: MessageEvent<PhysicsWorkerMessage>) => {
        const message = event.data;
        if (message.type === "ready") {
          const ready = message as PhysicsWorkerReady;
          count = ready.count;
          wc = ready.workerCount;
          workerWorldState = {
            count: ready.count,
            workerCount: ready.workerCount,
            positions: new Float32Array(ready.positions),
            rotations: new Float32Array(ready.rotations),
            awake: new Uint8Array(ready.awake),
            projectilePositions: new Float32Array(ready.projectilePositions),
            projectileRotations: new Float32Array(ready.projectileRotations),
            projectileAwake: new Uint8Array(ready.projectileAwake),
            state: new Int32Array(ready.state),
          };
          const u = new URL(window.location.href);
          if (Number(u.searchParams.get("workers")) !== wc) {
            u.searchParams.set("workers", String(wc));
            history.replaceState(null, "", u);
          }
          positions = new Float32Array(ready.positions);
          rotations = new Float32Array(ready.rotations);
          awake = new Uint8Array(ready.awake);
          projectilePositions = new Float32Array(ready.projectilePositions);
          projectileRotations = new Float32Array(ready.projectileRotations);
          projectileAwake = new Uint8Array(ready.projectileAwake);
          state = new Int32Array(ready.state);
          awCache = new Uint8Array(count);
          awCache.fill(1);
        } else if (message.type === "error") {
          console.error(`Physics worker error: ${message.message}`);
        }
      });
      worker.postMessage({ type: "init", data: {}, workerCount: wc, maxWorkers });

      return {
        world,
        bodies,
        controls: [],
        profile: true,
        info: `Washer | ${WASHER_CUBE_COUNT} cubes | worker simulation | ${wc} workers`,
        camera: { position: [25, 20, 55], target: [0, 15, 0] },
        onKey(key: string) {
          if (key === "t" || key === "T") {
            worker.postMessage({ type: "toggle-worker-count" });
          }
        },
        spawnProjectile(origin, velocity, spin, ragdoll) {
          if (projectileMeshes.length >= MAX_PROJECTILES) return;
          if (ragdoll) {
            const ragdollCount = Math.min(RAGDOLL_RENDER_BONES.length, MAX_PROJECTILES - projectileMeshes.length);
            for (let i = 0; i < ragdollCount; i++) {
              const bone = RAGDOLL_RENDER_BONES[i];
              const ragdollMesh = ragdollCapsuleMesh(bone.a, bone.b, bone.radius, bone.color);
              ragdollMesh.position.set(origin[0] + i * 0.5, origin[1], origin[2]);
              scene.add(ragdollMesh);
              projectileMeshes.push(ragdollMesh);
              projectileColors.push(new THREE.Color(bone.color));
              projectileAwakeCache[projectileMeshes.length - 1] = 1;
            }
            worker.postMessage({ type: "spawn-ragdoll", origin, velocity });
            return;
          }
          const projectileMesh = new THREE.Mesh(new THREE.SphereGeometry(0.3, 12, 8), new THREE.MeshStandardMaterial({ color: 0xf59e0b }));
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
        step() {
          if (positions === null || rotations === null || awake === null || state === null || awCache === null) return;
          const version = Atomics.load(state, SNAPSHOT_VERSION_INDEX);
          if (version === lastVersion) return;
          lastVersion = version;

          let needsMatrixUpdate = false;
          let needsColorUpdate = false;
          for (let i = 0; i < count; i++) {
            const isAwake = awake[i] !== 0;
            const wasAwake = awCache[i] !== 0;
            if (isAwake) {
              const pOff = i * 3;
              const rOff = i * 4;
              dummy.scale.set(1, 1, 1);
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

          const cumulativeSteps = Math.max(0, Atomics.load(state, SNAPSHOT_CUMULATIVE_STEPS_INDEX));
          const drumAngle = cumulativeSteps * (1 / 60) * 25 * Math.PI / 180;
          drumGroup.quaternion.set(0, 0, Math.sin(drumAngle / 2), Math.cos(drumAngle / 2));

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
        },
        dispose() {
          worker.postMessage({ type: "dispose" });
          worker.terminate();
          scene.remove(groundMesh);
          groundGeom.dispose();
          groundMat.dispose();
          scene.remove(drumGroup);
          wallGeo.dispose();
          wallMat.dispose();
          vaneGeom.dispose();
          vaneMat.dispose();
          postGeom.dispose();
          postMat.dispose();
          backCapGeo.dispose();
          endCapMat.dispose();
          scene.remove(mesh);
          cubeGeom.dispose();
          cubeMat.dispose();
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
}
