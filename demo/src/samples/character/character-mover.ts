import type { PhysicsWorld, Vec3 } from "box3d-wasm";
import { B3_PI } from "box3d-wasm";
import type { CollisionPlane } from "box3d-wasm";

const JUMP_SPEED = 5;
const MAX_SPEED = 6;
const MIN_SPEED = 0.01;
const STOP_SPEED = 1;
const ACCELERATE = 30;
const FRICTION = 4;
const GRAVITY = 15;
const CAPSULE = { center1: [0, -0.5, 0] as Vec3, center2: [0, 0.5, 0] as Vec3, radius: 0.3 };

export class CharacterMover {
  position: Vec3;
  velocity: Vec3 = [0, 0, 0];
  onGround = false;
  sprint = false;
  jumpQueued = false;
  pogoVelocity = 0;

  constructor(position: Vec3) {
    this.position = [position[0], position[1], position[2]];
  }

  step(world: PhysicsWorld, dt: number, forward: Vec3, right: Vec3, throttle: { x: number; y: number }, clipVelocity: boolean): void {
    let speed = Math.hypot(this.velocity[0], this.velocity[1], this.velocity[2]);
    if (speed < MIN_SPEED) {
      this.velocity[0] = 0;
      this.velocity[2] = 0;
    } else {
      const control = speed < STOP_SPEED ? STOP_SPEED : speed;
      const drop = control * FRICTION * dt;
      const newSpeed = Math.max(0, speed - drop);
      const ratio = newSpeed / speed;
      this.velocity[0] *= ratio;
      this.velocity[2] *= ratio;
    }

    const maxSpeed = this.sprint ? 1.5 * MAX_SPEED : MAX_SPEED;
    const desired: Vec3 = [
      maxSpeed * throttle.x * forward[0] + maxSpeed * throttle.y * right[0],
      0,
      maxSpeed * throttle.x * forward[2] + maxSpeed * throttle.y * right[2],
    ];
    let desiredSpeed = Math.hypot(desired[0], desired[1], desired[2]);
    let dir: Vec3 = [0, 0, 0];
    if (desiredSpeed > 1e-8) {
      dir = [desired[0] / desiredSpeed, desired[1] / desiredSpeed, desired[2] / desiredSpeed];
      if (desiredSpeed > maxSpeed) {
        desired[0] *= maxSpeed / desiredSpeed;
        desired[2] *= maxSpeed / desiredSpeed;
        desiredSpeed = maxSpeed;
      }
    }
    if (this.onGround) this.velocity[1] = 0;
    const currentSpeed = this.velocity[0] * dir[0] + this.velocity[1] * dir[1] + this.velocity[2] * dir[2];
    let addSpeed = desiredSpeed - currentSpeed;
    if (addSpeed > 0) {
      let accelSpeed = ACCELERATE * maxSpeed * dt;
      if (accelSpeed > addSpeed) accelSpeed = addSpeed;
      this.velocity[0] += accelSpeed * dir[0];
      this.velocity[1] += accelSpeed * dir[1];
      this.velocity[2] += accelSpeed * dir[2];
    }

    this.velocity[1] -= GRAVITY * dt;
    if (this.jumpQueued && this.onGround) {
      this.velocity[1] = JUMP_SPEED;
      this.onGround = false;
    }
    this.jumpQueued = false;

    const pogoRest = 3 * CAPSULE.radius;
    const rayLength = pogoRest + CAPSULE.radius;
    const rayOrigin: Vec3 = [this.position[0] + CAPSULE.center1[0], this.position[1] + CAPSULE.center1[1], this.position[2] + CAPSULE.center1[2]];
    const rayHit = world.rayCastClosest(rayOrigin, [0, -rayLength, 0], 1, 0xfffffffd);
    const suppressPogo = this.velocity[1] > 0;
    if (rayHit === null || suppressPogo) {
      this.onGround = false;
      this.pogoVelocity = 0;
    } else {
      this.onGround = true;
      const pogoCurrent = rayHit.fraction * rayLength;
      const zeta = 0.7;
      const hertz = 4;
      const omega = 2 * B3_PI * hertz;
      const omegaH = omega * dt;
      this.pogoVelocity = (this.pogoVelocity - omega * omegaH * (pogoCurrent - pogoRest)) / (1 + 2 * zeta * omegaH + omegaH * omegaH);
    }

    const start = this.position;
    const target: Vec3 = [
      this.position[0] + dt * this.velocity[0],
      this.position[1] + dt * this.velocity[1] + dt * this.pogoVelocity,
      this.position[2] + dt * this.velocity[2],
    ];
    for (let i = 0; i < 5; i++) {
      const hits = world.collideMover(this.position, CAPSULE, 8);
      const planes: CollisionPlane[] = hits.map((h) => ({ plane: h.plane, clipVelocity: true }));
      const deltaTarget: Vec3 = [target[0] - this.position[0], target[1] - this.position[1], target[2] - this.position[2]];
      const solved = world.solvePlanes(deltaTarget, planes);
      let delta = solved.delta;
      const fraction = world.castMover(this.position, CAPSULE, delta, 1, 0xfffffffd);
      delta = [delta[0] * fraction, delta[1] * fraction, delta[2] * fraction];
      this.position = [this.position[0] + delta[0], this.position[1] + delta[1], this.position[2] + delta[2]];
      if (delta[0] * delta[0] + delta[1] * delta[1] + delta[2] * delta[2] < 0.0001) break;
      if (clipVelocity) this.velocity = world.clipVector(this.velocity, planes);
    }
    if (!clipVelocity && dt > 0) {
      this.velocity = [(this.position[0] - start[0]) / dt, (this.position[1] - start[1]) / dt, (this.position[2] - start[2]) / dt];
    }
  }
}

export const moverCapsule = CAPSULE;
