import { PhysicsWorkerBase } from "../../physics-worker-base";
import { BodyType, type Vec3 } from "box3d-wasm";

class HullReductionWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return [10, 1, 10];
  }

  private mulberry32(a: number): () => number {
    return () => {
      a |= 0; a = a + 0x6D2B79F5 | 0;
      const t = Math.imul(a ^ a >>> 15, 1 | a);
      const t2 = t + Math.imul(t ^ t >>> 7, 61 | t) ^ t;
      return ((t2 ^ t2 >>> 14) >>> 0) / 4294967296;
    };
  }

  protected async buildScene(): Promise<number[]> {
    const rand = this.mulberry32(42);
    const count = 128;
    const points: number[] = [];

    for (let i = 0; i < count; i++) {
      const theta = 2 * Math.PI * rand();
      const phi = Math.acos(2 * rand() - 1);
      points.push(
        Math.sin(phi) * Math.cos(theta),
        Math.sin(phi) * Math.sin(theta),
        Math.cos(phi),
      );
    }

    const hullHandle = this.runtime!.createHullFromPoints(points);
    const body = this.world!.createBody({ type: BodyType.Dynamic, position: [0, 1, 0] });
    this.runtime!.createShapeFromHull(body, hullHandle, {});
    this.runtime!.destroyHull(hullHandle);
    return [body];
  }
}

new HullReductionWorker();
