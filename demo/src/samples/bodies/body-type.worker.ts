import { PhysicsWorkerBase } from "../../physics-worker-base";
import { BodyType, type BodyHandle, type Vec3 } from "box3d-wasm";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";
import { buildBodyTypeDynamicBodies, bodyTypeGroundSize, bodyTypeHandleIndex, stepBodyType } from "./body-type-scene";

class BodyTypeWorker extends PhysicsWorkerBase {
  private attachmentId: BodyHandle | null = null;
  private secondAttachmentId: BodyHandle | null = null;
  private platformId: BodyHandle | null = null;
  private secondPayloadId: BodyHandle | null = null;
  private touchingBodyId: BodyHandle | null = null;
  private floatingBodyId: BodyHandle | null = null;
  private bodyType = BodyType.Dynamic;
  private speed = 3;
  private platformVx = -3;

  protected getGroundSize(): Vec3 {
    return bodyTypeGroundSize();
  }

  protected async buildScene(): Promise<BodyHandle[]> {
    const handles = buildBodyTypeDynamicBodies(this.world!, this.runtime!);
    this.attachmentId = handles[bodyTypeHandleIndex.attachmentId];
    this.secondAttachmentId = handles[bodyTypeHandleIndex.secondAttachmentId];
    this.platformId = handles[bodyTypeHandleIndex.platformId];
    this.secondPayloadId = handles[bodyTypeHandleIndex.secondPayloadId];
    this.touchingBodyId = handles[bodyTypeHandleIndex.touchingBodyId];
    this.floatingBodyId = handles[bodyTypeHandleIndex.floatingBodyId];
    return handles;
  }

  protected stepPhysics(): void {
    if (this.platformId === null) return;
    this.world!.step(this.fixedTimeStep, this.subSteps);
    this.platformVx = stepBodyType(this.world!, this.runtime!, this.platformId, this.bodyType, this.platformVx);
    this.totalSteps += 1;
  }

  protected handleCustomCommand(cmd: PhysicsWorkerCommand): boolean {
    const msg = cmd as Record<string, unknown>;
    const R = this.runtime!;
    switch (msg.type) {
      case "setBodyType": {
        if (this.platformId === null || this.secondAttachmentId === null || this.secondPayloadId === null || this.touchingBodyId === null || this.floatingBodyId === null) return false;
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
        if (this.attachmentId === null || this.secondPayloadId === null || this.floatingBodyId === null) return false;
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
