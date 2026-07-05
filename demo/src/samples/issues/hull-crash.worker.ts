import { PhysicsWorkerBase } from "../../physics-worker-base";
import { BodyType, type Vec3 } from "box3d-wasm";

class HullCrashWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return [10, 1, 10];
  }

  protected async buildScene(): Promise<number[]> {
    const raw = [
      [100.0, -142.292389, 130.826111],
      [99.5354385, -71.3011093, 130.826111],
      [99.5930862, -80.1112213, -100.0],
      [100.0, -142.292389, -100.0],
      [99.5930862, -80.1112213, 130.826111],
    ];

    const s = 0.01;
    const points: number[] = [];
    for (const p of raw) {
      points.push(p[0] * s, p[1] * s, p[2] * s);
    }

    const hullHandle = this.runtime!.createHullFromPoints(points);
    const body = this.world!.createBody({ type: BodyType.Dynamic, position: [0, 1.5, 0] });
    this.runtime!.createShapeFromHull(body, hullHandle, {});
    this.runtime!.destroyHull(hullHandle);
    return [body];
  }
}

new HullCrashWorker();
