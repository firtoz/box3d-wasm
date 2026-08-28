import { B3_PI, type BodyId, type JointId, type Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";
import { buildWheelScene, wheelGroundSize } from "./wheel-scene";

class WheelWorker extends PhysicsWorkerBase {
  private joint: JointId | null = null;
  private lowerSusp = -1;
  private upperSusp = 1;
  private lowerSteerDeg = -45;
  private upperSteerDeg = 45;

  protected getGroundSize(): Vec3 {
    return wheelGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    const scene = buildWheelScene(this.world!, this.runtime!);
    this.joint = scene.joint;
    return scene.handles;
  }

  protected handleCustomCommand(cmd: PhysicsWorkerCommand): boolean {
    const msg = cmd as Record<string, unknown>;
    const world = this.world;
    const joint = this.joint;
    if (world === null || joint === null) return false;
    if (msg.type === "enable-suspension" && typeof msg.value === "boolean") {
      world.enableWheelSuspension(joint, msg.value);
      world.wakeJointBodies(joint);
      return true;
    }
    if (msg.type === "enable-suspension-limit" && typeof msg.value === "boolean") {
      world.enableWheelSuspensionLimit(joint, msg.value);
      world.wakeJointBodies(joint);
      return true;
    }
    if (msg.type === "enable-spin-motor" && typeof msg.value === "boolean") {
      world.enableWheelSpinMotor(joint, msg.value);
      world.wakeJointBodies(joint);
      return true;
    }
    if (msg.type === "enable-steering" && typeof msg.value === "boolean") {
      world.enableWheelSteering(joint, msg.value);
      world.wakeJointBodies(joint);
      return true;
    }
    if (msg.type === "enable-steering-limit" && typeof msg.value === "boolean") {
      world.enableWheelSteeringLimit(joint, msg.value);
      world.wakeJointBodies(joint);
      return true;
    }
    if (msg.type === "set-suspension-min" && typeof msg.value === "number") {
      this.lowerSusp = Math.min(msg.value, this.upperSusp);
      world.setWheelSuspensionLimits(joint, this.lowerSusp, this.upperSusp);
      return true;
    }
    if (msg.type === "set-suspension-max" && typeof msg.value === "number") {
      this.upperSusp = Math.max(msg.value, this.lowerSusp);
      world.setWheelSuspensionLimits(joint, this.lowerSusp, this.upperSusp);
      return true;
    }
    if (msg.type === "set-suspension-hertz" && typeof msg.value === "number") {
      world.setWheelSuspensionHertz(joint, msg.value);
      return true;
    }
    if (msg.type === "set-suspension-damping" && typeof msg.value === "number") {
      world.setWheelSuspensionDampingRatio(joint, msg.value);
      return true;
    }
    if (msg.type === "set-max-spin-torque" && typeof msg.value === "number") {
      world.setWheelMaxSpinTorque(joint, msg.value);
      return true;
    }
    if (msg.type === "set-spin-speed" && typeof msg.value === "number") {
      world.setWheelSpinMotorSpeed(joint, msg.value);
      return true;
    }
    if (msg.type === "set-steering-hertz" && typeof msg.value === "number") {
      world.setWheelSteeringHertz(joint, msg.value);
      return true;
    }
    if (msg.type === "set-steering-damping" && typeof msg.value === "number") {
      world.setWheelSteeringDampingRatio(joint, msg.value);
      return true;
    }
    if (msg.type === "set-steering-min-deg" && typeof msg.value === "number") {
      this.lowerSteerDeg = Math.min(msg.value, this.upperSteerDeg);
      world.setWheelSteeringLimits(joint, (B3_PI / 180) * this.lowerSteerDeg, (B3_PI / 180) * this.upperSteerDeg);
      return true;
    }
    if (msg.type === "set-steering-max-deg" && typeof msg.value === "number") {
      this.upperSteerDeg = Math.max(msg.value, this.lowerSteerDeg);
      world.setWheelSteeringLimits(joint, (B3_PI / 180) * this.lowerSteerDeg, (B3_PI / 180) * this.upperSteerDeg);
      return true;
    }
    return false;
  }
}

new WheelWorker();
