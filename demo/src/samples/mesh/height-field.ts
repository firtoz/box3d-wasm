import * as THREE from "three";
import { WorldCastMode, WorldCastType, type BodyId, type Box3DRuntime, type HeightFieldHandle, type Vec3 } from "box3d-wasm";
import {
  createDebugLine,
  createDebugPoint,
  createWireSphere,
  disposeDebugObject,
  updateDebugLine,
  updateDebugPoint,
} from "../debug-overlay";
import { f32 } from "../f32";
import { disposeObject3D } from "../grid-mesh-visual";
import type { DemoSample } from "../types";
import {
  createHeightFieldScene,
  defaultHeightFieldParams,
  HEIGHT_FIELD_COLUMN_FREQUENCY,
  HEIGHT_FIELD_RAY_ORIGIN,
  HEIGHT_FIELD_RAY_RADIUS,
  HEIGHT_FIELD_RAY_TRANSLATION,
  HEIGHT_FIELD_ROW_FREQUENCY,
  heightFieldCamera,
  heightFieldGroundPositionFrom,
  heightFieldScaleFromAmplitude,
  type HeightFieldParams,
} from "./height-field-scene";

function createWaveHeightFieldVisual(scene: THREE.Scene, params: HeightFieldParams): THREE.Group {
  const scale = heightFieldScaleFromAmplitude(params.amplitude);
  const position = heightFieldGroundPositionFrom(params, scale);
  const { rowCount, columnCount } = params;
  const positions = new Float32Array(rowCount * columnCount * 3);
  let cursor = 0;
  for (let row = 0; row < rowCount; row++) {
    const rowHeight = params.amplitude === 0 ? 0 : f32(Math.sin(f32(2 * Math.PI * HEIGHT_FIELD_ROW_FREQUENCY * row)));
    for (let column = 0; column < columnCount; column++) {
      const columnHeight = params.amplitude === 0 ? 0 : f32(Math.sin(f32(2 * Math.PI * HEIGHT_FIELD_COLUMN_FREQUENCY * column)));
      positions[cursor++] = f32(column * scale[0]);
      positions[cursor++] = f32(scale[1] * rowHeight * columnHeight);
      positions[cursor++] = f32(row * scale[2]);
    }
  }

  const indices: number[] = [];
  for (let row = 0; row < rowCount - 1; row++) {
    for (let column = 0; column < columnCount - 1; column++) {
      if (params.holes) {
        const k = row * (columnCount - 1) + column;
        if (k > 0 && k % 16 === 0) continue;
      }
      const i1 = row * columnCount + column;
      const i2 = i1 + 1;
      const i3 = i2 + columnCount;
      const i4 = i3 - 1;
      indices.push(i1, i2, i3, i3, i4, i1);
    }
  }

  const geom = new THREE.BufferGeometry();
  geom.setAttribute("position", new THREE.BufferAttribute(positions, 3));
  if (indices.length > 0) geom.setIndex(indices);
  geom.computeVertexNormals();

  const fill = new THREE.Mesh(
    geom,
    new THREE.MeshStandardMaterial({
      color: 0x2a2a2a,
      roughness: 0.9,
      side: THREE.DoubleSide,
      flatShading: true,
    }),
  );
  fill.receiveShadow = true;
  const edges = new THREE.LineSegments(
    new THREE.WireframeGeometry(geom),
    new THREE.LineBasicMaterial({ color: 0x64748b }),
  );
  const root = new THREE.Group();
  root.add(fill);
  root.add(edges);
  root.position.set(position[0], position[1], position[2]);
  scene.add(root);
  return root;
}

