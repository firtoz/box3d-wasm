import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { spinningBookBodies, spinningBookCamera, spinningBookGroundSize } from "./spinning-book-scene";

const half = spinningBookGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: spinningBookBodies,
  camera: spinningBookCamera,
};

export const spinningBookSample = createGenericSample(
  "bodies/spinning-book", "Bodies / Spinning Book", spec,
  () => new Worker(new URL("./spinning-book.worker.ts", import.meta.url), { type: "module" }),
);
