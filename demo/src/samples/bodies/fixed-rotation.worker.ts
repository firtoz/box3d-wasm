import { PhysicsWorkerBase } from "../../physics-worker-base";
import { BodyType, type Vec3 } from "box3d-wasm";

class FixedRotationWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return [10, 1, 10];
  }

  protected async buildScene(): Promise<number[]> {
    const handles: number[] = [];

    let body = this.world!.createBody({ type: BodyType.Static, position: [0, 0.5, 0] });
    this.runtime!.createCapsuleShape(body, [-0.5, 0, 0], [0.5, 0, 0], 0.3, {});
    handles.push(body);

    body = this.world!.createBody({
      type: BodyType.Dynamic, position: [0.3, 0.5, 0],
      gravityScale: 0, enableSleep: false, isAwake: true,
    });
    this.runtime!.setBodyMotionLocks(body, { lockRotationX: true, lockRotationY: true, lockRotationZ: true });
    this.runtime!.createCapsuleShape(body, [-0.5, 0, 0], [0.5, 0, 0], 0.2, {});
    handles.push(body);

    return handles;
  }
}

new FixedRotationWorker();
