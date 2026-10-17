import { createHash } from "node:crypto";
import { existsSync, readFileSync, readdirSync, statSync, statfsSync, writeFileSync } from "node:fs";
import { join, resolve } from "node:path";
import { spawnSync } from "node:child_process";

// Read-only publication audit: no builds, viewers, GPU jobs or retries.
const root = resolve("experiments/gpu-physics/benchmarks/production-readiness/pr04-behavior-2026-10-17");
const hash = (p: string) => createHash("sha256").update(readFileSync(p)).digest("hex");
const checked = new Map<string, string>();
const present = (pid: number) => {
  try { process.kill(pid, 0); return true; }
  catch (e: any) { if (e.code !== "ESRCH") throw e; return false; }
};
const receipts = readdirSync(root).filter(p => p.endsWith("-receipt.json")).map(name => {
  const r = JSON.parse(readFileSync(join(root, name), "utf8"));
  if (r.status === "RUNNING" || r.pid) throw Error("Live phase " + name);
  const protocol = join(root, r.phase + "-protocol.json");
  if (hash(protocol) !== r.protocolSha256) throw Error("Protocol changed " + name);
  for (const [path, expected] of Object.entries(r.inputHashes)) {
    const actual = checked.get(path) ?? hash(path);
    checked.set(path, actual);
    if (actual !== expected) throw Error("Input changed " + path);
  }
  for (const row of r.rows) {
    if (hash(join(root, row.log)) !== row.logSha256) throw Error("Log changed " + row.log);
    if (present(row.pid) || present(-row.pid)) throw Error("Recorded process/group still present " + row.pid);
  }
  return { name, status: r.status, completedJobs: r.rows.length, lastJob: r.rows.at(-1)?.label,
    lastJobAccepted: r.rows.at(-1)?.accepted, elapsedMs: r.elapsedMs, removedBytes: r.removedBytes };
});
const baseline = readdirSync(root).filter(p => /^baseline-(rain|ragdolls)-(cpu|ordinary|native)\.json$/.test(p)).map(name => {
  const r = JSON.parse(readFileSync(join(root, name), "utf8"));
  if (r.frames.length !== 600 || r.healthEnabled || r.frames.some((f: any, i: number) => f.step !== i + 1)) throw Error("Invalid baseline " + name);
  return { name, workerCount: r.workerCount, all: r.all, loadedTail120: r.loadedTail120 };
});
const headless = readdirSync(join(root, "checks")).filter(p => /^rain-batch1-headless-run-.*\.log$/.test(p)).map(name => {
  const lines = readFileSync(join(root, "checks", name), "utf8").split("\n");
  const records = lines.filter(s => s.startsWith('{"kind":')).map(s => JSON.parse(s));
  const steps = records.filter(r => r.kind === "step");
  if (steps.some((r, i) => r.step !== i + 1 || r.completed !== i + 1)) throw Error("Invalid progress " + name);
  const stats = (a: any[]) => {
    const values = a.map(r => r.physics_ms).sort((a, b) => a - b);
    const mean = (key: string) => a.reduce((n, r) => n + r[key], 0) / a.length;
    return { steps: a.length, meanPhysicsMs: mean("physics_ms"), completedStepsPerSecond: 1000 / mean("physics_ms"),
      simulatedSecondsPerSecond: 1000 / mean("physics_ms") / 60, p95Ms: values[Math.ceil(values.length * .95) - 1],
      maxMs: values.at(-1), meanSettersMs: mean("setters_ms"), meanCreateMs: mean("create_ms") };
  };
  return { name, completedSteps: steps.length, passMarker: lines.includes("PASS headless original Rain600 completed"),
    endpoint: records.find(r => r.kind === "endpoint") ?? null, all: stats(steps), final20Completed: stats(steps.slice(-20)) };
});
const bytes = (dir: string): number => !existsSync(dir) ? 0 : readdirSync(dir).reduce((n, name) => {
  const p = join(dir, name), s = statSync(p); return n + (s.isDirectory() ? bytes(p) : s.size);
}, 0);
const rawBytes = bytes(join(root, "raw"));
if (rawBytes) throw Error("Unconsumed raw files");
const transpiler = new Bun.Transpiler({ loader: "ts", target: "bun" });
for (const name of ["run.ts", "extract-baseline.ts", "checkpoint-audit.ts"]) transpiler.transformSync(readFileSync(join(root, name), "utf8"));
const cache = spawnSync("du", ["-sb", "/home/firtoz/.cache/box3d-gpu-physics/pipelines", "/home/firtoz/.cache/nvidia"], { timeout: 10000, encoding: "utf8", maxBuffer: 4096 });
if (cache.status !== 0) throw Error("Cache inventory failed");
const fs = statfsSync(root);
const report = { status: "CHECKPOINT_INTEGRITY_PASS_NOT_PR04_ACCEPTANCE", auditedAt: new Date().toISOString(),
  gatesDone: 0, gatesLeft: 6, receipts, distinctPinnedInputs: checked.size, checkedProcessesAbsent: true,
  baseline, headless, rawBytes, retainedBytesBeforeAudit: bytes(root), freeBytes: fs.bavail * fs.bsize,
  cacheBytesAfter: cache.stdout.trim(),
  limitations: ["Normal viewer baselines record workerCount=8; headless fixture explicitly uses one worker. Do not present them as matched one-worker timings.",
    "Timeouts remain failures. Unlaunched controls are unreviewed; no GPU Rain600 physical pass.",
    "Baseline health scans are disabled. Headless endpoints only check valid finite live body states, not joint/support/energy acceptance.",
    "A physics-step clock includes queue completion and mirror synchronization; this is not pure GPU kernel time."] };
writeFileSync(join(root, "checkpoint-audit.json"), JSON.stringify(report, null, 2) + "\n");
console.log(JSON.stringify({ status: report.status, receipts, distinctPinnedInputs: checked.size, rawBytes, headless }, null, 2));
