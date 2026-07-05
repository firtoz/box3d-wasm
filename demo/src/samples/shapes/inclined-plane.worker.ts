import { PhysicsWorkerBase } from "../../physics-worker-base";
import { BodyType, type Vec3 } from "box3d-wasm";
import { addBox } from "../shared-worker";

const PI = Math.PI;

class ShapesInclinedPlaneWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return [50, 1, 50];
  }

  protected async buildScene(): Promise<number[]> {
    const handles: number[] = [];
    const angle = 40 * PI / 180;
    addBox(this.world!, this.runtime!, handles, [0, 7.5, -5], [16, 0.5, 10], [Math.sin(angle / 2), 0, 0, Math.cos(angle / 2)], { type: BodyType.Static, friction: 1 });
    for (let i = 0; i < 5; i++) addBox(this.world!, this.runtime!, handles, [-10 + 5 * i, 15.75, -10.6], [1, 1, 1], [0, 0, 0, 1], { friction: (i + 1) * (i + 1) * 0.04 });
    return handles;
  }
}

new ShapesInclinedPlaneWorker();
