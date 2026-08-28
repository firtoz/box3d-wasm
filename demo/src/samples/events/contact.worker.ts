import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";
import {
  CONTACT_DEBRIS_COUNT,
  CONTACT_RAND_SEED,
  applyContactDrive,
  contactGroundSize,
  contactTrackedBodies,
  createContactState,
  processContactPostStep,
  type ContactState,
} from "./contact-scene";

class ContactEventWorker extends PhysicsWorkerBase {
  private contact: ContactState | null = null;
  private hudBuffer: SharedArrayBuffer | null = null;
  private hud: Int32Array | null = null;

  protected getGroundSize(): Vec3 {
    return contactGroundSize();
  }

  protected getTrackedBodyCapacity(): number {
    return 2 + CONTACT_DEBRIS_COUNT;
  }

  protected getReadyExtra(): Record<string, unknown> {
    return this.hudBuffer === null ? {} : { contactHud: this.hudBuffer };
  }

  protected async buildScene(): Promise<BodyId[]> {
    this.runtime!.setRandomSeed(CONTACT_RAND_SEED);
    this.contact = createContactState(this.world!, this.runtime!);
    this.hudBuffer = new SharedArrayBuffer(4);
    this.hud = new Int32Array(this.hudBuffer);
    return contactTrackedBodies(this.contact);
  }

  protected stepPhysics(): void {
    if (this.world === null || this.runtime === null || this.contact === null) return;
    applyContactDrive(this.world, this.runtime, this.contact);
    this.world.step(this.fixedTimeStep, this.subSteps);
    this.totalSteps += 1;
    processContactPostStep(this.world, this.runtime, this.contact, this.fixedTimeStep);
    this.setTrackedBodies(contactTrackedBodies(this.contact));
    if (this.hud !== null) {
      this.hud[0] = this.world.getBodyShapes(this.contact.player).length;
    }
  }

  protected handleCustomCommand(cmd: PhysicsWorkerCommand): boolean {
    const msg = cmd as Record<string, unknown>;
    const state = this.contact;
    if (state === null) return false;
    if (msg.type === "drive-throttle") {
      if (typeof msg.x === "number") state.throttleX = msg.x;
      if (typeof msg.y === "number") state.throttleY = -msg.y;
      return true;
    }
    if (msg.type === "drive-view") {
      if (typeof msg.fx === "number" && typeof msg.fy === "number" && typeof msg.fz === "number") {
        state.forward = [msg.fx, msg.fy, msg.fz];
      }
      if (typeof msg.rx === "number" && typeof msg.ry === "number" && typeof msg.rz === "number") {
        state.right = [msg.rx, msg.ry, msg.rz];
      }
      return true;
    }
    if (msg.type === "set-torque" && typeof msg.value === "number") {
      state.torque = msg.value;
      return true;
    }
    return false;
  }
}

new ContactEventWorker();
