import { createGenericSample } from "./generic-host";
import type { RenderSpec } from "./generic-host";
import { cardHouseCamera, cardHouseGroundSize, createCardHouseBodies } from "./card-house-scene";

const half = cardHouseGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createCardHouseBodies(),
  camera: cardHouseCamera,
};

export const cardHouseSample = createGenericSample("card-house", "Stacking / Card House", spec, () => new Worker(new URL("./card-house.worker.ts", import.meta.url), { type: "module" }));
