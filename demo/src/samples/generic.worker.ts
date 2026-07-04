import { PhysicsWorkerBase } from "../physics-worker-base";
import type { Quat, Vec3 } from "box3d-wasm";

export type GenericSceneId = "compound-simple" | "compound-material-dedup" | "single-box" | "cylinder" | "sphere-stack" | "box-stack" | "shapes-inclined-plane" | "card-house-thick" | "jenga-stack" | "pyramid2d" | "capsule-stack";

type Init = { sceneId: GenericSceneId };

const PI = Math.PI;

function groundSize(sceneId: GenericSceneId): Vec3 {
  switch (sceneId) {
    case "single-box": return [20, 1, 20];
    case "compound-material-dedup": return [12, 0.5, 12];
    case "cylinder": return [10, 1, 10];
    case "sphere-stack": return [15, 1, 15];
    case "box-stack": return [40, 1, 40];
    case "shapes-inclined-plane": return [50, 1, 50];
    case "card-house-thick": return [10, 1, 10];
    case "jenga-stack": return [30, 1, 30];
    case "pyramid2d": return [40, 1, 40];
    case "capsule-stack": return [20, 1, 20];
    case "compound-simple": return [20, 1, 20];
  }
}

class GenericSampleWorker extends PhysicsWorkerBase<Init> {
  protected getGroundSize(initData: Init): Vec3 {
    return groundSize(initData.sceneId);
  }

  private addBox(handles: number[], position: Vec3, halfWidths: Vec3, rotation: Quat = [0, 0, 0, 1], options: { type?: number; friction?: number; restitution?: number; rollingResistance?: number; density?: number; lock2d?: boolean; angularVelocity?: Vec3 } = {}): number {
    const body = this.world!.createBody({ type: options.type ?? 2, position, rotation, isAwake: true, angularVelocity: options.angularVelocity });
    this.runtime!.createHullShape(body, halfWidths, { friction: options.friction, restitution: options.restitution, rollingResistance: options.rollingResistance, density: options.density });
    if (options.lock2d) this.runtime!.setBodyMotionLocks(body, { lockLinearZ: true, lockRotationX: true, lockRotationY: true });
    handles.push(body);
    return body;
  }

