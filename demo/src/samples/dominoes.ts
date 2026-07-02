import * as THREE from "three";
import type { Box3DRuntime } from "box3d-wasm";
import type { DemoBody, DemoSample } from "./types";

const DOMINO_COUNT = 30 * 181;
const dummy = new THREE.Object3D();
const awakeColor = new THREE.Color(0xd2b48c);
const sleepColor = new THREE.Color(0x778899);

export const dominoesSample: DemoSample = {
  id: "dominoes",
  name: "Stacking / Dominoes",
  create(runtime: Box3DRuntime, scene: THREE.Scene) {
    const world = runtime.createWorld({ gravity: [0, -9.81, 0] });
    const bodies: DemoBody[] = [];

    const groundGeom = new THREE.BoxGeometry(160, 2, 160);
    const groundMat = new THREE.MeshStandardMaterial({ color: 0x222222 });
    const groundMesh = new THREE.Mesh(groundGeom, groundMat);
    groundMesh.position.set(0, -1, 0);
    groundMesh.receiveShadow = true;
    scene.add(groundMesh);

    const groundBody = world.createBody({ type: 0, position: [0, -1, 0] });
    runtime.createHullShape(groundBody, [80, 1, 80]);
    bodies.push({ handle: groundBody, mesh: groundMesh, type: 0 });

    const geometry = new THREE.BoxGeometry(0.4, 1.6, 0.1);
    const material = new THREE.MeshStandardMaterial({ color: 0xfbbf24, roughness: 0.75 });
    const mesh = new THREE.InstancedMesh(geometry, material, DOMINO_COUNT);
    mesh.castShadow = true;
    mesh.receiveShadow = true;
    scene.add(mesh);

    let idx = 0;
    for (let ring = 0; ring < 30; ring++) {
      const radius = 7.0 + 1.1 * ring;
      for (let deg = 0; deg <= 360; deg += 2) {
        const rad = deg * Math.PI / 180;
        const cs = Math.cos(rad);
        const sn = Math.sin(rad);
        const px = radius * cs - (deg / 630) * cs;
        const pz = radius * sn - (deg / 630) * sn;
        const p: [number, number, number] = [px, 0.8, pz];
        const bodyHandle = world.createBody({ type: 2, position: p, rotation: [0, -Math.sin(rad / 2), 0, Math.cos(rad / 2)], isAwake: true });
        runtime.createHullShape(bodyHandle, [0.2, 0.8, 0.05]);
        dummy.position.set(p[0], p[1], p[2]);
        dummy.quaternion.set(0, -Math.sin(rad / 2), 0, Math.cos(rad / 2));
        dummy.updateMatrix();
        mesh.setMatrixAt(idx, dummy.matrix);
        mesh.setColorAt(idx, awakeColor);
        bodies.push({ handle: bodyHandle, mesh, type: 2 });
        if (Math.abs(deg) < 0.1) {
          world.applyLinearImpulse(bodyHandle, [0, 0, 25], [p[0], p[1] + 0.8, p[2]]);
        }
        idx++;
      }
    }
    mesh.instanceMatrix.needsUpdate = true;
    mesh.instanceColor!.needsUpdate = true;

    const dominoCount = idx;

    // Pre-allocate WASM buffers for batched transform + awake read
    // Positions: count * 3 floats, Rotations: count * 4 floats, Awake: count bytes
    const mod = (runtime as any).module as any;
    const bodyHandlesPtr = mod._malloc(dominoCount * 4);
    const outPositionsPtr = mod._malloc(dominoCount * 3 * 4);
    const outRotationsPtr = mod._malloc(dominoCount * 4 * 4);
    const outAwakePtr = mod._malloc(dominoCount);

    const heap32 = new Int32Array(mod.HEAPF32.buffer);
    // Fill body handles array
    for (let i = 0; i < dominoCount; i++) {
      heap32[(bodyHandlesPtr >> 2) + i] = bodies[i + 1].handle;
    }

    // Awake state cache: track last seen awake flag per body
    const awakeCache = new Uint8Array(dominoCount);

    return {
      world,
      bodies,
      controls: [],
      profile: true,
      info: `30 rings of dominoes`,
      _profileTick: 0,
      step(dt) {
        world.step(dt, 4);

        // Single WASM call for all domino transforms + awake state
        world.writeBodyTransforms(dominoCount, bodyHandlesPtr, outPositionsPtr, outRotationsPtr, outAwakePtr);

        const positions = new Float32Array(mod.HEAPF32.buffer, outPositionsPtr, dominoCount * 3);
        const rotations = new Float32Array(mod.HEAPF32.buffer, outRotationsPtr, dominoCount * 4);
        const awake = new Uint8Array(mod.HEAPF32.buffer, outAwakePtr, dominoCount);

        let needsMatrixUpdate = false;
        let needsColorUpdate = false;

        for (let i = 0; i < dominoCount; i++) {
          const pOff = i * 3;
          const rOff = i * 4;
          const isAwake = awake[i] !== 0;
          const wasAwake = awakeCache[i];

          if (wasAwake && !isAwake) {
            // Just went to sleep — update color only
            mesh.setColorAt(i, sleepColor);
            awakeCache[i] = 0;
            needsColorUpdate = true;
            continue;
          }

          if (!wasAwake && isAwake) {
            // Just woke up — update matrix and color
            dummy.position.set(positions[pOff], positions[pOff + 1], positions[pOff + 2]);
            dummy.quaternion.set(rotations[rOff], rotations[rOff + 1], rotations[rOff + 2], rotations[rOff + 3]);
            dummy.updateMatrix();
            mesh.setMatrixAt(i, dummy.matrix);
            mesh.setColorAt(i, awakeColor);
            awakeCache[i] = 1;
            needsMatrixUpdate = true;
            needsColorUpdate = true;
            continue;
          }

          if (isAwake) {
            // Still awake — always update
            dummy.position.set(positions[pOff], positions[pOff + 1], positions[pOff + 2]);
            dummy.quaternion.set(rotations[rOff], rotations[rOff + 1], rotations[rOff + 2], rotations[rOff + 3]);
            dummy.updateMatrix();
            mesh.setMatrixAt(i, dummy.matrix);
            needsMatrixUpdate = true;
          }
        }

        if (needsMatrixUpdate) mesh.instanceMatrix.needsUpdate = true;
        if (needsColorUpdate) mesh.instanceColor!.needsUpdate = true;

        // Sync extra bodies (projectile/ragdoll) individually
        for (let i = dominoCount + 1; i < bodies.length; i++) {
          const t = world.getBodyTransform(bodies[i].handle);
          bodies[i].mesh.position.set(t.position[0], t.position[1], t.position[2]);
          bodies[i].mesh.quaternion.set(t.rotation[0], t.rotation[1], t.rotation[2], t.rotation[3]);
        }

        this._profileTick = (this._profileTick + 1) % 20;
        if (this._profileTick === 0) {
          const p = world.getProfile();
          this.info = `30 rings of dominoes | step ${p.step.toFixed(2)}ms solve ${p.solve.toFixed(2)}ms collide ${p.collide.toFixed(2)}ms pairs ${p.pairs.toFixed(2)}ms`;
        }
      },
      dispose() {
        mod._free(bodyHandlesPtr);
        mod._free(outPositionsPtr);
        mod._free(outRotationsPtr);
        mod._free(outAwakePtr);
        scene.remove(mesh);
        geometry.dispose();
        material.dispose();
        scene.remove(groundMesh);
        groundGeom.dispose();
        groundMat.dispose();
        world.destroy();
      },
    };
  },
};
