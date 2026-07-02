import { defineConfig } from "vite";
import { readFile } from "node:fs/promises";
import { resolve } from "node:path";

const stampPath = resolve(process.cwd(), "public/wasm/.build-stamp");

async function readWasmVersion(): Promise<string> {
  try {
    return (await readFile(stampPath, "utf8")).trim() || "0";
  } catch {
    return "0";
  }
}

export default defineConfig({
  server: {
    host: "0.0.0.0",
    headers: {
      "Cross-Origin-Opener-Policy": "same-origin",
      "Cross-Origin-Embedder-Policy": "require-corp",
    },
  },
  optimizeDeps: {
    exclude: ["box3d-wasm"],
  },
  plugins: [
    {
      name: "wasm-version-module",
      resolveId(id) {
        if (id === "virtual:wasm-version") {
          return "\0virtual:wasm-version";
        }
        return null;
      },
      async load(id) {
        if (id !== "\0virtual:wasm-version") {
          return null;
        }
        const version = await readWasmVersion();
        return `export const wasmBuildVersion = ${JSON.stringify(version)};`;
      },
      configureServer(server) {
        server.watcher.add(stampPath);
        const notifyWasmUpdate = async (file: string): Promise<void> => {
          if (resolve(file) === stampPath) {
            const module = server.moduleGraph.getModuleById("\0virtual:wasm-version");
            if (module !== undefined) {
              server.moduleGraph.invalidateModule(module);
            }
            server.ws.send("box3d-wasm:update", { version: await readWasmVersion() });
          }
        };
        server.watcher.on("add", notifyWasmUpdate);
        server.watcher.on("change", notifyWasmUpdate);
      },
    },
  ],
});
