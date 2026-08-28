import * as THREE from "three";
import { B3_AXIS_X, B3_AXIS_Y, B3_PI, type Box3DRuntime, type Vec3 } from "box3d-wasm";
import {
  createDebugLine,
  createDebugPoint,
  disposeDebugObject,
  updateDebugLine,
  updateDebugPoint,
} from "../debug-overlay";
import { f32 } from "../f32";
import { createWaveMeshVisual, disposeObject3D } from "../grid-mesh-visual";
import { meshFor } from "../generic-host";
import type { DemoSample } from "../types";
import {
  buildLongRayCastScene,
  LONG_RAY_CAST_AIM_HEIGHT,
  LONG_RAY_CAST_SHAPE_COUNT,
  longRayCastBodies,
  longRayCastCamera,
  longRayCastHeightFieldVisual,
  longRayCastTargetX,
  longRayCastWaveMeshVisual,
} from "./long-ray-cast-scene";

const TRAIL_COUNT = 180;
const LABELS = ["sphere", "capsule", "hull", "mesh", "hf"];

function createWaveHeightFieldVisual(scene: THREE.Scene): THREE.Group {
  const { rowCount, columnCount, scale, rowFrequency, columnFrequency, position } = longRayCastHeightFieldVisual;
  const positions = new Float32Array(rowCount * columnCount * 3);
  let cursor = 0;
  for (let row = 0; row < rowCount; row++) {
    const rowHeight = f32(Math.sin(f32(2 * Math.PI * rowFrequency * row)));
    for (let column = 0; column < columnCount; column++) {
      const columnHeight = f32(Math.sin(f32(2 * Math.PI * columnFrequency * column)));
      positions[cursor++] = f32(column * scale[0]);
      positions[cursor++] = f32(scale[1] * rowHeight * columnHeight);
      positions[cursor++] = f32(row * scale[2]);
    }
  }
  const indices: number[] = [];
  for (let row = 0; row < rowCount - 1; row++) {
    for (let column = 0; column < columnCount - 1; column++) {
      const i1 = row * columnCount + column;
      indices.push(i1, i1 + 1, i1 + 1 + columnCount, i1 + 1 + columnCount, i1 + columnCount, i1);
    }
  }
  const geom = new THREE.BufferGeometry();
  geom.setAttribute("position", new THREE.BufferAttribute(positions, 3));
  geom.setIndex(indices);
  geom.computeVertexNormals();
  const fill = new THREE.Mesh(
    geom,
    new THREE.MeshStandardMaterial({ color: 0x2a2a2a, roughness: 0.9, side: THREE.DoubleSide, flatShading: true }),
  );
  fill.receiveShadow = true;
  const root = new THREE.Group();
  root.add(fill);
  root.add(new THREE.LineSegments(new THREE.WireframeGeometry(geom), new THREE.LineBasicMaterial({ color: 0x64748b })));
  root.position.set(position[0], position[1], position[2]);
  scene.add(root);
  return root;
}

