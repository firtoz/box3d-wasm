import * as THREE from "three";
import type { Box3DRuntime, PhysicsWorld } from "box3d-wasm";
import type { DemoBody, DemoSample } from "./types";
import { addBox, disposeBodies, syncBodies } from "./shared";

const ALPHA_DEG = 25;
const ALPHA = ALPHA_DEG * Math.PI / 180;
const CARD_HALF_DEPTH = 0.04;
const CARD_HALF_HEIGHT = 0.49;
const CARD_HALF_WIDTH = 0.19;

function addVerticalPair(
  runtime: Box3DRuntime, world: PhysicsWorld, scene: THREE.Scene, bodies: DemoBody[],
  startX: number, offsetX: number, startY: number, alpha: number,
): void {
  const color = 0xfde68a;
  for (let i = 0; i < 2; i++) {
    const sign = i === 0 ? -1 : 1;
    const p: [number, number, number] = [startX + sign * offsetX, startY, 0];
    const qz = Math.sin(sign * alpha / 2);
    const qwZ = Math.cos(sign * alpha / 2);
    const bodyHandle = world.createBody({ type: 2, position: p, isAwake: true });
    world.setBodyTransform(bodyHandle, p, [0, 0, qz, qwZ]);
    runtime.createHullShape(bodyHandle, [CARD_HALF_DEPTH, CARD_HALF_HEIGHT, CARD_HALF_WIDTH], { friction: 0.8 });
    const mesh = new THREE.Mesh(
      new THREE.BoxGeometry(CARD_HALF_DEPTH * 2, CARD_HALF_HEIGHT * 2, CARD_HALF_WIDTH * 2),
      new THREE.MeshStandardMaterial({ color, roughness: 0.5 }),
    );
    mesh.position.set(p[0], p[1], p[2]);
    mesh.quaternion.set(0, 0, qz, qwZ);
    mesh.castShadow = true;
    mesh.receiveShadow = true;
    scene.add(mesh);
    bodies.push({ handle: bodyHandle, mesh, type: 2 });
  }
}

function addHorizontalRow(
  runtime: Box3DRuntime, world: PhysicsWorld, scene: THREE.Scene, bodies: DemoBody[],
  startX: number, offsetX: number, startY: number, count: number,
): void {
  const color = 0xfde68a;
  for (let i = 0; i < count; i++) {
    const p: [number, number, number] = [startX + i * offsetX, startY, 0];
    const bodyHandle = world.createBody({ type: 2, position: p, isAwake: true });
    world.setBodyTransform(bodyHandle, p, [0, 0, Math.sin(Math.PI / 4), Math.cos(Math.PI / 4)]);
    runtime.createHullShape(bodyHandle, [CARD_HALF_DEPTH, CARD_HALF_HEIGHT, CARD_HALF_WIDTH], { friction: 0.8 });
    const mesh = new THREE.Mesh(
      new THREE.BoxGeometry(CARD_HALF_DEPTH * 2, CARD_HALF_HEIGHT * 2, CARD_HALF_WIDTH * 2),
      new THREE.MeshStandardMaterial({ color, roughness: 0.5 }),
    );
    mesh.position.set(p[0], p[1], p[2]);
    mesh.quaternion.set(0, 0, Math.sin(Math.PI / 4), Math.cos(Math.PI / 4));
    mesh.castShadow = true;
    mesh.receiveShadow = true;
    scene.add(mesh);
    bodies.push({ handle: bodyHandle, mesh, type: 2 });
  }
}

export const cardHouseThickSample: DemoSample = {
  id: "card-house-thick",
  name: "Stacking / Card House Thick",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -9.81, 0] });
    const bodies: DemoBody[] = [];

    addBox(world, scene, bodies, [5, 1, 5], [0, -1, 0], 0x222222, true);

    const offsetX = 0.5 * CARD_HALF_HEIGHT * 2 * Math.sin(ALPHA) + 0.045;
    const offsetY = 0.5 * CARD_HALF_HEIGHT * 2 * Math.cos(ALPHA) + 0.035;

    addVerticalPair(runtime, world, scene, bodies, -6 * offsetX, offsetX, offsetY, ALPHA);
    addHorizontalRow(runtime, world, scene, bodies, -4 * offsetX, 4 * offsetX, 2 * offsetY + 0.04, 3);
    addVerticalPair(runtime, world, scene, bodies, -4 * offsetX, offsetX, 3 * offsetY + 0.08, ALPHA);
    addHorizontalRow(runtime, world, scene, bodies, -2 * offsetX, 4 * offsetX, 4 * offsetY + 0.12, 2);
    addVerticalPair(runtime, world, scene, bodies, -2 * offsetX, offsetX, 5 * offsetY + 0.16, ALPHA);
    addHorizontalRow(runtime, world, scene, bodies, 0, 4 * offsetX, 6 * offsetY + 0.20, 1);
    addVerticalPair(runtime, world, scene, bodies, 0, offsetX, 7 * offsetY + 0.24, ALPHA);

    return {
      world,
      bodies,
      controls: [],
      step(dt) {
        world.step(dt, 4);
        syncBodies(world, bodies);
      },
      dispose() {
        disposeBodies(scene, bodies);
        world.destroy();
      },
    };
  },
};
