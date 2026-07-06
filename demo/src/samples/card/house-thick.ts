import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createCardHouseThickBodies, cardHouseThickCamera, cardHouseThickGroundSize } from "./house-thick-scene";

const half = cardHouseThickGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createCardHouseThickBodies(),
  camera: cardHouseThickCamera,
};

export const cardHouseThickSample = createGenericSample("card-house-thick", "Stacking / Card House Thick", spec, () => new Worker(new URL("./house-thick.worker.ts", import.meta.url), { type: "module" }));
