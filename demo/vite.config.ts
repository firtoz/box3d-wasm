import { defineConfig, type Plugin, type ViteDevServer } from "vite";
import { readFile } from "node:fs/promises";
import { resolve } from "node:path";

const stampPath = resolve(process.cwd(), "public/wasm/.build-stamp");
const buildMetaPath = resolve(process.cwd(), "public/wasm/.build-meta.json");
type DemoWasmVariant = "release" | "profile" | "growable";

function resolveDevWasmVariant(): DemoWasmVariant {
  const raw = process.env.BOX3D_WASM_VARIANT?.trim().toLowerCase();
  if (raw === "profile" || raw === "growable") return raw;
  return "release";
}

async function readBuiltVariants(): Promise<DemoWasmVariant[]> {
  try {
    const meta = JSON.parse(await readFile(buildMetaPath, "utf8")) as { variants?: unknown };
    if (Array.isArray(meta.variants) && meta.variants.length > 0) {
      return meta.variants.filter(
        (variant): variant is DemoWasmVariant =>
          variant === "release" || variant === "profile" || variant === "growable",
      );
    }
  } catch {
    // Fall back until the first wasm build writes metadata.
  }
  return ["release"];
}

async function readWasmVersion(): Promise<string> {
  try {
    return (await readFile(stampPath, "utf8")).trim() || "0";
  } catch {
    return "0";
  }
}

function wasmVersionPlugin(): Plugin {
  return {
    name: "wasm-version-module",
    resolveId(id: string) {
      if (id === "virtual:wasm-version") {
        return "\0virtual:wasm-version";
      }
      return null;
    },
    async load(id: string) {
      if (id !== "\0virtual:wasm-version") {
        return null;
      }
      const version = await readWasmVersion();
      return `export const wasmBuildVersion = ${JSON.stringify(version)};`;
    },
    configureServer(server: ViteDevServer) {
      server.watcher.add(stampPath);
      server.watcher.add(buildMetaPath);
      const notifyWasmUpdate = async (file: string): Promise<void> => {
        const resolved = resolve(file);
        if (resolved === stampPath || resolved === buildMetaPath) {
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
  };
}

export default defineConfig(async () => {
  const builtVariants = await readBuiltVariants();
  const devWasmVariant = resolveDevWasmVariant();
  const defaultWasmVariant = builtVariants.includes(devWasmVariant)
    ? devWasmVariant
    : (builtVariants[0] ?? "release");

  return {
  base: process.env.VITE_BASE_PATH ?? "/",
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
  define: {
    __BOX3D_DEMO_WASM_VARIANT__: JSON.stringify(defaultWasmVariant),
    __BOX3D_DEMO_WASM_VARIANTS__: JSON.stringify(builtVariants),
    // Strip objects-API dispose asserts when BOX3D_OBJECT_ASSERTS=0 (default: enabled).
    __BOX3D_OBJECT_ASSERTS__: JSON.stringify(process.env.BOX3D_OBJECT_ASSERTS !== "0"),
  },
  plugins: [wasmVersionPlugin()],
};
});
