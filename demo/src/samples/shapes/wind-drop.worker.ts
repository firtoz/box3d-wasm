import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3, ShapeId, BodyHandle } from "box3d-wasm";
import { buildWindDropDynamicBodies, windDropGroundSize } from "./wind-drop-scene";

class WindDropWorker extends PhysicsWorkerBase {
  private shapeId: ShapeId | null = null;

  protected getGroundSize(): Vec3 { return windDropGroundSize(); }

  protected async buildScene(): Promise<number[]> {
    const handles = buildWindDropDynamicBodies(this.world!, this.runtime!);
    if (handles.length > 0) {
      const shapes = this.world!.getBodyShapes(handles[0] as BodyHandle);
      this.shapeId = shapes[0] ?? null;
    }
    return handles;
  }

  protected stepPhysics(): number {
    if (this.world === null) return 0;
    const start = performance.now();
    if (this.shapeId !== null) {
      this.runtime!.applyShapeWind(this.shapeId, [0, 0, 0], 1.0, 4.0, 10.0, false);
    }
    this.world.step(this.fixedTimeStep, this.subSteps);
    return performance.now() - start;
  }
}

new WindDropWorker();
