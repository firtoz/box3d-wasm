import { mkdir, readFile } from "node:fs/promises";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";
import { getWasmBuildMode, variantsToBuild, type WasmBuildVariant } from "./wasm-variant";

const scriptDir = dirname(fileURLToPath(import.meta.url));
const repoRoot = join(scriptDir, "..", "..", "..");
const packageRoot = join(repoRoot, "packages", "box3d-wasm");
/** Clean submodule is archived + patched here by prepare-box3d.ts (keep box3d/ clean). */
const box3dSourceDir = join(packageRoot, ".box3d-patched");
const generatedDir = join(repoRoot, "demo", "public", "wasm");
const emcmakeCommand =
  process.platform === "win32"
    ? "emcmake"
    : "source /etc/profile.d/emscripten.sh 2>/dev/null || true; emcmake";

const buildMode = getWasmBuildMode();
const variants = variantsToBuild(buildMode);

console.log("[box3d-wasm] preparing upstream engine checkout");
console.log(`[box3d-wasm] script: ${scriptDir}`);
console.log(`[box3d-wasm] package: ${packageRoot}`);
console.log(`[box3d-wasm] source: ${box3dSourceDir}`);
console.log(`[box3d-wasm] output: ${generatedDir}`);
console.log(`[box3d-wasm] build mode: ${buildMode} (${variants.join(", ")})`);

interface BuildVariantOptions {
  profileNames: boolean;
  allowMemoryGrowth: boolean;
  initialMemory: number;
}

interface VariantSpec {
  label: WasmBuildVariant;
  buildDir: string;
  outputDir: string;
  options: BuildVariantOptions;
}

const VARIANT_SPECS: Record<WasmBuildVariant, Omit<VariantSpec, "label">> = {
  release: {
    buildDir: join(packageRoot, "build-web"),
    outputDir: generatedDir,
    options: { profileNames: false, allowMemoryGrowth: false, initialMemory: 268435456 },
  },
  growable: {
    buildDir: join(packageRoot, "build-web-growable"),
    outputDir: join(generatedDir, "growable"),
    options: { profileNames: false, allowMemoryGrowth: true, initialMemory: 67108864 },
  },
  profile: {
    buildDir: join(packageRoot, "build-web-profile"),
    outputDir: join(generatedDir, "profile"),
    options: { profileNames: true, allowMemoryGrowth: false, initialMemory: 268435456 },
  },
};

async function run(command: string[], hint: string): Promise<void> {
  const proc = Bun.spawn(command, {
    stdio: ["inherit", "inherit", "inherit"],
    env: process.env,
  });

  const exitCode = await proc.exited;
  if (exitCode !== 0) {
    throw new Error(`${command[0]} failed with exit code ${exitCode}. ${hint}`);
  }
}

async function verifyWasmGlue(outputDir: string, label: WasmBuildVariant): Promise<void> {
  const jsPath = join(outputDir, "box3d-web.js");
  const wasmPath = join(outputDir, "box3d-web.wasm");
  const js = await readFile(jsPath, "utf8");
  const wasm = await readFile(wasmPath);
  const imports = WebAssembly.Module.imports(new WebAssembly.Module(wasm));
  const memoryImport = imports.find((entry) => entry.kind === "memory");
  if (memoryImport === undefined) {
    throw new Error(`[box3d-wasm] ${label}: wasm has no memory import`);
  }
  const pattern = new RegExp(`\\b${memoryImport.name}\\s*:\\s*wasmMemory\\b`);
  if (!pattern.test(js)) {
    throw new Error(
      `[box3d-wasm] ${label}: glue JS at ${outputDir} does not map memory import "${memoryImport.name}" to wasmMemory. ` +
        "Delete the build/output directories and rebuild — JS and WASM are out of sync.",
    );
  }
}

async function buildVariant(spec: VariantSpec): Promise<void> {
  console.log(`[box3d-wasm] building ${spec.label} wasm target`);
  await mkdir(spec.outputDir, { recursive: true });
  await Bun.write(join(spec.outputDir, ".gitkeep"), "");
  const cmakeArgs = [
    `${emcmakeCommand} cmake -S ${join(packageRoot, "cmake")} -B ${spec.buildDir}`,
    `-DBOX3D_SOURCE_DIR=${box3dSourceDir}`,
    "-DBOX3D_SAMPLES=OFF",
    "-DBOX3D_BENCHMARKS=OFF",
    "-DBOX3D_DOCS=OFF",
    "-DBOX3D_UNIT_TESTS=OFF",
    "-DBOX3D_VALIDATE=OFF",
    "-DBOX3D_DISABLE_SIMD=OFF",
    `-DBOX3D_WASM_PROFILE_NAMES=${spec.options.profileNames ? "ON" : "OFF"}`,
    `-DBOX3D_WASM_ALLOW_MEMORY_GROWTH=${spec.options.allowMemoryGrowth ? "ON" : "OFF"}`,
    `-DBOX3D_WASM_INITIAL_MEMORY=${spec.options.initialMemory}`,
    "-DCMAKE_BUILD_TYPE=Release",
    "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
    `-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=${spec.outputDir}`,
    `&& cmake --build ${spec.buildDir}`,
  ].join(" ");
  await run(
    ["bash", "-lc", cmakeArgs],
    "Install Emscripten and ensure emcmake is available.",
  );
  await verifyWasmGlue(spec.outputDir, spec.label);
}

await run(["cmake", "--version"], "Install CMake first.");
await run(["git", "--version"], "Install Git first.");

await mkdir(generatedDir, { recursive: true });
await Bun.write(join(generatedDir, ".gitkeep"), "");

console.log("[box3d-wasm] ensuring upstream box3d source is available");
await run(["bun", join(scriptDir, "prepare-box3d.ts")], "Failed to prepare the upstream engine checkout.");
console.log("[box3d-wasm] configuring and building wasm target");
for (const label of variants) {
  const spec = VARIANT_SPECS[label];
  await buildVariant({ label, ...spec });
}
await Bun.write(join(generatedDir, ".build-stamp"), `${Date.now()}\n`);
await Bun.write(
  join(generatedDir, ".build-meta.json"),
  `${JSON.stringify({ mode: buildMode, variants, builtAt: Date.now() }, null, 2)}\n`,
);
console.log("[box3d-wasm] wasm build complete");
