import * as THREE from "three";
import { WorldCastMode, WorldCastType, type Box3DRuntime } from "box3d-wasm";
import { disposeObject3D } from "../grid-mesh-visual";
import { f32 } from "../f32";
import type { DemoSample } from "../types";
import {
  BENCHMARK_HEIGHT_FIELD_COLUMN_COUNT,
  BENCHMARK_HEIGHT_FIELD_COLUMN_FREQUENCY,
  BENCHMARK_HEIGHT_FIELD_ROW_COUNT,
  BENCHMARK_HEIGHT_FIELD_ROW_FREQUENCY,
  BENCHMARK_HEIGHT_FIELD_SCALE,
  benchmarkHeightFieldCamera,
  benchmarkHeightFieldGroundPosition,
  createBenchmarkHeightFieldScene,
} from "./height-field-scene";

function createWaveHeightFieldVisual(scene: THREE.Scene): THREE.Group {
  const rowCount = BENCHMARK_HEIGHT_FIELD_ROW_COUNT;
  const columnCount = BENCHMARK_HEIGHT_FIELD_COLUMN_COUNT;
  const scale = BENCHMARK_HEIGHT_FIELD_SCALE;
  const position = benchmarkHeightFieldGroundPosition();
  const positions = new Float32Array(rowCount * columnCount * 3);
  let cursor = 0;
  for (let row = 0; row < rowCount; row++) {
    const rowHeight = f32(Math.sin(f32(2 * Math.PI * BENCHMARK_HEIGHT_FIELD_ROW_FREQUENCY * row)));
    for (let column = 0; column < columnCount; column++) {
      const columnHeight = f32(Math.sin(f32(2 * Math.PI * BENCHMARK_HEIGHT_FIELD_COLUMN_FREQUENCY * column)));
      positions[cursor++] = f32(column * scale[0]);
      positions[cursor++] = f32(scale[1] * rowHeight * columnHeight);
      positions[cursor++] = f32(row * scale[2]);
    }
  }
  const indices: number[] = [];
  for (let row = 0; row < rowCount - 1; row++) {
    for (let column = 0; column < columnCount - 1; column++) {
      const k = row * (columnCount - 1) + column;
      if (k > 0 && k % 16 === 0) continue;
      const i1 = row * columnCount + column;
      const i2 = i1 + 1;
      const i3 = i2 + columnCount;
      const i4 = i3 - 1;
      indices.push(i1, i2, i3, i3, i4, i1);
    }
  }
  const geom = new THREE.BufferGeometry();
  geom.setAttribute("position", new THREE.BufferAttribute(positions, 3));
  geom.setIndex(indices);
  geom.computeVertexNormals();
  const fill = new THREE.Mesh(
    geom,
    new THREE.MeshStandardMaterial({ color: 0x2a2a2a, roughness: 0.9, side: THREE.DoubleSide, flatShading: true }),
  );
  fill.receiveShadow = true;
  const root = new THREE.Group();
  root.add(fill);
  root.add(new THREE.LineSegments(new THREE.WireframeGeometry(geom), new THREE.LineBasicMaterial({ color: 0x64748b })));
  root.position.set(position[0], position[1], position[2]);
  scene.add(root);
  return root;
}

export const benchmarkHeightFieldSample: DemoSample = {
  id: "benchmark/height-field",
  name: "Benchmark / Height Field",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -10, 0] });
    const { ground, heightField } = createBenchmarkHeightFieldScene(world);
    void ground;
    let radius = 0.1;
    let info = "";
    let frame = 0;
    let dirty = true;
    const waveVisual = createWaveHeightFieldVisual(scene);
    const axes = new THREE.AxesHelper(0.4);
    scene.add(axes);
    const translation: [number, number, number] = [80000, -80000, 8];
    const spanX = 0.94 * 0.5 * BENCHMARK_HEIGHT_FIELD_COLUMN_COUNT;
    const spanZ = 0.96 * 0.5 * BENCHMARK_HEIGHT_FIELD_ROW_COUNT;
    const delta = 0.4;

    return {
      world,
      bodies: [],
      camera: benchmarkHeightFieldCamera,
      getInfo: () => info,
      controls: [
        { key: "radius", label: "Radius", type: "range", min: 0, max: 1, step: 0.1, value: radius, onChange: (v) => { if (typeof v === "number") { radius = v; dirty = true; } } },
      ],
      step() {
        world.step();
        frame += 1;
        if (!dirty && frame % 30 !== 0) return;
        dirty = false;
        let castCount = 0;
        let hitCount = 0;
        for (let x = -spanX; x <= spanX; x += delta) {
          for (let z = -spanZ; z <= spanZ; z += delta) {
            const origin: [number, number, number] = [x, 2, z];
            castCount += 1;
            if (radius === 0) {
              if (world.rayCastClosest(origin, translation) !== null) hitCount += 1;
            } else {
              const hits = world.worldCast({
                mode: WorldCastMode.Closest,
                type: WorldCastType.Sphere,
                origin,
                translation,
                radius,
              });
              if (hits.length > 0) hitCount += 1;
            }
          }
        }
        info = `count = ${castCount}, hit count = ${hitCount}`;
      },
      dispose() {
        disposeObject3D(scene, waveVisual);
        world.destroyHeightField(heightField);
        scene.remove(axes);
        axes.dispose();
        world.destroy();
      },
    };
  },
};
