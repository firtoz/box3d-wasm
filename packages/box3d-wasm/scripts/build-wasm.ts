import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";

const scriptDir = dirname(fileURLToPath(import.meta.url));
const repoRoot = join(scriptDir, "..", "..", "..");
const packageRoot = join(repoRoot, "packages", "box3d-wasm");
const box3dSourceDir = join(repoRoot, "box3d");
const generatedDir = join(repoRoot, "demo", "public", "wasm");
const emcmakeCommand =
  process.platform === "win32"
    ? "emcmake"
    : "source /etc/profile.d/emscripten.sh 2>/dev/null || true; emcmake";

console.log("[box3d-wasm] preparing upstream engine checkout");
console.log(`[box3d-wasm] script: ${scriptDir}`);
console.log(`[box3d-wasm] package: ${packageRoot}`);
console.log(`[box3d-wasm] source: ${box3dSourceDir}`);
console.log(`[box3d-wasm] output: ${generatedDir}`);

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

await run(["cmake", "--version"], "Install CMake first.");
await run(["git", "--version"], "Install Git first.");

await Bun.write(join(generatedDir, ".gitkeep"), "");

console.log("[box3d-wasm] ensuring upstream box3d source is available");
await run(["bun", "scripts/prepare-box3d.ts"], "Failed to prepare the upstream engine checkout.");
console.log("[box3d-wasm] configuring and building wasm target");
console.log("[box3d-wasm] running emcmake/cmake build");
await run(
  [
    "bash",
    "-lc",
    `${emcmakeCommand} cmake -S ${join(packageRoot, "cmake")} -B ${join(packageRoot, "build-web")} -DBOX3D_SOURCE_DIR=${box3dSourceDir} -DBOX3D_SAMPLES=OFF -DBOX3D_BENCHMARKS=OFF -DBOX3D_DOCS=OFF -DBOX3D_UNIT_TESTS=OFF -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=${generatedDir} && cmake --build ${join(packageRoot, "build-web")}`,
  ],
  "Install Emscripten and ensure emcmake is available.",
);
await Bun.write(join(generatedDir, ".build-stamp"), `${Date.now()}\n`);
console.log("[box3d-wasm] wasm build complete");
