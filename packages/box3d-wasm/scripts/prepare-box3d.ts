import { createHash } from "node:crypto";
import { mkdir, readdir, readFile, rm, writeFile } from "node:fs/promises";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";

const scriptDir = dirname(fileURLToPath(import.meta.url));
const repoRoot = join(scriptDir, "..", "..", "..");
const packageRoot = join(repoRoot, "packages", "box3d-wasm");
const submoduleDir = join(repoRoot, "box3d");
const patchesDir = join(repoRoot, "patches", "box3d");
const patchedDir = join(packageRoot, ".box3d-patched");
const stampPath = join(patchedDir, ".patch-stamp");

async function capture(command: string[], cwd?: string): Promise<string> {
  const proc = Bun.spawn(command, {
    cwd,
    stdout: "pipe",
    stderr: "pipe",
    env: process.env,
  });
  const [stdout, stderr, exitCode] = await Promise.all([
    new Response(proc.stdout).text(),
    new Response(proc.stderr).text(),
    proc.exited,
  ]);
  if (exitCode !== 0) {
    throw new Error(`${command.join(" ")} failed (${exitCode}): ${stderr || stdout}`);
  }
  return stdout.trim();
}

if (!(await Bun.file(join(submoduleDir, "CMakeLists.txt")).exists())) {
  throw new Error(
    `Missing submodule checkout at ${submoduleDir}. Run git submodule update --init --recursive.`,
  );
}

const submoduleSha = await capture(["git", "rev-parse", "HEAD"], submoduleDir);
const patchFiles = (await readdir(patchesDir))
  .filter((name) => name.endsWith(".patch"))
  .sort();

if (patchFiles.length === 0) {
  throw new Error(`No *.patch files found in ${patchesDir}`);
}

const baseShaPath = join(patchesDir, "BASE_SHA");
const baseSha = (await Bun.file(baseShaPath).exists())
  ? (await readFile(baseShaPath, "utf8")).trim()
  : "";

if (baseSha && baseSha !== submoduleSha) {
  console.warn(
    `[box3d-wasm] box3d submodule HEAD ${submoduleSha} differs from patches/box3d/BASE_SHA ${baseSha}. ` +
      "Patches will still be applied; refresh them if apply fails.",
  );
}

const patchHasher = createHash("sha256");
patchHasher.update(submoduleSha);
patchHasher.update("\n");
for (const name of patchFiles) {
  patchHasher.update(name);
  patchHasher.update("\n");
  patchHasher.update(await readFile(join(patchesDir, name)));
  patchHasher.update("\n");
}
const stamp = patchHasher.digest("hex");

const existingStamp = (await Bun.file(stampPath).exists()) ? (await readFile(stampPath, "utf8")).trim() : "";
if (existingStamp === stamp && (await Bun.file(join(patchedDir, "CMakeLists.txt")).exists())) {
  console.log(`[box3d-wasm] patched box3d up to date at ${patchedDir}`);
  console.log(`[box3d-wasm] submodule ${submoduleSha}; patches: ${patchFiles.join(", ")}`);
  process.exit(0);
}

console.log(`[box3d-wasm] preparing patched box3d at ${patchedDir}`);
await rm(patchedDir, { recursive: true, force: true });
await mkdir(patchedDir, { recursive: true });

const exportProc = Bun.spawn(
  ["bash", "-lc", `git -C ${JSON.stringify(submoduleDir)} archive HEAD | tar -x -C ${JSON.stringify(patchedDir)}`],
  { stdout: "inherit", stderr: "inherit", env: process.env },
);
const exportCode = await exportProc.exited;
if (exportCode !== 0) {
  throw new Error(`Failed to export clean box3d tree (exit ${exportCode})`);
}

const patchedDirRel = join("packages", "box3d-wasm", ".box3d-patched");

for (const name of patchFiles) {
  const patchPathRel = join("patches", "box3d", name);
  console.log(`[box3d-wasm] applying ${name}`);

  // Must use --directory from the repo root: cwd inside the worktree makes
  // `git apply` resolve paths against the parent git repo and skip hunks.
  const check = Bun.spawn(
    ["git", "apply", "--check", "--directory", patchedDirRel, "-p1", patchPathRel],
    { cwd: repoRoot, stdout: "pipe", stderr: "pipe", env: process.env },
  );
  const [checkOut, checkErr, checkCode] = await Promise.all([
    new Response(check.stdout).text(),
    new Response(check.stderr).text(),
    check.exited,
  ]);
  if (checkCode !== 0) {
    throw new Error(
      [
        `Failed to apply Box3D patch ${name} onto submodule ${submoduleSha}.`,
        "The box3d submodule likely moved past this patch — update files under patches/box3d/, set BASE_SHA, keep box3d/ clean, then rebuild.",
        "See patches/box3d/README.md and AGENTS.md (Box3D patches).",
        checkErr || checkOut,
      ].join("\n"),
    );
  }

  const apply = Bun.spawn(
    ["git", "apply", "--directory", patchedDirRel, "-p1", patchPathRel],
    { cwd: repoRoot, stdout: "pipe", stderr: "pipe", env: process.env },
  );
  const [applyOut, applyErr, applyCode] = await Promise.all([
    new Response(apply.stdout).text(),
    new Response(apply.stderr).text(),
    apply.exited,
  ]);
  if (applyCode !== 0) {
    throw new Error(`git apply failed for ${name}: ${applyErr || applyOut}`);
  }
}

await writeFile(stampPath, `${stamp}\n`);
console.log(`[box3d-wasm] patched box3d ready (${patchFiles.length} patch(es))`);
