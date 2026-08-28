import * as THREE from "three";
import type { DemoSample } from "../types";
import { getWasmBaseUrl } from "../shared";
import { treeBenchmarkCamera } from "./benchmark-scene";

const FILES = [
  { name: "bounds01.txt", scale: 1 },
  { name: "bounds02.txt", scale: 1 },
  { name: "bounds03.txt", scale: 0.01 },
];

function parseBounds(text: string, scale: number): { min: THREE.Vector3; max: THREE.Vector3 }[] {
  const boxes: { min: THREE.Vector3; max: THREE.Vector3 }[] = [];
  for (const raw of text.split(/\r?\n/)) {
    const line = raw.trim();
    if (line.length === 0 || line.startsWith("#")) continue;
    const p = line.split(/\s+/).map(Number);
    if (p.length < 6 || p.some((n) => Number.isNaN(n))) continue;
    boxes.push({
      min: new THREE.Vector3(scale * p[0]!, scale * p[1]!, scale * p[2]!),
      max: new THREE.Vector3(scale * p[3]!, scale * p[4]!, scale * p[5]!),
    });
  }
  return boxes;
}

function makeWireBoxes(boxes: { min: THREE.Vector3; max: THREE.Vector3 }[]): THREE.LineSegments {
  const positions: number[] = [];
  const limit = Math.min(boxes.length, 4000);
  for (let i = 0; i < limit; i++) {
    const b = boxes[i]!;
    const x0 = b.min.x, y0 = b.min.y, z0 = b.min.z;
    const x1 = b.max.x, y1 = b.max.y, z1 = b.max.z;
    const c = [
      [x0, y0, z0], [x1, y0, z0], [x1, y1, z0], [x0, y1, z0],
      [x0, y0, z1], [x1, y0, z1], [x1, y1, z1], [x0, y1, z1],
    ];
    const e = [0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7];
    for (const idx of e) {
      const p = c[idx]!;
      positions.push(p[0]!, p[1]!, p[2]!);
    }
  }
  const geometry = new THREE.BufferGeometry();
  geometry.setAttribute("position", new THREE.Float32BufferAttribute(positions, 3));
  return new THREE.LineSegments(geometry, new THREE.LineBasicMaterial({ color: 0x60a5fa }));
}

export const treeBenchmarkSample: DemoSample = {
  id: "tree/benchmark",
  name: "Tree / Benchmark",
  create(runtime, scene) {
    const world = runtime.createWorld({ gravity: [0, -10, 0] });
    let fileIndex = 0;
    let lines: THREE.LineSegments | null = null;
    let info = "loading…";
    const load = async () => {
      if (lines !== null) {
        scene.remove(lines);
        lines.geometry.dispose();
        (lines.material as THREE.Material).dispose();
        lines = null;
      }
      const file = FILES[fileIndex]!;
      const response = await fetch(`${getWasmBaseUrl()}trees/${file.name}`);
      const boxes = parseBounds(await response.text(), file.scale);
      lines = makeWireBoxes(boxes);
      scene.add(lines);
      info = `${file.name} | ${boxes.length} AABBs (drawing ${Math.min(boxes.length, 4000)})`;
    };
    void load();
    return {
      world,
      bodies: [],
      camera: treeBenchmarkCamera,
      getInfo: () => info,
      controls: [
        { key: "file", label: "file", type: "range", min: 0, max: 2, step: 1, value: 0, onChange: (v) => { if (typeof v === "number") { fileIndex = v | 0; void load(); } } },
      ],
      step() {},
      dispose() {
        if (lines !== null) {
          scene.remove(lines);
          lines.geometry.dispose();
          (lines.material as THREE.Material).dispose();
        }
        world.destroy();
        void runtime;
      },
    };
  },
};
