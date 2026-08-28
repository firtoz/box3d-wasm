import * as THREE from "three";
import type { Box3DRuntime, HullHandle } from "box3d-wasm";
import type { DemoBody, DemoSample } from "../types";
import { cameraFromSetView, disposeBodies, syncBodies } from "../shared";
import { disposeHullMesh, hullMeshFromHandle } from "../geometry/hull-draw";

const HEIGHT = 2;
const RADIUS1 = 0.25;
const RADIUS2 = 1;
const SPHERE_RADIUS = 1;

export const extraConeMassSample: DemoSample = {
  id: "extra/cone-mass",
  name: "Extra / Cone Mass",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -10, 0] });
    let slices = 16;
    let hull: HullHandle | null = null;
    let hullMesh: THREE.Object3D | null = null;
    let edgeLines: THREE.LineSegments | null = null;
    let info = "";
    const sphere = new THREE.Mesh(
      new THREE.SphereGeometry(SPHERE_RADIUS, 24, 16),
      new THREE.MeshBasicMaterial({ color: 0x22d3ee, wireframe: true, transparent: true, opacity: 0.35 }),
    );
    scene.add(sphere);
    const axes = new THREE.AxesHelper(1);
    scene.add(axes);
    const bodies: DemoBody[] = [];

    const rebuild = (sliceCount: number) => {
      disposeHullMesh(scene, hullMesh);
      hullMesh = null;
      if (hull !== null) runtime.destroyHull(hull);
      hull = runtime.createCone(HEIGHT, RADIUS1, RADIUS2, sliceCount);
      hullMesh = hullMeshFromHandle(runtime, hull, 0xf97316);
      scene.add(hullMesh);
      const coneMass = runtime.computeHullMass(hull);
      const sphereMass = runtime.computeSphereMass([0, 0, 0], SPHERE_RADIUS);
      const hullInfo = runtime.getHullInfo(hull);
      if (edgeLines !== null) {
        scene.remove(edgeLines);
        edgeLines.geometry.dispose();
        (edgeLines.material as THREE.Material).dispose();
        edgeLines = null;
      }
      const edges = runtime.getHullEdgeLines(hull);
      const geom = new THREE.BufferGeometry();
      geom.setAttribute("position", new THREE.Float32BufferAttribute(edges, 3));
      edgeLines = new THREE.LineSegments(geom, new THREE.LineBasicMaterial({ color: 0xfde68a }));
      scene.add(edgeLines);
      info = [
        `cone slices=${sliceCount} verts=${hullInfo.vertexCount} faces=${hullInfo.faceCount} edges=${hullInfo.edgeCount} volume=${hullInfo.volume.toPrecision(6)}`,
        `mass=${coneMass.mass.toPrecision(6)} Ixx=${coneMass.ixx.toPrecision(6)} Iyy=${coneMass.iyy.toPrecision(6)} Izz=${coneMass.izz.toPrecision(6)}`,
        `sphere r=${SPHERE_RADIUS} mass=${sphereMass.mass.toPrecision(6)} Ixx=${sphereMass.ixx.toPrecision(6)} Iyy=${sphereMass.iyy.toPrecision(6)} Izz=${sphereMass.izz.toPrecision(6)}`,
      ].join("\n");
    };
    rebuild(slices);

    return {
      world,
      bodies,
      camera: cameraFromSetView(0, 15, 6, [0, 1, 0]),
      getInfo: () => info,
      controls: [
        {
          key: "slices",
          label: "slices",
          type: "range",
          min: 4,
          max: 32,
          step: 1,
          value: slices,
          onChange: (v) => {
            if (typeof v === "number") {
              slices = Math.round(v);
              rebuild(slices);
            }
          },
        },
      ],
      step(dt, subSteps) {
        world.step(dt ?? 1 / 60, subSteps ?? 4);
        syncBodies(world, bodies);
      },
      dispose() {
        disposeBodies(scene, bodies);
        disposeHullMesh(scene, hullMesh);
        if (edgeLines !== null) {
          scene.remove(edgeLines);
          edgeLines.geometry.dispose();
          (edgeLines.material as THREE.Material).dispose();
        }
        if (hull !== null) runtime.destroyHull(hull);
        scene.remove(sphere);
        sphere.geometry.dispose();
        (sphere.material as THREE.Material).dispose();
        scene.remove(axes);
        axes.dispose();
        world.destroy();
      },
    };
  },
};
