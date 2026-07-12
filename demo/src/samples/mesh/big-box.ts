import * as THREE from "three";
import { createGenericSample, meshFor } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import type { DemoBody, DemoSample } from "../types";
import { disposeObject3D } from "../grid-mesh-visual";
import {
  BIG_BOX_DEFAULT_SCALE,
  BIG_BOX_EXTENT,
  BIG_BOX_SCALE_MAX,
  BIG_BOX_SCALE_MIN,
  type BigBoxShapeType,
  bigBoxBodyFor,
  bigBoxBodies,
  bigBoxCamera,
  bigBoxGroundSize,
} from "./big-box-scene";

const half = bigBoxGroundSize();

function replaceDynamicMesh(scene: THREE.Scene, bodies: DemoBody[], shapeType: BigBoxShapeType): void {
  const body = bodies[0];
  if (body === undefined) return;
  scene.remove(body.mesh);
  body.mesh.geometry.dispose();
  const mat = body.mesh.material;
  if (Array.isArray(mat)) mat.forEach((m) => m.dispose());
  else mat.dispose();
  const next = meshFor(bigBoxBodyFor(shapeType));
  scene.add(next);
  body.mesh = next;
}

function createBigBoxVisual(
  scene: THREE.Scene,
  scale: [number, number, number],
): THREE.Group {
  // Top face of `b3CreateBoxMesh({0,-1,0}, {50,1,50})` sits at y=0.
  const sizeX = 2 * BIG_BOX_EXTENT[0];
  const sizeZ = 2 * BIG_BOX_EXTENT[2];
  const geom = new THREE.PlaneGeometry(sizeX, sizeZ, 20, 20);
  geom.rotateX(-Math.PI / 2);

  const fill = new THREE.Mesh(
    geom,
    new THREE.MeshStandardMaterial({
      color: 0x3f3f46,
      roughness: 0.9,
      metalness: 0,
      side: THREE.DoubleSide,
      flatShading: true,
    }),
  );
  fill.receiveShadow = true;

  const edges = new THREE.LineSegments(
    new THREE.WireframeGeometry(geom),
    new THREE.LineBasicMaterial({ color: 0x9ca3af }),
  );

  const root = new THREE.Group();
  root.add(fill);
  root.add(edges);
  root.position.set(0, 0, 0);
  root.scale.set(scale[0], 1, scale[2]);
  root.frustumCulled = false;
  fill.frustumCulled = false;
  edges.frustumCulled = false;
  scene.add(root);
  return root;
}

export const bigBoxSample: DemoSample = {
  id: "mesh/big-box",
  name: "Mesh / Big Box",
  create(runtime, scene, solverParams) {
    const holder: { bodies: DemoBody[] } = { bodies: [] };
    let boxVisual: THREE.Group | null = null;
    let axes: THREE.AxesHelper | null = null;

    const spec: RenderSpec = {
      groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
      groundKind: "none",
      bodies: bigBoxBodies,
      camera: bigBoxCamera,
      info: "box mesh floor — Scale X/Z [-2,2] + Sphere/Capsule/Box/Cylinder (matches upstream)",
      overlay: (overlayScene) => {
        boxVisual = createBigBoxVisual(overlayScene, [
          BIG_BOX_DEFAULT_SCALE[0],
          BIG_BOX_DEFAULT_SCALE[1],
          BIG_BOX_DEFAULT_SCALE[2],
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
          min: BIG_BOX_SCALE_MIN,
          max: BIG_BOX_SCALE_MAX,
          step: 0.1,
          value: BIG_BOX_DEFAULT_SCALE[0],
          onHostChange: (value) => {
            if (boxVisual !== null) boxVisual.scale.x = value;
          },
        },
        {
          type: "range",
          label: "Scale Z",
          message: { type: "set-mesh-scale-z" },
          min: BIG_BOX_SCALE_MIN,
          max: BIG_BOX_SCALE_MAX,
          step: 0.1,
          value: BIG_BOX_DEFAULT_SCALE[2],
          onHostChange: (value) => {
            if (boxVisual !== null) boxVisual.scale.z = value;
          },
        },
      ],
    };

    const instance = createGenericSample(
      "mesh/big-box",
      "Mesh / Big Box",
      spec,
      () => new Worker(new URL("./big-box.worker.ts", import.meta.url), { type: "module" }),
    ).create(runtime, scene, solverParams);

    holder.bodies = instance.bodies;
    return instance;
  },
};
