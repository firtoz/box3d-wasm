/**
 * Fixed-frame Junkyard correctness/performance harness.
 *
 * Measures: init/restart time, slot usage across restarts, profile levels,
 * dense transform publish cost, and single-thread transform hashes.
 *
 * Usage:
 *   bun packages/box3d-wasm/scripts/bench-junkyard.ts
 *   bun packages/box3d-wasm/scripts/bench-junkyard.ts --frames=120 --workers=1 --profile=off
 */
import { createHash } from "node:crypto";
import path from "node:path";
import { performance } from "node:perf_hooks";
import { fileURLToPath, pathToFileURL } from "node:url";
import {
  BodyType,
  Box3DRuntime,
  type BodyBatchBuffers,
  type BodyHandle,
  type PhysicsWorld,
  type SlotUsage,
} from "../src/index";

const scriptDir = path.dirname(fileURLToPath(import.meta.url));
const repoRoot = path.resolve(scriptDir, "..", "..", "..");
const wasmDir = path.resolve(repoRoot, "demo", "public", "wasm");

const SUBSTEPS = 4;
const DT = 1 / 60;
const ROCK_Y = 24;
const ROCK_XZ = 21;
const ROCK_RADIUS = 1.5;
const PUSHER_RADIUS = 35;
const PUSHER_HEIGHT = 24;
const PUSHER_CYLINDER_RADIUS = 4;

type ProfileLevelName = "off" | "coarse" | "full";
type PublishMode = "dense" | "sparse";

function parseArgs(argv: string[]): {
  frames: number;
  warmup: number;
  workers: number;
  profile: ProfileLevelName;
  restarts: number;
  publish: PublishMode;
  hashOnly: boolean;
} {
  const get = (name: string, fallback: string): string => {
    const prefix = `--${name}=`;
    const hit = argv.find((a) => a.startsWith(prefix));
    return hit !== undefined ? hit.slice(prefix.length) : fallback;
  };
  return {
    frames: Number(get("frames", "90")),
    warmup: Number(get("warmup", "10")),
    workers: Number(get("workers", "1")),
    profile: get("profile", "full") as ProfileLevelName,
    restarts: Number(get("restarts", "3")),
    publish: get("publish", "dense") as PublishMode,
    hashOnly: argv.includes("--hash-only"),
  };
}

function median(values: number[]): number {
  if (values.length === 0) return 0;
  const sorted = [...values].sort((a, b) => a - b);
  return sorted[Math.floor(sorted.length / 2)]!;
}

function mean(values: number[]): number {
  if (values.length === 0) return 0;
  return values.reduce((a, b) => a + b, 0) / values.length;
}

async function loadRuntime(): Promise<Box3DRuntime> {
  const modPath = pathToFileURL(path.join(wasmDir, "box3d-web.js")).href;
  const factory = (await import(modPath)).default as (opts: {
    locateFile(file: string): string;
  }) => Promise<ConstructorParameters<typeof Box3DRuntime>[0]>;
  const module = await factory({
    locateFile: (file) => pathToFileURL(path.join(wasmDir, file)).href,
  });
  return new Box3DRuntime(module as never);
}

function buildJunkyard(runtime: Box3DRuntime, workerCount: number): {
  world: PhysicsWorld;
  handles: BodyHandle[];
  pusher: BodyHandle;
} {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount });
  const ground = world.createBody({ type: BodyType.Static, position: [0, -1, 0] });
  runtime.createHullShape(ground, [120, 1, 120]);
  runtime.createTransformedHullShape(ground, [1, 8, 50], { position: [-50, 8, 0] });
  runtime.createTransformedHullShape(ground, [1, 8, 50], { position: [50, 8, 0] });
  runtime.createTransformedHullShape(ground, [50, 8, 1], { position: [0, 8, -50] });
  runtime.createTransformedHullShape(ground, [50, 8, 1], { position: [0, 8, 50] });

  const rocks: BodyHandle[] = [];
  const rockHull = runtime.createRock(ROCK_RADIUS);
  const height = 24;
  for (let Y = 0; Y < ROCK_Y; Y++) {
    for (let X = 0; X <= 20; X++) {
      for (let Z = 0; Z <= 20; Z++) {
        const body = world.createBody({
          type: BodyType.Dynamic,
          position: [-40 + 4 * X, 4 * Y + height + 1, -40 + 4 * Z],
        });
        runtime.createShapeFromHull(body, rockHull, {});
        rocks.push(body);
      }
    }
  }
  runtime.destroyHull(rockHull);

  const cylinder = runtime.createCylinder(PUSHER_HEIGHT, PUSHER_CYLINDER_RADIUS, 0, 16);
  const pusher = world.createBody({
    type: BodyType.Kinematic,
    position: [PUSHER_RADIUS, 0, 0],
  });
  runtime.createShapeFromHull(pusher, cylinder, {});
  runtime.destroyHull(cylinder);

  return { world, handles: [...rocks, pusher], pusher };
}

