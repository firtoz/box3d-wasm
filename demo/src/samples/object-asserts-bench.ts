import * as THREE from "three";
import { BodyType, type BodyHandle, type Box3DRuntime, type JointHandle, type PhysicsWorld, type Quat, type Vec3 } from "box3d-wasm";
import {
  ObjectRuntime,
  areObjectAssertsCompiledIn,
  runObjectAssertBench,
  setObjectAssertGuardsEnabled,
  type BodyRef,
  type ObjectAssertBenchResult,
  type ObjectWorld,
} from "box3d-wasm/objects";
import type { DemoBody, DemoSample, DemoSampleInstance } from "./types";
import { syncBodies } from "./shared";

/** Pyramid of boxes + mixed shapes for a livelier scene; bench round-robins them. */
const PYRAMID_BASE = 10;
const SPHERE_COUNT = 24;
const CAPSULE_COUNT = 16;
const DEFAULT_ITERS = 50_000;
const DEFAULT_WARMUP = 5_000;
const DEFAULT_ROUNDS = 5;
/** Full kick-plan applications per timed sample; many samples → median. */
const SHOCKWAVE_PASSES = 40;
const SHOCKWAVE_WARMUP_PASSES = 20;
const SHOCKWAVE_ROUNDS = 21;
/** Relative-to-raw within this band is treated as a tie (timer noise). */
const REL_NOISE = 0.03;

const BOX_COLORS = [0x38bdf8, 0x818cf8, 0x34d399, 0xfbbf24, 0xf472b6];

/** Outward impulse wave so velocity writes are obvious in the viewport. */
const SHOCKWAVE_SPEED = 11;
const SHOCKWAVE_UP = 5.5;
const SHOCKWAVE_MS_PER_METER = 38;

const IDENTITY: Quat = [0, 0, 0, 1];
const ZERO: Vec3 = [0, 0, 0];

type SpawnPose = {
  position: Vec3;
  rotation: Quat;
};

type KickPlan = {
  delayMs: number;
  velocity: Vec3;
  angular: Vec3;
};

type LaneId = "raw" | "bare" | "on";

type Lane = {
  id: LaneId;
  label: string;
  scene: THREE.Scene;
  objectWorld: ObjectWorld;
  world: PhysicsWorld;
  ground: BodyRef;
  dynamicRefs: BodyRef[];
  spawns: SpawnPose[];
  bodies: DemoBody[];
};

type MultiShock = {
  t0: number;
  plan: KickPlan[];
  done: boolean[];
  /** CPU time spent applying kicks during the live staggered wave (ms). */
  liveApplyMs: { raw: number; bare: number; on: number };
};

type ShockwaveBenchResult = {
  /** Timed full-plan applications per sample. */
  passes: number;
  rounds: number;
  bodyCount: number;
  /** Median ns to set lin+ang velocity for one body. */
  nsPerBody: {
    rawHandles: number;
    objectsBare: number;
    objectsAssertsOn: number;
  };
  relativeToRaw: {
    objectsBare: number;
    objectsAssertsOn: number;
  };
  wrapperOverheadNs: number;
  assertOverheadNs: number;
  assertOverheadPctOfBare: number;
};

type BenchRunResult = {
  velocity: ObjectAssertBenchResult;
  shockwave: ShockwaveBenchResult;
  liveApplyMs: { raw: number; bare: number; on: number };
};

function el<K extends keyof HTMLElementTagNameMap>(
  tag: K,
  className?: string,
  text?: string,
): HTMLElementTagNameMap[K] {
  const node = document.createElement(tag);
  if (className !== undefined) node.className = className;
  if (text !== undefined) node.textContent = text;
  return node;
}

function cloneVec3(v: Vec3): Vec3 {
  return [v[0], v[1], v[2]];
}

function buildKickPlan(spawns: readonly SpawnPose[]): KickPlan[] {
  return spawns.map((spawn, i) => {
    const x = spawn.position[0];
    const z = spawn.position[2];
    const dist = Math.hypot(x, z);
    const angle = dist < 1e-3 ? (i / Math.max(spawns.length, 1)) * Math.PI * 2 : Math.atan2(z, x);
    const speed = SHOCKWAVE_SPEED + dist * 0.45;
    const spin = 4 + (i % 5) * 1.2;
    return {
      delayMs: dist * SHOCKWAVE_MS_PER_METER,
      velocity: [Math.cos(angle) * speed, SHOCKWAVE_UP + Math.min(dist, 10) * 0.2, Math.sin(angle) * speed],
      angular: [Math.sin(angle) * spin * 0.35, spin * ((i & 1) === 0 ? 1 : -1), -Math.cos(angle) * spin * 0.35],
    };
  });
}

