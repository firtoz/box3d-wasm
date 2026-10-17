import { spawnSync } from "node:child_process";
import { createHash } from "node:crypto";
import { readFileSync, writeFileSync, existsSync } from "node:fs";
import { resolve, join, relative, basename } from "node:path";

// Bounded, read-only source/link audit. No Python automation, builds or GPU work.
const root = resolve("experiments/gpu-physics");
const repo = resolve(".");
const out = join(root, "benchmarks/production-readiness/pr02-contract-refresh-2026-10-17");
const started = Date.now();
const protocol = JSON.parse(readFileSync(join(out, "protocol.json"), "utf8"));
if (existsSync(join(out, "inventory.json"))) throw new Error("Inventory already exists; inspect it rather than overwriting it.");
const sha = (s: string | Buffer) => createHash("sha256").update(s).digest("hex");
const read = (p: string) => readFileSync(p, "utf8");
const stripComments = (s: string) => s.replace(/\/\*[\s\S]*?\*\/|\/\/[^\n]*/g, m => m.replace(/[^\n]/g, " "));
const mask = (s: string) => s.replace(/\/\*[\s\S]*?\*\/|\/\/[^\n]*|"(?:\\.|[^"\\])*"|'(?:\\.|[^'\\])*'/g, m => m.replace(/[^\n]/g, " "));
function run(args: string[]) {
  if (Date.now() - started > protocol.budget.inventorySeconds * 1000) throw new Error("Inventory total timeout");
  const r = spawnSync(args[0], args.slice(1), { encoding: "utf8", timeout: protocol.budget.inventoryCommandSeconds * 1000, maxBuffer: 32 * 1024 * 1024 });
  if (r.status !== 0) throw new Error(`${args[0]}: ${r.error ?? r.stderr}`);
  return r.stdout;
}
type Definition = { line: number; text: string; macroExpanded: boolean };
function definitions(input: string, rust = false) {
  let source = input;
  const originalLines = source.split("\n").length;
  const lines = input.split("\n");
  // Finite source indexing only. Actual compiler objects and successful links,
  // rather than this textual expansion, establish compilation/link coverage.
  for (let i = 0; i < lines.length; ++i) {
    const macro = lines[i].match(/^#define ((?:BOTH_|MOTOR_)\w+)\(([^)]*)\)/);
    if (!macro) continue;
    const body = [lines[i].slice(macro[0].length)];
    let end = i;
    while (lines[end].trimEnd().endsWith("\\")) body.push(lines[++end]);
    const template = body.map(s => s.trimEnd().replace(/\\$/, "").trim()).join("\n");
    const params = macro[2].split(",").map(s => s.trim());
    for (const call of input.matchAll(new RegExp("^" + macro[1] + "\\(([^)]*)\\)", "gm"))) {
      let expanded = template;
      const args = call[1].split(",").map(s => s.trim());
      params.forEach((p, n) => { expanded = expanded.replace(new RegExp("\\b" + p + "\\b", "g"), args[n]); });
      source += "\n" + expanded.replace(/\s*##\s*/g, "") + "\n";
    }
  }
  const matches = rust
    ? source.matchAll(/pub\s+(?:unsafe\s+)?extern\s+"C"\s+fn\s+((?:gpu_)?b3\w+)\s*\([^{}]*?\)\s*(?:->[^{}]*?)?\s*\{/g)
    : source.matchAll(/^(?:B3_API\s+)?[A-Za-z_][\w \t*]*?\b(b3\w+)\([^;]*?\)\s*\{/gm);
  const masked = mask(source);
  const result = new Map<string, Definition>();
  for (const m of matches) {
    let end = m.index! + m[0].length;
    let level = 1;
    while (level && end < masked.length) { level += Number(masked[end] === "{") - Number(masked[end] === "}"); ++end; }
    if (level) throw new Error("Unbalanced source definition " + m[1]);
    const line = source.slice(0, m.index).split("\n").length;
    result.set(m[1], { line: Math.min(line, originalLines), text: source.slice(m.index, end), macroExpanded: line > originalLines });
  }
  return result;
}
const headers = ["box3d.h", "collision.h", "math_functions.h", "types.h", "constants.h"].map(n => resolve("box3d/include/box3d", n));
const declarations = new Map<string, { header: string; signature: string; args: string }>();
for (const path of headers) {
  const s = stripComments(read(path));
  for (const m of s.matchAll(/B3_API\s+((?:const\s+)?[\w\s*]+?)\s+(b3[A-Za-z0-9_]+)\s*\(([\s\S]*?)\)\s*;/g)) {
    if (m[1].includes("#") || m[3].includes("#") || m[1].includes('"')) continue;
    declarations.set(m[2], { header: relative(repo, path), signature: m[0].replace(/\s+/g, " "), args: m[3] });
  }
}
const required = [...declarations].filter(([n, d]) => /\bb3(?:World|Body|Shape|Joint)Id\b/.test(d.args)
  || /^b3(?:RecPlayer|Recording)_/.test(n)
  || ["b3CreateWorld", "b3CreateRecording", "b3DestroyRecording", "b3CreatePlayer", "b3DestroyPlayer", "b3SaveRecordingToFile", "b3LoadRecordingFromFile", "b3ValidateReplay"].includes(n)).map(([n]) => n).sort();
const defaults = [...declarations.keys()].filter(n => n.startsWith("b3Default")).sort();
const unavailable = new Set([...read(join(root, "c_abi/native_unavailable.inc")).matchAll(/GPU_UNAVAILABLE_API\((b3\w+)\)/g)].map(m => m[1]));
const shared = new Set([...read(join(root, "c_abi/native_shared.inc")).matchAll(/GPU_SHARED_API\((b3\w+)\)/g)].map(m => m[1]));
const rustPath = join(root, "src/c_abi.rs");
const rustDefs = definitions(read(rustPath), true);
const providers = new Map<string, Map<string, { symbol: string; object: string; kind: string }[]>>();
function symbols(path: string) {
  if (providers.has(path)) return providers.get(path)!;
  const result = new Map<string, { symbol: string; object: string; kind: string }[]>();
  for (const line of run(["nm", "-A", "-g", "--defined-only", path]).split("\n")) {
    const m = line.match(/^(.*):\s*(?:[0-9a-fA-F]+\s+)?([TW])\s+((?:gpu_)?b3\w+)$/);
    if (!m) continue;
    const row = { symbol: m[3], object: m[1].slice(path.length + 1), kind: m[2] };
    result.set(row.symbol, [...(result.get(row.symbol) ?? []), row]);
  }
  providers.set(path, result); return result;
}
const libraries = JSON.parse(read(join(out, "identities.json"))).libraries;
const rows: any[] = [], issues: any[] = [], sourceRecords: Record<string, any> = {}, summaries: any[] = [];
const original = JSON.parse(read(join(root, "benchmarks/production-readiness/pr02-current-acceptance-2026-10-02/api-inventory-ordinary.json")));
const previous = new Map(original.map((r: any) => [r.cell.replace("ordinary", "") + r.symbol, r.implementation.definition_sha256]));
for (const backend of ["ordinary", "native"]) {
  const rustLibrary = libraries[backend].path;
  if (sha(readFileSync(rustLibrary)) !== libraries[backend].sha256) throw new Error("Current library does not match fresh fixtures");
  for (const mode of ["gpu", "both"]) {
    const cell = `${backend}-${mode}`;
    const dir = join(root, `native-samples/build-${mode}${backend === "native" ? "-native-cache" : ""}-portable`);
    const archives = [rustLibrary, join(dir, "libgpu_samples_api.a"), ...(mode === "both" ? [join(dir, "libgpu_both_api.a")] : []), join(dir, "box3d_src/libbox3d.a")];
    const files = ["shim.c", "samples_api.c", "samples_stubs.c"].map(n => join(dir, "api-sources", n));
    if (mode === "both") files.push(join(root, "c_abi/both_dual.c"), join(dir, "both_passthrough.c"));
    const indices = new Map(files.map(p => [p, definitions(read(p))]));
    const compiled = JSON.parse(read(join(dir, "compile_commands.json")));
    for (const p of files) {
      const command = compiled.find((c: any) => c.file === p);
      if (!command) throw new Error("No actual compiler recipe " + p);
      const object = resolve(command.directory, command.output);
      sourceRecords[relative(repo, p)] = { sha256: sha(read(p)), object: relative(repo, object), objectSha256: sha(readFileSync(object)), command: command.command };
    }
    let changed = 0;
    for (const name of [...required, ...defaults]) {
      const isDefault = defaults.includes(name);
      const linked = archives.flatMap(a => (symbols(a).get(name) ?? []).map(s => ({ archive: relative(repo, a), ...s })));
      if (linked.length !== 1) issues.push({ cell, name, problem: "Expected exactly one public provider", linked });
      const provider = linked[0];
      let definition: Definition | undefined, source: string | undefined;
      if (provider) {
        if (provider.archive === relative(repo, rustLibrary)) { definition = rustDefs.get(name); source = rustPath; }
        else {
          const object = provider.object.match(/([^/:]+)\.o$/)?.[1];
          source = files.find(p => basename(p) === object);
          if (!source && object) {
            const generated = join(dir, "api-sources", object);
            if (existsSync(generated)) source = generated;
          }
          if (source) definition = (indices.get(source) ?? definitions(read(source))).get(name);
        }
      }
      if (!definition || !source) issues.push({ cell, name, problem: "Missing source definition for actual provider", provider });
      const calls = [...new Set([...(definition?.text ?? "").matchAll(/\b(gpu_b3_\w+)\s*\(/g)].map(m => m[1]))].sort();
      for (const call of calls) if (!archives.some(a => symbols(a).has(call))) issues.push({ cell, name, problem: "GPU call not linked", call });
      const excluded = unavailable.has(name);
      if (excluded && !definition?.text.includes("gpu_native_api_unavailable(__func__)")) issues.push({ cell, name, problem: "Exclusion lacks error hook" });
      if (mode === "both" && !excluded && !isDefault && source?.endsWith("both_passthrough.c")) issues.push({ cell, name, problem: "Stateful CPU-only passthrough" });
      if (mode === "both" && shared.has(name) && source?.endsWith("both_passthrough.c")) issues.push({ cell, name, problem: "Shared diagnostics routed to CPU" });
      const hash = definition ? sha(definition.text) : null;
      const previousHash = previous.get("-" + mode + name);
      if (!isDefault && hash !== previousHash) ++changed;
      rows.push({ cell, symbol: name, signature: declarations.get(name)!.signature, header: declarations.get(name)!.header,
        contract: isDefault ? "default alias: inventoried; only filter/shape-filter/query-filter defaults independently rechecked here" : excluded ? "explicitly unavailable: ENOTSUP and sticky thread-local operation name" : "implemented candidate; named PR02 contracts pass only when linked to applicable tests, full physical/runtime release qualification remains open",
        classification: isDefault ? "default-alias" : excluded ? "declared-unavailable" : "implemented-behavior-unqualified",
        provider, source: source ? relative(repo, source) : null, sourceSha256: source ? sha(read(source)) : null,
        definitionSha256: hash, line: definition?.line, macroExpanded: definition?.macroExpanded, GPU_calls: calls,
        unchangedDefinitionSinceOriginalPR02: isDefault ? null : hash === previousHash });
    }
    summaries.push({ cell, stateful: required.length, implemented: required.length - unavailable.size, exclusions: unavailable.size, defaultAliases: defaults.length, changedDefinitionHashes: changed });
  }
}
if (required.length !== 415 || unavailable.size !== 39) issues.push({ problem: "Header/exclusion inventory changed", stateful: required.length, exclusions: unavailable.size });
sourceRecords[relative(repo, rustPath)] = { sha256: sha(read(rustPath)), definitions: rustDefs.size };
const result = { status: issues.length ? "FAILED" : "PASS", elapsedMs: Date.now() - started, summaries, issues, sourceRecords, rows,
  limitations: ["Finite macro expansion indexes sources; fresh compiled objects, symbol tables and actual linked fixtures establish compilation/link coverage.", "No claim of 376 independent physical behavior passes or universal API compatibility.", "Only filter/shape-filter/query-filter defaults have new independent behavioral checks; the remaining default aliases are source/link coverage.", "This inventory does not replace PR04 physics, PR05 lifecycle/concurrency, PR06 clean builds or PR07-PR08 release qualification."] };
writeFileSync(join(out, "inventory.json"), JSON.stringify(result, null, 2) + "\n");
console.log(JSON.stringify({ status: result.status, summaries, issues: issues.slice(0, 12), totalIssues: issues.length, elapsedMs: result.elapsedMs }));
if (issues.length) process.exitCode = 1;
