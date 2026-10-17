import { spawn } from "node:child_process";
import { createHash } from "node:crypto";
import { mkdirSync, readFileSync, writeFileSync, rmSync, statSync, statfsSync, existsSync } from "node:fs";
import { resolve, join } from "node:path";

// One bounded refresh, never an implicit rerun of a historical campaign.
const root = resolve("experiments/gpu-physics");
const out = join(root, "benchmarks/production-readiness/pr02-contract-refresh-2026-10-17");
const raw = join(out, "raw");
const remaining = process.argv[2] === "remaining";
const nativePolicy = process.argv[2] === "native-policy";
const followup = remaining || nativePolicy;
const prefix = nativePolicy ? "native-policy-" : remaining ? "remaining-" : "";
const protocolPath = join(out, nativePolicy ? "native-policy-protocol.json" : remaining ? "remaining-protocol.json" : "protocol.json");
const receiptName = prefix + "runs.json";
const protocol = JSON.parse(readFileSync(protocolPath, "utf8"));
if (existsSync(join(out, receiptName))) throw new Error("Refresh already launched; read its receipt instead of replaying it.");
if (remaining) {
  const previous = JSON.parse(readFileSync(join(out, "runs.json"), "utf8"));
  if (previous.status !== "FAILED" || previous.receipts.filter((r: any) => r.label.startsWith("run-")).length !== 36
    || previous.receipts.at(-1).label !== "link-cpu-pr02_default_filter_test") throw new Error("Unexpected continuation boundary");
}
if (nativePolicy && JSON.parse(readFileSync(join(out, "remaining-runs.json"), "utf8")).status !== "PASS") throw new Error("Filter continuation must be complete");
mkdirSync(raw, { recursive: true });
mkdirSync(join(out, "checks"), { recursive: true });
const receipts: any[] = [];
const start = Date.now();
const sha = (p: string) => createHash("sha256").update(readFileSync(p)).digest("hex");
const save = (p: string, value: unknown) => writeFileSync(join(out, p), JSON.stringify(value, null, 2) + "\n");
const libs = {
  ordinary: join(root, "target/samples/release/libgpu_physics.a"),
  native: join(root, "target/native-cache-build/release/libgpu_physics.a"),
};
const builds = Object.fromEntries(["ordinary", "native"].flatMap(backend => ["gpu", "both"].map(mode =>
  [`${backend}-${mode}`, join(root, `native-samples/build-${mode}${backend === "native" ? "-native-cache" : ""}-portable`)])));
const cache = resolve(process.env.HOME!, ".cache/box3d-gpu-physics/pipelines");
const baseEnv = nativePolicy ? Object.fromEntries(Object.entries(process.env).filter(([k]) => !/^(GPU_|WGPU_|VK_|__NV|__GLX)/.test(k))) : process.env;
const env = { ...baseEnv, WGPU_BACKEND: "vulkan", GPU_PHYSICS_ADAPTER: "nvidia",
  VK_DRIVER_FILES: "/usr/share/vulkan/icd.d/nvidia_icd.json", GPU_PHYSICS_PIPELINE_CACHE_DIR: cache };
if (nativePolicy) Object.assign(env, protocol.environment);
let active: number | undefined;
function guard() {
  const fs = statfsSync(out);
  if (fs.bavail * fs.bsize < protocol.budget.minFreeBytes) throw new Error("Free-space floor");
  if (Date.now() - start > protocol.budget.batchSeconds * 1000) throw new Error("Total batch timeout");
}
function terminate() { if (active) { try { process.kill(-active, "SIGKILL"); } catch {} } }
process.once("SIGTERM", () => { terminate(); process.exit(143); });
process.once("SIGINT", () => { terminate(); process.exit(130); });
async function run(label: string, argv: string[], seconds: number) {
  guard();
  const log = join(out, "checks", prefix + label + ".log");
  writeFileSync(log, "");
  const started = Date.now();
  const child = spawn("bash", ["-c", 'ulimit -c 0; exec "$@"', "pr02", ...argv], {
    cwd: process.cwd(), env, detached: true, stdio: ["ignore", "pipe", "pipe"],
  });
  active = child.pid;
  let bytes = 0;
  const chunks: Buffer[] = [];
  let reason: string | null = null;
  function consume(chunk: Buffer) {
    bytes += chunk.length;
    if (bytes <= protocol.budget.logBytesPerCommand) chunks.push(chunk);
    else { reason = "output cap"; terminate(); }
  }
  child.stdout.on("data", consume); child.stderr.on("data", consume);
  const watchdog = setInterval(() => {
    try { guard(); if (Date.now() - started > seconds * 1000) throw new Error("command timeout"); }
    catch (e) { reason = String(e); terminate(); }
  }, 250);
  let code: number | null;
  try {
    code = await new Promise<number | null>((resolve, reject) => { child.once("error", reject); child.once("close", resolve); });
  } catch (error) {
    code = -1;
    reason = String(error);
  } finally {
    clearInterval(watchdog);
    // Also clean up after spawn errors and unexpected helper processes.
    terminate(); active = undefined;
  }
  const output = Buffer.concat(chunks).toString();
  writeFileSync(log, output);
  const receipt = { label, argv, seconds, code, reason, bytes, pid: child.pid, elapsedMs: Date.now() - started, log: "checks/" + prefix + label + ".log", logSha256: sha(log), environment: Object.fromEntries(Object.entries(env).filter(([k]) => /^(GPU_|WGPU_|VK_)/.test(k))) };
  receipts.push(receipt); save(receiptName, { status: "RUNNING", protocolSha256: sha(protocolPath), receipts });
  console.log(label, code, receipt.elapsedMs + "ms");
  guard();
  if (code !== 0 || reason) throw new Error(`${label}: ${reason ?? code}`);
  return output;
}
async function sizes(label: string) {
  const paths = [...Object.values(builds), cache, resolve(process.env.HOME!, ".cache/nvidia"), raw].filter(p => existsSync(p));
  const result = await run("sizes-" + label, ["du", "-sb", ...paths], 15);
  return Object.fromEntries(result.trim().split("\n").map(line => { const m = line.match(/^(\d+)\s+(.+)$/)!; return [m[2], Number(m[1])]; }));
}
const identities: any = { protocolSha256: sha(protocolPath), libraries: Object.fromEntries(Object.entries(libs).map(([k, p]) => [k, { path: p, sha256: sha(p) }])), sources: {} };
for (const name of ["shim.c", "samples_api.c", "samples_stubs.c", "both_dual.c", "both_map.c", "native_unavailable.inc", "pr02_default_filter_test.cpp"])
  identities.sources["c_abi/" + name] = sha(join(root, "c_abi", name));