function toneForRelative(rel: number): "good" | "ok" | "bad" {
  if (rel <= 1 + REL_NOISE) return "good";
  if (rel <= 1.25) return "ok";
  return "bad";
}

function toneForAssertDelta(pct: number): "good" | "ok" | "bad" {
  if (pct <= 2) return "good";
  if (pct <= 8) return "ok";
  return "bad";
}

function formatRelToRaw(key: string, rel: number): string {
  if (key === "raw") return "baseline";
  if (Math.abs(rel - 1) <= REL_NOISE) return "≈baseline";
  return `${rel.toFixed(2)}×`;
}

function median(values: number[]): number {
  const sorted = [...values].sort((a, b) => a - b);
  return sorted[Math.floor(sorted.length / 2)]!;
}

/** Apply kick plan indices via A/B/C pathways on the matching lanes. */
function applyShockwaveKicks(
  plan: readonly KickPlan[],
  indices: readonly number[],
  rawLane: Lane,
  bareLane: Lane,
  onLane: Lane,
): { rawMs: number; bareMs: number; onMs: number } {
  let t0 = performance.now();
  for (const i of indices) {
    const kick = plan[i]!;
    const handle = rawLane.dynamicRefs[i]!.handle;
    rawLane.world.setBodyLinearVelocity(handle, kick.velocity);
    rawLane.world.setBodyAngularVelocity(handle, kick.angular);
  }
  const rawMs = performance.now() - t0;

  let bareMs = 0;
  let onMs = 0;
  try {
    t0 = performance.now();
    setObjectAssertGuardsEnabled(false);
    for (const i of indices) {
      const kick = plan[i]!;
      const ref = bareLane.dynamicRefs[i]!;
      ref.setLinearVelocity(kick.velocity);
      ref.setAngularVelocity(kick.angular);
    }
    bareMs = performance.now() - t0;

    setObjectAssertGuardsEnabled(true);
    t0 = performance.now();
    for (const i of indices) {
      const kick = plan[i]!;
      const ref = onLane.dynamicRefs[i]!;
      ref.setLinearVelocity(kick.velocity);
      ref.setAngularVelocity(kick.angular);
    }
    onMs = performance.now() - t0;
  } finally {
    setObjectAssertGuardsEnabled(true);
  }
  return { rawMs, bareMs, onMs };
}

/**
 * Time shockwave lin+ang writes A/B/C on the **same** world.
 */
function timeShockwaveWrites(
  plan: readonly KickPlan[],
  lane: Lane,
  passes: number,
  warmupPasses: number,
  rounds: number,
): ShockwaveBenchResult {
  const n = plan.length;
  if (n === 0) throw new Error("timeShockwaveWrites requires a non-empty plan");
  const refs = lane.dynamicRefs;
  const world = lane.world;
  if (refs.length !== n) throw new Error("kick plan length must match lane body count");

  const handles: BodyHandle[] = refs.map((r) => r.handle);

  const applyRaw = () => {
    for (let i = 0; i < n; i++) {
      const kick = plan[i]!;
      const handle = handles[i]!;
      world.setBodyLinearVelocity(handle, kick.velocity);
      world.setBodyAngularVelocity(handle, kick.angular);
    }
  };
  const applyObjects = () => {
    for (let i = 0; i < n; i++) {
      const kick = plan[i]!;
      const ref = refs[i]!;
      ref.setLinearVelocity(kick.velocity);
      ref.setAngularVelocity(kick.angular);
    }
  };

  const nsPerBody = (fn: () => void): number => {
    const t0 = performance.now();
    for (let p = 0; p < passes; p++) fn();
    return (performance.now() - t0) * (1_000_000 / (n * passes));
  };

  try {
    setObjectAssertGuardsEnabled(false);
    for (let i = 0; i < warmupPasses; i++) applyObjects();
    setObjectAssertGuardsEnabled(true);
    for (let i = 0; i < warmupPasses; i++) applyObjects();
    for (let i = 0; i < warmupPasses; i++) applyRaw();

    const rawSamples: number[] = [];
    const bareSamples: number[] = [];
    const onSamples: number[] = [];

    for (let r = 0; r < rounds; r++) {
      const modes = [0, 1, 2].map((i) => (r + i) % 3);
      for (const mode of modes) {
        if (mode === 0) {
          rawSamples.push(nsPerBody(applyRaw));
        } else if (mode === 1) {
          setObjectAssertGuardsEnabled(false);
          bareSamples.push(nsPerBody(applyObjects));
        } else {
          setObjectAssertGuardsEnabled(true);
          onSamples.push(nsPerBody(applyObjects));
        }
      }
    }

    const rawMed = median(rawSamples);
    const bareMed = median(bareSamples);
    const onMed = median(onSamples);

    return {
      passes,
      rounds,
      bodyCount: n,
      nsPerBody: {
        rawHandles: rawMed,
        objectsBare: bareMed,
        objectsAssertsOn: onMed,
      },
      relativeToRaw: {
        objectsBare: bareMed / rawMed,
        objectsAssertsOn: onMed / rawMed,
      },
      wrapperOverheadNs: bareMed - rawMed,
      assertOverheadNs: onMed - bareMed,
      assertOverheadPctOfBare: ((onMed - bareMed) / bareMed) * 100,
    };
  } finally {
    setObjectAssertGuardsEnabled(true);
  }
}

