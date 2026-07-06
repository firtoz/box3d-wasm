import { PhysicsWorkerBase } from "../../physics-worker-base";
import { B3_AXIS_X, B3_DEG_TO_RAD, BodyType, type Vec3 } from "box3d-wasm";
import { addBox } from "../shared-worker";

class ShapesInclinedPlaneWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return [50, 1, 50];
  }

  protected async buildScene(): Promise<number[]> {
    const handles: number[] = [];
    const angle = 40 * B3_DEG_TO_RAD;
    addBox(this.world!, this.runtime!, handles, [0, 7.5, -5], [16, 0.5, 10], this.runtime!.makeQuatFromAxisAngle(B3_AXIS_X, angle), { type: BodyType.Static, friction: 1 });
    for (let i = 0; i < 5; i++) addBox(this.world!, this.runtime!, handles, [-10 + 5 * i, 15.75, -10.6], [1, 1, 1], [0, 0, 0, 1], { friction: (i + 1) * (i + 1) * 0.04 });
    return handles;
  }
}

new ShapesInclinedPlaneWorker();
