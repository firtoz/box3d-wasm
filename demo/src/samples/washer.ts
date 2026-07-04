import * as THREE from "three";
import type { Box3DRuntime } from "box3d-wasm";
import type { DemoBody, DemoSample } from "./types";
import type { PhysicsWorkerMessage, PhysicsWorkerReady } from "../physics-worker-protocol";
import { MAX_PROJECTILES, SNAPSHOT_PROJECTILE_COUNT_INDEX, SNAPSHOT_VERSION_INDEX } from "../physics-worker-protocol";
import { createWorkerWorld, type WorkerWorldState } from "../worker-world-bridge";
import { RAGDOLL_RENDER_BONES, ragdollCapsuleMesh } from "../ragdoll-render";
import { wasmBuildVersion } from "virtual:wasm-version";
import { getWasmVariant, getWorkerCounts } from "./shared";

const dummy = new THREE.Object3D();
const awakeColor = new THREE.Color(0xd2b48c);
const debugColor = new THREE.Color();

const B3_PI = 3.14159265359;
const ANGLE_STEP = B3_PI / 18;
const QO_ANGLE = 0.1 * ANGLE_STEP;
const R0 = 14, R1 = 16, R2 = 18;

function hullVertices(
  rInner: number, rOuter: number,
  e1x: number, e1y: number,
  e2x: number, e2y: number
): [number, number, number][] {
  return [
    [rInner * e1x, rInner * e1y, -10],
    [rOuter * e1x, rOuter * e1y, -10],
    [rInner * e2x, rInner * e2y, -10],
    [rOuter * e2x, rOuter * e2y, -10],
    [rInner * e1x, rInner * e1y,  10],
    [rOuter * e1x, rOuter * e1y,  10],
    [rInner * e2x, rInner * e2y,  10],
    [rOuter * e2x, rOuter * e2y,  10],
  ];
}

// Each quad defines a face of the hexahedron hull: 2 triangles (v0,v1,v2) and (v0,v2,v3).
// Winding must be CCW when viewed from outside for correct outward normals.
const HULL_FACE_QUADS: [number, number, number, number][] = [
  [0, 2, 3, 1],  // near (z=-10): outward -Z
  [4, 5, 7, 6],  // far  (z=+10): outward +Z
  [0, 4, 6, 2],  // inner (r=R1): inward radial
  [1, 3, 7, 5],  // outer (r=R2): outward radial
  [0, 1, 5, 4],  // e1 side (a1 dir): -azimuthal
  [2, 6, 7, 3],  // e2 side (a2 dir): +azimuthal
];

const HULL_EDGE_PAIRS: [number, number][] = [
  [0, 1], [1, 3], [3, 2], [2, 0],
  [4, 5], [5, 7], [7, 6], [6, 4],
  [0, 4], [1, 5], [2, 6], [3, 7],
];

