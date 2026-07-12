import * as THREE from "three";
import { BodyType, type BodyHandle, type Box3DRuntime } from "box3d-wasm";
import type { DemoBody, DemoSample, DemoSampleInstance, SolverParams } from "./types";
import type { PhysicsWorkerMessage, PhysicsWorkerReady } from "../physics-worker-protocol";
import { MAX_PROJECTILES, SNAPSHOT_PROJECTILE_COUNT_INDEX, SNAPSHOT_VERSION_INDEX } from "../physics-worker-protocol";
import { createWorkerWorld, type WorkerWorldState } from "../worker-world-bridge";
import { RAGDOLL_RENDER_BONES, ragdollCapsuleMesh } from "../ragdoll-render";
import {
  bindSnapshotTransforms,
  createShaderBoxMesh,
  createShaderInstanceMesh,
  hexToRgb,
  type ShaderInstanceMesh,
} from "../shader-instanced-boxes";
import { wasmBuildVersion } from "virtual:wasm-version";
import { getWasmBaseUrl, getWasmVariant, getWorkerCounts } from "./shared";

export type ShaderSyncContext = {
  bodyCount: number;
  positions: Float32Array;
  rotations: Float32Array;
  colors: Uint32Array | null;
  state: Int32Array;
};

export type ShaderLayerGeometry =
  | { kind: "box"; size?: number | [number, number, number] }
  | { kind: "sphere"; radius: number; widthSegments?: number; heightSegments?: number }
  | { kind: "geometry"; create: () => THREE.BufferGeometry };

export type ShaderLayerBind =
  | { mode: "direct"; bodyOffset?: number }
  | { mode: "gather"; bodyIndex: (instanceIndex: number, ctx: ShaderSyncContext) => number };

export type ShaderLayerSpec = {
  id?: string;
  capacity: number;
  geometry: ShaderLayerGeometry;
  bind: ShaderLayerBind;
  /** snapshot = per-body debug colors; fixed = layer fixedColor / seed; none = leave seeded colors */
  colors?: "snapshot" | "fixed" | "none";
  fixedColor?: number;
  /** Y used to hide unused gather instances before they are needed */
  parkY?: number;
  forEachInstance?: (callback: (position: [number, number, number], color: number) => void) => void;
  resolveInstanceCount?: (ctx: ShaderSyncContext) => number;
};

export type ShaderSceneSetup = {
  sync?(ctx: ShaderSyncContext): void;
  dispose(): void;
};

export type ShaderInstancedHostSpec = {
  id: string;
  name: string;
  createWorker: () => Worker;
  layers: ShaderLayerSpec[];
  /** Three.js ground dimensions (full extents), same convention as generic-host. */
  groundSize: [number, number, number];
  groundKind?: "box" | "plane" | "none";
  groundPosition?: [number, number, number];
  camera: { position: [number, number, number]; target: [number, number, number] };
  profile?: boolean;
  info?: string | ((ctx: { workerCount: number; colorMode: "light" | "full"; bodyCount: number }) => string);
  getInfo?: (ctx: { workerCount: number; colorMode: "light" | "full"; bodyCount: number }) => string | undefined;
  defaultColor?: number;
  initSolverParams?: (params: SolverParams | undefined) => SolverParams | undefined;
  mapSolverParams?: (params: SolverParams) => SolverParams;
  setupScene?: (scene: THREE.Scene) => ShaderSceneSetup;
  resolveBodyCount?: (state: Int32Array, readyCount: number) => number;
  onKey?: (key: string, api: { worker: Worker }) => void;
  /** Default projectile radius/color; rain uses a slightly smaller orange ball. */
  projectile?: { radius?: number; color?: number; metalness?: number };
};

/** Convenience single-layer shape used by box/sphere benchmark samples. */
export type ShaderInstancedSimpleSpec = Omit<ShaderInstancedHostSpec, "layers"> & {
  instanceCount: number;
  shape:
    | { kind: "box"; size?: number | [number, number, number] }
    | { kind: "sphere"; radius: number; widthSegments?: number; heightSegments?: number };
  forEachInstance: (callback: (position: [number, number, number], color: number) => void) => void;
  bodyOffset?: number;
};

type RuntimeLayer = {
  spec: ShaderLayerSpec;
  mesh: ShaderInstanceMesh;
  colorCache: Uint32Array;
  boundInstanceCount: number;
};