type ChartRow = { key: string; label: string; ns: number; rel: number; tone: "good" | "ok" | "bad" };

type DeltaLine = { label: string; ns: number; pctOf: number };

function appendNsChart(
  panel: HTMLElement,
  title: string,
  rows: ChartRow[],
  deltas: DeltaLine[],
): void {
  const section = el("div", "assert-bench-section");
  section.append(el("div", "assert-bench-section-title", title));

  const maxNs = Math.max(...rows.map((r) => r.ns), 1e-6);
  const chart = el("div", "assert-bench-chart");
  for (const row of rows) {
    const rowEl = el("div", "assert-bench-row");
    const label = el("div", "assert-bench-label", row.label);
    const track = el("div", "assert-bench-track");
    const bar = el("div", `assert-bench-bar tone-${row.tone}`);
    bar.style.width = `${Math.max(4, (row.ns / maxNs) * 100)}%`;
    track.append(bar);
    const value = el("div", `assert-bench-value tone-${row.tone}`);
    value.textContent = `${row.ns.toFixed(1)} ns`;
    const rel = el("div", "assert-bench-rel", formatRelToRaw(row.key, row.rel));
    rowEl.append(label, track, value, rel);
    chart.append(rowEl);
  }
  section.append(chart);

  for (const d of deltas) {
    const pct = d.pctOf !== 0 ? (d.ns / d.pctOf) * 100 : 0;
    const deltaTone = toneForAssertDelta(Math.abs(pct));
    const delta = el("div", `assert-bench-delta tone-${deltaTone}`);
    const sign = d.ns >= 0 ? "+" : "";
    delta.innerHTML =
      `<span class="assert-bench-delta-label">${d.label}</span>` +
      `<span class="assert-bench-delta-value">${sign}${d.ns.toFixed(2)} ns` +
      ` <small>(${sign}${pct.toFixed(1)}%)</small></span>`;
    section.append(delta);
  }
  panel.append(section);
}

function pathwayRows(
  ns: { rawHandles: number; objectsBare: number; objectsAssertsOn: number },
  rel: { objectsBare: number; objectsAssertsOn: number },
): ChartRow[] {
  return [
    { key: "raw", label: "A · Raw handles", ns: ns.rawHandles, rel: 1, tone: "good" },
    {
      key: "bare",
      label: "B · Objects bare",
      ns: ns.objectsBare,
      rel: rel.objectsBare,
      tone: toneForRelative(rel.objectsBare),
    },
    {
      key: "on",
      label: "C · Asserts on",
      ns: ns.objectsAssertsOn,
      rel: rel.objectsAssertsOn,
      tone: toneForRelative(rel.objectsAssertsOn),
    },
  ];
}

function pathwayDeltas(result: {
  wrapperOverheadNs: number;
  assertOverheadNs: number;
  ns: { rawHandles: number; objectsBare: number };
}): DeltaLine[] {
  return [
    { label: "Wrapper B−A", ns: result.wrapperOverheadNs, pctOf: result.ns.rawHandles },
    { label: "Asserts C−B", ns: result.assertOverheadNs, pctOf: result.ns.objectsBare },
  ];
}

