import type { DemoSample } from "../types";
import { getWasmBaseUrl } from "../shared";
import { parseObjText, transformObjVertices } from "../meshes/parse-obj";
import { meshCreationCamera } from "./creation-benchmark-scene";

const FILES = ["voxel_mesh_01.obj", "voxel_mesh_02.obj", "voxel_mesh_03.obj", "voxel_mesh_04.obj"];

async function loadTemp(name: string): Promise<{ vertices: number[]; indices: number[] } | null> {
  const response = await fetch(`${getWasmBaseUrl()}meshes/${name}`);
  if (!response.ok) return null;
  const parsed = parseObjText(await response.text());
  return { vertices: transformObjVertices(parsed.vertices, 0.01, true), indices: parsed.indices };
}

export const meshCreationBenchmarkSample: DemoSample = {
  id: "mesh/creation-benchmark",
  name: "Mesh / Creation Benchmark",
  create(runtime, scene) {
    const world = runtime.createWorld({ gravity: [0, -10, 0] });
    let temps: ({ vertices: number[]; indices: number[] } | null)[] | null = null;
    let info = "loading…";
    const run = () => {
      if (temps === null) return;
      let best = Number.POSITIVE_INFINITY;
      let triangleCount = 0;
      for (let i = 0; i < 4; i++) {
        const t0 = performance.now();
        for (let j = 0; j < temps.length; j++) {
          const mesh = temps[j];
          if (mesh === null) continue;
          const handle = world.createMesh(mesh.vertices, mesh.indices, {
            useMedianSplit: true,
            identifyEdges: false,
            weldVertices: true,
            weldTolerance: 0.0015,
          });
          if (i === 0) triangleCount += mesh.indices.length / 3;
          world.destroyMesh(handle);
        }
        best = Math.min(best, performance.now() - t0);
      }
      info = `triangle count = ${triangleCount}\ntotal time = ${best.toFixed(4)} ms\ntime per mesh = ${(best / FILES.length).toFixed(4)} ms`;
    };
    void Promise.all(FILES.map((name) => loadTemp(name))).then((loaded) => {
      temps = loaded;
      run();
    });
    return {
      world,
      bodies: [],
      camera: meshCreationCamera,
      getInfo: () => info,
      controls: [],
      step() { run(); },
      dispose() {
        world.destroy();
        void scene;
      },
    };
  },
};