export type WorkerSampleShellOptions = {
  createWorker: () => Worker;
  initialSolverParams?: SolverParams;
  initSolverParams?: (params: SolverParams | undefined) => SolverParams | undefined;
  mapSolverParams?: (params: SolverParams) => SolverParams;
  colorMode?: "light" | "full";
  projectile?: { radius?: number; color?: number; metalness?: number };
  onReady?: (ready: PhysicsWorkerReady, state: WorkerWorldState) => void;
};

export type WorkerSampleShell = {
  worker: Worker;
  world: ReturnType<typeof createWorkerWorld>;
  getWorkerCount: () => number;
  setWorkerCount: (count: number) => void;
  getSnapshot: () => WorkerWorldState | null;
  getLastVersion: () => number;
  setLastVersion: (version: number) => void;
  consumeSnapshotVersion: () => boolean;
  spawnProjectile: DemoSampleInstance["spawnProjectile"];
  startMouseDragRay: NonNullable<DemoSampleInstance["startMouseDragRay"]>;
  updateMouseDragRay: NonNullable<DemoSampleInstance["updateMouseDragRay"]>;
  stopMouseDrag: NonNullable<DemoSampleInstance["stopMouseDrag"]>;
  setPaused: NonNullable<DemoSampleInstance["setPaused"]>;
  stepOnce: NonNullable<DemoSampleInstance["stepOnce"]>;
  sendSolverParams: NonNullable<DemoSampleInstance["sendSolverParams"]>;
  syncProjectiles: (sceneMeshes: THREE.Mesh[], colorCache: Uint32Array) => void;
  disposeProjectiles: (scene: THREE.Scene, meshes: THREE.Mesh[]) => void;
  dispose: () => void;
  projectileMeshes: THREE.Mesh[];
  projectileColorCache: Uint32Array;
};

/** Shared worker / SAB / projectile / drag lifecycle for shader host and Washer matrix mode. */
export function createWorkerSampleShell(
  scene: THREE.Scene,
  options: WorkerSampleShellOptions,
): WorkerSampleShell {
  const { defaultWorkerCount, maxWorkerCount: maxWorkers, poolSize } = getWorkerCounts();
  let wc = Math.min(maxWorkers, Math.max(1, options.initialSolverParams?.workerCount ?? defaultWorkerCount));
  let workerWorldState: WorkerWorldState | null = null;
  let lastVersion = -1;

  const projectileMeshes: THREE.Mesh[] = [];
  const projectileColorCache = new Uint32Array(MAX_PROJECTILES);
  const projectileRadius = options.projectile?.radius ?? 0.3;
  const projectileColor = options.projectile?.color ?? 0xf59e0b;
  const projectileMetalness = options.projectile?.metalness ?? 0;

  const worker = options.createWorker();
  const world = createWorkerWorld(worker, () => workerWorldState, () => wc);

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
      options.onReady?.(ready, workerWorldState);
    } else if (message.type === "error") {
      console.error(`Physics worker error: ${message.message}`);
    }
  });

  const solverParams = options.initSolverParams !== undefined
    ? options.initSolverParams(options.initialSolverParams)
    : options.initialSolverParams;
  worker.postMessage({
    type: "init",
    data: {},
    workerCount: wc,
    maxWorkers,
    poolSize,
    solverParams,
    wasmVersion: wasmBuildVersion,
    wasmVariant: getWasmVariant(),
    wasmBaseUrl: getWasmBaseUrl(),
  });
  if (options.colorMode !== undefined) {
    worker.postMessage({ type: "set-color-mode", mode: options.colorMode });
  }

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
    const mesh = new THREE.Mesh(
      new THREE.SphereGeometry(projectileRadius, 12, 8),
      new THREE.MeshStandardMaterial({
        color: spin ? 0x8b5cf6 : projectileColor,
        metalness: projectileMetalness,
        roughness: projectileMetalness > 0 ? 0.25 : 0.75,
      }),
    );
    mesh.castShadow = true;
    mesh.position.set(origin[0], origin[1], origin[2]);
    scene.add(mesh);
    projectileMeshes.push(mesh);
    projectileColorCache[projectileMeshes.length - 1] = spin ? 0x8b5cf6 : projectileColor;
    worker.postMessage({ type: "spawn-projectile", origin, velocity });
  }

  return {
    worker,
    world,
    getWorkerCount: () => wc,
    setWorkerCount: (count) => {
      wc = Math.min(maxWorkers, Math.max(1, count));
    },
    getSnapshot: () => workerWorldState,
    getLastVersion: () => lastVersion,
    setLastVersion: (version) => {
      lastVersion = version;
    },
    consumeSnapshotVersion: () => {
      if (workerWorldState === null) return false;
      const version = Atomics.load(workerWorldState.state, SNAPSHOT_VERSION_INDEX);
      if (version === lastVersion) return false;
      lastVersion = version;
      return true;
    },
    spawnProjectile,
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
      if (params.workerCount !== undefined) {
        wc = Math.min(maxWorkers, Math.max(1, params.workerCount));
      }
      const mapped = options.mapSolverParams !== undefined ? options.mapSolverParams(params) : params;
      worker.postMessage({ type: "set-solver-params", params: mapped });
    },
    syncProjectiles(meshes, colorCache) {
      const snap = workerWorldState;
      if (snap === null) return;
      const projectileCount = Math.min(Atomics.load(snap.state, SNAPSHOT_PROJECTILE_COUNT_INDEX), meshes.length);
      for (let i = 0; i < projectileCount; i++) {
        const pOff = i * 3;
        const rOff = i * 4;
        meshes[i].position.set(snap.projectilePositions[pOff], snap.projectilePositions[pOff + 1], snap.projectilePositions[pOff + 2]);
        meshes[i].quaternion.set(
          snap.projectileRotations[rOff],
          snap.projectileRotations[rOff + 1],
          snap.projectileRotations[rOff + 2],
          snap.projectileRotations[rOff + 3],
        );
        const colorHex = snap.projectileColors[i] & 0xffffff;
        if ((colorCache[i] & 0xffffff) !== colorHex) {
          (meshes[i].material as THREE.MeshStandardMaterial).color.setHex(colorHex);
          colorCache[i] = colorHex;
        }
      }
    },
    disposeProjectiles(targetScene, meshes) {
      for (const mesh of meshes) {
        targetScene.remove(mesh);
        mesh.geometry.dispose();
        const material = mesh.material;
        if (Array.isArray(material)) material.forEach((entry) => entry.dispose());
        else material.dispose();
      }
      meshes.length = 0;
    },
    dispose() {
      worker.postMessage({ type: "dispose" });
      worker.terminate();
    },
    projectileMeshes,
    projectileColorCache,
  };
}

