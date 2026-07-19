/**
 * Microbench: objects-API assertActive cost vs raw handle API.
 *
 * Usage:
 *   bun packages/box3d-wasm/scripts/bench-object-asserts.ts
 *   bun packages/box3d-wasm/scripts/bench-object-asserts.ts --iters=500000 --warmup=50000
 */
import path from "node:path";
import { fileURLToPath, pathToFileURL } from "node:url";
import { BodyType, Box3DRuntime } from "../src/index";
import { ObjectRuntime, runObjectAssertBench } from "../src/objects";

const scriptDir = path.dirname(fileURLToPath(import.meta.url));
const repoRoot = path.resolve(scriptDir, "..", "..", "..");
const wasmDir = path.resolve(repoRoot, "demo/public/wasm");

function parseArgs(argv: string[]): { iters: number; warmup: number; rounds: number } {
  const get = (name: string, fallback: string): string => {
    const prefix = `--${name}=`;
    const hit = argv.find((a) => a.startsWith(prefix));
    return hit !== undefined ? hit.slice(prefix.length) : fallback;
  };
  return {
    iters: Number(get("iters", "200000")),
    warmup: Number(get("warmup", "20000")),
    rounds: Number(get("rounds", "5")),
  };
}

async function loadRuntime(): Promise<Box3DRuntime> {
  const modPath = pathToFileURL(path.join(wasmDir, "box3d-web.js")).href;
  const factory = (await import(modPath)).default as (opts: {
    locateFile(file: string): string;
  }) => Promise<ConstructorParameters<typeof Box3DRuntime>[0]>;
  const module = await factory({ locateFile: (file) => path.join(wasmDir, file) });
  return new Box3DRuntime(module);
}

async function main(): Promise<void> {
  const { iters, warmup, rounds } = parseArgs(process.argv.slice(2));
  const rawRuntime = await loadRuntime();
  const objects = ObjectRuntime.fromRuntime(rawRuntime);
  const world = objects.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const body = world.createBody({ type: BodyType.Dynamic, position: [0, 1, 0] });
  body.createHullShape([0.5, 0.5, 0.5]);

  const result = runObjectAssertBench(world, body, { iters, warmup, rounds });

  console.log(JSON.stringify({
    ...result,
    note: "Hot JS get/set velocity loop only — not world.step. A raw · B bare (guards off) · C asserts on. Prod squeeze: setObjectAssertGuardsEnabled(false) or BOX3D_OBJECT_ASSERTS=0.",
  }, null, 2));

  body.dispose();
  world.dispose();
  objects.dispose();
}

main().catch((err) => {
  console.error(err);
  process.exit(1);
});
