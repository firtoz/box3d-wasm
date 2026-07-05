import { PhysicsWorkerBase } from "../../physics-worker-base";
import { BodyType, type Vec3 } from "box3d-wasm";

class OverflowColorPileWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return [25, 1, 25];
  }

  protected async buildScene(): Promise<number[]> {
    const handles: number[] = [];

    const hubHalf: Vec3 = [3, 0.5, 3];
    const hub = this.world!.createBody({
      type: BodyType.Dynamic, position: [0, 0.5, 0], enableSleep: false,
    });
    this.runtime!.createHullShape(hub, hubHalf, { density: 100 });
    handles.push(hub);

    const neighborHalf: Vec3 = [0.4, 0.4, 0.4];
    const radius = 3.2;
    const count = 24;

    for (let i = 0; i < count; i++) {
      const angle = (2 * Math.PI * i) / count;
      const x = radius * Math.cos(angle);
      const z = radius * Math.sin(angle);
      const body = this.world!.createBody({
        type: BodyType.Dynamic, position: [x, 0.4, z], enableSleep: false,
      });
      this.runtime!.createHullShape(body, neighborHalf, {});
      handles.push(body);
    }

    return handles;
  }
}

new OverflowColorPileWorker();
