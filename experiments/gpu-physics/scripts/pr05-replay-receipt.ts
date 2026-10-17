import { readFileSync, writeFileSync, rmSync } from "node:fs";
import { createHash } from "node:crypto";
import { resolve, join } from "node:path";
const out = resolve("experiments/gpu-physics/benchmarks/production-readiness/pr05-pr06-native-2026-10-17");
const path = join(out, "raw/replay-final.jsonl");
const bytes = readFileSync(path);
if (bytes.length > 32 * 1024 ** 2) throw new Error("Trace cap");
const lines = bytes.toString().trim().split("\n");
if (lines.length !== 48) throw new Error("Expected complete 48-step capture");
const hash = (b: string | Buffer) => createHash("sha256").update(b).digest("hex");
function replay(value: any): any {
  if (!value || typeof value !== "object") return undefined;
  if (value.native?.replay) return value.native.replay;
  for (const child of Object.values(value)) { const result = replay(child); if (result) return result; }
}
const rows = lines.map((line, i) => {
  const record = JSON.parse(line), key = replay(record);
  if (!key || key.schedule.length !== 16 || !key.current_bind_group) throw new Error(`Replay key at step ${i + 1}`);
  return { step: i + 1, recordSha256: hash(line), parameterBytes: key.parameters.length, schedule: key.schedule };
});
const selected = [19, 24, 32, 48].map(step => ({ step, record: JSON.parse(lines[step - 1]) }));
writeFileSync(join(out, "replay-capture-selected.json"), JSON.stringify(selected) + "\n");
writeFileSync(join(out, "replay-capture-summary.json"), JSON.stringify({ status: "PASS", records: 48, completeStreamSha256: hash(bytes), rows, selectedSteps: selected.map(r => r.step), consumedAndRemovedBytes: bytes.length, scope: "Diagnostic replay-key completeness and existing hit/invalidation assertions only; not final-build repeat or physical qualification" }, null, 2) + "\n");
rmSync(path);
console.log(JSON.stringify({ status: "PASS", records: 48, scheduleBytes: 16, removedBytes: bytes.length }));