function buildDrumMeshes(): { mesh: THREE.Mesh, edges: THREE.LineSegments } {
  const vanePositions: number[] = [];
  const vaneEdgePositions: number[] = [];
  const vaneNormals: number[] = [];

  const postPositions: number[] = [];
  const postEdgePositions: number[] = [];
  const postNormals: number[] = [];

  for (let i = 0; i < 36; i++) {
    const u1a = i * ANGLE_STEP;
    const u2a = i === 35 ? 0 : (i + 1) * ANGLE_STEP;

    const cu1 = Math.cos(u1a), su1 = Math.sin(u1a);
    const cu2 = Math.cos(u2a), su2 = Math.sin(u2a);

    const cq = Math.cos(QO_ANGLE), sq = Math.sin(QO_ANGLE);
    const a1x = cu1 * cq + su1 * sq;
    const a1y = -cu1 * sq + su1 * cq;
    const a2x = cu2 * cq - su2 * sq;
    const a2y = cu2 * sq + su2 * cq;

    const pts = hullVertices(R1, R2, a1x, a1y, a2x, a2y);
    for (const quad of HULL_FACE_QUADS) {
      const v0 = pts[quad[0]], v1 = pts[quad[1]], v2 = pts[quad[2]], v3 = pts[quad[3]];

      const ex1 = v1[0] - v0[0], ey1 = v1[1] - v0[1], ez1 = v1[2] - v0[2];
      const ex2 = v2[0] - v0[0], ey2 = v2[1] - v0[1], ez2 = v2[2] - v0[2];
      let nx = ey1 * ez2 - ez1 * ey2;
      let ny = ez1 * ex2 - ex1 * ez2;
      let nz = ex1 * ey2 - ey1 * ex2;
      const len = Math.sqrt(nx * nx + ny * ny + nz * nz) || 1;
      nx /= len; ny /= len; nz /= len;

      const pushTri = (a: [number, number, number], b: [number, number, number], c: [number, number, number]) => {
        for (const p of [a, b, c]) {
          vanePositions.push(p[0], p[1], p[2]);
          vaneNormals.push(nx, ny, nz);
        }
      };
      pushTri(v0, v1, v2);
      pushTri(v0, v2, v3);
    }

    for (const [ea, eb] of HULL_EDGE_PAIRS) {
      vaneEdgePositions.push(pts[ea][0], pts[ea][1], pts[ea][2]);
      vaneEdgePositions.push(pts[eb][0], pts[eb][1], pts[eb][2]);
    }

    if (i % 9 === 0) {
      const pts2 = hullVertices(R0, R1, cu1, su1, cu2, su2);
      for (const quad of HULL_FACE_QUADS) {
        const v0 = pts2[quad[0]], v1 = pts2[quad[1]], v2 = pts2[quad[2]], v3 = pts2[quad[3]];

        const ex1 = v1[0] - v0[0], ey1 = v1[1] - v0[1], ez1 = v1[2] - v0[2];
        const ex2 = v2[0] - v0[0], ey2 = v2[1] - v0[1], ez2 = v2[2] - v0[2];
        let nx = ey1 * ez2 - ez1 * ey2;
        let ny = ez1 * ex2 - ex1 * ez2;
        let nz = ex1 * ey2 - ey1 * ex2;
        const len = Math.sqrt(nx * nx + ny * ny + nz * nz) || 1;
        nx /= len; ny /= len; nz /= len;

        const pushTri = (a: [number, number, number], b: [number, number, number], c: [number, number, number]) => {
          for (const p of [a, b, c]) {
            postPositions.push(p[0], p[1], p[2]);
            postNormals.push(nx, ny, nz);
          }
        };
        pushTri(v0, v1, v2);
        pushTri(v0, v2, v3);
      }

      for (const [ea, eb] of HULL_EDGE_PAIRS) {
        postEdgePositions.push(pts2[ea][0], pts2[ea][1], pts2[ea][2]);
        postEdgePositions.push(pts2[eb][0], pts2[eb][1], pts2[eb][2]);
      }
    }
  }

  // Merge vane and post into single geometries
  const allPos = new Float32Array([...vanePositions, ...postPositions]);
  const allNormals = new Float32Array([...vaneNormals, ...postNormals]);
  const allEdges = new Float32Array([...vaneEdgePositions, ...postEdgePositions]);

  const geo = new THREE.BufferGeometry();
  geo.setAttribute("position", new THREE.BufferAttribute(allPos, 3));
  geo.setAttribute("normal", new THREE.BufferAttribute(allNormals, 3));

  const edgeGeo = new THREE.BufferGeometry();
  edgeGeo.setAttribute("position", new THREE.BufferAttribute(allEdges, 3));

  const mat = new THREE.MeshStandardMaterial({
    color: 0x555555,
    roughness: 0.5,
    metalness: 0.3,
    side: THREE.DoubleSide,
  });
  const mesh = new THREE.Mesh(geo, mat);
  mesh.castShadow = true;

  const edgeMat = new THREE.LineBasicMaterial({ color: 0x888888 });
  const edges = new THREE.LineSegments(edgeGeo, edgeMat);

  return { mesh, edges };
}

const WASHER_CUBE_COUNT = 8000;