function hashTransforms(positions: Float32Array, rotations: Float32Array, count: number): string {
  const hash = createHash("sha256");
  hash.update(Buffer.from(positions.buffer, positions.byteOffset, count * 3 * 4));
  hash.update(Buffer.from(rotations.buffer, rotations.byteOffset, count * 4 * 4));
  return hash.digest("hex").slice(0, 16);
}

function usageKey(usage: SlotUsage): string {
  return `w${usage.worlds}/b${usage.bodies}/j${usage.joints}/h${usage.hulls}/s${usage.shapes}/m${usage.meshes}/c${usage.compounds}/u${usage.humans}`;
}

function publishDense(
  world: PhysicsWorld,
  count: number,
  batch: BodyBatchBuffers,
): { positions: Float32Array; rotations: Float32Array; ms: number } {
  const t0 = performance.now();
  world.writeBodyTransformsLight(
    count,
    batch.bodyHandlesPtr,
    batch.positionsPtr,
    batch.rotationsPtr,
    batch.awakePtr,
    batch.colorsPtr,
  );
  const memory = world.getMemoryView();
  const positions = new Float32Array(memory.heapF32.buffer, batch.positionsPtr, count * 3).slice();
  const rotations = new Float32Array(memory.heapF32.buffer, batch.rotationsPtr, count * 4).slice();
  return { positions, rotations, ms: performance.now() - t0 };
}

function publishSparse(
  world: PhysicsWorld,
  count: number,
  batch: BodyBatchBuffers,
  forceDense: boolean,
): { positions: Float32Array; rotations: Float32Array; ms: number; moveCount: number } {
  const t0 = performance.now();
  let moveCount = 0;
  if (forceDense) {
    world.writeBodyTransformsLight(
      count,
      batch.bodyHandlesPtr,
      batch.positionsPtr,
      batch.rotationsPtr,
      batch.awakePtr,
      batch.colorsPtr,
    );
    world.configureBodyMoveTracking(count, batch.bodyHandlesPtr);
  } else {
    moveCount = world.scatterBodyMoveEvents(
      batch.positionsPtr,
      batch.rotationsPtr,
      batch.awakePtr,
      batch.colorsPtr,
      true,
    );
  }
  const memory = world.getMemoryView();
  const positions = new Float32Array(memory.heapF32.buffer, batch.positionsPtr, count * 3).slice();
  const rotations = new Float32Array(memory.heapF32.buffer, batch.rotationsPtr, count * 4).slice();
  return { positions, rotations, ms: performance.now() - t0, moveCount };
}

