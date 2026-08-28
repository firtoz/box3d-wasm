import * as THREE from "three";
import { createGenericSample, meshFor } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import type { DemoBody, DemoSample } from "../types";
import { createWaveMeshVisual, disposeObject3D } from "../grid-mesh-visual";
import {
  CONTINUOUS_MESH_DROP_SHAPE_TYPES,
  continuousMeshDropBodyFor,
  continuousMeshDropCamera,
  continuousMeshDropGroundSize,
  continuousMeshDropWaveParams,
  createContinuousMeshDropBodies,
  type ContinuousMeshDropShapeType,
} from "./mesh-drop-scene";

const half = continuousMeshDropGroundSize();

function replaceDropMeshes(scene: THREE.Scene, bodies: DemoBody[], shapeType: ContinuousMeshDropShapeType): void {
  for (let i = 1; i < bodies.length; i++) {
    const body = bodies[i];
    if (body === undefined) continue;
    const position: [number, number, number] = [body.mesh.position.x, body.mesh.position.y, body.mesh.position.z];
    scene.remove(body.mesh);
    body.mesh.geometry.dispose();
    const material = body.mesh.material;
    if (Array.isArray(material)) material.forEach((entry) => entry.dispose());
    else material.dispose();
    const next = meshFor(continuousMeshDropBodyFor(shapeType, position));
    scene.add(next);
    body.mesh = next;
  }
}

export const continuousMeshDropSample: DemoSample = {
  id: "continuous/mesh-drop",
  name: "Continuous / Mesh Drop",
  create(runtime, scene, solverParams) {
    const holder: { bodies: DemoBody[] } = { bodies: [] };
    let waveVisual: THREE.Group | null = null;
    let overlayScene: THREE.Scene | null = null;
    let shapeType: ContinuousMeshDropShapeType = "box";
    let amplitude = 0.5;
    let collide = true;

    function rebuildWave(nextAmplitude: number): void {
      if (overlayScene === null) return;
      if (waveVisual !== null) disposeObject3D(overlayScene, waveVisual);
      waveVisual = createWaveMeshVisual(overlayScene, continuousMeshDropWaveParams(nextAmplitude));
    }

    const spec: RenderSpec = {
      groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
      groundKind: "none",
      bodies: createContinuousMeshDropBodies(),
      camera: continuousMeshDropCamera,
      info: "32×32 drop on wave mesh + walls",
      getInfo: () => `type ${shapeType}  amp ${amplitude.toFixed(2)}  collide ${collide ? "on" : "off"}`,
      overlay: (overlay) => {
        overlayScene = overlay;
        rebuildWave(amplitude);
        return {
          update() {},
          dispose() {
            if (waveVisual !== null && overlayScene !== null) disposeObject3D(overlayScene, waveVisual);
            waveVisual = null;
            overlayScene = null;
          },
        };
      },
      controls: [
        ...CONTINUOUS_MESH_DROP_SHAPE_TYPES.map((type) => ({
          type: "button" as const,
          label: type,
          message: { type: "set-shape-type", shape: type },
          onHostClick: () => {
            shapeType = type;
            replaceDropMeshes(scene, holder.bodies, type);
          },
        })),
        {
          type: "range",
          label: "Amplitude",
          message: { type: "set-amplitude" },
          min: 0,
          max: 1,
          step: 0.01,
          value: 0.5,
          onHostChange: (value) => {
            amplitude = value;
            rebuildWave(value);
          },
        },
        {
          type: "toggle",
          label: "Collide",
          message: { type: "set-collide" },
          value: true,
          onHostChange: (value) => {
            collide = value;
          },
        },
        {
          type: "button",
          label: "Generate",
          message: { type: "generate" },
        },
      ],
    };

    const instance = createGenericSample(
      "continuous/mesh-drop",
      "Continuous / Mesh Drop",
      spec,
      () => new Worker(new URL("./mesh-drop.worker.ts", import.meta.url), { type: "module" }),
    ).create(runtime, scene, solverParams);
    holder.bodies = instance.bodies;
    return instance;
  },
};