function createLayerMesh(spec: ShaderLayerSpec): ShaderInstanceMesh {
  if (spec.geometry.kind === "box") {
    return createShaderBoxMesh(spec.capacity, spec.geometry.size ?? 1, { shadows: false });
  }
  if (spec.geometry.kind === "sphere") {
    const base = new THREE.SphereGeometry(
      spec.geometry.radius,
      spec.geometry.widthSegments ?? 12,
      spec.geometry.heightSegments ?? 8,
    );
    const mesh = createShaderInstanceMesh(base, spec.capacity, { shadows: false });
    base.dispose();
    return mesh;
  }
  const geom = spec.geometry.create();
  const mesh = createShaderInstanceMesh(geom, spec.capacity, { shadows: false });
  geom.dispose();
  return mesh;
}

function seedLayer(layer: RuntimeLayer, defaultColor: number): void {
  const { spec, mesh } = layer;
  const parkY = spec.parkY ?? -1000;
  const fixed = spec.fixedColor ?? defaultColor;
  let idx = 0;
  const seedOne = (position: [number, number, number], color: number) => {
    if (idx >= spec.capacity) return;
    const pOff = idx * 3;
    const qOff = idx * 4;
    mesh.positionArray[pOff] = position[0];
    mesh.positionArray[pOff + 1] = position[1];
    mesh.positionArray[pOff + 2] = position[2];
    mesh.quaternionArray[qOff] = 0;
    mesh.quaternionArray[qOff + 1] = 0;
    mesh.quaternionArray[qOff + 2] = 0;
    mesh.quaternionArray[qOff + 3] = 1;
    hexToRgb(color, mesh.colorArray, pOff);
    layer.colorCache[idx] = color;
    idx++;
  };
  if (spec.forEachInstance !== undefined) {
    spec.forEachInstance(seedOne);
  } else {
    for (let i = 0; i < spec.capacity; i++) {
      seedOne([0, parkY, 0], fixed);
    }
  }
  if (spec.bind.mode === "gather" || spec.resolveInstanceCount !== undefined) {
    (mesh.mesh.geometry as THREE.InstancedBufferGeometry).instanceCount = 0;
  }
  mesh.positionAttribute.needsUpdate = true;
  mesh.quaternionAttribute.needsUpdate = true;
  mesh.colorAttribute.needsUpdate = true;
}