export const longRayCastSample: DemoSample = {
  id: "collision/long-ray-cast",
  name: "Collision / Long Ray Cast",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, 0, 0] });
    const { resources } = buildLongRayCastScene(world, runtime);
    const visuals: THREE.Object3D[] = [];
    for (let i = 0; i < 3; i++) {
      const mesh = meshFor(longRayCastBodies[i]!);
      scene.add(mesh);
      visuals.push(mesh);
    }
    const waveMesh = createWaveMeshVisual(scene, { ...longRayCastWaveMeshVisual, position: [...longRayCastWaveMeshVisual.position] });
    const heightFieldVisual = createWaveHeightFieldVisual(scene);
    const groundGrid = new THREE.GridHelper(40, 40, 0x4b5563, 0x4b5563);
    scene.add(groundGrid);
    let rayLengthKilometers = 1;
    let coneAngle = 5;
    let phase = 0;
    const failRate = [0, 0, 0, 0, 0];
    const trailNext = [0, 0, 0, 0, 0];
    const trailCount = [0, 0, 0, 0, 0];
    const trails: Vec3[][] = Array.from({ length: LONG_RAY_CAST_SHAPE_COUNT }, () => []);
    const hitPoints = Array.from({ length: LONG_RAY_CAST_SHAPE_COUNT }, () => createDebugPoint(scene, 0x22c55e, 8));
    const missPoints = Array.from({ length: LONG_RAY_CAST_SHAPE_COUNT }, () => createDebugPoint(scene, 0xef4444, 14));
    const aquaLines = Array.from({ length: LONG_RAY_CAST_SHAPE_COUNT }, () => createDebugLine(scene, 0x22d3ee));
    const normalLines = Array.from({ length: LONG_RAY_CAST_SHAPE_COUNT }, () => createDebugLine(scene, 0xeab308));
    const missLines = Array.from({ length: LONG_RAY_CAST_SHAPE_COUNT }, () => createDebugLine(scene, 0xef4444));
    const trailMeshes = Array.from({ length: LONG_RAY_CAST_SHAPE_COUNT }, () => {
      const geometry = new THREE.BufferGeometry();
      geometry.setAttribute("position", new THREE.BufferAttribute(new Float32Array(TRAIL_COUNT * 3), 3));
      geometry.setAttribute("color", new THREE.BufferAttribute(new Float32Array(TRAIL_COUNT * 3), 3));
      geometry.setDrawRange(0, 0);
      const points = new THREE.Points(
        geometry,
        new THREE.PointsMaterial({ size: 4, vertexColors: true, sizeAttenuation: false, toneMapped: false }),
      );
      scene.add(points);
      return points;
    });
    let info = "";

    const castAlong = (aim: Vec3, coneDir: Vec3, distance: number, reach: number) => {
      const origin: Vec3 = [
        aim[0] + distance * coneDir[0],
        aim[1] + distance * coneDir[1],
        aim[2] + distance * coneDir[2],
      ];
      const translation: Vec3 = [
        -(distance + reach) * coneDir[0],
        -(distance + reach) * coneDir[1],
        -(distance + reach) * coneDir[2],
      ];
      return world.rayCastClosest(origin, translation);
    };

    return {
      world,
      bodies: [],
      camera: longRayCastCamera,
      getInfo: () => info,
      controls: [
        { key: "ray-length", label: "Ray Length", type: "range", min: 1, max: 10000, step: 1, value: 1, onChange: (v) => { if (typeof v === "number") rayLengthKilometers = v; } },
        { key: "cone-angle", label: "Cone Angle", type: "range", min: 0, max: 12, step: 0.1, value: 5, onChange: (v) => { if (typeof v === "number") coneAngle = v; } },
      ],
      step() {
        world.step();
        phase += 2 * B3_PI / TRAIL_COUNT;
        if (phase > 2 * B3_PI) phase -= 2 * B3_PI;
        const halfAngle = coneAngle * (B3_PI / 180);
        const qx = runtime.makeQuatFromAxisAngle(B3_AXIS_X, halfAngle);
        const tilted = runtime.rotateVector(qx, [0, 1, 0]);
        const qy = runtime.makeQuatFromAxisAngle(B3_AXIS_Y, phase);
        const coneDir = runtime.rotateVector(qy, tilted);
        const farDistance = 1000 * rayLengthKilometers;
        for (let i = 0; i < LONG_RAY_CAST_SHAPE_COUNT; i++) {
          const aim: Vec3 = [longRayCastTargetX(i), LONG_RAY_CAST_AIM_HEIGHT, 0];
          const truth = castAlong(aim, coneDir, 50, 5);
          const cast = castAlong(aim, coneDir, farDistance, 5);
          let fail = 0;
          hitPoints[i]!.visible = false;
          missPoints[i]!.visible = false;
          aquaLines[i]!.visible = false;
          normalLines[i]!.visible = false;
          missLines[i]!.visible = false;
          if (cast !== null) {
            const error = truth !== null
              ? Math.hypot(cast.point[0] - truth.point[0], cast.point[1] - truth.point[1], cast.point[2] - truth.point[2])
              : 0;
            hitPoints[i]!.visible = true;
            (hitPoints[i]!.material as THREE.PointsMaterial).color.setHex(error < 0.05 ? 0x22c55e : 0xf97316);
            updateDebugPoint(hitPoints[i]!, cast.point);
            aquaLines[i]!.visible = true;
            updateDebugLine(aquaLines[i]!, [
              cast.point[0] + 3 * coneDir[0],
              cast.point[1] + 3 * coneDir[1],
              cast.point[2] + 3 * coneDir[2],
            ], cast.point);
            normalLines[i]!.visible = true;
            updateDebugLine(normalLines[i]!, cast.point, [
              cast.point[0] + 1.5 * cast.normal[0],
              cast.point[1] + 1.5 * cast.normal[1],
              cast.point[2] + 1.5 * cast.normal[2],
            ]);
            const next = trailNext[i]!;
            const trail = trails[i]!;
            trail[next] = cast.point;
            trailNext[i] = (next + 1) % TRAIL_COUNT;
            const stored = trailCount[i] ?? 0;
            if (stored < TRAIL_COUNT) trailCount[i] = stored + 1;
          } else if (truth !== null) {
            fail = 1;
            missPoints[i]!.visible = true;
            updateDebugPoint(missPoints[i]!, truth.point);
            missLines[i]!.visible = true;
            updateDebugLine(missLines[i]!, [
              truth.point[0] + 2 * coneDir[0],
              truth.point[1] + 2 * coneDir[1],
              truth.point[2] + 2 * coneDir[2],
            ], [
              truth.point[0] - 2 * coneDir[0],
              truth.point[1] - 2 * coneDir[1],
              truth.point[2] - 2 * coneDir[2],
            ]);
          } else {
            missLines[i]!.visible = true;
            updateDebugLine(missLines[i]!, [
              aim[0] + 2 * coneDir[0],
              aim[1] + 2 * coneDir[1],
              aim[2] + 2 * coneDir[2],
            ], [
              aim[0] - 4 * coneDir[0],
              aim[1] - 4 * coneDir[1],
              aim[2] - 4 * coneDir[2],
            ]);
          }
          failRate[i] = 0.95 * (failRate[i] ?? 0) + 0.05 * fail;
          const count = trailCount[i] ?? 0;
          const trailMesh = trailMeshes[i]!;
          const positions = trailMesh.geometry.getAttribute("position") as THREE.BufferAttribute;
          const colors = trailMesh.geometry.getAttribute("color") as THREE.BufferAttribute;
          const start = ((trailNext[i] ?? 0) - count + TRAIL_COUNT) % TRAIL_COUNT;
          const trail = trails[i]!;
          for (let j = 0; j < count; j++) {
            const point = trail[(start + j) % TRAIL_COUNT];
            if (point === undefined) continue;
            const alpha = (j + 1) / count;
            positions.setXYZ(j, point[0], point[1], point[2]);
            colors.setXYZ(j, 0.13 * alpha, 0.77 * alpha, 0.37 * alpha);
          }
          positions.needsUpdate = true;
          colors.needsUpdate = true;
          trailMesh.geometry.setDrawRange(0, count);
          trailMesh.geometry.computeBoundingSphere();
        }
        info = `Origin ${rayLengthKilometers.toFixed(0)} km. Failures: ${LABELS.map((name, i) => `${name} ${(100 * (failRate[i] ?? 0)).toFixed(0)}%`).join("  ")}`;
      },
      dispose() {
        for (const visual of visuals) {
          scene.remove(visual);
          visual.traverse((child) => {
            const mesh = child as THREE.Mesh;
            mesh.geometry?.dispose();
            const material = mesh.material;
            if (material === undefined) return;
            if (Array.isArray(material)) material.forEach((entry) => entry.dispose());
            else material.dispose();
          });
        }
        disposeObject3D(scene, waveMesh);
        disposeObject3D(scene, heightFieldVisual);
        scene.remove(groundGrid);
        groundGrid.geometry.dispose();
        const gridMat = groundGrid.material;
        if (Array.isArray(gridMat)) gridMat.forEach((entry) => entry.dispose());
        else gridMat.dispose();
        for (const object of [...hitPoints, ...missPoints, ...aquaLines, ...normalLines, ...missLines, ...trailMeshes]) {
          disposeDebugObject(scene, object);
        }
        runtime.destroyHull(resources.hull);
        world.destroyMesh(resources.mesh);
        world.destroyHeightField(resources.heightField);
        world.destroy();
      },
    };
  },
};