export function createWasherSample(): DemoSample {
  return {
    id: "washer",
    name: "Benchmark / Washer",
    create(_runtime: Box3DRuntime, scene: THREE.Scene) {
      const { defaultWorkerCount, maxWorkerCount: maxWorkers } = getWorkerCounts();
      let wc = defaultWorkerCount;

      let workerWorldState: WorkerWorldState | null = null;
      let bodyCount = WASHER_CUBE_COUNT + 1;
      let positions: Float32Array | null = null;
      let rotations: Float32Array | null = null;
      let awake: Uint8Array | null = null;
      let colors: Uint32Array | null = null;
      let state: Int32Array | null = null;
      let projectilePositions: Float32Array | null = null;
      let projectileRotations: Float32Array | null = null;
      let projectileAwake: Uint8Array | null = null;
      let projectileDebugColors: Uint32Array | null = null;
      let lastVersion = -1;
      let colorCache: Uint32Array | null = null;

      const projectileMeshes: THREE.Mesh[] = [];
      const projectileColorCache = new Uint32Array(MAX_PROJECTILES);

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
      drumGroup.quaternion.set(0, 0, 0, 1);

      const { mesh: drumMesh, edges: drumEdges } = buildDrumMeshes();
      drumGroup.add(drumMesh);
      drumGroup.add(drumEdges);

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
          positions = new Float32Array(ready.positions);
          rotations = new Float32Array(ready.rotations);
          awake = new Uint8Array(ready.awake);
          colors = new Uint32Array(ready.colors);
          projectilePositions = new Float32Array(ready.projectilePositions);
          projectileRotations = new Float32Array(ready.projectileRotations);
          projectileAwake = new Uint8Array(ready.projectileAwake);
          projectileDebugColors = new Uint32Array(ready.projectileColors);
          state = new Int32Array(ready.state);
          colorCache = new Uint32Array(WASHER_CUBE_COUNT);
        } else if (message.type === "error") {
          console.error(`Physics worker error: ${message.message}`);
        }
      });
      worker.postMessage({ type: "init", data: {}, workerCount: wc, maxWorkers, wasmVersion: wasmBuildVersion, wasmVariant: getWasmVariant() });

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
              projectileColorCache[projectileMeshes.length - 1] = bone.color;
            }
            worker.postMessage({ type: "spawn-ragdoll", origin, velocity });
            return;
          }
          const projectileMesh = new THREE.Mesh(new THREE.SphereGeometry(0.3, 12, 8), new THREE.MeshStandardMaterial({ color: 0xf59e0b }));
          projectileMesh.castShadow = true;
          projectileMesh.position.set(origin[0], origin[1], origin[2]);
          scene.add(projectileMesh);
          projectileMeshes.push(projectileMesh);
          projectileColorCache[projectileMeshes.length - 1] = spin ? 0x8b5cf6 : 0xf59e0b;
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
          if (positions === null || rotations === null || awake === null || colors === null || state === null || colorCache === null) return;
          const version = Atomics.load(state, SNAPSHOT_VERSION_INDEX);
          if (version === lastVersion) return;
          lastVersion = version;

          let needsMatrixUpdate = false;
          let needsColorUpdate = false;
          drumGroup.position.set(positions[0], positions[1], positions[2]);
          drumGroup.quaternion.set(rotations[0], rotations[1], rotations[2], rotations[3]);

          const cubeCount = Math.min(WASHER_CUBE_COUNT, Math.max(0, bodyCount - 1));
          for (let i = 0; i < cubeCount; i++) {
            const bodyIndex = i + 1;
            const isAwake = awake[bodyIndex] !== 0;
            if (isAwake) {
              const pOff = bodyIndex * 3;
              const rOff = bodyIndex * 4;
              dummy.scale.set(1, 1, 1);
              dummy.position.set(positions[pOff], positions[pOff + 1], positions[pOff + 2]);
              dummy.quaternion.set(rotations[rOff], rotations[rOff + 1], rotations[rOff + 2], rotations[rOff + 3]);
              dummy.updateMatrix();
              mesh.setMatrixAt(i, dummy.matrix);
              needsMatrixUpdate = true;
            }
            const colorHex = colors[bodyIndex] & 0xffffff;
            if ((colorCache[i] & 0xffffff) !== colorHex) {
              mesh.setColorAt(i, debugColor.setHex(colorHex));
              colorCache[i] = colorHex;
              needsColorUpdate = true;
            }
          }
          if (needsMatrixUpdate) mesh.instanceMatrix.needsUpdate = true;
          if (needsColorUpdate) mesh.instanceColor!.needsUpdate = true;

          if (projectilePositions !== null && projectileRotations !== null && projectileAwake !== null && projectileDebugColors !== null) {
            const projectileCount = Math.min(Atomics.load(state, SNAPSHOT_PROJECTILE_COUNT_INDEX), projectileMeshes.length);
            for (let i = 0; i < projectileCount; i++) {
              const pOff = i * 3;
              const rOff = i * 4;
              projectileMeshes[i].position.set(projectilePositions[pOff], projectilePositions[pOff + 1], projectilePositions[pOff + 2]);
              projectileMeshes[i].quaternion.set(projectileRotations[rOff], projectileRotations[rOff + 1], projectileRotations[rOff + 2], projectileRotations[rOff + 3]);
              const colorHex = projectileDebugColors[i] & 0xffffff;
              if ((projectileColorCache[i] & 0xffffff) !== colorHex) {
                const material = projectileMeshes[i].material as THREE.MeshStandardMaterial;
                material.color.setHex(colorHex);
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
          scene.remove(drumGroup);
          drumMesh.geometry.dispose();
          (drumMesh.material as THREE.Material).dispose();
          drumEdges.geometry.dispose();
          (drumEdges.material as THREE.Material).dispose();
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
