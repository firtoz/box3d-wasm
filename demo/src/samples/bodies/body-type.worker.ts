import { PhysicsWorkerBase } from "../../physics-worker-base";
import { BodyType, type Vec3 } from "box3d-wasm";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";

const PI = Math.PI;

class BodyTypeWorker extends PhysicsWorkerBase {
  private attachmentId = 0;
  private secondAttachmentId = 0;
  private platformId = 0;
  private secondPayloadId = 0;
  private touchingBodyId = 0;
  private floatingBodyId = 0;
  private bodyType = BodyType.Dynamic;
  private speed = 3;
  private platformVx = -3;

  protected getGroundSize(): Vec3 {
    return [10, 1, 10];
  }

  protected async buildScene(): Promise<number[]> {
    const handles: number[] = [];
    const W = this.world!;
    const R = this.runtime!;

    this.attachmentId = W.createBody({ type: BodyType.Dynamic, position: [-2, 3, 0] });
    R.createHullShape(this.attachmentId, [0.5, 2, 0.5], { density: 1 });
    handles.push(this.attachmentId);

    this.secondAttachmentId = W.createBody({ type: this.bodyType, position: [3, 3, 0] });
    R.createHullShape(this.secondAttachmentId, [0.5, 2, 0.5], { density: 1 });
    handles.push(this.secondAttachmentId);

    this.platformId = W.createBody({ type: this.bodyType, position: [-4, 5, 0] });
    R.createTransformedHullShape(this.platformId, [0.5, 4, 0.5], {
      position: [4, 0, 0],
      rotation: [0, 0, Math.sin(PI / 4), Math.cos(PI / 4)],
    }, undefined, { density: 2 });
    handles.push(this.platformId);

    W.createRevoluteJoint(this.attachmentId, this.platformId, {
      localFrameA: { position: R.getBodyLocalPoint(this.attachmentId, [-2, 5, 0]) },
      localFrameB: { position: R.getBodyLocalPoint(this.platformId, [-2, 5, 0]) },
      enableMotor: true,
      maxMotorTorque: 50,
    });

    W.createRevoluteJoint(this.secondAttachmentId, this.platformId, {
      localFrameA: { position: R.getBodyLocalPoint(this.secondAttachmentId, [3, 5, 0]) },
      localFrameB: { position: R.getBodyLocalPoint(this.platformId, [3, 5, 0]) },
      enableMotor: true,
      maxMotorTorque: 50,
    });

    W.createPrismaticJoint(0, this.platformId, {
      localFrameA: { position: [0, 5, 0] },
      localFrameB: { position: R.getBodyLocalPoint(this.platformId, [0, 5, 0]) },
      enableMotor: true,
      maxMotorForce: 1000,
      motorSpeed: 0,
      enableLimit: true,
      lowerTranslation: -10,
      upperTranslation: 10,
    });

    const crate1 = W.createBody({ type: BodyType.Dynamic, position: [-3, 8, 0] });
    R.createHullShape(crate1, [0.75, 0.75, 0.75], { density: 2 });
    handles.push(crate1);

    this.secondPayloadId = W.createBody({ type: this.bodyType, position: [2, 8, 0] });
    R.createHullShape(this.secondPayloadId, [0.75, 0.75, 0.75], { density: 2 });
    handles.push(this.secondPayloadId);

    this.touchingBodyId = W.createBody({ type: this.bodyType, position: [8, 0.2, 0] });
    R.createCapsuleShape(this.touchingBodyId, [0, 0, 0], [1, 0, 0], 0.25, { density: 2 });
    handles.push(this.touchingBodyId);

    this.floatingBodyId = W.createBody({ type: this.bodyType, position: [-8, 12, 0], gravityScale: 0 });
    R.createSphereShape(this.floatingBodyId, [0, 0.5, 0], 0.25, { density: 2 });
    handles.push(this.floatingBodyId);

    return handles;
  }

  protected stepPhysics(): number {
    const start = performance.now();
    this.world!.step(this.fixedTimeStep, this.subSteps);

    if (this.bodyType === BodyType.Kinematic) {
      const pos = this.runtime!.readBodyTransform(this.platformId).position;
      if ((pos[0] < -14 && this.platformVx < 0) || (pos[0] > 6 && this.platformVx > 0)) {
        this.platformVx = -this.platformVx;
      }
      this.runtime!.setBodyLinearVelocity(this.platformId, [this.platformVx, 0, 0]);
    }

    return performance.now() - start;
  }

  protected handleCustomCommand(cmd: PhysicsWorkerCommand): boolean {
    const msg = cmd as Record<string, unknown>;
    const R = this.runtime!;
    switch (msg.type) {
      case "setBodyType": {
        this.bodyType = msg.bodyType as number;
        const bodies = [this.platformId, this.secondAttachmentId, this.secondPayloadId, this.touchingBodyId, this.floatingBodyId];
        for (const b of bodies) R.setBodyType(b, this.bodyType);
        if (this.bodyType === BodyType.Kinematic) {
          this.platformVx = -this.speed;
          R.setBodyLinearVelocity(this.platformId, [this.platformVx, 0, 0]);
          R.setBodyAngularVelocity(this.platformId, [0, 0, 0]);
          R.setBodyLinearVelocity(this.secondAttachmentId, [0, 0, 0]);
          R.setBodyAngularVelocity(this.secondAttachmentId, [0, 0, 0]);
        }
        return true;
      }
      case "setEnabled": {
        if (msg.enabled) {
          R.bodyEnable(this.attachmentId);
          R.bodyEnable(this.secondPayloadId);
          R.bodyEnable(this.floatingBodyId);
        } else {
          R.bodyDisable(this.attachmentId);
          R.bodyDisable(this.secondPayloadId);
          R.bodyDisable(this.floatingBodyId);
        }
        return true;
      }
    }
    return false;
  }
}

new BodyTypeWorker();
