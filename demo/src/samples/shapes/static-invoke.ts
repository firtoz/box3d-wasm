import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { staticInvokeBodies, staticInvokeCamera, staticInvokeGroundSize } from "./static-invoke-scene";

const half = staticInvokeGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: staticInvokeBodies,
  camera: staticInvokeCamera,
  controls: [
    // Upstream radios: Invoke / Passive (default Passive).
    { type: "button", label: "Invoke", message: { type: "set-invoke", value: true } },
    { type: "button", label: "Passive", message: { type: "set-invoke", value: false } },
    { type: "button", label: "Create", message: { type: "create-static" } },
    { type: "button", label: "Destroy", message: { type: "destroy-static" } },
  ],
  info: "Create overlapping static sphere; Invoke forces contacts, Passive may leave intersection",
};

export const staticInvokeSample = createGenericSample(
  "shapes/static-invoke",
  "Shapes / Static Invoke",
  spec,
  () => new Worker(new URL("./static-invoke.worker.ts", import.meta.url), { type: "module" }),
);