save(prefix + "identities.json", identities);
let before: Record<string, number> = {};
let binariesDeleted = 0;
try {
  before = await sizes("before");
  for (const [cell, dir] of followup ? [] : Object.entries(builds)) {
    await run("adapter-" + cell, ["cmake", "--build", dir, "--target", "gpu_samples_api", ...(cell.endsWith("both") ? ["gpu_both_api"] : ["box3d"]), "-j2"], protocol.budget.adapterBuildSeconds);
  }
  const cases = nativePolicy ? protocol.cases : [...(remaining ? [] : protocol.cases),
    ...(remaining ? [] : Object.keys(builds).map(cell => ({ cell, fixture: "native_unavailable_fixture", flags: [] }))),
    { cell: "cpu", fixture: "pr02_default_filter_test", flags: [] },
    ...Object.keys(builds).map(cell => ({ cell, fixture: "pr02_default_filter_test", flags: [] })),
  ];
  if (cases.length !== protocol.budget.CProcesses) throw new Error("Frozen case count mismatch");
  for (const test of cases) {
    const cell: string = test.cell;
    const combined = cell.endsWith("both");
    const binary = join(raw, "fixture");
    const source = join(root, "c_abi", test.fixture + ".cpp");
    const dir = builds[cell];
    const link: string[] = [];
    if (cell === "cpu") link.push(join(builds["ordinary-both"], "libbox3d_unprefixed.a"));
    else {
      if (combined) link.push("-DGPU_API_DUAL", "-Wl,--whole-archive", join(dir, "libgpu_both_api.a"), "-Wl,--no-whole-archive");
      link.push("-Wl,--whole-archive", join(dir, "libgpu_samples_api.a"), "-Wl,--no-whole-archive", libs[cell.startsWith("ordinary") ? "ordinary" : "native"]);
      if (combined) link.push(join(dir, "libbox3d_cpu.a"));
      else if (test.independent_prefixed_CPU) link.push(join(builds[cell.startsWith("ordinary") ? "ordinary-both" : "native-both"], "libbox3d_cpu.a"));
      link.push(join(dir, "box3d_src/libbox3d.a"));
    }
    const label = cell + "-" + test.fixture;
    await run("link-" + label, ["g++", "-O2", "-std=c++17", source, "-I", resolve("box3d/include"), "-I", join(root, "c_abi"), ...(test.flags ?? []), ...link, "-ldl", "-lpthread", "-lm", "-lgcc_s", "-lGL", "-o", binary], protocol.budget.linkSeconds);
    const host = test.fixture === "native_unavailable_fixture" || cell === "cpu";
    await run("run-" + label, [binary, ...(test.fixture === "native_unavailable_fixture" ? [join(raw, "must-not-exist.rec")] : [])], host ? protocol.budget.hostSeconds : protocol.budget.gpuSeconds);
    binariesDeleted += statSync(binary).size;
    rmSync(binary);
    console.log(`C contracts ${receipts.filter(r => r.label.startsWith("run-")).length}/${cases.length} done`);
  }
  const after = await sizes("after");
  for (const [backend, p] of Object.entries(libs)) if (sha(p) !== identities.libraries[backend].sha256) throw new Error("Rust library changed during refresh");
  for (const [cell, dir] of Object.entries(builds)) {
    identities[cell] = {};
    for (const name of ["libgpu_samples_api.a", "libgpu_both_api.a", "libbox3d_cpu.a", "libbox3d_unprefixed.a", "box3d_src/libbox3d.a"])
      if (existsSync(join(dir, name))) identities[cell][name] = sha(join(dir, name));
  }
  save(prefix + "identities.json", identities);
  save(receiptName, { status: "PASS", protocolSha256: sha(protocolPath), receipts, processes: cases.length, binariesDeleted, before, after, elapsedMs: Date.now() - start });
} catch (error) {
  terminate();
  save(receiptName, { status: "FAILED", error: String(error), protocolSha256: sha(protocolPath), receipts, binariesDeleted, before, elapsedMs: Date.now() - start });
  throw error;
} finally {
  rmSync(raw, { recursive: true, force: true });
}
