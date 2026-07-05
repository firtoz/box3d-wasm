import { PhysicsWorkerBase } from "../../physics-worker-base";
import { BodyType, type Vec3 } from "box3d-wasm";

class LockMixingWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return [10, 1, 10];
  }

  protected async buildScene(): Promise<number[]> {
    const handles: number[] = [];
    const halfWidths: Vec3 = [0.5, 0.5, 0.5];

    let body = this.world!.createBody({ type: BodyType.Dynamic, position: [0, 2, 0] });
    this.runtime!.createHullShape(body, halfWidths, {});
    handles.push(body);

    body = this.world!.createBody({ type: BodyType.Dynamic, position: [2, 2, 0] });
    this.runtime!.setBodyMotionLocks(body, { lockRotationX: true, lockRotationZ: true });
    this.runtime!.createHullShape(body, halfWidths, {});
    handles.push(body);

    body = this.world!.createBody({ type: BodyType.Dynamic, position: [-2, 2, 0] });
    this.runtime!.setBodyMotionLocks(body, { lockX: true, lockY: true, lockLinearZ: true });
    this.runtime!.createHullShape(body, halfWidths, {});
    handles.push(body);

    body = this.world!.createBody({ type: BodyType.Dynamic, position: [0, 1, 2] });
    this.runtime!.setBodyMotionLocks(body, {
      lockX: true, lockY: true, lockLinearZ: true,
      lockRotationX: true, lockRotationY: true, lockRotationZ: true,
    });
    this.runtime!.createHullShape(body, halfWidths, {});
    handles.push(body);

    body = this.world!.createBody({ type: BodyType.Static, position: [0, 1, -3] });
    this.runtime!.createHullShape(body, halfWidths, {});
    handles.push(body);

    return handles;
  }
}

new LockMixingWorker();