function syncLayer(layer: RuntimeLayer, ctx: ShaderSyncContext): void {
  const { spec, mesh } = layer;
  const instanceCount = Math.min(
    spec.capacity,
    spec.resolveInstanceCount !== undefined
      ? Math.max(0, spec.resolveInstanceCount(ctx))
      : spec.bind.mode === "direct"
        ? Math.max(0, ctx.bodyCount - (spec.bind.bodyOffset ?? 0))
        : spec.capacity,
  );
  (mesh.mesh.geometry as THREE.InstancedBufferGeometry).instanceCount = instanceCount;

  if (spec.bind.mode === "direct") {
    const bodyOffset = spec.bind.bodyOffset ?? 0;
    // Zero-copy SAB attributes; rebind only when the visible range changes.
    if (instanceCount > 0 && layer.boundInstanceCount !== instanceCount) {
      bindSnapshotTransforms(mesh, ctx.positions, ctx.rotations, instanceCount, bodyOffset);
      layer.boundInstanceCount = instanceCount;
    }
    mesh.positionAttribute.needsUpdate = true;
    mesh.quaternionAttribute.needsUpdate = true;
  } else {
    for (let i = 0; i < instanceCount; i++) {
      const bodyIndex = spec.bind.bodyIndex(i, ctx);
      const pOff = i * 3;
      const qOff = i * 4;
      const srcP = bodyIndex * 3;
      const srcQ = bodyIndex * 4;
      mesh.positionArray[pOff] = ctx.positions[srcP];
      mesh.positionArray[pOff + 1] = ctx.positions[srcP + 1];
      mesh.positionArray[pOff + 2] = ctx.positions[srcP + 2];
      mesh.quaternionArray[qOff] = ctx.rotations[srcQ];
      mesh.quaternionArray[qOff + 1] = ctx.rotations[srcQ + 1];
      mesh.quaternionArray[qOff + 2] = ctx.rotations[srcQ + 2];
      mesh.quaternionArray[qOff + 3] = ctx.rotations[srcQ + 3];
    }
    mesh.positionAttribute.needsUpdate = true;
    mesh.quaternionAttribute.needsUpdate = true;
  }

  const colorMode = spec.colors ?? (spec.bind.mode === "direct" ? "snapshot" : "fixed");
  if (colorMode === "snapshot" && ctx.colors !== null) {
    const bodyOffset = spec.bind.mode === "direct" ? (spec.bind.bodyOffset ?? 0) : 0;
    let needsColorUpdate = false;
    for (let i = 0; i < instanceCount; i++) {
      const bodyIndex = spec.bind.mode === "direct" ? bodyOffset + i : spec.bind.bodyIndex(i, ctx);
      const colorHex = ctx.colors[bodyIndex] & 0xffffff;
      if ((layer.colorCache[i] & 0xffffff) !== colorHex) {
        hexToRgb(colorHex, mesh.colorArray, i * 3);
        layer.colorCache[i] = colorHex;
        needsColorUpdate = true;
      }
    }
    if (needsColorUpdate) mesh.colorAttribute.needsUpdate = true;
  }
}

function normalizeSpec(spec: ShaderInstancedHostSpec | ShaderInstancedSimpleSpec): ShaderInstancedHostSpec {
  if ("layers" in spec && spec.layers !== undefined) return spec;
  const simple = spec as ShaderInstancedSimpleSpec;
  return {
    ...simple,
    layers: [{
      capacity: simple.instanceCount,
      geometry: simple.shape,
      bind: { mode: "direct", bodyOffset: simple.bodyOffset ?? 0 },
      colors: "snapshot",
      fixedColor: simple.defaultColor,
      forEachInstance: simple.forEachInstance,
    }],
  };
}

