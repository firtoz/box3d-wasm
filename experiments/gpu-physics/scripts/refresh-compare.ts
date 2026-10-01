#!/usr/bin/env bun
/** Write compare/snapshots.js from recordings/snapshots/*. */

import { mkdirSync, readdirSync, readFileSync, statSync, writeFileSync } from "node:fs";
import { dirname, join, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const SAMPLES = [
  "single-box",
  "box-stack",
  "sphere-stack",
  "capsule-stack",
  "revolute",
  "weld",
  "stack",
  "pyramid",
  "bounce",
  "mixed",
  "spinner",
  "ramp",
  "spheres",
  "dominoes",
  "high-resistance",
  "mixed-stacks",
  "falling-cubes",
  "mixed-topology",
  "anchored-mechanisms",
  "joint-chain",
  // Native viewer captures; the Rust demo recorder does not host these samples.
  "shape-replacement",
  "offset-kinematic",
  "falling-ragdolls",
  "ragdoll-rain",
  "sbox-ghost-collisions",
  "compound-simple",
  "compound-spheres",
  "compound-hulls",
  "compound-tile-floor",
  "compound-village",
  "compound-mesh-tile",
  // Fresh ownership captures: independent CPU, then synchronized CPU/GPU viewer.
  "compound-ownership-simple",
  "compound-ownership-spheres",
  "compound-ownership-hulls",
  "compound-ownership-tile-floor",
  "compound-ownership-village",
  "compound-ownership-mesh-tile",
  // Fresh public-property captures: independent CPU, then native combined.
  "compound-properties-simple",
  "compound-properties-spheres",
  "compound-properties-hulls",
  "compound-properties-tile-floor",
  "compound-properties-village",
  "compound-properties-mesh-tile",
] as const;

type Metrics = {
  version?: string;
  engine?: string;
  [key: string]: unknown;
};

const scriptDir = dirname(fileURLToPath(import.meta.url));
const root = resolve(process.argv[2] ?? join(scriptDir, ".."));
const snapRoot = join(root, "recordings", "snapshots");

function metricsOf(dir: string): Metrics | null {
  const mp = join(dir, "metrics.json");
  try {
    if (!statSync(mp).isFile()) {
      return null;
    }
  } catch {
    return null;
  }
  return JSON.parse(readFileSync(mp, "utf8")) as Metrics;
}

function versionTuple(metrics: Metrics | null): number[] {
  const raw = metrics?.version;
  if (typeof raw === "string") {
    const s = raw.replace(/^v/i, "");
    const parts = s.split(".").map((x) => Number.parseInt(x, 10));
    if (parts.length > 0 && parts.every((n) => Number.isFinite(n))) {
      return parts;
    }
  }
  return [999];
}

// Persisted capture timestamps survive checkout/copy, unlike filesystem times.
// A scoped recording can have its own manifest without relabelling older clips.
function recordingTime(name: string): number {
  const dir = join(snapRoot, name);
  let latest = Number.NaN;
  for (const file of readdirSync(dir)) {
    if (!file.endsWith("recording-manifest.json")) continue;
    const manifest = JSON.parse(readFileSync(join(dir, file), "utf8"));
    const time = Date.parse(manifest.recording_started_utc);
    if (Number.isFinite(time) && (!Number.isFinite(latest) || time > latest)) latest = time;
  }
  return Number.isFinite(latest) ? latest : Date.parse(name.slice(0, 10));
}

function isCpu(name: string, metrics: Metrics | null): boolean {
  if (name.includes("box3d-cpu")) {
    return true;
  }
  return metrics?.engine === "box3d-cpu";
}

function cmpTuples(a: number[], b: number[]): number {
  const n = Math.max(a.length, b.length);
  for (let i = 0; i < n; i++) {
    const d = (a[i] ?? 0) - (b[i] ?? 0);
    if (d !== 0) {
      return d;
    }
  }
  return 0;
}

function labelFor(name: string, metrics: Metrics | null, gpuIndex: number | null): string {
  if (isCpu(name, metrics)) {
    return "Box3D CPU";
  }
  let ver: string | null = null;
  if (typeof metrics?.version === "string") {
    ver = metrics.version.replace(/^v/i, "");
  } else if (gpuIndex !== null) {
    ver = `0.${gpuIndex + 1}`;
  }
  return ver ? `v${ver}` : name;
}

const dirs = (() => {
  try {
    return readdirSync(snapRoot, { withFileTypes: true })
      .filter((e) => e.isDirectory())
      .map((e) => e.name)
      .sort((a, b) => {
        const ma = metricsOf(join(snapRoot, a));
        const mb = metricsOf(join(snapRoot, b));
        const cpuDelta = Number(isCpu(b, mb)) - Number(isCpu(a, ma));
        if (cpuDelta !== 0) {
          return cpuDelta;
        }
        const recorded = recordingTime(a) - recordingTime(b);
        if (Number.isFinite(recorded) && recorded !== 0) {
          return recorded;
        }
        const v = cmpTuples(versionTuple(ma), versionTuple(mb));
        if (v !== 0) {
          return v;
        }
        return a.localeCompare(b);
      });
  } catch {
    return [];
  }
})();

let gpuIndex = 0;
const snapshots = dirs.map((name) => {
  const dir = join(snapRoot, name);
  const videos: Record<string, string> = {};
  for (const s of SAMPLES) {
    const p = join(dir, `${s}.mp4`);
    try {
      const st = statSync(p);
      if (st.isFile() && st.size > 0) {
        videos[s] = `../recordings/snapshots/${name}/${s}.mp4`;
      }
    } catch {
      // missing clip
    }
  }
  const metrics = metricsOf(dir);
  const cpu = isCpu(name, metrics);
  const idx = cpu ? null : gpuIndex;
  if (!cpu) {
    gpuIndex += 1;
  }
  return {
    id: name,
    label: labelFor(name, metrics, idx),
    videos,
    metrics,
  };
});

const out = join(root, "compare", "snapshots.js");
mkdirSync(dirname(out), { recursive: true });
const payload = { samples: SAMPLES, snapshots };
writeFileSync(
  out,
  `/* generated by scripts/refresh-compare.ts — do not edit by hand */\nwindow.GPU_COMPARE = ${JSON.stringify(payload, null, 2)};\n`,
  "utf8",
);
console.log(`wrote ${out} (${snapshots.length} snapshot(s))`);
