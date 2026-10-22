import { expect, test } from "bun:test";
import { mkdtemp, mkdir, rm, writeFile } from "node:fs/promises";
import { tmpdir } from "node:os";
import { join } from "node:path";
import { isWasmInput, watchWasm } from "./watch-wasm";

test("matches the original watch inputs and excludes generated/unrelated files", () => {
  for (const path of [
    "src/index.ts",
    "scripts/build-wasm.ts",
    "cmake/bridge.c",
    "cmake/bridge.h",
    "cmake/CMakeLists.txt",
    "../../patches/box3d/fix.patch",
  ])
    expect(isWasmInput(path)).toBe(true);
  for (const path of [
    "build-web/output.c",
    "src/image.png",
    "cmake/readme.md",
    "../../patches/box3d/README.md",
  ])
    expect(isWasmInput(path)).toBe(false);
});

test("real file events debounce and queue changes without overlapping builds", async () => {
  const root = await mkdtemp(join(tmpdir(), "box3d-watch-"));
  const directory = join(root, "packages", "box3d-wasm");
  const directories = ["src", "scripts", "cmake", "../../patches/box3d"];
  for (const path of directories) await mkdir(join(directory, path), { recursive: true });
  let runs = 0;
  let active = 0;
  let maxActive = 0;
  let releaseFirst: (() => void) | undefined;
  const firstBuild = new Promise<void>((resolve) => {
    releaseFirst = resolve;
  });
  const watcher = await watchWasm(
    directory,
    async () => {
      runs++;
      active++;
      maxActive = Math.max(maxActive, active);
      if (runs === 1) await firstBuild;
      active--;
    },
    20,
  );
  async function until(condition: () => boolean) {
    const deadline = Date.now() + 4000;
    while (!condition()) {
      if (Date.now() > deadline) throw new Error("Timed out waiting for watcher");
      await Bun.sleep(10);
    }
  }
  try {
    await writeFile(join(directory, "src/ignored.png"), "ignore");
    await Bun.sleep(70);
    expect(runs).toBe(0);
    await writeFile(join(directory, "src/index.ts"), "first");
    await until(() => runs === 1);
    for (const path of ["cmake/bridge.c", "scripts/build-wasm.ts", "../../patches/box3d/fix.patch"])
      await writeFile(join(directory, path), "change");
    await Bun.sleep(100);
    expect(runs).toBe(1);
    releaseFirst?.();
    await until(() => runs === 2);
    expect(maxActive).toBe(1);
    await watcher.close();
    await writeFile(join(directory, "src/index.ts"), "after close");
    await Bun.sleep(70);
    expect(runs).toBe(2);
  } finally {
    releaseFirst?.();
    await watcher.close();
    await rm(root, { recursive: true, force: true });
  }
});
