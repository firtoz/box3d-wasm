import { spawn } from "node:child_process";
import { createHash } from "node:crypto";
import { existsSync, mkdirSync, readFileSync, writeFileSync, rmSync, statSync, statfsSync, readdirSync } from "node:fs";
import { resolve, join } from "node:path";

// Every phase is separately frozen before launch. This driver never retries.
const phase = process.argv[2];
if (!phase || !/^[a-z0-9-]+$/.test(phase)) throw new Error("Choose a frozen phase");
const out = resolve("experiments/gpu-physics/benchmarks/production-readiness/pr04-behavior-2026-10-17");
const protocolFile = join(out, phase + "-protocol.json");
const receiptFile = join(out, phase + "-receipt.json");
if (existsSync(receiptFile)) throw new Error("Phase already launched: read its receipt");
const protocol = JSON.parse(readFileSync(protocolFile, "utf8"));
const start = Date.now();
const rows: any[] = [];
mkdirSync(join(out, "checks"), { recursive: true });
mkdirSync(join(out, "raw"), { recursive: true });
const sha = (p: string) => createHash("sha256").update(readFileSync(p)).digest("hex");
const inputHashes = Object.fromEntries((protocol.inputs ?? []).map((p: string) => [p, sha(p)]));
let pid: number | undefined, running: string | undefined;
function save(status: string, extra = {}) {
  writeFileSync(receiptFile, JSON.stringify({ phase, status, protocolSha256: sha(protocolFile), inputHashes, rows, running, pid, elapsedMs: Date.now() - start, ...extra }, null, 2) + "\n");
}
function kill(signal: NodeJS.Signals = "SIGKILL") { if (pid) { try { process.kill(-pid, signal); } catch {} } }
for (const signal of ["SIGTERM", "SIGINT"] as const) process.once(signal, () => { kill(); save("INTERRUPTED"); process.exit(signal === "SIGTERM" ? 143 : 130); });
function rawBytes(dir: string): number {
  try{return readdirSync(dir,{withFileTypes:true}).reduce((n,e)=>{try{return n+(e.isDirectory()?rawBytes(join(dir,e.name)):e.isFile()?statSync(join(dir,e.name)).size:0);}catch(error:any){if(error.code==='ENOENT')return n;throw error;}},0);}catch(error:any){if(error.code==='ENOENT')return 0;throw error;}
}
let lastStorage = 0, peakRawBytes = 0;
function guard() {
  if (Date.now() - start > protocol.totalSeconds * 1000) throw new Error("Phase timeout");
  const fs = statfsSync(out);
  if (fs.bavail * fs.bsize < 20 * 1024 ** 3) throw new Error("Free-space floor");
  if (Date.now() - lastStorage > 2000) {
    const retained=rawBytes("/home/firtoz/work/2026/box3d-wasm/experiments/gpu-physics/artifacts/pr04-release-candidate-2026-10-17");if(retained>512*1024**2)throw Error("Retained candidate cap");
    const bytes = rawBytes(join(out, "raw")); peakRawBytes = Math.max(peakRawBytes, bytes); lastStorage = Date.now();
    if (bytes > protocol.maxRawBytes) throw new Error("Raw storage cap");
  }
}
let removedBytes = 0;
function cleanup(paths: string[]) {
  for (const path of paths) {
    const p = resolve(path);
    if (!p.startsWith(join(out, "raw") + "/")) throw new Error("Cleanup outside task raw tree");
    if (existsSync(p)) { removedBytes += statSync(p).isDirectory() ? rawBytes(p) : statSync(p).size; rmSync(p, { recursive: true, force: true }); }
  }
}
save("RUNNING");
try {
  for (const job of protocol.jobs) {
    guard();
    running = job.label;
    const logfile = join(out, "checks", phase + "-" + job.label + ".log");
    if (existsSync(logfile)) throw new Error("Output collision " + logfile);
    const began = Date.now();
    const baseEnv = Object.fromEntries(Object.entries(process.env).filter(([k]) => !/^(GPU_|WGPU_|VK_|__NV|__GLX)/.test(k)));
    const child = spawn("bash", ["-c", 'ulimit -c 0; exec "$@"', "pr04-behavior", ...job.argv], {
      cwd: job.cwd ?? process.cwd(), env: { ...baseEnv, ...(protocol.environment ?? {}), ...(job.environment ?? {}) }, detached: true, stdio: ["ignore", "pipe", "pipe"],
    });
    pid = child.pid; save("RUNNING");
    let bytes = 0, reason: string | undefined;
    const chunks: Buffer[] = [];
    const consume = (b: Buffer) => { bytes += b.length; if (bytes <= 16 * 1024 ** 2) chunks.push(b); else { reason = "output cap"; kill(); } };
    child.stdout.on("data", consume); child.stderr.on("data", consume);
    const timer = setInterval(() => {
      try { guard(); if (Date.now() - began > job.seconds * 1000) throw new Error("command timeout"); }
      catch (error) { reason = String(error); kill(); }
    }, 250);
    let code: number | null = null, signal: NodeJS.Signals | null = null;
    try { [code, signal] = await new Promise<[number | null, NodeJS.Signals | null]>((resolve, reject) => { child.once("error", reject); child.once("close", (code, signal) => resolve([code, signal])); }); }
    catch (error) { reason = String(error); }
    finally { clearInterval(timer); kill(); }
    writeFileSync(logfile, Buffer.concat(chunks));
    const expectedTimeout = job.expectTimeout === true && reason?.includes("command timeout");
    let accepted = expectedTimeout || code === (job.expectedCode ?? 0) && !reason;
    const output = Buffer.concat(chunks).toString();
    if (job.testCount !== undefined && !output.includes(`test result: ok. ${job.testCount} passed; 0 failed;`)) {
      accepted = false; reason = "Rust selector count mismatch";
    }
    for (const marker of job.requireStrings ?? []) if (!output.includes(marker)) { accepted = false; reason = "Missing required output: " + marker; }
    rows.push({ label: job.label, argv: job.argv, cwd: job.cwd, environment: { ...protocol.environment, ...job.environment }, pid, code, signal, reason, expectedTimeout, accepted, bytes, elapsedMs: Date.now() - began, log: "checks/" + phase + "-" + job.label + ".log", logSha256: sha(logfile) });
    pid = undefined; running = undefined;
    cleanup(job.cleanup ?? []); save("RUNNING");
    console.log(phase, job.label, expectedTimeout ? "EXPECTED DEFECT TIMEOUT" : code, `${rows.length}/${protocol.jobs.length}`);
    if (!accepted) throw new Error(job.label + " failed; inspect its retained log");
  }
  for (const [path, hash] of Object.entries(inputHashes)) if (sha(path) !== hash) throw new Error("Pinned input changed " + path);
  cleanup(protocol.cleanup ?? []);
  save(protocol.resultStatus ?? "PASS", { removedBytes, peakRawBytes });
} catch (error) {
  kill(); pid = undefined;
  cleanup(protocol.cleanupOnFailure ?? []);
  save("FAILED", { error: String(error), removedBytes, peakRawBytes });
  throw error;
}
