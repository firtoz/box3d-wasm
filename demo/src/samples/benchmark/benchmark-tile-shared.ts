import { BodyType, type Box3DRuntime, type PhysicsWorld } from "box3d-wasm";
import { collectHumanBoneHandles } from "../ragdoll/ragdoll-scene-shared";
import { f32, f32Add, f32Div, f32Mul } from "../f32";

export function createBenchmarkTileMeshes(world: PhysicsWorld) {
  const gridSize = f32(15);
  const halfMeshGridRows = 4;
  const meshGridCellWidth = f32Div(gridSize, f32Mul(2, halfMeshGridRows));
  const gridMesh = world.createGridMesh(8, 8, meshGridCellWidth, 1, true);
  const torusMesh = world.createTorusMesh(16, 16, f32Mul(0.25, gridSize), 1);
  return { gridSize, gridMesh, torusMesh };
}

export function buildBenchmarkTileGround(world: PhysicsWorld, gridCount: number): number[] {
  const { gridSize, gridMesh, torusMesh } = createBenchmarkTileMeshes(world);
  const handles: number[] = [];
  const span = f32Mul(gridSize, gridCount);
  let x = f32Add(f32Mul(-0.5, span), f32Mul(0.5, gridSize));
  for (let i = 0; i < gridCount; i++) {
    let z = f32Add(f32Mul(-0.5, span), f32Mul(0.5, gridSize));
    for (let j = 0; j < gridCount; j++) {
      const body = world.createBody({ type: BodyType.Static, position: [x, 0, z] });
      world.createMeshShape(body, gridMesh, { scale: [1, 1, 1] });
      world.createMeshShape(body, torusMesh, { scale: [1, 1, 1] });
      handles.push(body);
      z = f32Add(z, gridSize);
    }
    x = f32Add(x, gridSize);
  }
  return handles;
}

export function spawnHumanGroup(
  world: PhysicsWorld,
  runtime: Box3DRuntime,
  handles: number[],
  gridCount: number,
  gridSize: number,
  groupSize: number,
  rowIndex: number,
  columnIndex: number,
  spawnY: number,
  frictionTorque: number,
  hertz: number,
  dampingRatio: number,
): void {
  const span = f32Mul(gridCount, gridSize);
  const groupDistance = f32Div(span, gridCount);
  let x = f32Add(f32Mul(-0.5, span), f32Mul(groupDistance, f32Add(columnIndex, 0.5)));
  const z = f32Add(f32Mul(-0.5, span), f32Mul(groupDistance, f32Add(rowIndex, 0.5)));
  const groupIndex = rowIndex * gridCount + columnIndex;
  for (let i = 0; i < groupSize; i++) {
    const human = world.createHuman([x, spawnY, z], {
      frictionTorque,
      hertz,
      dampingRatio,
      groupIndex,
      colorize: false,
    });
    handles.push(...collectHumanBoneHandles(runtime, human));
    x = f32Add(x, 0.75);
  }
}
