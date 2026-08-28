import { B3_PI, type BodyId, type HeightFieldHandle, type JointId, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";
import {
  DRIVING_LOWER_STEERING_DEG,
  DRIVING_LOWER_SUSPENSION,
  DRIVING_SPIN_SPEED,
  DRIVING_UPPER_STEERING_DEG,
  DRIVING_UPPER_SUSPENSION,
  buildDrivingScene,
  drivingGroundSize,
} from "./driving-scene";

class DrivingWorker extends PhysicsWorkerBase {
  private heightField: HeightFieldHandle | null = null;
  private chassis: BodyId | null = null;
  private frontLeft: JointId | null = null;
  private frontRight: JointId | null = null;
  private rearLeft: JointId | null = null;
  private rearRight: JointId | null = null;
  private throttleX = 0;
  private throttleY = 0;
  private spinSpeed = DRIVING_SPIN_SPEED;
  private lowerSuspension = DRIVING_LOWER_SUSPENSION;
  private upperSuspension = DRIVING_UPPER_SUSPENSION;
  private lowerSteerDeg = DRIVING_LOWER_STEERING_DEG;
  private upperSteerDeg = DRIVING_UPPER_STEERING_DEG;
  private hudBuffer: SharedArrayBuffer | null = null;
  private hud: Float32Array | null = null;

  protected setupGround(): void {}

  protected getGroundSize(): Vec3 {
    return drivingGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    const scene = buildDrivingScene(this.world!, this.runtime!);
    this.heightField = scene.heightField;
    this.chassis = scene.handles[1] ?? null;
    this.frontLeft = scene.joints.frontLeft;
    this.frontRight = scene.joints.frontRight;
    this.rearLeft = scene.joints.rearLeft;
    this.rearRight = scene.joints.rearRight;
    this.hudBuffer = new SharedArrayBuffer(9 * 4);
    this.hud = new Float32Array(this.hudBuffer);
    return scene.handles;
  }

  protected getReadyExtra(): Record<string, unknown> {
    return this.hudBuffer === null ? {} : { driveHud: this.hudBuffer };
  }

  protected stepPhysics(): void {
    this.applyDrive();
    super.stepPhysics();
  }

  protected publishExtra(): void {
    const world = this.world;
    const runtime = this.runtime;
    const chassis = this.chassis;
    const hud = this.hud;
    if (world === null || runtime === null || chassis === null || hud === null) return;
    if (this.frontLeft === null || this.frontRight === null || this.rearLeft === null || this.rearRight === null) return;
    const xf = world.getBodyTransform(chassis);
    const forward = runtime.rotateVector(xf.rotation, [-1, 0, 0]);
    const vel = world.getBodyLinearVelocity(chassis);
    hud[0] = vel[0] * forward[0] + vel[1] * forward[1] + vel[2] * forward[2];
    hud[1] = world.getWheelSpinSpeed(this.rearLeft);
    hud[2] = world.getWheelSpinSpeed(this.rearRight);
    hud[3] = world.getWheelSpinTorque(this.rearLeft);
    hud[4] = world.getWheelSpinTorque(this.rearRight);
    hud[5] = (180 / B3_PI) * world.getWheelSteeringAngle(this.frontLeft);
    hud[6] = (180 / B3_PI) * world.getWheelSteeringAngle(this.frontRight);
    hud[7] = world.getWheelSteeringTorque(this.frontLeft);
    hud[8] = world.getWheelSteeringTorque(this.frontRight);
  }

  protected handleCustomCommand(cmd: PhysicsWorkerCommand): boolean {
    const msg = cmd as Record<string, unknown>;
    const world = this.world;
    if (world === null) return false;
    if (msg.type === "drive-throttle") {
      if (typeof msg.x === "number") this.throttleX = msg.x;
      if (typeof msg.y === "number") this.throttleY = msg.y;
      return true;
    }
    const joints = this.allJoints();
    if (joints === null) return false;
    const [fl, fr, rl, rr] = joints;
    if (msg.type === "set-suspension-min" && typeof msg.value === "number") {
      this.lowerSuspension = Math.min(msg.value, this.upperSuspension);
      for (const j of joints) world.setWheelSuspensionLimits(j, this.lowerSuspension, this.upperSuspension);
      return true;
    }
    if (msg.type === "set-suspension-max" && typeof msg.value === "number") {
      this.upperSuspension = Math.max(msg.value, this.lowerSuspension);
      for (const j of joints) world.setWheelSuspensionLimits(j, this.lowerSuspension, this.upperSuspension);
      return true;
    }
    if (msg.type === "set-suspension-hertz" && typeof msg.value === "number") {
      for (const j of joints) world.setWheelSuspensionHertz(j, msg.value);
      return true;
    }
    if (msg.type === "set-suspension-damping" && typeof msg.value === "number") {
      for (const j of joints) world.setWheelSuspensionDampingRatio(j, msg.value);
      return true;
    }
    if (msg.type === "set-max-spin-torque" && typeof msg.value === "number") {
      world.setWheelMaxSpinTorque(rl, msg.value);
      world.setWheelMaxSpinTorque(rr, msg.value);
      return true;
    }
    if (msg.type === "set-spin-speed" && typeof msg.value === "number") {
      this.spinSpeed = msg.value;
      return true;
    }
    if (msg.type === "set-steering-hertz" && typeof msg.value === "number") {
      world.setWheelSteeringHertz(fl, msg.value);
      world.setWheelSteeringHertz(fr, msg.value);
      return true;
    }
    if (msg.type === "set-steering-damping" && typeof msg.value === "number") {
      world.setWheelSteeringDampingRatio(fl, msg.value);
      world.setWheelSteeringDampingRatio(fr, msg.value);
      return true;
    }
    if (msg.type === "set-steering-torque" && typeof msg.value === "number") {
      world.setWheelMaxSteeringTorque(fl, msg.value);
      world.setWheelMaxSteeringTorque(fr, msg.value);
      return true;
    }
    if (msg.type === "set-steering-min-deg" && typeof msg.value === "number") {
      this.lowerSteerDeg = Math.min(msg.value, this.upperSteerDeg);
      this.applySteerLimits(world, fl, fr);
      return true;
    }
    if (msg.type === "set-steering-max-deg" && typeof msg.value === "number") {
      this.upperSteerDeg = Math.max(msg.value, this.lowerSteerDeg);
      this.applySteerLimits(world, fl, fr);
      return true;
    }
    return false;
  }

  private applySteerLimits(world: PhysicsWorld, fl: JointId, fr: JointId): void {
    const lower = (B3_PI / 180) * this.lowerSteerDeg;
    const upper = (B3_PI / 180) * this.upperSteerDeg;
    world.setWheelSteeringLimits(fl, lower, upper);
    world.setWheelSteeringLimits(fr, lower, upper);
  }

  private allJoints(): [JointId, JointId, JointId, JointId] | null {
    if (this.frontLeft === null || this.frontRight === null || this.rearLeft === null || this.rearRight === null) return null;
    return [this.frontLeft, this.frontRight, this.rearLeft, this.rearRight];
  }

  private applyDrive(): void {
    const world = this.world;
    const joints = this.allJoints();
    const chassis = this.chassis;
    if (world === null || joints === null || chassis === null) return;
    const [fl, fr, rl, rr] = joints;
    if (this.throttleX !== 0 || this.throttleY !== 0) world.setBodyAwake(chassis, true);
    const maxSteer = 0.25 * B3_PI;
    world.setWheelTargetSteeringAngle(fl, maxSteer * this.throttleY);
    world.setWheelTargetSteeringAngle(fr, maxSteer * this.throttleY);
    world.setWheelSpinMotorSpeed(rl, -this.spinSpeed * this.throttleX);
    world.setWheelSpinMotorSpeed(rr, -this.spinSpeed * this.throttleX);
  }

  protected onBeforeDisposeWorld(): void {
    if (this.heightField !== null && this.world !== null) {
      this.world.destroyHeightField(this.heightField);
      this.heightField = null;
    }
    this.chassis = null;
    this.frontLeft = this.frontRight = this.rearLeft = this.rearRight = null;
    this.hudBuffer = null;
    this.hud = null;
  }
}

new DrivingWorker();
