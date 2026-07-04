import { mkdir } from "node:fs/promises";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";

const scriptDir = dirname(fileURLToPath(import.meta.url));
const repoRoot = join(scriptDir, "..", "..", "..");
const packageRoot = join(repoRoot, "packages", "box3d-wasm");
const box3dSourceDir = join(repoRoot, "box3d");
const generatedDir = join(repoRoot, "demo", "public", "wasm");
const profileGeneratedDir = join(generatedDir, "profile");
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

async function buildVariant(label: string, buildDir: string, outputDir: string, profileNames: boolean): Promise<void> {
  console.log(`[box3d-wasm] building ${label} wasm target`);
  await mkdir(outputDir, { recursive: true });
  await Bun.write(join(outputDir, ".gitkeep"), "");
  await run(
    [
      "bash",
      "-lc",
      `${emcmakeCommand} cmake -S ${join(packageRoot, "cmake")} -B ${buildDir} -DBOX3D_SOURCE_DIR=${box3dSourceDir} -DBOX3D_SAMPLES=OFF -DBOX3D_BENCHMARKS=OFF -DBOX3D_DOCS=OFF -DBOX3D_UNIT_TESTS=OFF -DBOX3D_WASM_PROFILE_NAMES=${profileNames ? "ON" : "OFF"} -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=${outputDir} && cmake --build ${buildDir}`,
    ],
    "Install Emscripten and ensure emcmake is available.",
  );
}

await run(["cmake", "--version"], "Install CMake first.");
await run(["git", "--version"], "Install Git first.");

await mkdir(generatedDir, { recursive: true });
await Bun.write(join(generatedDir, ".gitkeep"), "");

console.log("[box3d-wasm] ensuring upstream box3d source is available");
await run(["bun", "scripts/prepare-box3d.ts"], "Failed to prepare the upstream engine checkout.");
console.log("[box3d-wasm] configuring and building wasm target");
await buildVariant("release", join(packageRoot, "build-web"), generatedDir, false);
await buildVariant("profile", join(packageRoot, "build-web-profile"), profileGeneratedDir, true);
await Bun.write(join(generatedDir, ".build-stamp"), `${Date.now()}\n`);
console.log("[box3d-wasm] wasm build complete");
