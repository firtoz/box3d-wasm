import * as THREE from "three";
import { SlotExhaustedError, type Box3DRuntime, type PhysicsWorld } from "box3d-wasm";
import type { DemoBody, DemoSample } from "../types";
import { cameraFromSetView, disposeBodies, syncBodies } from "../shared";

const EMPTY = 0x334155;
const FILLED = 0x22d3ee;
const EXHAUSTED = 0xf43f5e;
const FRAMES_PER_WORLD = 8;

function usageLines(runtime: Box3DRuntime): string[] {
  const usage = runtime.getSlotUsage();
  const limits = runtime.limits;
  const trees = runtime.getTreeSlotUsage();
  return [
    `worlds ${usage.worlds}/${limits.worlds}  hulls ${usage.hulls}/${limits.hulls}  humans ${usage.humans}/${limits.humans}`,
    `meshes ${usage.meshes}/${limits.meshes}  compounds ${usage.compounds}/${limits.compounds}  heightFields ${usage.heightFields}/${limits.heightFields}`,
    `dynamic trees ${trees.used}/${trees.max} (private pool, not in SlotLimits)`,
  ];
}

function slotPosition(index: number, columns: number): [number, number, number] {
  const col = index % columns;
  const row = Math.floor(index / columns);
  return [(col - (columns - 1) * 0.5) * 1.4, 0.4, (row - (columns - 1) * 0.5) * 1.4];
}

export const extraSlotExhaustionSample: DemoSample = {
  id: "extra/slot-exhaustion",
  name: "Extra / Slot Exhaustion",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
    const extras: PhysicsWorld[] = [];
    const maxWorlds = runtime.limits.worlds;
    const columns = Math.ceil(Math.sqrt(maxWorlds));
    let status = "each cyan cube is one world slot — filling gradually";
    let done = false;
    let framesUntilNext = FRAMES_PER_WORLD;

    const floor = new THREE.Mesh(
      new THREE.BoxGeometry(columns * 1.6 + 1, 0.15, columns * 1.6 + 1),
      new THREE.MeshBasicMaterial({ color: 0x1e293b }),
    );
    floor.position.y = -0.15;
    scene.add(floor);

    const cubes: THREE.Mesh[] = [];
    for (let i = 0; i < maxWorlds; i++) {
      const mesh = new THREE.Mesh(
        new THREE.BoxGeometry(1, 0.8, 1),
        new THREE.MeshBasicMaterial({ color: EMPTY }),
      );
      mesh.position.set(...slotPosition(i, columns));
      scene.add(mesh);
      cubes.push(mesh);
    }
    (cubes[0]!.material as THREE.MeshBasicMaterial).color.setHex(FILLED);
    const bodies: DemoBody[] = [];

    const paint = (count: number, exhausted: boolean) => {
      for (let i = 0; i < cubes.length; i++) {
        const mat = cubes[i]!.material as THREE.MeshBasicMaterial;
        if (i < count) mat.color.setHex(FILLED);
        else if (exhausted) mat.color.setHex(EXHAUSTED);
        else mat.color.setHex(EMPTY);
      }
    };

    return {
      world,
      bodies,
      camera: cameraFromSetView(25, 28, 14, [0, 0.4, 0]),
      getInfo: () => [status, ...usageLines(runtime)].join("\n"),
      controls: [],
      step(dt, subSteps) {
        world.step(dt ?? 1 / 60, subSteps ?? 4);
        syncBodies(world, bodies);
        if (done) return;
        framesUntilNext -= 1;
        if (framesUntilNext > 0) return;
        framesUntilNext = FRAMES_PER_WORLD;
        try {
          extras.push(runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 }));
          const used = 1 + extras.length;
          paint(used, false);
          status = `created ${used}/${maxWorlds} worlds`;
        } catch (caught) {
          done = true;
          paint(1 + extras.length, true);
          if (caught instanceof SlotExhaustedError) {
            status = caught.message;
          } else {
            throw caught;
          }
        }
      },
      dispose() {
        disposeBodies(scene, bodies);
        for (const extra of extras) extra.destroy();
        world.destroy();
        scene.remove(floor);
        floor.geometry.dispose();
        (floor.material as THREE.Material).dispose();
        for (const mesh of cubes) {
          scene.remove(mesh);
          mesh.geometry.dispose();
          (mesh.material as THREE.Material).dispose();
        }
      },
    };
  },
};
