import * as THREE from "three";
import { B3_PI, BodyType, type BodyHandle, type Box3DRuntime } from "box3d-wasm";
import type { DemoBody, DemoSample, SolverParams } from "./types";
import { SNAPSHOT_VERSION_INDEX } from "../physics-worker-protocol";
import {
  createShaderInstancedSample,
  createWorkerSampleShell,
} from "./shader-instanced-host";

const dummy = new THREE.Object3D();
const awakeColor = new THREE.Color(0xd2b48c);
const debugColor = new THREE.Color();
type WasherRenderMode = "matrix" | "shader";

const ANGLE_STEP = B3_PI / 18;
const QO_ANGLE = 0.1 * ANGLE_STEP;
const R0 = 14, R1 = 16, R2 = 18;
const WASHER_CUBE_COUNT = 8000;
const WASHER_CUBE_COLOR = 0xd2b48c;
const WASHER_CAMERA = { position: [25, 20, 55] as [number, number, number], target: [0, 15, 0] as [number, number, number] };

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

function buildDrumMeshes(): { mesh: THREE.Mesh; edges: THREE.LineSegments } {
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

function forEachWasherCube(callback: (position: [number, number, number], color: number) => void): void {
  const a = 0.2;
  const gridCount = 20;
  for (let i = 0; i < gridCount; i++) {
    const x = -2 * a * gridCount + i * 4 * a;
    for (let j = 0; j < gridCount; j++) {
      const y = -2 * a * gridCount + 21 + j * 4 * a;
      for (let k = 0; k < gridCount; k++) {
        const z = -2 * a * gridCount + k * 4 * a;
        callback([x, y, z], WASHER_CUBE_COLOR);
      }
    }
  }
}

function attachWasherDrum(scene: THREE.Scene): {
  drumGroup: THREE.Group;
  sync(positions: Float32Array, rotations: Float32Array): void;
  dispose(): void;
} {
  const drumGroup = new THREE.Group();
  drumGroup.position.set(0, 21, 0);
  drumGroup.quaternion.set(0, 0, 0, 1);
  const { mesh: drumMesh, edges: drumEdges } = buildDrumMeshes();
  drumGroup.add(drumMesh);
  drumGroup.add(drumEdges);
  scene.add(drumGroup);
  return {
    drumGroup,
    sync(positions, rotations) {
      drumGroup.position.set(positions[0], positions[1], positions[2]);
      drumGroup.quaternion.set(rotations[0], rotations[1], rotations[2], rotations[3]);
    },
    dispose() {
      scene.remove(drumGroup);
      drumMesh.geometry.dispose();
      (drumMesh.material as THREE.Material).dispose();
      drumEdges.geometry.dispose();
      (drumEdges.material as THREE.Material).dispose();
    },
  };
}

function getWasherRenderMode(): WasherRenderMode {
  const configured = (globalThis as { __BOX3D_WASHER_RENDER_MODE?: unknown }).__BOX3D_WASHER_RENDER_MODE;
  if (configured === "matrix" || configured === "shader") return configured;
  const param = new URL(globalThis.location.href).searchParams.get("washerRender");
  return param === "matrix" ? "matrix" : "shader";
}

function createWasherShaderSample(
  id: string,
  name: string,
  options: { physicsCharts?: boolean },
): DemoSample {
  const sample = createShaderInstancedSample({
    id,
    name,
    createWorker: () => new Worker(new URL("./washer.worker.ts", import.meta.url), { type: "module" }),
    instanceCount: WASHER_CUBE_COUNT,
    groundSize: [120, 2, 120],
    camera: WASHER_CAMERA,
    profile: options.physicsCharts ?? true,
    shape: { kind: "box", size: 0.4 },
    defaultColor: WASHER_CUBE_COLOR,
    bodyOffset: 1,
    forEachInstance: forEachWasherCube,
    setupScene(scene) {
      const drum = attachWasherDrum(scene);
      return {
        sync(ctx) {
          drum.sync(ctx.positions, ctx.rotations);
        },
        dispose() {
          drum.dispose();
        },
      };
    },
    onKey(key, { worker }) {
      if (key === "t" || key === "T") {
        worker.postMessage({ type: "toggle-worker-count" });
      }
    },
    info: ({ workerCount, colorMode }) =>
      `Washer | ${WASHER_CUBE_COUNT} cubes | worker simulation | ${workerCount} workers | shader render | ${colorMode} colors`,
  });
  return sample;
}

function createWasherMatrixSample(
  id: string,
  name: string,
  options: { physicsCharts?: boolean },
): DemoSample {
  return {
    id,
    name,
    create(_runtime: Box3DRuntime, scene: THREE.Scene, initialSolverParams?: SolverParams) {
      let colorMode: "light" | "full" = localStorage.getItem("box3d:color-mode") === "full" ? "full" : "light";
      let bodyCount = WASHER_CUBE_COUNT + 1;
      const bodies: DemoBody[] = [];
      const colorCache = new Uint32Array(WASHER_CUBE_COUNT);

      const groundGeom = new THREE.BoxGeometry(120, 2, 120);
      const groundMat = new THREE.MeshStandardMaterial({ color: 0x222222 });
      const groundMesh = new THREE.Mesh(groundGeom, groundMat);
      groundMesh.position.set(0, -1, 0);
      groundMesh.receiveShadow = true;
      scene.add(groundMesh);
      bodies.push({ handle: 0 as BodyHandle, mesh: groundMesh, type: BodyType.Static });

      const drum = attachWasherDrum(scene);
      bodies.push({ handle: 1 as BodyHandle, mesh: drum.drumGroup as unknown as THREE.Mesh, type: BodyType.Dynamic });

      const cubeGeom = new THREE.BoxGeometry(0.4, 0.4, 0.4);
      const cubeMat = new THREE.MeshStandardMaterial({ color: WASHER_CUBE_COLOR });
      const matrixMesh = new THREE.InstancedMesh(cubeGeom, cubeMat, WASHER_CUBE_COUNT);
      matrixMesh.instanceMatrix.setUsage(THREE.DynamicDrawUsage);
      matrixMesh.castShadow = true;
      matrixMesh.receiveShadow = true;

      let idx = 0;
      forEachWasherCube((position, color) => {
        dummy.position.set(position[0], position[1], position[2]);
        dummy.scale.set(1, 1, 1);
        dummy.quaternion.set(0, 0, 0, 1);
        dummy.updateMatrix();
        matrixMesh.setMatrixAt(idx, dummy.matrix);
        matrixMesh.setColorAt(idx, awakeColor.setHex(color));
        colorCache[idx] = color;
        idx++;
      });
      matrixMesh.instanceMatrix.needsUpdate = true;
      matrixMesh.instanceColor!.needsUpdate = true;
      scene.add(matrixMesh);

      const shell = createWorkerSampleShell(scene, {
        createWorker: () => new Worker(new URL("./washer.worker.ts", import.meta.url), { type: "module" }),
        initialSolverParams,
        colorMode,
        onReady(ready) {
          bodyCount = ready.count;
        },
      });

      return {
        world: shell.world,
        bodies,
        controls: [],
        profile: options.physicsCharts ?? true,
        info: `Washer | ${WASHER_CUBE_COUNT} cubes | worker simulation | ${shell.getWorkerCount()} workers | matrix render | ${colorMode} colors`,
        camera: WASHER_CAMERA,
        onKey(key: string) {
          if (key === "t" || key === "T") {
            shell.worker.postMessage({ type: "toggle-worker-count" });
          } else if (key === "c" || key === "C") {
            colorMode = colorMode === "full" ? "light" : "full";
            localStorage.setItem("box3d:color-mode", colorMode);
            shell.worker.postMessage({ type: "set-color-mode", mode: colorMode });
            console.log(`[washer] color mode: ${colorMode}`);
          }
        },
        spawnProjectile: shell.spawnProjectile,
        startMouseDragRay: shell.startMouseDragRay,
        updateMouseDragRay: shell.updateMouseDragRay,
        stopMouseDrag: shell.stopMouseDrag,
        setPaused: shell.setPaused,
        stepOnce: shell.stepOnce,
        sendSolverParams: shell.sendSolverParams,
        step() {
          const snap = shell.getSnapshot();
          if (snap === null) return;
          const version = Atomics.load(snap.state, SNAPSHOT_VERSION_INDEX);
          if (version === shell.getLastVersion()) return;
          shell.setLastVersion(version);

          drum.sync(snap.positions, snap.rotations);

          let needsMatrixUpdate = false;
          let needsColorUpdate = false;
          const cubeCount = Math.min(WASHER_CUBE_COUNT, Math.max(0, bodyCount - 1));
          for (let i = 0; i < cubeCount; i++) {
            const bodyIndex = i + 1;
            const isAwake = snap.awake[bodyIndex] !== 0;
            if (isAwake) {
              const pOff = bodyIndex * 3;
              const rOff = bodyIndex * 4;
              dummy.scale.set(1, 1, 1);
              dummy.position.set(snap.positions[pOff], snap.positions[pOff + 1], snap.positions[pOff + 2]);
              dummy.quaternion.set(snap.rotations[rOff], snap.rotations[rOff + 1], snap.rotations[rOff + 2], snap.rotations[rOff + 3]);
              dummy.updateMatrix();
              matrixMesh.setMatrixAt(i, dummy.matrix);
              needsMatrixUpdate = true;
            }
            const colorHex = snap.colors[bodyIndex] & 0xffffff;
            if ((colorCache[i] & 0xffffff) !== colorHex) {
              matrixMesh.setColorAt(i, debugColor.setHex(colorHex));
              colorCache[i] = colorHex;
              needsColorUpdate = true;
            }
          }
          if (needsMatrixUpdate) matrixMesh.instanceMatrix.needsUpdate = true;
          if (needsColorUpdate) matrixMesh.instanceColor!.needsUpdate = true;

          shell.syncProjectiles(shell.projectileMeshes, shell.projectileColorCache);
        },
        dispose() {
          shell.disposeProjectiles(scene, shell.projectileMeshes);
          shell.dispose();
          scene.remove(groundMesh);
          groundGeom.dispose();
          groundMat.dispose();
          drum.dispose();
          scene.remove(matrixMesh);
          cubeGeom.dispose();
          cubeMat.dispose();
        },
      };
    },
  };
}

export function createWasherSample(forcedRenderMode?: WasherRenderMode, options: { physicsCharts?: boolean; idSuffix?: string; nameSuffix?: string } = {}): DemoSample {
  const renderMode = forcedRenderMode ?? getWasherRenderMode();
  const id = `${forcedRenderMode === undefined ? "washer" : `washer-${forcedRenderMode}`}${options.idSuffix ?? ""}`;
  const suffix = options.nameSuffix ?? (forcedRenderMode === "shader" ? " (optimized)" : forcedRenderMode === "matrix" ? " (legacy matrix)" : " (optimized)");
  const name = `Benchmark / Washer${suffix}`;
  if (renderMode === "shader") return createWasherShaderSample(id, name, options);
  return createWasherMatrixSample(id, name, options);
}