export const heightFieldSample: DemoSample = {
  id: "mesh/height-field",
  name: "Mesh / Height Field",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -10, 0] });
    const params: HeightFieldParams = defaultHeightFieldParams();
    const rayOrigin: Vec3 = [...HEIGHT_FIELD_RAY_ORIGIN];
    const rayTranslation: Vec3 = [...HEIGHT_FIELD_RAY_TRANSLATION];
    let radius = HEIGHT_FIELD_RAY_RADIUS;
    let heightField: HeightFieldHandle | null = null;
    let groundId: BodyId | null = null;
    let waveVisual: THREE.Group | null = null;
    const axes = new THREE.AxesHelper(0.5);
    axes.position.set(0, 0.1, 0);
    scene.add(axes);
    const rayLine = createDebugLine(scene, 0xeab308);
    const originPoint = createDebugPoint(scene, 0x22c55e, 6);
    const endPoint = createDebugPoint(scene, 0xef4444, 6);
    const hitPoint = createDebugPoint(scene, 0xa855f7, 8);
    const hitNormal = createDebugLine(scene, 0x22c55e);
    const sphere = createWireSphere(scene, radius, 0xf97316);

    const rebuild = () => {
      if (groundId !== null) {
        world.destroyBody(groundId);
        groundId = null;
      }
      if (heightField !== null) {
        world.destroyHeightField(heightField);
        heightField = null;
      }
      const created = createHeightFieldScene(world, runtime, params);
      heightField = created.heightField;
      groundId = created.ground;
      if (waveVisual !== null) disposeObject3D(scene, waveVisual);
      waveVisual = createWaveHeightFieldVisual(scene, params);
    };
    rebuild();

    const updateRay = () => {
      const end: Vec3 = [
        rayOrigin[0] + rayTranslation[0],
        rayOrigin[1] + rayTranslation[1],
        rayOrigin[2] + rayTranslation[2],
      ];
      updateDebugLine(rayLine, rayOrigin, end);
      updateDebugPoint(originPoint, rayOrigin);
      updateDebugPoint(endPoint, end);
      let fraction = 1;
      let hit: { point: Vec3; normal: Vec3 } | null = null;
      if (radius === 0) {
        const result = world.rayCastClosest(rayOrigin, rayTranslation);
        if (result !== null) {
          fraction = result.fraction;
          hit = { point: result.point, normal: result.normal };
        }
      } else {
        const hits = world.worldCast({
          mode: WorldCastMode.Closest,
          type: WorldCastType.Sphere,
          origin: rayOrigin,
          translation: rayTranslation,
          radius,
        });
        const closest = hits[0];
        if (closest !== undefined) {
          fraction = closest.fraction;
          hit = { point: closest.point, normal: closest.normal };
        }
      }
      sphere.visible = radius > 0;
      if (radius > 0) {
        sphere.scale.setScalar(radius / HEIGHT_FIELD_RAY_RADIUS);
        sphere.position.set(
          rayOrigin[0] + fraction * rayTranslation[0],
          rayOrigin[1] + fraction * rayTranslation[1],
          rayOrigin[2] + fraction * rayTranslation[2],
        );
      }
      if (hit !== null) {
        hitPoint.visible = true;
        hitNormal.visible = true;
        updateDebugPoint(hitPoint, hit.point);
        updateDebugLine(hitNormal, hit.point, [
          hit.point[0] + 0.5 * hit.normal[0],
          hit.point[1] + 0.5 * hit.normal[1],
          hit.point[2] + 0.5 * hit.normal[2],
        ]);
      } else {
        hitPoint.visible = false;
        hitNormal.visible = false;
      }
    };

    return {
      world,
      bodies: [],
      camera: heightFieldCamera,
      info: "wave heightfield + ray/sphere cast",
      controls: [
        { key: "columns", label: "columns", type: "range", min: 1, max: 500, step: 1, value: params.columnCount, onChange: (v) => { if (typeof v === "number") { params.columnCount = Math.round(v); rebuild(); } } },
        { key: "rows", label: "rows", type: "range", min: 1, max: 500, step: 1, value: params.rowCount, onChange: (v) => { if (typeof v === "number") { params.rowCount = Math.round(v); rebuild(); } } },
        { key: "amplitude", label: "amplitude", type: "range", min: 0, max: 2, step: 0.01, value: params.amplitude, onChange: (v) => { if (typeof v === "number") { params.amplitude = v; rebuild(); } } },
        { key: "holes", label: "holes", type: "toggle", value: params.holes, onChange: (v) => { if (typeof v === "boolean") { params.holes = v; rebuild(); } } },
        { key: "ray-x", label: "ray x", type: "range", min: -500, max: 500, step: 0.1, value: rayOrigin[0], onChange: (v) => { if (typeof v === "number") rayOrigin[0] = v; } },
        { key: "ray-z", label: "ray z", type: "range", min: -500, max: 500, step: 0.1, value: rayOrigin[2], onChange: (v) => { if (typeof v === "number") rayOrigin[2] = v; } },
        { key: "delta-x", label: "delta x", type: "range", min: -1000, max: 1000, step: 0.1, value: rayTranslation[0], onChange: (v) => { if (typeof v === "number") rayTranslation[0] = v; } },
        { key: "delta-z", label: "delta z", type: "range", min: -1000, max: 1000, step: 0.1, value: rayTranslation[2], onChange: (v) => { if (typeof v === "number") rayTranslation[2] = v; } },
        { key: "radius", label: "radius", type: "range", min: 0, max: 1, step: 0.01, value: radius, onChange: (v) => { if (typeof v === "number") radius = v; } },
      ],
      step() {
        world.step();
        updateRay();
      },
      dispose() {
        if (waveVisual !== null) disposeObject3D(scene, waveVisual);
        if (heightField !== null) world.destroyHeightField(heightField);
        scene.remove(axes);
        axes.dispose();
        disposeDebugObject(scene, rayLine);
        disposeDebugObject(scene, originPoint);
        disposeDebugObject(scene, endPoint);
        disposeDebugObject(scene, hitPoint);
        disposeDebugObject(scene, hitNormal);
        disposeDebugObject(scene, sphere);
        world.destroy();
      },
    };
  },
};