  protected async buildScene(initData: Init): Promise<number[]> {
    const handles: number[] = [];
    switch (initData.sceneId) {
      case "single-box": {
        this.addBox(handles, [0, 0.5, 0], [0.5, 0.5, 0.5], [0, 0, 0, 1], { angularVelocity: [0, 10, 0] });
        break;
      }
      case "compound-material-dedup": {
        this.addBox(handles, [-2, 4, 0], [1, 1, 1], [0, 0, 0, 1], { friction: 0.3, density: 3000 });
        this.addBox(handles, [2, 4, 0], [1, 1, 1], [0, 0, 0, 1], { restitution: 0.5 });
        break;
      }
      case "compound-simple": {
        const q: Quat = [0, Math.sin(PI / 8), 0, Math.cos(PI / 8)];
        this.addBox(handles, [3, -1.5, 0], [4, 0.5, 4], q, { type: 0, friction: 0.5 });
        const sphere = this.world!.createBody({ type: 2, position: [0, 2, 0], isAwake: true });
        this.runtime!.createSphereShape(sphere, [0, 0, 0], 0.25);
        handles.push(sphere);
        break;
      }
      case "cylinder": {
        const hull = this.runtime!.createCylinder(1, 0.25, -0.5, 12);
        const body = this.world!.createBody({ type: 2, position: [0, 2, 0], isAwake: true });
        this.runtime!.createShapeFromHull(body, hull, { density: 1000, rollingResistance: 0.05 });
        this.runtime!.destroyHull(hull);
        handles.push(body);
        break;
      }
      case "sphere-stack": {
        for (let i = 0, y = 0.75; i < 30; i++, y += 1.5) {
          const body = this.world!.createBody({ type: 2, position: [0, y, 0], isAwake: true });
          this.runtime!.createSphereShape(body, [0, 0, 0], 0.5, { rollingResistance: 0.1 });
          handles.push(body);
        }
        break;
      }
      case "box-stack": {
        for (let i = 0; i < 40; i++) this.addBox(handles, [0, 0.75 + 1.25 * i, 0], [0.5, 0.5, 0.5], [0, 0, 0, 1], { rollingResistance: 0.1 });
        break;
      }
      case "shapes-inclined-plane": {
        const angle = 40 * PI / 180;
        this.addBox(handles, [0, 7.5, -5], [16, 0.5, 10], [Math.sin(angle / 2), 0, 0, Math.cos(angle / 2)], { type: 0, friction: 1 });
        for (let i = 0; i < 5; i++) this.addBox(handles, [-10 + 5 * i, 15.75, -10.6], [1, 1, 1], [0, 0, 0, 1], { friction: (i + 1) * (i + 1) * 0.04 });
        break;
      }
      case "card-house-thick": {
        const alpha = 25 * PI / 180;
        const offsetX = 0.5 * 0.98 * Math.sin(alpha) + 0.045;
        const offsetY = 0.5 * 0.98 * Math.cos(alpha) + 0.035;
        const addPair = (startX: number, y: number, count: number) => {
          for (let j = 0; j < count; j++) {
            for (const sign of [-1, 1]) this.addBox(handles, [startX + sign * offsetX, y, 0], [0.04, 0.49, 0.19], [0, 0, Math.sin(sign * alpha / 2), Math.cos(sign * alpha / 2)], { friction: 0.8 });
            startX += 4 * offsetX;
          }
        };
        const addRow = (startX: number, y: number, count: number) => {
          for (let i = 0; i < count; i++) this.addBox(handles, [startX + i * 4 * offsetX, y, 0], [0.04, 0.49, 0.19], [0, 0, Math.sin(PI / 4), Math.cos(PI / 4)], { friction: 0.8 });
        };
        addPair(-6 * offsetX, offsetY, 4); addRow(-4 * offsetX, 2 * offsetY + 0.04, 3); addPair(-4 * offsetX, 3 * offsetY + 0.08, 3); addRow(-2 * offsetX, 4 * offsetY + 0.12, 2); addPair(-2 * offsetX, 5 * offsetY + 0.16, 2); addRow(0, 6 * offsetY + 0.20, 1); addPair(0, 7 * offsetY + 0.24, 1);
        break;
      }
      case "jenga-stack": {
        for (let i = 0; i < 24; i++) {
          const even = (i & 1) === 0;
          const alpha = even ? 0.5 * PI : 0;
          const q: Quat = [0, Math.sin(alpha / 2), 0, Math.cos(alpha / 2)];
          const x = even ? 1.75 : 0;
          const z = even ? 0 : 1.75;
          this.addBox(handles, [x, 0.5 * i + 0.25, z], [2.5, 0.25, 0.25], q);
          this.addBox(handles, [-x, 0.5 * i + 0.25, -z], [2.5, 0.25, 0.25], q);
        }
        break;
      }
      case "pyramid2d": {
        for (let row = 0; row < 12; row++) {
          for (let col = 0; col < 12 - row; col++) this.addBox(handles, [(-10 + 2 * col + row), 1.5 + 2.5 * row, 0], [1, 1, 1], [0, 0, 0, 1], { lock2d: true });
        }
        break;
      }
      case "capsule-stack": {
        for (let i = 0, y = 0.75; i < 20; i++, y += 1) {
          const body = this.world!.createBody({ type: 2, position: [0, y, 0], isAwake: true });
          this.runtime!.createCapsuleShape(body, [-1, 0, 0], [1, 0, 0], 0.5);
          this.runtime!.setBodyMotionLocks(body, { lockLinearZ: true, lockRotationX: true, lockRotationY: true, lockRotationZ: true });
          handles.push(body);
        }
        break;
      }
    }
    return handles;
  }
}

new GenericSampleWorker();