function renderAssertBenchPanel(panel: HTMLElement, result: BenchRunResult | null, status: string): void {
  panel.replaceChildren();

  const header = el("div", "assert-bench-header");
  header.append(
    el("div", "assert-bench-title", "Assert bench"),
    el("div", "assert-bench-sub", status),
  );
  panel.append(header);

  if (result === null) {
    panel.append(
      el(
        "div",
        "assert-bench-hint",
        "A raw · B objects bare (guards off) · C asserts on (guards on). Dev default = C; prod squeeze = B or compile strip. Results after the wave.",
      ),
    );
    return;
  }

  const { velocity: v, shockwave: s, liveApplyMs } = result;
  const meta = el("div", "assert-bench-meta");
  meta.append(
    el("span", "assert-bench-pill", `${v.bodyCount} bodies`),
    el(
      "span",
      `assert-bench-pill ${v.compiledIn ? "tone-ok" : "tone-good"}`,
      v.compiledIn ? "compile ON" : "compile STRIPPED",
    ),
    el("span", "assert-bench-pill", `${(v.iters / 1000).toFixed(0)}k vel iters`),
    el("span", "assert-bench-pill", `${s.rounds}×${s.passes} shock`),
  );
  panel.append(meta);

  appendNsChart(
    panel,
    "get/set linear velocity (ns/op)",
    pathwayRows(v.nsPerOp, v.relativeToRaw),
    pathwayDeltas({
      wrapperOverheadNs: v.wrapperOverheadNs,
      assertOverheadNs: v.assertOverheadNs,
      ns: v.nsPerOp,
    }),
  );

  appendNsChart(
    panel,
    "shockwave lin+ang write (median ns/body)",
    pathwayRows(s.nsPerBody, s.relativeToRaw),
    pathwayDeltas({
      wrapperOverheadNs: s.wrapperOverheadNs,
      assertOverheadNs: s.assertOverheadNs,
      ns: s.nsPerBody,
    }),
  );

  const liveUs = (ms: number) => (ms * 1000).toFixed(0);
  panel.append(
    el(
      "div",
      "assert-bench-foot",
      `Live wave CPU: A ${liveUs(liveApplyMs.raw)} · B ${liveUs(liveApplyMs.bare)} · C ${liveUs(liveApplyMs.on)} µs. ` +
        `≤${(REL_NOISE * 100).toFixed(0)}% vs raw ≈ noise. Prod squeeze: setObjectAssertGuardsEnabled(false) or BOX3D_OBJECT_ASSERTS=0.`,
    ),
  );
}

function createLaneScene(bg: number): THREE.Scene {
  const laneScene = new THREE.Scene();
  laneScene.background = new THREE.Color(bg);
  laneScene.add(new THREE.HemisphereLight(0xe0f2fe, 0x0f172a, 1.15));
  const sun = new THREE.DirectionalLight(0xffffff, 2.5);
  sun.position.set(10, 18, 12);
  laneScene.add(sun);
  return laneScene;
}

