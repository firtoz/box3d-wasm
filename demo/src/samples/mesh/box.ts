import * as THREE from "three";
import { BodyType, B3_AXIS_Y, B3_PI, quatFromAxisAngle } from "box3d-wasm";
import { createGenericSample, meshFor } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import type { DemoBody, DemoSample } from "../types";
import { disposeObject3D } from "../grid-mesh-visual";
import {
  MESH_BOX_DEFAULT_SCALE,
  MESH_BOX_EXTENT,
  MESH_BOX_SCALE_MAX,
  MESH_BOX_SCALE_MIN,
  type MeshBoxShapeType,
  meshBoxBodyFor,
  meshBoxBodies,
  meshBoxCamera,
  meshBoxGroundSize,
  MESH_BOX_PLATFORM_POS,
} from "./box-scene";

const half = meshBoxGroundSize();

function replaceDynamicMesh(scene: THREE.Scene, bodies: DemoBody[], shapeType: MeshBoxShapeType): void {
  // bodies[0] is the static mesh platform; dynamic is bodies[1]
  const body = bodies[1];
  if (body === undefined) return;
  scene.remove(body.mesh);
  body.mesh.geometry.dispose();
  const mat = body.mesh.material;
  if (Array.isArray(mat)) mat.forEach((m) => m.dispose());
  else mat.dispose();
  const next = meshFor(meshBoxBodyFor(shapeType));
  scene.add(next);
  body.mesh = next;
}

function createBoxPlatformVisual(scene: THREE.Scene, scale: [number, number, number]): THREE.Group {
  const sizeX = 2 * MESH_BOX_EXTENT[0];
  const sizeY = 2 * MESH_BOX_EXTENT[1];
  const sizeZ = 2 * MESH_BOX_EXTENT[2];
  // Mesh local center is (0,1,0); body at (0,-1,0) → world center at (0,0,0).
  const geom = new THREE.BoxGeometry(sizeX, sizeY, sizeZ);
  const fill = new THREE.Mesh(
    geom,
    new THREE.MeshStandardMaterial({
      color: 0x3f3f46,
      roughness: 0.9,
      metalness: 0,
      flatShading: true,
    }),
  );
  fill.receiveShadow = true;
  fill.castShadow = true;

  const edges = new THREE.LineSegments(
    new THREE.EdgesGeometry(geom),
    new THREE.LineBasicMaterial({ color: 0x9ca3af }),
  );

  const root = new THREE.Group();
  root.add(fill);
  root.add(edges);
  root.position.set(MESH_BOX_PLATFORM_POS[0], MESH_BOX_PLATFORM_POS[1] + 1, MESH_BOX_PLATFORM_POS[2]);
  const q = quatFromAxisAngle(B3_AXIS_Y, 0.25 * B3_PI);
  root.quaternion.set(q[0], q[1], q[2], q[3]);
  root.scale.set(scale[0], 1, scale[2]);
  scene.add(root);
  return root;
}

export const meshBoxSample: DemoSample = {
  id: "mesh/box",
  name: "Mesh / Box",
  create(runtime, scene, solverParams) {
    const holder: { bodies: DemoBody[] } = { bodies: [] };
    let boxVisual: THREE.Group | null = null;
    let axes: THREE.AxesHelper | null = null;

    const platformRotation = quatFromAxisAngle(B3_AXIS_Y, 0.25 * B3_PI);
    const spec: RenderSpec = {
      groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
      bodies: [
        {
          kind: "box",
          size: [2, 2, 2],
          position: [MESH_BOX_PLATFORM_POS[0], MESH_BOX_PLATFORM_POS[1] + 1, MESH_BOX_PLATFORM_POS[2]],
          rotation: platformRotation,
          color: 0x52525b,
          type: BodyType.Static,
        },
        ...meshBoxBodies,
      ],
      camera: meshBoxCamera,
      info: "box mesh platform on ground — Scale X/Z + Sphere/Capsule/Box/Cylinder",
      overlay: (overlayScene) => {
        boxVisual = createBoxPlatformVisual(overlayScene, [
          MESH_BOX_DEFAULT_SCALE[0],
          MESH_BOX_DEFAULT_SCALE[1],
          MESH_BOX_DEFAULT_SCALE[2],
        ]);
        axes = new THREE.AxesHelper(1);
        axes.position.set(0, 0.01, 0);
        overlayScene.add(axes);
        return {
          update() {},
          dispose() {
            if (boxVisual !== null) disposeObject3D(overlayScene, boxVisual);
            boxVisual = null;
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
          min: MESH_BOX_SCALE_MIN,
          max: MESH_BOX_SCALE_MAX,
          step: 0.1,
          value: MESH_BOX_DEFAULT_SCALE[0],
          onHostChange: (value) => {
            if (boxVisual !== null) boxVisual.scale.x = value;
          },
        },
        {
          type: "range",
          label: "Scale Z",
          message: { type: "set-mesh-scale-z" },
          min: MESH_BOX_SCALE_MIN,
          max: MESH_BOX_SCALE_MAX,
          step: 0.1,
          value: MESH_BOX_DEFAULT_SCALE[2],
          onHostChange: (value) => {
            if (boxVisual !== null) boxVisual.scale.z = value;
          },
        },
      ],
    };

    const instance = createGenericSample(
      "mesh/box",
      "Mesh / Box",
      spec,
      () => new Worker(new URL("./box.worker.ts", import.meta.url), { type: "module" }),
    ).create(runtime, scene, solverParams);

    holder.bodies = instance.bodies;
    const platformMesh = instance.bodies[0]?.mesh;
    if (platformMesh !== undefined) platformMesh.visible = false;
    return instance;
  },
};
