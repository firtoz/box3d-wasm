import {BodyType, type BodyId, type Box3DRuntime, type MeshHandle, type PhysicsWorld, type Vec3} from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";

const GRID_CELL_COUNT = 20;
const GRID_CELL_WIDTH = 2;
const GRID_MATERIAL_COUNT = 2;
const SPHERE_RADIUS = 0.5;
const SPHERE_POS: Vec3 = [-18, 1, 0.5];
const SPHERE_VEL: Vec3 = [4, 0, 0];

export interface PersistentContactState {
  mesh: MeshHandle | null;
}

export function buildPersistentContactScene(
  world: PhysicsWorld,
  runtime: Box3DRuntime,
): { ground: BodyId; sphere: BodyId; mesh: MeshHandle } {
  const ground = world.createBody({ type: BodyType.Static });
  const mesh = world.createGridMesh(GRID_CELL_COUNT, GRID_CELL_COUNT, GRID_CELL_WIDTH, GRID_MATERIAL_COUNT, true);
  world.createMeshShape(ground, mesh, { scale: [1, 1, 1] });

  const sphere = world.createBody({
    type: BodyType.Dynamic,
    position: SPHERE_POS,
    linearVelocity: SPHERE_VEL,
  });
  runtime.createSphereShape(sphere, [0, 0, 0], SPHERE_RADIUS, {
    density: 20,
    enableContactEvents: true,
    rollingResistance: 0.01,
  });

  return { ground, sphere, mesh };
}

export function buildPersistentContactDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyId[] {
  return [buildPersistentContactScene(world, runtime).sphere];
}

export function persistentContactGroundSize(): Vec3 {
  return [20, 1, 20];
}

export const persistentContactBodies: RenderBody[] = [
  {
    kind: "sphere",
    radius: SPHERE_RADIUS,
    position: SPHERE_POS,
    color: 0x60a5fa,
  },
];

export const persistentContactCamera: RenderSpec["camera"] = cameraFromSetView(0, 30, 40, [0, 5, 0]);

export const dumpSampleName = "Persistent Contact";
export const dumpSampleId = "events/persistent-contact";
export const dumpCppSampleName = "Persistent Contact";

export function dumpCreate(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: BodyId[];
  state: PersistentContactState;
  dispose: () => void;
} {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const { ground, sphere, mesh } = buildPersistentContactScene(world, runtime);
  return {
    world,
    handles: [ground, sphere],
    state: { mesh },
    dispose: () => {
      world.destroyMesh(mesh);
    },
  };
}