function populateLane(
  objects: ObjectRuntime,
  laneScene: THREE.Scene,
  id: LaneId,
  label: string,
  shared: {
    boxGeo: THREE.BoxGeometry;
    sphereGeo: THREE.SphereGeometry;
    capsuleGeo: THREE.CapsuleGeometry;
    groundGeo: THREE.BoxGeometry;
  },
): Lane {
  const objectWorld = objects.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const world = objectWorld.raw;
  const bodies: DemoBody[] = [];
  const dynamicRefs: BodyRef[] = [];
  const spawns: SpawnPose[] = [];

  const addMesh = (mesh: THREE.Mesh, handle: BodyHandle, type: BodyType) => {
    mesh.castShadow = false;
    mesh.receiveShadow = false;
    laneScene.add(mesh);
    bodies.push({ handle, mesh, type });
  };

  const ground = objectWorld.createBody({ type: BodyType.Static, position: [0, -1, 0] });
  ground.createHullShape([30, 1, 30]);
  const groundMesh = new THREE.Mesh(
    shared.groundGeo,
    new THREE.MeshStandardMaterial({ color: 0x1f2937, roughness: 0.9 }),
  );
  groundMesh.position.set(0, -1, 0);
  addMesh(groundMesh, ground.handle, BodyType.Static);

  let boxIndex = 0;
  for (let row = 0; row < PYRAMID_BASE; row++) {
    const count = PYRAMID_BASE - row;
    const y = 0.5 + row;
    for (let i = 0; i < count; i++) {
      for (let j = 0; j < count; j++) {
        const x = (i - (count - 1) / 2) * 1.05;
        const z = (j - (count - 1) / 2) * 1.05;
        const position: Vec3 = [x, y, z];
        const ref = objectWorld.createBody({ type: BodyType.Dynamic, position });
        ref.createHullShape([0.5, 0.5, 0.5], { density: 1 });
        dynamicRefs.push(ref);
        spawns.push({ position: cloneVec3(position), rotation: [...IDENTITY] });
        const mesh = new THREE.Mesh(
          shared.boxGeo,
          new THREE.MeshStandardMaterial({ color: BOX_COLORS[boxIndex % BOX_COLORS.length]!, roughness: 0.7 }),
        );
        mesh.position.set(x, y, z);
        addMesh(mesh, ref.handle, BodyType.Dynamic);
        boxIndex += 1;
      }
    }
  }

  for (let i = 0; i < SPHERE_COUNT; i++) {
    const angle = (i / SPHERE_COUNT) * Math.PI * 2;
    const r = 8 + (i % 3) * 0.4;
    const x = Math.cos(angle) * r;
    const z = Math.sin(angle) * r;
    const y = 2 + (i % 5) * 0.9;
    const position: Vec3 = [x, y, z];
    const ref = objectWorld.createBody({ type: BodyType.Dynamic, position });
    ref.createSphereShape([0, 0, 0], 0.45, { density: 1, rollingResistance: 0.05 });
    dynamicRefs.push(ref);
    spawns.push({ position: cloneVec3(position), rotation: [...IDENTITY] });
    const mesh = new THREE.Mesh(
      shared.sphereGeo,
      new THREE.MeshStandardMaterial({ color: 0xf97316, roughness: 0.55 }),
    );
    mesh.position.set(x, y, z);
    addMesh(mesh, ref.handle, BodyType.Dynamic);
  }

  for (let i = 0; i < CAPSULE_COUNT; i++) {
    const angle = (i / CAPSULE_COUNT) * Math.PI * 2 + 0.2;
    const r = 11;
    const x = Math.cos(angle) * r;
    const z = Math.sin(angle) * r;
    const y = 1.2 + (i % 4) * 0.75;
    const position: Vec3 = [x, y, z];
    const ref = objectWorld.createBody({ type: BodyType.Dynamic, position });
    ref.createCapsuleShape([0, -0.4, 0], [0, 0.4, 0], 0.25, { density: 1, rollingResistance: 0.05 });
    dynamicRefs.push(ref);
    spawns.push({ position: cloneVec3(position), rotation: [...IDENTITY] });
    const mesh = new THREE.Mesh(
      shared.capsuleGeo,
      new THREE.MeshStandardMaterial({ color: 0xa78bfa, roughness: 0.6 }),
    );
    mesh.position.set(x, y, z);
    addMesh(mesh, ref.handle, BodyType.Dynamic);
  }

  return { id, label, scene: laneScene, objectWorld, world, ground, dynamicRefs, spawns, bodies };
}

function resetLane(lane: Lane): void {
  if (lane.id === "raw") {
    for (let i = 0; i < lane.dynamicRefs.length; i++) {
      const handle = lane.dynamicRefs[i]!.handle;
      const spawn = lane.spawns[i]!;
      lane.world.setBodyTransform(handle, spawn.position, spawn.rotation);
      lane.world.setBodyLinearVelocity(handle, ZERO);
      lane.world.setBodyAngularVelocity(handle, ZERO);
      lane.world.setBodyAwake(handle, true);
    }
  } else if (lane.id === "bare") {
    try {
      setObjectAssertGuardsEnabled(false);
      for (let i = 0; i < lane.dynamicRefs.length; i++) {
        const ref = lane.dynamicRefs[i]!;
        const spawn = lane.spawns[i]!;
        ref.setTransform(spawn.position, spawn.rotation);
        ref.setLinearVelocity(ZERO);
        ref.setAngularVelocity(ZERO);
        lane.world.setBodyAwake(ref.handle, true);
      }
    } finally {
      setObjectAssertGuardsEnabled(true);
    }
  } else {
    setObjectAssertGuardsEnabled(true);
    for (let i = 0; i < lane.dynamicRefs.length; i++) {
      const ref = lane.dynamicRefs[i]!;
      const spawn = lane.spawns[i]!;
      ref.setTransform(spawn.position, spawn.rotation);
      ref.setLinearVelocity(ZERO);
      ref.setAngularVelocity(ZERO);
      lane.world.setBodyAwake(ref.handle, true);
    }
  }
  syncBodies(lane.world, lane.bodies);
}

