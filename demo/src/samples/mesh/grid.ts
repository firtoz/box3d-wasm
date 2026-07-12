import * as THREE from "three";
import { createGenericSample, meshFor } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import type { DemoBody, DemoSample } from "../types";
import { createGridMeshVisual, disposeObject3D } from "../grid-mesh-visual";
import {
  MESH_GRID_CELL_COUNT,
  MESH_GRID_CELL_WIDTH,
  MESH_GRID_DEFAULT_SCALE,
  MESH_GRID_SCALE_MAX,
  MESH_GRID_SCALE_MIN,
  type MeshGridShapeType,
  meshGridBodyFor,
  meshGridBodies,
  meshGridCamera,
  meshGridGroundSize,
} from "./grid-scene";

const half = meshGridGroundSize();

function replaceDynamicMesh(scene: THREE.Scene, bodies: DemoBody[], shapeType: MeshGridShapeType): void {
  const body = bodies[0];
  if (body === undefined) return;
  scene.remove(body.mesh);
  body.mesh.geometry.dispose();
  const mat = body.mesh.material;
  if (Array.isArray(mat)) mat.forEach((m) => m.dispose());
  else mat.dispose();
  const next = meshFor(meshGridBodyFor(shapeType));
  scene.add(next);
  body.mesh = next;
}

export const meshGridSample: DemoSample = {
  id: "mesh/grid",
  name: "Mesh / Grid",
  create(runtime, scene, solverParams) {
    const holder: { bodies: DemoBody[] } = { bodies: [] };
    let gridVisual: THREE.Group | null = null;
    let axes: THREE.AxesHelper | null = null;

    const spec: RenderSpec = {
      groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
      groundKind: "none",
      bodies: meshGridBodies,
      camera: meshGridCamera,
      info: "grid mesh floor (Scale X/Z + shapes)",
      overlay: (overlayScene) => {
        gridVisual = createGridMeshVisual(overlayScene, {
          cellCount: MESH_GRID_CELL_COUNT,
          cellWidth: MESH_GRID_CELL_WIDTH,
          scale: [MESH_GRID_DEFAULT_SCALE[0], 1, MESH_GRID_DEFAULT_SCALE[2]],
          position: [0, 0.01, 0],
        });
        // Upstream DrawAxes at y=0.01, length 1.
        axes = new THREE.AxesHelper(1);
        axes.position.set(0, 0.01, 0);
        overlayScene.add(axes);
        return {
          update() {},
          dispose() {
            if (gridVisual !== null) disposeObject3D(overlayScene, gridVisual);
            gridVisual = null;
            if (axes !== null) {
              overlayScene.remove(axes);
              axes.dispose();
              axes = null;
            }
          },
        };
      },
      controls: [
        {
          type: "button",
          label: "Sphere",
          message: { type: "set-shape", shape: "sphere" },
          onHostClick: () => replaceDynamicMesh(scene, holder.bodies, "sphere"),
        },
        {
          type: "button",
          label: "Capsule",
          message: { type: "set-shape", shape: "capsule" },
          onHostClick: () => replaceDynamicMesh(scene, holder.bodies, "capsule"),
        },
        {
          type: "button",
          label: "Box",
          message: { type: "set-shape", shape: "box" },
          onHostClick: () => replaceDynamicMesh(scene, holder.bodies, "box"),
        },
        {
          type: "button",
          label: "Cylinder",
          message: { type: "set-shape", shape: "cylinder" },
          onHostClick: () => replaceDynamicMesh(scene, holder.bodies, "cylinder"),
        },
        {
          type: "range",
          label: "Scale X",
          message: { type: "set-mesh-scale-x" },
          min: MESH_GRID_SCALE_MIN,
          max: MESH_GRID_SCALE_MAX,
          step: 0.1,
          value: MESH_GRID_DEFAULT_SCALE[0],
          onHostChange: (value) => {
            if (gridVisual !== null) gridVisual.scale.x = value;
          },
        },
        {
          type: "range",
          label: "Scale Z",
          message: { type: "set-mesh-scale-z" },
          min: MESH_GRID_SCALE_MIN,
          max: MESH_GRID_SCALE_MAX,
          step: 0.1,
          value: MESH_GRID_DEFAULT_SCALE[2],
          onHostChange: (value) => {
            if (gridVisual !== null) gridVisual.scale.z = value;
          },
        },
      ],
    };

    const instance = createGenericSample(
      "mesh/grid",
      "Mesh / Grid",
      spec,
      () => new Worker(new URL("./grid.worker.ts", import.meta.url), { type: "module" }),
    ).create(runtime, scene, solverParams);

    holder.bodies = instance.bodies;
    return instance;
  },
};
