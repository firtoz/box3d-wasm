import type { BodyRef, ObjectWorld } from "./objects";
import { areObjectAssertsCompiledIn } from "./object-assert-flags";
import {
  areObjectAssertGuardsInstalled,
  setObjectAssertGuardsEnabled,
} from "./object-assert-guards";
import type { BodyHandle, PhysicsWorld, Vec3 } from "./index";

export type ObjectAssertBenchOptions = {
  /** Velocity get/set pairs per timed trial (default 50_000). */
  iters?: number;
  warmup?: number;
  rounds?: number;
  now?: () => number;
};

export type ObjectAssertBenchResult = {
  iters: number;
  warmup: number;
  rounds: number;
  bodyCount: number;
  compiledIn: boolean;
  nsPerOp: {
    /** A — PhysicsWorld + handle. */
    rawHandles: number;
    /** B — BodyRef with assert guards uninstalled. */
    objectsBare: number;
    /** C — BodyRef with assert guards installed. */
    objectsAssertsOn: number;
  };
  relativeToRaw: {
    objectsBare: number;
    objectsAssertsOn: number;
  };
  /** B − A: BodyRef wrapper with no assert call. */
  wrapperOverheadNs: number;
  /** C − B: cost of assert guards / assertActive. */
  assertOverheadNs: number;
  assertOverheadPctOfBare: number;
};

function median(values: number[]): number {
  const sorted = [...values].sort((a, b) => a - b);
  return sorted[Math.floor(sorted.length / 2)]!;
}

function nsPerOp(fn: () => void, iters: number, now: () => number): number {
  const t0 = now();
  fn();
  const t1 = now();
  return (t1 - t0) * (1_000_000 / iters);
}

/**
 * Microbench A/B/C velocity get/set:
 *   A raw · B objects bare (guards off) · C objects asserts on (guards on)
 */
export function runObjectAssertBench(
  world: ObjectWorld,
  bodies: BodyRef | readonly BodyRef[],
  options: ObjectAssertBenchOptions = {},
): ObjectAssertBenchResult {
  const list = Array.isArray(bodies) ? bodies : [bodies];
  if (list.length === 0) throw new Error("runObjectAssertBench requires at least one body");

  const iters = options.iters ?? 50_000;
  const warmup = options.warmup ?? 5_000;
  const rounds = options.rounds ?? 5;
  const now = options.now ?? (() => performance.now());

  const physics: PhysicsWorld = world.raw;
  const handles: BodyHandle[] = list.map((b) => b.handle);
  const n = list.length;
  const v: Vec3 = [1, 0, 0];
  const out: Vec3 = [0, 0, 0];

  const runRaw = (count: number) => {
    for (let i = 0; i < count; i++) {
      const handle = handles[i % n]!;
      v[0] = i * 1e-6;
      physics.setBodyLinearVelocity(handle, v);
      physics.getBodyLinearVelocityTo(handle, out);
    }
  };

  const runObjects = (count: number) => {
    for (let i = 0; i < count; i++) {
      const body = list[i % n]!;
      v[0] = i * 1e-6;
      body.setLinearVelocity(v);
      body.getLinearVelocityTo(out);
    }
  };

  const prevGuards = areObjectAssertGuardsInstalled();
  try {
    setObjectAssertGuardsEnabled(true);
    nsPerOp(() => runObjects(warmup), warmup, now);
    setObjectAssertGuardsEnabled(false);
    nsPerOp(() => runObjects(warmup), warmup, now);
    nsPerOp(() => runRaw(warmup), warmup, now);

    const rawSamples: number[] = [];
    const bareSamples: number[] = [];
    const onSamples: number[] = [];

    for (let r = 0; r < rounds; r++) {
      const modes = [0, 1, 2].map((i) => (r + i) % 3);
      for (const mode of modes) {
        if (mode === 0) {
          rawSamples.push(nsPerOp(() => runRaw(iters), iters, now));
        } else if (mode === 1) {
          setObjectAssertGuardsEnabled(false);
          bareSamples.push(nsPerOp(() => runObjects(iters), iters, now));
        } else {
          setObjectAssertGuardsEnabled(true);
          onSamples.push(nsPerOp(() => runObjects(iters), iters, now));
        }
      }
    }

    const rawMed = median(rawSamples);
    const bareMed = median(bareSamples);
    const onMed = median(onSamples);

    return {
      iters,
      warmup,
      rounds,
      bodyCount: n,
      compiledIn: areObjectAssertsCompiledIn(),
      nsPerOp: {
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
    setObjectAssertGuardsEnabled(prevGuards);
  }
}

export function formatObjectAssertBenchResult(result: ObjectAssertBenchResult): string {
  const {
    nsPerOp,
    relativeToRaw,
    compiledIn,
    assertOverheadNs,
    assertOverheadPctOfBare,
    bodyCount,
    wrapperOverheadNs,
  } = result;
  const fmt = (ns: number) => ns.toFixed(2);
  return [
    `${bodyCount} bodies`,
    `compile asserts ${compiledIn ? "ON" : "STRIPPED"}`,
    `A raw ${fmt(nsPerOp.rawHandles)} ns`,
    `B bare ${fmt(nsPerOp.objectsBare)} ns (${relativeToRaw.objectsBare.toFixed(2)}×)`,
    `C asserts ${fmt(nsPerOp.objectsAssertsOn)} ns (${relativeToRaw.objectsAssertsOn.toFixed(2)}×)`,
    `wrapper Δ ${fmt(wrapperOverheadNs)} ns`,
    `assert Δ ${fmt(assertOverheadNs)} ns (${assertOverheadPctOfBare.toFixed(1)}%)`,
  ].join(" | ");
}