export const objectAssertsBenchSample: DemoSample = {
  id: "extra/object-asserts-bench",
  name: "Extra / Object Asserts Bench",
  create(_runtime: Box3DRuntime, _scene: THREE.Scene): DemoSampleInstance {
    const objects = ObjectRuntime.fromRuntime(_runtime);
    const shared = {
      boxGeo: new THREE.BoxGeometry(1, 1, 1),
      sphereGeo: new THREE.SphereGeometry(0.45, 16, 12),
      capsuleGeo: new THREE.CapsuleGeometry(0.25, 0.8, 4, 8),
      groundGeo: new THREE.BoxGeometry(60, 2, 60),
    };

    const lanes: Lane[] = [
      populateLane(objects, createLaneScene(0x0b1220), "raw", "A · Raw handles", shared),
      populateLane(objects, createLaneScene(0x0c1a14), "bare", "B · Objects bare", shared),
      populateLane(objects, createLaneScene(0x111827), "on", "C · Asserts on", shared),
    ];
    const [rawLane, bareLane, onLane] = lanes as [Lane, Lane, Lane];
    const bodies = lanes.flatMap((lane) => lane.bodies);
    const bodyCount = onLane.dynamicRefs.length;

    const infoPanel = el("div", "assert-bench-panel");
    renderAssertBenchPanel(
      infoPanel,
      null,
      `${bodyCount} dynamics × 3 lanes · compile asserts ${areObjectAssertsCompiledIn() ? "ON" : "STRIPPED"}`,
    );

    const splitOverlay = el("div", "assert-bench-split");
    for (const lane of lanes) {
      splitOverlay.append(el("div", "assert-bench-lane-label", lane.label));
    }
    document.body.append(splitOverlay);

    let shock: MultiShock | null = null;
    let pendingBenchResult: BenchRunResult | null = null;
    let pickLaneIndex = 0;
    let dragLockLane: number | null = null;
    let viewWidth = 1;
    let viewHeight = 1;
    let drag:
      | {
          world: PhysicsWorld;
          body: BodyHandle;
          target: BodyHandle;
          joint: JointHandle;
          distance: number;
        }
      | null = null;

    const resetAll = () => {
      for (const lane of lanes) resetLane(lane);
    };

    const fireShockwave = (opts?: { reset?: boolean }) => {
      if (opts?.reset !== false) resetAll();
      const plan = buildKickPlan(onLane.spawns);
      shock = {
        t0: performance.now(),
        plan,
        done: plan.map(() => false),
        liveApplyMs: { raw: 0, bare: 0, on: 0 },
      };
    };

    const finishBenchIfReady = () => {
      if (pendingBenchResult === null) return;
      const last = pendingBenchResult;
      pendingBenchResult = null;
      renderAssertBenchPanel(infoPanel, last, "Latest run · velocity + shockwave timing");
      console.log("[object-asserts-bench]", last);
    };

    const advanceShockwave = (): boolean => {
      if (shock === null) return false;
      const elapsed = performance.now() - shock.t0;
      const due: number[] = [];
      let pending = false;
      for (let i = 0; i < shock.plan.length; i++) {
        if (shock.done[i]) continue;
        if (elapsed < shock.plan[i]!.delayMs) {
          pending = true;
          continue;
        }
        due.push(i);
      }
      if (due.length === 0) return pending;

      const sample = applyShockwaveKicks(shock.plan, due, rawLane, bareLane, onLane);
      shock.liveApplyMs.raw += sample.rawMs;
      shock.liveApplyMs.bare += sample.bareMs;
      shock.liveApplyMs.on += sample.onMs;
      for (const i of due) shock.done[i] = true;

      return pending;
    };

    const run = () => {
      pendingBenchResult = null;
      shock = null;
      resetAll();
      renderAssertBenchPanel(infoPanel, null, `Timing A/B/C over ${bodyCount} bodies…`);
      queueMicrotask(() => {
        const plan = buildKickPlan(onLane.spawns);
        const velocity = runObjectAssertBench(onLane.objectWorld, onLane.dynamicRefs, {
          iters: DEFAULT_ITERS,
          warmup: DEFAULT_WARMUP,
          rounds: DEFAULT_ROUNDS,
        });
        const shockwave = timeShockwaveWrites(
          plan,
          onLane,
          SHOCKWAVE_PASSES,
          SHOCKWAVE_WARMUP_PASSES,
          SHOCKWAVE_ROUNDS,
        );
        pendingBenchResult = {
          velocity,
          shockwave,
          liveApplyMs: { raw: 0, bare: 0, on: 0 },
        };
        fireShockwave({ reset: true });
        renderAssertBenchPanel(
          infoPanel,
          null,
          `Shockwave… results after wave (${bodyCount} bodies × 3 lanes)`,
        );
      });
    };

    const GRAVITY_MAGNITUDE = 10;
    const MOUSE_FORCE_SCALE = 100;
    const pickRaycaster = new THREE.Raycaster();

    const laneIndexFromU = (u: number): number =>
      Math.min(lanes.length - 1, Math.max(0, Math.floor(u * lanes.length + 1e-9)));

    const activePickLane = (): number => dragLockLane ?? pickLaneIndex;

    return {
      world: onLane.world,
      bodies,
      camera: { position: [22, 16, 28], target: [0, 4, 0] },
      infoPanel,
      controls: [
        { key: "run", label: "Run Bench", type: "button", onClick: run },
        { key: "shock", label: "Shockwave", type: "button", onClick: () => fireShockwave() },
        {
          key: "reset",
          label: "Reset",
          type: "button",
          onClick: () => {
            shock = null;
            pendingBenchResult = null;
            resetAll();
          },
        },
      ],
      remapPointerNdc(ndc) {
        if (drag === null) dragLockLane = null;
        const u = (ndc.x + 1) * 0.5;
        if (dragLockLane === null) {
          pickLaneIndex = laneIndexFromU(u);
        }
        const lane = activePickLane();
        ndc.x = (u * lanes.length - lane) * 2 - 1;
      },
      pickWorld: () => lanes[activePickLane()]!.world,
      pickCameraAspect: () => {
        const lane = activePickLane();
        const x0 = Math.floor((viewWidth * lane) / lanes.length);
        const x1 = Math.floor((viewWidth * (lane + 1)) / lanes.length);
        return Math.max(1, x1 - x0) / Math.max(1, viewHeight);
      },
      startMouseDragRay(origin, translation) {
        if (drag === null) dragLockLane = null;
        const lane = lanes[activePickLane()]!;
        pickRaycaster.ray.origin.set(origin[0], origin[1], origin[2]);
        const len = Math.hypot(translation[0], translation[1], translation[2]) || 1;
        pickRaycaster.ray.direction.set(translation[0] / len, translation[1] / len, translation[2] / len);
        for (const b of lane.bodies) b.mesh.updateMatrixWorld(true);

        let body: DemoBody | undefined;
        let point: THREE.Vector3 | undefined;
        const meshHits = pickRaycaster.intersectObjects(
          lane.bodies.map((b) => b.mesh),
          false,
        );
        for (const hit of meshHits) {
          const found = lane.bodies.find((b) => b.mesh === hit.object);
          if (found !== undefined && found.type === BodyType.Dynamic) {
            body = found;
            point = hit.point.clone();
            break;
          }
        }
        if (body === undefined || point === undefined) {
          const hit = lane.world.rayCastClosest(origin, translation);
          if (hit === null || hit.bodyHandle === 0) return false;
          body = lane.bodies.find((b) => b.handle === hit.bodyHandle);
          if (body === undefined || body.type !== BodyType.Dynamic) return false;
          point = new THREE.Vector3(hit.point[0], hit.point[1], hit.point[2]);
        }

        dragLockLane = activePickLane();
        const distance = Math.hypot(point.x - origin[0], point.y - origin[1], point.z - origin[2]);
        const kinematic = lane.world.createBody({
          type: BodyType.Kinematic,
          position: [point.x, point.y, point.z],
          enableSleep: false,
        });
        const localBodyPoint = lane.world.getBodyLocalPoint(body.handle, [point.x, point.y, point.z]);
        const massData = lane.world.getBodyMassData(body.handle);
        const mg = massData.mass * GRAVITY_MAGNITUDE;
        const lever = massData.mass > 0 ? Math.sqrt(massData.inertiaTrace / (3 * massData.mass)) : 0;
        const joint = lane.world.createMotorJoint(kinematic, body.handle, {
          localFrameA: [0, 0, 0],
          localFrameB: localBodyPoint,
          linearHertz: 7.5,
          linearDampingRatio: 1,
          maxSpringForce: MOUSE_FORCE_SCALE * mg,
          maxVelocityTorque: 0.5 * lever * mg,
        });
        lane.world.setBodyAwake(body.handle, true);
        drag = { world: lane.world, body: kinematic, target: body.handle, joint, distance };
        return true;
      },
      updateMouseDragRay(origin, translation) {
        if (drag === null) return;
        const len = Math.hypot(translation[0], translation[1], translation[2]) || 1;
        const t = drag.distance / len;
        const nextPos: [number, number, number] = [
          origin[0] + translation[0] * t,
          origin[1] + translation[1] * t,
          origin[2] + translation[2] * t,
        ];
        drag.world.setBodyTransform(drag.body, nextPos);
      },
      stopMouseDrag() {
        if (drag !== null) {
          drag.world.destroyJoint(drag.joint);
          drag.world.destroyBody(drag.body);
          drag = null;
        }
        dragLockLane = null;
      },
      render(renderer, camera) {
        // Match setSize CSS pixels — clientWidth can disagree with the GL viewport space.
        const size = new THREE.Vector2();
        renderer.getSize(size);
        const width = Math.max(1, size.x);
        const height = Math.max(1, size.y);
        viewWidth = width;
        viewHeight = height;

        const persp = camera instanceof THREE.PerspectiveCamera ? camera : null;
        const prevAspect = persp?.aspect ?? width / height;
        const prevAutoClear = renderer.autoClear;

        renderer.autoClear = false;
        renderer.setScissorTest(false);
        renderer.setViewport(0, 0, width, height);
        renderer.clear();

        renderer.setScissorTest(true);
        for (let i = 0; i < lanes.length; i++) {
          const x = Math.floor((width * i) / lanes.length);
          const nextX = Math.floor((width * (i + 1)) / lanes.length);
          const vw = Math.max(1, nextX - x);
          renderer.setViewport(x, 0, vw, height);
          renderer.setScissor(x, 0, vw, height);
          if (persp) {
            // Same centered framing per pane (twin compare). setViewOffset would crop one
            // frustum into thirds so left/center wouldn't show the pile.
            persp.clearViewOffset();
            persp.aspect = vw / height;
            persp.updateProjectionMatrix();
          }
          renderer.clear();
          renderer.render(lanes[i]!.scene, camera);
        }

        renderer.setScissorTest(false);
        renderer.setViewport(0, 0, width, height);
        renderer.autoClear = prevAutoClear;
        if (persp) {
          persp.clearViewOffset();
          persp.aspect = prevAspect;
          persp.updateProjectionMatrix();
        }
        return true;
      },
      step(dt, subSteps) {
        if (shock !== null && !advanceShockwave()) {
          if (pendingBenchResult !== null) {
            pendingBenchResult.liveApplyMs = { ...shock.liveApplyMs };
          }
          shock = null;
          finishBenchIfReady();
        }
        const hertz = dt ?? 1 / 60;
        const subs = subSteps ?? 4;
        for (const lane of lanes) {
          lane.world.step(hertz, subs);
          syncBodies(lane.world, lane.bodies);
        }
      },
      dispose() {
        shock = null;
        pendingBenchResult = null;
        if (drag !== null) {
          drag.world.destroyJoint(drag.joint);
          drag.world.destroyBody(drag.body);
          drag = null;
        }
        dragLockLane = null;
        splitOverlay.remove();
        for (const lane of lanes) {
          for (const ref of lane.dynamicRefs) ref.dispose();
          lane.ground.dispose();
          lane.objectWorld.dispose();
          for (const body of lane.bodies) {
            lane.scene.remove(body.mesh);
            const mat = body.mesh.material;
            if (Array.isArray(mat)) mat.forEach((m) => m.dispose());
            else mat.dispose();
          }
        }
        shared.boxGeo.dispose();
        shared.sphereGeo.dispose();
        shared.capsuleGeo.dispose();
        shared.groundGeo.dispose();
      },
    };
  },
};
