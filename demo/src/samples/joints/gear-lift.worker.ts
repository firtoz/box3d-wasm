import type { BodyId, JointId, MeshHandle, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";
import { buildGearLiftScene, gearLiftGroundSize } from "./gear-lift-scene";

class GearLiftWorker extends PhysicsWorkerBase {
  private mesh: MeshHandle | null = null;
  private driverJoint: JointId | null = null;

  protected getGroundSize(): Vec3 {
    return gearLiftGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    const scene = buildGearLiftScene(this.world!, this.runtime!);
    this.mesh = scene.mesh;
    this.driverJoint = scene.driverJoint;
    return scene.handles;
  }

  protected handleCustomCommand(cmd: PhysicsWorkerCommand): boolean {
    const msg = cmd as Record<string, unknown>;
    const world = this.world;
    const joint = this.driverJoint;
    if (world === null || joint === null) return false;
    if (msg.type === "enable-motor") {
      if (typeof msg.value === "boolean") {
        world.enableRevoluteMotor(joint, msg.value);
        world.wakeJointBodies(joint);
      }
      return true;
    }
    if (msg.type === "set-max-torque") {
      if (typeof msg.value === "number") {
        world.setRevoluteMaxMotorTorque(joint, msg.value);
        world.wakeJointBodies(joint);
      }
      return true;
    }
    if (msg.type === "set-motor-speed") {
      if (typeof msg.value === "number") {
        world.setRevoluteMotorSpeed(joint, msg.value);
        world.wakeJointBodies(joint);
      }
      return true;
    }
    return false;
  }

  protected onBeforeDisposeWorld(): void {
    if (this.mesh !== null && this.world !== null) {
      this.world.destroyMesh(this.mesh);
      this.mesh = null;
    }
    this.driverJoint = null;
  }
}

new GearLiftWorker();
