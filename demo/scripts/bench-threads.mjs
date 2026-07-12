import path from "node:path";
import { fileURLToPath } from "node:url";
import { performance } from "node:perf_hooks";

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const wasmDir = path.resolve(__dirname, "..", "public", "wasm");

const SUBSTEPS = 4;
const WARMUP = 10;
const BENCH = 30;

async function loadRuntime() {
  const modPath = path.join(wasmDir, "box3d-web.js");
  const factory = (await import(modPath)).default;
  const module = await factory({ locateFile: (f) => path.join(wasmDir, f) });
  return module;
}

function bench(module, workerCount, label) {
  const tsc = module.cwrap("b3wCheckThreadingSupport", "number", []);
  const gwc = module.cwrap("b3wGetWorldWorkerCount", "number", ["number"]);
  const cw = module.cwrap("b3wCreateWorld", "number", ["number", "number", "number", "number"]);
  const cb = module.cwrap("b3wCreateBox", "number", ["number", "number", "number", "number", "number", "number", "number", "number", "number"]);
  const createBody = module.cwrap("b3wCreateBody", "number", ["number", "number", "number", "number", "number", "number", "number"]);
  const createHull = module.cwrap("b3wCreateHullShape", "number", ["number", "number", "number", "number", "number", "number", "number", "number", "number", "number", "number", "number", "number", "number", "number"]);
  const step = module.cwrap("b3wStep", null, ["number", "number", "number"]);
  const getProfile = module.cwrap("b3wGetWorldProfile", null, ["number", "number"]);
  const getCounters = module.cwrap("b3wGetWorldCounters", null, ["number", "number"]);
  const applyImpulse = module.cwrap("b3wApplyLinearImpulse", null, ["number", "number", "number", "number", "number", "number", "number", "number"]);
  const destroyWorld = module.cwrap("b3wDestroyWorld", null, ["number"]);

  const threadingSupport = tsc();
  console.log(`\n${label} (requested workerCount=${workerCount})`);
  console.log(`  b3wCheckThreadingSupport: ${threadingSupport} ${threadingSupport ? "(SharedArrayBuffer OK)" : "(NO SharedArrayBuffer)"}`);

  const worldHandle = cw(0, -10, 0, workerCount);
  const actualCount = gwc(worldHandle);
  console.log(`  b3wGetWorldWorkerCount: ${actualCount} (requested ${workerCount})`);

  // Ground
  cb(worldHandle, 0, 0, -1, 40, 0.5, 40, 1, 1000);

  // Dominoes
  for (let ring = 0; ring < 30; ring++) {
    const radius = 7.0 + 1.1 * ring;
    for (let deg = 0; deg <= 360; deg += 2) {
      const rad = (deg * Math.PI) / 180;
      const cs = Math.cos(rad);
      const sn = Math.sin(rad);
      const px = radius * cs - (deg / 630) * cs;
      const pz = radius * sn - (deg / 630) * sn;
      const body = createBody(worldHandle, 2, px, 0.8, pz, 1, 1);
      createHull(body, 1000, 0.5, 0.1, 0, 0, 0, 0, 0, 0, 0, 1, 0.1, 0.4, 0.025);
      if (deg === 0) {
        applyImpulse(worldHandle, body, 0, 0, 25, px, 0.8 + 0.8, pz, 1);
      }
    }
  }

  const profilePtr = module._malloc(23 * 4);
  const countersPtr = module._malloc(7 * 4);

  for (let i = 0; i < WARMUP; i++) {
    step(worldHandle, 1 / 60, SUBSTEPS);
  }

  const times = [];
  for (let i = 0; i < BENCH; i++) {
    const t0 = performance.now();
    step(worldHandle, 1 / 60, SUBSTEPS);
    times.push(performance.now() - t0);
  }

  getProfile(worldHandle, profilePtr);
  const prof = new Float32Array(module.HEAPF32.buffer, profilePtr, 23);
  getCounters(worldHandle, countersPtr);
  const counters = new Int32Array(module.HEAP32.buffer, countersPtr, 7);

  times.sort((a, b) => a - b);
  const avg = times.reduce((a, b) => a + b, 0) / times.length;
  const med = times[Math.floor(times.length / 2)];
  console.log(`  Step: min=${times[0].toFixed(1)}ms  max=${times[times.length - 1].toFixed(1)}ms  avg=${avg.toFixed(1)}ms  med=${med.toFixed(1)}ms`);
  console.log(`  Profile: step=${prof[0].toFixed(2)}ms  pairs=${prof[1].toFixed(2)}ms  collide=${prof[2].toFixed(2)}ms  solve=${prof[3].toFixed(2)}ms`);
  console.log(`  Bodies=${counters[0]}  Shapes=${counters[1]}  Contacts=${counters[2]}  Joints=${counters[3]}  Islands=${counters[4]}`);

  module._free(profilePtr);
  module._free(countersPtr);
  destroyWorld(worldHandle);
  return { avg, actualCount, threadingSupport };
}

const module = await loadRuntime();
console.log("WASM loaded");

const r1 = bench(module, 1, "Single-threaded");
const r4 = bench(module, 4, "Multi-threaded (4 workers)");
const r2 = bench(module, 2, "Multi-threaded (2 workers)");
console.log("\n=== Summary ===");
console.log(`Threading support: ${r1.threadingSupport}`);
console.log(`workerCount=1: ${r1.avg.toFixed(1)}ms`);
console.log(`workerCount=4: ${r4.avg.toFixed(1)}ms (${(r1.avg / r4.avg).toFixed(1)}x speedup)`);
console.log(`workerCount=2: ${r2.avg.toFixed(1)}ms (${(r1.avg / r2.avg).toFixed(1)}x speedup)`);