async function main(): Promise<void> {
  const args = parseArgs(process.argv.slice(2));
  console.log("Junkyard bench");
  console.log(
    `  frames=${args.frames} warmup=${args.warmup} workers=${args.workers} profile=${args.profile} publish=${args.publish} restarts=${args.restarts}`,
  );
  console.log(`  expected rocks=${ROCK_Y * ROCK_XZ * ROCK_XZ}`);

  const runtime = await loadRuntime();
  console.log(`  slot limits: ${usageKey(runtime.limits)}`);

  const baselineUsage = runtime.getSlotUsage();
  console.log(`  baseline usage: ${usageKey(baselineUsage)}`);

  const restartMs: number[] = [];
  let lastHash = "";
  const stepMs: number[] = [];
  const publishMs: number[] = [];
  let peakContacts = 0;
  let finalMoveCount = 0;

  for (let restart = 0; restart < args.restarts; restart++) {
    const tInit = performance.now();
    const { world, handles } = buildJunkyard(runtime, args.workers);
    const initMs = performance.now() - tInit;
    restartMs.push(initMs);

    world.setProfileLevel(args.profile);
    const batch = world.allocBodyBatchBuffers(handles.length);
    world.writeBodyHandles(batch, handles);
    if (args.publish === "sparse") {
      world.configureBodyMoveTracking(handles.length, batch.bodyHandlesPtr);
    }

    for (let i = 0; i < args.warmup; i++) {
      world.step(DT, SUBSTEPS);
      if (args.publish === "sparse") {
        publishSparse(world, handles.length, batch, i === 0);
      } else {
        publishDense(world, handles.length, batch);
      }
    }

    let positions: Float32Array | null = null;
    let rotations: Float32Array | null = null;
    for (let i = 0; i < args.frames; i++) {
      const t0 = performance.now();
      world.step(DT, SUBSTEPS);
      stepMs.push(performance.now() - t0);

      const published =
        args.publish === "sparse"
          ? publishSparse(world, handles.length, batch, false)
          : publishDense(world, handles.length, batch);
      publishMs.push(published.ms);
      positions = published.positions;
      rotations = published.rotations;
      if ("moveCount" in published) finalMoveCount = Number(published.moveCount);

      const counters = world.getCounters();
      peakContacts = Math.max(peakContacts, counters.contactCount);
    }

    if (positions !== null && rotations !== null && args.workers === 1) {
      lastHash = hashTransforms(positions, rotations, handles.length);
    }

    const usageBeforeDestroy = runtime.getSlotUsage();
    world.destroy();
    const usageAfterDestroy = runtime.getSlotUsage();

    console.log(
      `  restart ${restart + 1}: init=${initMs.toFixed(1)}ms bodies=${handles.length} usageBefore=${usageKey(usageBeforeDestroy)} usageAfter=${usageKey(usageAfterDestroy)}`,
    );

    if (
      usageAfterDestroy.bodies !== baselineUsage.bodies ||
      usageAfterDestroy.shapes !== baselineUsage.shapes ||
      usageAfterDestroy.worlds !== baselineUsage.worlds ||
      usageAfterDestroy.joints !== baselineUsage.joints
    ) {
      throw new Error(
        `Slot leak detected: after destroy ${usageKey(usageAfterDestroy)} != baseline ${usageKey(baselineUsage)}`,
      );
    }
  }

  // Body-only destroy path: create small world, destroy bodies individually, then world.
  {
    const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
    const before = runtime.getSlotUsage();
    const bodies: BodyHandle[] = [];
    for (let i = 0; i < 64; i++) {
      const body = world.createBody({ type: BodyType.Dynamic, position: [i, 2, 0] });
      runtime.createHullShape(body, [0.5, 0.5, 0.5]);
      bodies.push(body);
    }
    const mid = runtime.getSlotUsage();
    for (const body of bodies) {
      runtime.destroyBody(body);
    }
    const afterBodies = runtime.getSlotUsage();
    world.destroy();
    const afterWorld = runtime.getSlotUsage();
    if (afterBodies.shapes > before.shapes || afterBodies.bodies > before.bodies) {
      throw new Error(
        `Body-only destroy left slots: before=${usageKey(before)} mid=${usageKey(mid)} afterBodies=${usageKey(afterBodies)}`,
      );
    }
    if (afterWorld.shapes !== baselineUsage.shapes || afterWorld.bodies !== baselineUsage.bodies) {
      throw new Error(`World destroy after body-only destroy leaked: ${usageKey(afterWorld)}`);
    }
    console.log(`  body-only destroy OK (created ${bodies.length} boxes)`);
  }

  // Freed handles reusable: allocate until shapes used, free world, allocate again.
  {
    const first = buildJunkyard(runtime, 1);
    const used = runtime.getSlotUsage().shapes;
    first.world.destroy();
    const second = buildJunkyard(runtime, 1);
    const used2 = runtime.getSlotUsage().shapes;
    second.world.destroy();
    if (used2 !== used) {
      throw new Error(`Handle reuse mismatch: first shapes=${used} second shapes=${used2}`);
    }
    console.log(`  handle reuse OK (shapes peaked at ${used})`);
  }

  if (!args.hashOnly) {
    console.log("\n=== Results ===");
    console.log(`  init/restart median: ${median(restartMs).toFixed(1)}ms  mean=${mean(restartMs).toFixed(1)}ms`);
    console.log(
      `  step median: ${median(stepMs).toFixed(2)}ms  mean=${mean(stepMs).toFixed(2)}ms  min=${Math.min(...stepMs).toFixed(2)} max=${Math.max(...stepMs).toFixed(2)}`,
    );
    console.log(
      `  publish(${args.publish}) median: ${median(publishMs).toFixed(3)}ms  mean=${mean(publishMs).toFixed(3)}ms`,
    );
    console.log(`  peak contacts: ${peakContacts}`);
    if (args.publish === "sparse") console.log(`  last moveCount: ${finalMoveCount}`);
  }
  if (args.workers === 1 && lastHash !== "") {
    console.log(`  transformHash@frame${args.frames}: ${lastHash}`);
  }
  console.log("  slot usage returned to baseline after every restart: OK");
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
