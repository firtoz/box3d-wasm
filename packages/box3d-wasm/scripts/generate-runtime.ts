import { mkdir, writeFile } from 'node:fs/promises';
import { dirname, resolve } from 'node:path';

const outFile = resolve(new URL('../generated/index.ts', import.meta.url).pathname);

const source = `export type Vec3 = [number, number, number];
export type Quat = [number, number, number, number];

export enum BodyType {
  Static = 0,
  Kinematic = 1,
  Dynamic = 2,
}

export interface WorldOptions {
  gravity?: Vec3;
}

export interface BodyOptions {
  type?: BodyType;
  position?: Vec3;
  awake?: boolean;
  enableSleep?: boolean;
}

export interface BodyTransform {
  position: Vec3;
  rotation: Quat;
}

export interface WorldAPI {
  createBody(options?: BodyOptions): number;
  destroyBody(bodyHandle: number): void;
  setBodyTransform(bodyHandle: number, position: Vec3, rotation?: Quat): void;
  getBodyTransform(bodyHandle: number): BodyTransform;
  step(dt?: number, substeps?: number): void;
}

export interface RuntimeAPI {
  createWorld(options?: WorldOptions): WorldAPI;
}

class MockWorld implements WorldAPI {
  private nextBodyHandle = 1;
  private readonly bodies = new Map<number, BodyTransform>();

  createBody(options: BodyOptions = {}): number {
    const handle = this.nextBodyHandle;
    this.nextBodyHandle += 1;
    this.bodies.set(handle, {
      position: options.position ?? [0, 0, 0],
      rotation: [0, 0, 0, 1],
    });
    return handle;
  }

  destroyBody(bodyHandle: number): void {
    this.bodies.delete(bodyHandle);
  }

  setBodyTransform(bodyHandle: number, position: Vec3, rotation: Quat = [0, 0, 0, 1]): void {
    this.bodies.set(bodyHandle, { position, rotation });
  }

  getBodyTransform(bodyHandle: number): BodyTransform {
    const body = this.bodies.get(bodyHandle);
    return body ?? { position: [0, 0, 0], rotation: [0, 0, 0, 1] };
  }

  step(_dt = 1 / 60, _substeps = 1): void {
    for (const [handle, body] of this.bodies) {
      if (handle === 1) continue;
      const [x, y, z] = body.position;
      this.bodies.set(handle, { ...body, position: [x, Math.max(-1, y - 0.05), z] });
    }
  }
}

export function createRuntime(): RuntimeAPI {
  return {
    createWorld() {
      return new MockWorld();
    },
  };
}
`;

await mkdir(dirname(outFile), { recursive: true });
await writeFile(outFile, source);
