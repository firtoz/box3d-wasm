import { relative, resolve } from "node:path";
import { watch } from "chokidar";

export function isWasmInput(path: string): boolean {
  const normalized = path.replaceAll("\\", "/");
  return (
    /^(src|scripts)\/.*\.ts$/.test(normalized) ||
    /^cmake\/.*\.(c|h|txt)$/.test(normalized) ||
    /^\.\.\/\.\.\/patches\/box3d\/.*\.patch$/.test(normalized)
  );
}

export async function watchWasm(
  directory: string,
  build: (signal: AbortSignal) => Promise<void>,
  debounceMs = 100,
) {
  const controller = new AbortController();
  let closed = false;
  let pending = false;
  let running = false;
  let timer: ReturnType<typeof setTimeout> | undefined;
  const watcher = watch(
    ["src", "cmake", "scripts", "../../patches/box3d"].map((path) => resolve(directory, path)),
    { ignoreInitial: true },
  );

  async function rebuild() {
    if (closed || running || !pending) return;
    pending = false;
    running = true;
    try {
      await build(controller.signal);
    } catch (error) {
      if (!closed) console.error("[watch] build failed", error);
    } finally {
      running = false;
      if (pending && !closed) timer = setTimeout(rebuild, debounceMs);
    }
  }

  watcher.on("all", (event, path) => {
    if (
      closed ||
      !["add", "change", "unlink"].includes(event) ||
      !isWasmInput(relative(directory, path))
    )
      return;
    pending = true;
    clearTimeout(timer);
    if (!running) timer = setTimeout(rebuild, debounceMs);
  });
  await new Promise<void>((resolveReady, reject) => {
    watcher.once("ready", resolveReady);
    watcher.once("error", reject);
  });
  return {
    async close() {
      closed = true;
      clearTimeout(timer);
      controller.abort();
      await watcher.close();
    },
  };
}

if (import.meta.main) {
  const watcher = await watchWasm(process.cwd(), async (signal) => {
    const child = Bun.spawn(["bun", "run", "build:wasm"], {
      cwd: process.cwd(),
      stdout: "inherit",
      stderr: "inherit",
      signal,
    });
    const code = await child.exited;
    if (code !== 0) throw new Error("WASM build exited with code " + code);
  });
  for (const signal of ["SIGINT", "SIGTERM"] as const) {
    process.once(signal, async () => {
      await watcher.close();
      process.exit(signal === "SIGINT" ? 130 : 143);
    });
  }
  console.log("[watch] watching TypeScript, C bridge and engine patch inputs");
}
