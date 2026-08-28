import type { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import {
  EXTRA_EVENT_BUFFER_HUD_INTS,
  buildExtraEventBufferDynamicBodies,
  extraEventBufferGroundSize,
} from "./event-buffer-scene";

class ExtraEventBufferWorker extends PhysicsWorkerBase {
  private visitor: BodyId | null = null;
  private hudBuffer: SharedArrayBuffer | null = null;
  private hud: Int32Array | null = null;

  protected getGroundSize(): Vec3 {
    return extraEventBufferGroundSize();
  }

  protected getReadyExtra(): Record<string, unknown> {
    return this.hudBuffer === null ? {} : { eventBufferHud: this.hudBuffer };
  }

  protected async buildScene(): Promise<BodyId[]> {
    this.hudBuffer = new SharedArrayBuffer(EXTRA_EVENT_BUFFER_HUD_INTS * 4);
    this.hud = new Int32Array(this.hudBuffer);
    const scene = buildExtraEventBufferDynamicBodies(this.world!, this.runtime!);
    this.visitor = scene.visitor;
    return scene.handles;
  }

  protected stepPhysics(): void {
    super.stepPhysics();
    if (this.world === null || this.hud === null || this.visitor === null) return;
    const contacts = this.world.fillContactEvents();
    const sensors = this.world.fillSensorEvents();
    const bodyContacts = this.world.fillBodyContactData(this.visitor);
    this.hud[0] = contacts.beginCount;
    this.hud[1] = contacts.endCount;
    this.hud[2] = contacts.hitCount;
    this.hud[3] = sensors.beginCount;
    this.hud[4] = sensors.endCount;
    this.hud[5] = bodyContacts.beginCount;
    this.hud[6] = contacts.pairs.length + sensors.pairs.length + bodyContacts.pairs.length;
    const manifolds = this.world.fillBodyContactManifolds(this.visitor);
    this.hud[7] = manifolds.contactCount;
    this.hud[8] = manifolds.pointCount;
    let maxImpulse = 0;
    for (let i = 0; i < manifolds.pointCount; i++) {
      const impulse = manifolds.points[i * 8 + 7] ?? 0;
      if (impulse > maxImpulse) maxImpulse = impulse;
    }
    this.hud[9] = Math.round(maxImpulse * 1000);
    const transform = this.world.getBodyTransform(this.visitor);
    if (transform.position[1] < -2) {
      this.world.setBodyTransform(this.visitor, [0, 6, 0]);
      this.world.setBodyLinearVelocity(this.visitor, [0, 0, 0]);
      this.world.setBodyAngularVelocity(this.visitor, [0, 0, 0]);
    }
  }
}

new ExtraEventBufferWorker();