export function createShaderInstancedSample(input: ShaderInstancedHostSpec | ShaderInstancedSimpleSpec): DemoSample {
  const spec = normalizeSpec(input);
  const defaultColor = spec.defaultColor ?? 0x60a5fa;
  const groundKind = spec.groundKind ?? "box";

  return {
    id: spec.id,
    name: spec.name,
    create(_runtime: Box3DRuntime, scene: THREE.Scene, initialSolverParams?: SolverParams) {
      let colorMode: "light" | "full" = localStorage.getItem("box3d:color-mode") === "full" ? "full" : "light";
      let bodyCount = 0;
      const bodies: DemoBody[] = [];

      const layers: RuntimeLayer[] = spec.layers.map((layerSpec) => {
        const mesh = createLayerMesh(layerSpec);
        const layer: RuntimeLayer = {
          spec: layerSpec,
          mesh,
          colorCache: new Uint32Array(layerSpec.capacity),
          boundInstanceCount: -1,
        };
        seedLayer(layer, defaultColor);
        scene.add(mesh.mesh);
        return layer;
      });

      let groundGeom: THREE.BufferGeometry | null = null;
      let groundMat: THREE.MeshStandardMaterial | null = null;
      let groundMesh: THREE.Mesh | null = null;
      if (groundKind !== "none") {
        groundGeom = groundKind === "plane"
          ? new THREE.PlaneGeometry(spec.groundSize[0], spec.groundSize[2])
          : new THREE.BoxGeometry(...spec.groundSize);
        groundMat = new THREE.MeshStandardMaterial({ color: 0x222222 });
        groundMesh = new THREE.Mesh(groundGeom, groundMat);
        const groundPos = spec.groundPosition ?? (groundKind === "plane" ? [0, 0, 0] : [0, -1, 0]);
        groundMesh.position.set(groundPos[0], groundPos[1], groundPos[2]);
        if (groundKind === "plane") groundMesh.rotation.x = -0.5 * Math.PI;
        groundMesh.receiveShadow = true;
        scene.add(groundMesh);
        bodies.push({ handle: 0 as BodyHandle, mesh: groundMesh, type: BodyType.Static });
      }

      const sceneSetup = spec.setupScene?.(scene);

      const shell = createWorkerSampleShell(scene, {
        createWorker: spec.createWorker,
        initialSolverParams,
        initSolverParams: spec.initSolverParams,
        mapSolverParams: spec.mapSolverParams,
        colorMode,
        projectile: spec.projectile,
        onReady(ready, state) {
          bodyCount = spec.resolveBodyCount !== undefined
            ? spec.resolveBodyCount(state.state, ready.count)
            : ready.count;
          const ctx: ShaderSyncContext = {
            bodyCount,
            positions: state.positions,
            rotations: state.rotations,
            colors: state.colors,
            state: state.state,
          };
          for (const layer of layers) syncLayer(layer, ctx);
          sceneSetup?.sync?.(ctx);
        },
      });

      function infoCtx() {
        return { workerCount: shell.getWorkerCount(), colorMode, bodyCount };
      }

      return {
        world: shell.world,
        bodies,
        controls: [],
        profile: spec.profile ?? true,
        info: typeof spec.info === "function" ? spec.info(infoCtx()) : spec.info,
        getInfo: spec.getInfo !== undefined ? () => spec.getInfo!(infoCtx()) : undefined,
        camera: spec.camera,
        onKey(key: string) {
          if (key === "c" || key === "C") {
            colorMode = colorMode === "full" ? "light" : "full";
            localStorage.setItem("box3d:color-mode", colorMode);
            shell.worker.postMessage({ type: "set-color-mode", mode: colorMode });
          }
          spec.onKey?.(key, { worker: shell.worker });
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
          if (!shell.consumeSnapshotVersion()) return;
          bodyCount = spec.resolveBodyCount !== undefined
            ? spec.resolveBodyCount(snap.state, snap.count)
            : snap.count;
          const ctx: ShaderSyncContext = {
            bodyCount,
            positions: snap.positions,
            rotations: snap.rotations,
            colors: snap.colors,
            state: snap.state,
          };
          for (const layer of layers) syncLayer(layer, ctx);
          sceneSetup?.sync?.(ctx);
          shell.syncProjectiles(shell.projectileMeshes, shell.projectileColorCache);
        },
        dispose() {
          shell.disposeProjectiles(scene, shell.projectileMeshes);
          shell.dispose();
          sceneSetup?.dispose();
          if (groundMesh !== null && groundGeom !== null && groundMat !== null) {
            scene.remove(groundMesh);
            groundGeom.dispose();
            groundMat.dispose();
          }
          for (const layer of layers) {
            scene.remove(layer.mesh.mesh);
            layer.mesh.dispose();
          }
        },
      };
    },
  };
}
