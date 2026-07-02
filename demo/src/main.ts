import * as THREE from "three";
import { OrbitControls } from "three/addons/controls/OrbitControls.js";
import Stats from "stats.js";
import { Box3DRuntime } from "box3d-wasm";
import { wasmBuildVersion } from "virtual:wasm-version";
import { samples, type ControlSpec, type DemoBody, type DemoSampleInstance } from "./samples";
import "./style.css";

const app = document.querySelector<HTMLDivElement>("#app");
if (app === null) throw new Error("App container not found");

app.innerHTML = `
  <div class="topbar">
    <div class="topbar-left">
      <span class="sample-name" id="status">Loading...</span>
    </div>
    <div class="topbar-right">
      <button class="controls-toggle" id="controls-toggle" title="Toggle controls panel">Controls</button>
      <span class="sep">&bull;</span>
      <span class="keys">P</span>
      <span class="sep">&bull;</span>
      <span class="keys">.</span>
      <span class="sep">&bull;</span>
      <span class="keys">R</span>
      <span class="sep">&bull;</span>
      <span class="keys">[</span> <span class="keys">]</span>
      <span class="sep">&bull;</span>
      <span class="keys">Shift+Click</span>
      <div class="samples-dropdown">
        <button class="samples-btn" id="samples-toggle">Samples \u25be</button>
        <div class="samples-panel" id="sample-list"></div>
      </div>
    </div>
  </div>
  <div class="samples-panel" id="sample-list" style="display:none"></div>
  <div class="controls-panel" id="controls"></div>
  <div class="controls-dialog" id="controls-dialog" style="display:none">
    <div class="controls-dialog-header" id="controls-dialog-header">Controls <span class="controls-dialog-close" id="controls-dialog-close">&times;</span></div>
    <div class="controls-dialog-body">
      <div class="controls-dialog-section">
        <div class="controls-dialog-section-title">Keyboard</div>
        <table>
          <tr><td class="cd-key">Tab</td><td>Show / hide UI</td></tr>
          <tr><td class="cd-key">M</td><td>Show / hide diagnostics</td></tr>
          <tr><td class="cd-key">P</td><td>Pause / resume</td></tr>
          <tr><td class="cd-key">O</td><td>Single step (Shift: 5)</td></tr>
          <tr><td class="cd-key">R</td><td>Restart sample</td></tr>
          <tr><td class="cd-key">[  ]</td><td>Previous / next sample</td></tr>
          <tr><td class="cd-key">Ctrl+O</td><td>Open sample picker</td></tr>
          <tr><td class="cd-key">F</td><td>Frame selection / world</td></tr>
          <tr><td class="cd-key">?</td><td>Show / hide controls</td></tr>
          <tr><td class="cd-key">Esc</td><td>Cancel / close</td></tr>
          <tr><td class="cd-key">Ctrl+Q</td><td>Quit</td></tr>
        </table>
      </div>
      <div class="controls-dialog-section">
        <div class="controls-dialog-section-title">Mouse</div>
        <table>
          <tr><td class="cd-key">Left click</td><td>Select body/shape</td></tr>
          <tr><td class="cd-key">Ctrl + left drag</td><td>Move bodies (mouse joint)</td></tr>
          <tr><td class="cd-key">Alt + left drag</td><td>Orbit camera</td></tr>
          <tr><td class="cd-key">Alt + middle drag</td><td>Pan camera</td></tr>
          <tr><td class="cd-key">Alt + right drag</td><td>Zoom (dolly)</td></tr>
          <tr><td class="cd-key">Right drag</td><td>Fly look (WASD to move)</td></tr>
          <tr><td class="cd-key">Scroll</td><td>Zoom</td></tr>
          <tr><td class="cd-key">Shift + left</td><td>Shoot (Ctrl spin, Alt ragdoll)</td></tr>
        </table>
      </div>
    </div>
  </div>
  <div class="metrics-bar">
    <div class="left" id="metrics"></div>
    <div class="right" id="info"></div>
  </div>
  <canvas id="view"></canvas>
`;

const canvas = document.querySelector<HTMLCanvasElement>("#view");
const status = document.querySelector<HTMLDivElement>("#status");
const sampleList = document.querySelector<HTMLDivElement>("#sample-list");
const controls = document.querySelector<HTMLDivElement>("#controls");
const metrics = document.querySelector<HTMLDivElement>("#metrics");
const info = document.querySelector<HTMLDivElement>("#info");
const samplesToggle = document.querySelector<HTMLButtonElement>("#samples-toggle");
const controlsToggle = document.querySelector<HTMLButtonElement>("#controls-toggle");
const controlsDialog = document.querySelector<HTMLDivElement>("#controls-dialog");
const controlsDialogHeader = document.querySelector<HTMLDivElement>("#controls-dialog-header");
const controlsDialogClose = document.querySelector<HTMLSpanElement>("#controls-dialog-close");
if (!canvas || !status || !sampleList || !controls || !metrics || !info || !samplesToggle || !controlsToggle || !controlsDialog || !controlsDialogHeader || !controlsDialogClose) {
  throw new Error("Required demo elements are missing");
}
const statusLabel = status;
const sampleListElement = sampleList;
const controlsElement = controls;
const metricsElement = metrics;
const infoElement = info;
const samplesBtn = samplesToggle;
const controlsBtn = controlsToggle;

const renderer = new THREE.WebGLRenderer({ canvas, antialias: true });
renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
renderer.shadowMap.enabled = true;
renderer.shadowMap.type = THREE.PCFSoftShadowMap;

const scene = new THREE.Scene();
scene.background = new THREE.Color(0x0b1220);

const camera = new THREE.PerspectiveCamera(50, window.innerWidth / window.innerHeight, 0.1, 1000);
camera.position.set(14, 10, 14);

const orbit = new OrbitControls(camera, canvas);
orbit.target.set(0, 0, 0);
orbit.enableDamping = true;
orbit.mouseButtons.LEFT = THREE.MOUSE.ROTATE;
orbit.mouseButtons.MIDDLE = THREE.MOUSE.PAN;
orbit.mouseButtons.RIGHT = THREE.MOUSE.DOLLY;

scene.add(new THREE.HemisphereLight(0xe0f2fe, 0x0f172a, 1.15));
const sun = new THREE.DirectionalLight(0xffffff, 2.5);
sun.position.set(10, 18, 12);
sun.castShadow = true;
sun.shadow.mapSize.set(2048, 2048);
scene.add(sun);

const stats = new Stats();
stats.showPanel(0);
stats.dom.style.position = "fixed";
stats.dom.style.right = "10px";
stats.dom.style.bottom = "26px";
stats.dom.style.top = "auto";
stats.dom.style.left = "auto";
stats.dom.style.zIndex = "20";
stats.dom.style.pointerEvents = "none";
stats.dom.style.opacity = "0.5";
document.body.appendChild(stats.dom);

let runtime: Box3DRuntime | null = null;
let activeSampleIndex = 0;
let activeSample: DemoSampleInstance | null = null;
let rafId = 0;
let lastTime = 0;
let launchSpeed = 5.0;
let paused = false;
let singleStep = 0;
let controlsVisible = true;
let showControlsDialog = false;
let selectedBody: DemoBody | null = null;
let mouseDragBody = 0;
let mouseDragJoint = 0;
let mouseDragDistance = 0;
let flyLook = false;
let flyPointerId = -1;
let flySpeed = 8;
const flyKeys = new Set<string>();
const raycaster = new THREE.Raycaster();
const pointerNdc = new THREE.Vector2();
let lastPointerDown: PointerEvent | null = null;
type RagdollBone = {
  name: string;
  parent: number;
  position: [number, number, number];
  rotation: [number, number, number, number];
  a: [number, number, number];
  b: [number, number, number];
  radius: number;
  color: number;
  joint?:
    | { kind: "spherical"; localFrameA: { position: [number, number, number]; rotation: [number, number, number, number] }; localFrameB: { position: [number, number, number]; rotation: [number, number, number, number] }; coneAngle: number; lowerTwist: number; upperTwist: number; hertz: number; damping: number; maxTorque: number }
    | { kind: "revolute"; localFrameA: { position: [number, number, number]; rotation: [number, number, number, number] }; localFrameB: { position: [number, number, number]; rotation: [number, number, number, number] }; lowerAngle: number; upperAngle: number; hertz: number; damping: number; maxTorque: number };
};

const RAGDOLL_BONES: RagdollBone[] = [
  { name: "pelvis", parent: -1, position: [0, 0.932087, -0.051708], rotation: [0.739169, 0, 0, 0.67352], a: [0.07, 0, -0.08], b: [-0.07, 0, -0.08], radius: 0.13, color: 0x1e90ff },
  { name: "spine_01", parent: 0, position: [0, 1.113505, -0.03481], rotation: [0.739973, 0, 0, 0.672637], a: [0.06, 0, -0.052264], b: [-0.06, 0, -0.052264], radius: 0.12, color: 0x7dd3fc, joint: { kind: "spherical", localFrameA: { position: [0, 0, -0.182204], rotation: [-0.999999, 0, 0, 0.001194] }, localFrameB: { position: [0, 0, -0.007736], rotation: [-1, 0, 0, 0] }, coneAngle: 25 * Math.PI / 180, lowerTwist: -15 * Math.PI / 180, upperTwist: 15 * Math.PI / 180, hertz: 1, damping: 1, maxTorque: 1 } },
  { name: "spine_02", parent: 1, position: [0, 1.194336, -0.027087], rotation: [0.703611, 0, 0, 0.710586], a: [0.08, -0.015133, -0.091801], b: [-0.08, -0.015133, -0.091801], radius: 0.1, color: 0x7dd3fc, joint: { kind: "spherical", localFrameA: { position: [0, 0, -0.088935], rotation: [-0.998619, 0, 0, -0.05254] }, localFrameB: { position: [0, 0, -0.008199], rotation: [-1, 0, 0, 0] }, coneAngle: 25 * Math.PI / 180, lowerTwist: -15 * Math.PI / 180, upperTwist: 15 * Math.PI / 180, hertz: 1, damping: 1, maxTorque: 1 } },
  { name: "spine_03", parent: 2, position: [0, 1.31043, -0.028232], rotation: [0.669856, 0.000001, -0.000001, 0.742491], a: [0.11, -0.039753, -0.13], b: [-0.11, -0.039753, -0.13], radius: 0.145, color: 0x7dd3fc, joint: { kind: "spherical", localFrameA: { position: [0, 0, -0.124298], rotation: [-0.998921, 0.000001, -0.000001, -0.046434] }, localFrameB: { position: [0, 0, 0], rotation: [-1, 0, 0, 0] }, coneAngle: 15 * Math.PI / 180, lowerTwist: -10 * Math.PI / 180, upperTwist: 10 * Math.PI / 180, hertz: 1, damping: 1, maxTorque: 1 } },
  { name: "neck", parent: 3, position: [0, 1.575582, -0.055837], rotation: [0.879922, 0, 0, 0.475118], a: [-0.000001, 0, -0.02], b: [0, -0.005, -0.08], radius: 0.07, color: 0xffdead, joint: { kind: "spherical", localFrameA: { position: [0.000001, -0.000259, -0.266585], rotation: [-0.942192, -0.000001, 0, 0.335074] }, localFrameB: { position: [0, 0, 0], rotation: [-1, 0, 0, 0] }, coneAngle: 45 * Math.PI / 180, lowerTwist: -15 * Math.PI / 180, upperTwist: 15 * Math.PI / 180, hertz: 1, damping: 1, maxTorque: 0.8 } },
  { name: "head", parent: 4, position: [0, 1.653348, -0.003241], rotation: [0.750288, 0, 0, 0.661111], a: [-0.000001, 0.016892, -0.05869], b: [0, -0.003629, -0.115072], radius: 0.0975, color: 0xffdead, joint: { kind: "spherical", localFrameA: { position: [0, 0.001321, -0.093873], rotation: [-0.974301, 0, 0, -0.225251] }, localFrameB: { position: [0, 0.001268, -0.005104], rotation: [-1, 0, 0, 0] }, coneAngle: 15 * Math.PI / 180, lowerTwist: -15 * Math.PI / 180, upperTwist: 15 * Math.PI / 180, hertz: 1, damping: 1, maxTorque: 0.4 } },
  { name: "thigh_l", parent: 0, position: [0.090416, 0.986104, -0.03509], rotation: [-0.703287, -0.070715, 0.053866, 0.705327], a: [0.023719, 0.006008, -0.039068], b: [-0.064492, -0.004664, -0.424718], radius: 0.09, color: 0x1e90ff, joint: { kind: "spherical", localFrameA: { position: [0.05, 0.011537, -0.055325], rotation: [-0.714896, -0.022305, -0.698361, -0.02679] }, localFrameB: { position: [0, 0, 0], rotation: [-0.002064, 0.758987, 0.017046, 0.65088] }, coneAngle: 10 * Math.PI / 180, lowerTwist: -60 * Math.PI / 180, upperTwist: 40 * Math.PI / 180, hertz: 1, damping: 1, maxTorque: 1 } },
  { name: "calf_l", parent: 6, position: [0.101198, 0.527027, -0.037374], rotation: [-0.653328, -0.06686, 0.058582, 0.751838], a: [0.001778, 0, 0.009841], b: [-0.078577, 0.014707, -0.41816], radius: 0.075, color: 0x1e90ff, joint: { kind: "revolute", localFrameA: { position: [-0.069989, 0.000253, -0.453844], rotation: [-0.000677, 0.760087, 0.105674, 0.641171] }, localFrameB: { position: [0, 0, 0], rotation: [-0.044589, 0.76554, 0.053368, 0.639619] }, lowerAngle: -5 * Math.PI / 180, upperAngle: 45 * Math.PI / 180, hertz: 1, damping: 1, maxTorque: 1 } },
  { name: "thigh_r", parent: 0, position: [-0.090416, 0.986104, -0.03509], rotation: [-0.703287, 0.070715, -0.053865, 0.705326], a: [-0.023719, 0.006008, -0.039068], b: [0.064492, -0.004664, -0.424718], radius: 0.09, color: 0x1e90ff, joint: { kind: "spherical", localFrameA: { position: [-0.05, 0.011537, -0.055326], rotation: [-0.039089, -0.714094, 0.043177, 0.697623] }, localFrameB: { position: [0, 0, 0], rotation: [0.758805, -0.019886, -0.651012, -0.001759] }, coneAngle: 10 * Math.PI / 180, lowerTwist: -30 * Math.PI / 180, upperTwist: 60 * Math.PI / 180, hertz: 1, damping: 1, maxTorque: 1 } },
  { name: "calf_r", parent: 8, position: [-0.101198, 0.527027, -0.037373], rotation: [-0.653327, 0.06686, -0.058582, 0.751839], a: [-0.00182, 0, 0.010071], b: [0.077883, 0.014825, -0.418047], radius: 0.075, color: 0x1e90ff, joint: { kind: "revolute", localFrameA: { position: [0.069988, 0.000253, -0.453844], rotation: [0.760086, -0.000675, -0.641171, -0.105676] }, localFrameB: { position: [0, 0, 0], rotation: [0.76554, -0.044589, -0.639619, -0.053368] }, lowerAngle: -45 * Math.PI / 180, upperAngle: 5 * Math.PI / 180, hertz: 1, damping: 1, maxTorque: 1 } },
  { name: "upper_arm_l", parent: 3, position: [0.20378, 1.484275, -0.115897], rotation: [0.143082, 0.69598, -0.69013, 0.13733], a: [0, 0, 0], b: [-0.091118, 0.037775, 0.229719], radius: 0.075, color: 0x7dd3fc, joint: { kind: "spherical", localFrameA: { position: [0.20378, -0.069369, -0.181921], rotation: [-0.278486, 0.4456, -0.097014, 0.845266] }, localFrameB: { position: [0, 0, 0], rotation: [-0.201396, -0.001586, 0.90185, 0.382234] }, coneAngle: 60 * Math.PI / 180, lowerTwist: -5 * Math.PI / 180, upperTwist: 5 * Math.PI / 180, hertz: 1, damping: 1, maxTorque: 1 } },
  { name: "lower_arm_l", parent: 10, position: [0.305614, 1.242908, -0.117599], rotation: [0.165048, 0.563437, -0.802002, 0.109959], a: [0, 0, 0], b: [-0.142406, 0.039392, 0.261092], radius: 0.05, color: 0xffdead, joint: { kind: "revolute", localFrameA: { position: [-0.095482, 0.039584, 0.240723], rotation: [0.512487, -0.180629, 0.839474, 0.003742] }, localFrameB: { position: [0, 0, 0], rotation: [0.503803, -0.029831, 0.858168, 0.094017] }, lowerAngle: -5 * Math.PI / 180, upperAngle: 60 * Math.PI / 180, hertz: 1, damping: 1, maxTorque: 1 } },
  { name: "upper_arm_r", parent: 3, position: [-0.20378, 1.484276, -0.115899], rotation: [0.143083, -0.695978, 0.690132, 0.137329], a: [0, 0, 0], b: [0.091118, 0.037775, 0.229718], radius: 0.075, color: 0x7dd3fc, joint: { kind: "spherical", localFrameA: { position: [-0.203779, -0.069371, -0.181922], rotation: [-0.253621, -0.414842, 0.106962, 0.867261] }, localFrameB: { position: [0, 0, 0], rotation: [-0.201397, 0.001587, -0.90185, 0.382233] }, coneAngle: 60 * Math.PI / 180, lowerTwist: -5 * Math.PI / 180, upperTwist: 5 * Math.PI / 180, hertz: 1, damping: 1, maxTorque: 1 } },
  { name: "lower_arm_r", parent: 12, position: [-0.305614, 1.242907, -0.117599], rotation: [0.165048, -0.563437, 0.802002, 0.109959], a: [0, 0, 0], b: [0.142406, 0.039392, 0.261092], radius: 0.05, color: 0xffdead, joint: { kind: "revolute", localFrameA: { position: [0.095484, 0.039585, 0.240723], rotation: [-0.180627, 0.512487, -0.003744, -0.839474] }, localFrameB: { position: [0, 0, 0], rotation: [-0.029831, 0.503803, -0.094017, -0.858169] }, lowerAngle: -60 * Math.PI / 180, upperAngle: 5 * Math.PI / 180, hertz: 1, damping: 1, maxTorque: 1 } },
];

function ragdollCapsuleMesh(a: [number, number, number], b: [number, number, number], radius: number, color: number): THREE.Mesh {
  const va = new THREE.Vector3(...a);
  const vb = new THREE.Vector3(...b);
  const delta = vb.clone().sub(va);
  const length = delta.length();
  const geom = new (THREE as any).CapsuleGeometry(radius, length, 6, 12) as THREE.BufferGeometry;
  geom.applyQuaternion(new THREE.Quaternion().setFromUnitVectors(new THREE.Vector3(0, 1, 0), delta.normalize()));
  geom.translate((a[0] + b[0]) * 0.5, (a[1] + b[1]) * 0.5, (a[2] + b[2]) * 0.5);
  const mesh = new THREE.Mesh(geom, new THREE.MeshStandardMaterial({ color, roughness: 0.75 }));
  mesh.castShadow = true;
  mesh.receiveShadow = true;
  return mesh;
}

function spawnRagdoll(origin: THREE.Vector3, velocity: THREE.Vector3): void {
  if (runtime === null || activeSample === null) return;
  const humanHandle = activeSample.world.createHuman([origin.x, origin.y, origin.z], {
    frictionTorque: 1,
    hertz: 1,
    dampingRatio: 1,
    groupIndex: 0,
    colorize: true,
  });
  if (humanHandle === 0) return;

  runtime.setHumanBullet(humanHandle, true);
  runtime.setHumanVelocity(humanHandle, [velocity.x, velocity.y, velocity.z]);

  const boneCount = Math.min(runtime.getHumanBoneCount(), RAGDOLL_BONES.length);
  for (let i = 0; i < boneCount; i++) {
    const bone = RAGDOLL_BONES[i];
    const handle = runtime.getHumanBoneBody(humanHandle, i);
    if (handle === 0) continue;
    const mesh = ragdollCapsuleMesh(bone.a, bone.b, bone.radius, bone.color);
    scene.add(mesh);
    activeSample.bodies.push({ handle, mesh, type: 2, preserveColor: true });
  }
}

function spawnProjectile(spin = false, ragdoll = false): void {
  if (runtime === null || activeSample === null) return;
  const dir = new THREE.Vector3();
  if (lastPointerDown !== null) {
    setPointerFromEvent(lastPointerDown);
    raycaster.setFromCamera(pointerNdc, camera);
    dir.copy(raycaster.ray.direction).normalize();
  } else {
    camera.getWorldDirection(dir).normalize();
  }
  const origin = lastPointerDown === null
    ? camera.position.clone().add(dir.clone().multiplyScalar(2))
    : raycaster.ray.origin.clone().addScaledVector(raycaster.ray.direction, 2);

  if (ragdoll) {
    spawnRagdoll(origin, dir.clone().multiplyScalar(10 * launchSpeed));
    return;
  }

  const speed = 20 * launchSpeed * (spin ? 2.5 : 1);
  const color = spin ? 0x8b5cf6 : 0xf59e0b;

  if (spin) {
    const bodyHandle = activeSample.world.createBody({ type: 2, position: [origin.x, origin.y, origin.z] });
    runtime.createCapsuleShape(bodyHandle, [0, -0.2, 0], [0, 0.2, 0], 0.08, { density: 6 });
    runtime.setBodyLinearVelocity(bodyHandle, [dir.x * speed, dir.y * speed, dir.z * speed]);
    runtime.setBodyAngularVelocity(bodyHandle, [dir.x * 30, dir.y * 30, dir.z * 30]);
    const mesh = new THREE.Mesh(
      new THREE.CylinderGeometry(0.08, 0.08, 0.4, 8),
      new THREE.MeshStandardMaterial({ color, roughness: 0.4, metalness: 0.5 }),
    );
    mesh.castShadow = true;
    scene.add(mesh);
    activeSample.bodies.push({ handle: bodyHandle, mesh, type: 2 });
  } else {
    const bodyHandle = runtime.createSphere(activeSample.world.handle, {
      radius: 0.25,
      position: [origin.x, origin.y, origin.z],
      velocity: [dir.x * speed, dir.y * speed, dir.z * speed],
      density: 4,
    });
    const mesh = new THREE.Mesh(
      new THREE.SphereGeometry(0.25, 16, 12),
      new THREE.MeshStandardMaterial({ color, roughness: 0.6 }),
    );
    mesh.castShadow = true;
    scene.add(mesh);
    activeSample.bodies.push({ handle: bodyHandle, mesh, type: 2 });
  }
}

function clearScene(): void {
  activeSample?.dispose();
  activeSample = null;
}

function renderControls(specs: ControlSpec[]): void {
  controlsElement.innerHTML = "";

  const launchRow = document.createElement("div");
  launchRow.className = "ctrl-row";
  launchRow.innerHTML = `<div class="ctrl-header"><span>Launch Speed</span><span class="ctrl-value">${launchSpeed.toFixed(2)}</span></div>`;
  const launchInput = document.createElement("input");
  launchInput.type = "range";
  launchInput.min = "0";
  launchInput.max = "20";
  launchInput.step = "0.25";
  launchInput.value = String(launchSpeed);
  launchInput.addEventListener("input", () => {
    launchSpeed = Number(launchInput.value);
    const valueLabel = launchRow.querySelector<HTMLSpanElement>(".ctrl-value");
    if (valueLabel !== null) valueLabel.textContent = launchSpeed.toFixed(2);
  });
  launchRow.appendChild(launchInput);
  controlsElement.appendChild(launchRow);

  for (const spec of specs) {
    const row = document.createElement("div");
    row.className = "ctrl-row";
    row.innerHTML = `<div class="ctrl-header"><span>${spec.label}</span><span class="ctrl-value">${spec.value.toFixed(2)}</span></div>`;
    const input = document.createElement("input");
    input.type = "range";
    input.min = String(spec.min);
    input.max = String(spec.max);
    input.step = String(spec.step);
    input.value = String(spec.value);
    input.addEventListener("input", () => {
      const value = Number(input.value);
      const valueLabel = row.querySelector<HTMLSpanElement>(".ctrl-value");
      if (valueLabel !== null) valueLabel.textContent = value.toFixed(2);
      spec.onChange?.(value);
    });
    row.appendChild(input);
    controlsElement.appendChild(row);
  }

  if (activeSample?.info) {
    const infoRow = document.createElement("div");
    infoRow.className = "info-text";
    infoRow.textContent = activeSample.info;
    controlsElement.appendChild(infoRow);
  }

  if (controlsVisible) {
    controlsElement.classList.remove("hidden");
  } else {
    controlsElement.classList.add("hidden");
  }
}

function updateStatus(): void {
  const name = samples[activeSampleIndex].name;
  const icon = paused ? "\u23f8" : "";
  const parts = name.split(" / ");
  const crumbs = ["Box3D", ...parts];
  statusLabel.innerHTML = icon + crumbs.map((c) => `<span class="crumb">${c}</span>`).join('<span class="sep">/</span>');
  statusLabel.className = "sample-name" + (paused ? " paused" : "");
}

function updateMetrics(): void {
  if (activeSample === null) return;
  const c = activeSample.world.getCounters();
  metricsElement.textContent = `Body:${c.bodyCount}  Shape:${c.shapeCount}  Contact:${c.contactCount}  Joint:${c.jointCount}  Island:${c.islandCount}  Tree:${c.treeHeight}  Static:${c.staticTreeHeight}`;
}

function frameCamera(): void {
  if (activeSample === null) return;
  const box = new THREE.Box3();
  let hasBody = false;
  for (const b of activeSample.bodies) {
    const worldPos = new THREE.Vector3();
    b.mesh.getWorldPosition(worldPos);
    box.expandByPoint(worldPos);
    hasBody = true;
  }
  if (!hasBody) return;
  const size = box.getSize(new THREE.Vector3());
  const center = box.getCenter(new THREE.Vector3());
  const maxDim = Math.max(size.x, size.y, size.z, 1);
  const dist = maxDim * 2.5;
  camera.position.set(center.x + dist * 0.7, center.y + dist * 0.5, center.z + dist * 0.7);
  orbit.target.copy(center);
  orbit.update();
}

function setPointerFromEvent(e: PointerEvent): void {
  const rect = canvas!.getBoundingClientRect();
  pointerNdc.x = ((e.clientX - rect.left) / rect.width) * 2 - 1;
  pointerNdc.y = -(((e.clientY - rect.top) / rect.height) * 2 - 1);
}

function bodyFromPointer(e: PointerEvent): { body: DemoBody; point: THREE.Vector3 } | null {
  if (activeSample === null) return null;
  setPointerFromEvent(e);
  raycaster.setFromCamera(pointerNdc, camera);
  const meshes = activeSample.bodies.map((b) => b.mesh);
  const hits = raycaster.intersectObjects(meshes, false);
  if (hits.length === 0) return null;
  const hit = hits[0];
  const body = activeSample.bodies.find((b) => b.mesh === hit.object);
  return body === undefined ? null : { body, point: hit.point };
}

function setSelectedBody(body: DemoBody | null): void {
  if (selectedBody !== null) {
    const mat = selectedBody.mesh.material as THREE.MeshStandardMaterial;
    mat.emissive.setHex(0x000000);
  }
  selectedBody = body;
  if (selectedBody !== null) {
    const mat = selectedBody.mesh.material as THREE.MeshStandardMaterial;
    mat.emissive.setHex(0x24364a);
  }
}

function pointOnPickRay(e: PointerEvent, distance: number): THREE.Vector3 {
  setPointerFromEvent(e);
  raycaster.setFromCamera(pointerNdc, camera);
  return raycaster.ray.origin.clone().addScaledVector(raycaster.ray.direction, distance);
}

function startMouseDrag(e: PointerEvent): boolean {
  if (activeSample === null) return false;
  const hit = bodyFromPointer(e);
  if (hit === null || hit.body.type !== 2) return false;
  setSelectedBody(hit.body);
  mouseDragDistance = camera.position.distanceTo(hit.point);
  mouseDragBody = activeSample.world.createBody({ type: 1, position: [hit.point.x, hit.point.y, hit.point.z] });
  const localBodyPoint = activeSample.world.getBodyLocalPoint(hit.body.handle, [hit.point.x, hit.point.y, hit.point.z]);
  mouseDragJoint = activeSample.world.createMotorJoint(mouseDragBody, hit.body.handle, {
    localFrameA: [0, 0, 0],
    localFrameB: localBodyPoint,
    linearHertz: 8,
    linearDampingRatio: 1,
    maxSpringForce: 1500,
    angularHertz: 4,
    angularDampingRatio: 1,
    maxSpringTorque: 80,
  });
  canvas!.setPointerCapture(e.pointerId);
  return true;
}

function updateMouseDrag(e: PointerEvent): void {
  if (activeSample === null || mouseDragBody === 0) return;
  const p = pointOnPickRay(e, mouseDragDistance);
  activeSample.world.setBodyTransform(mouseDragBody, [p.x, p.y, p.z]);
}

function stopMouseDrag(): void {
  if (activeSample === null) return;
  if (mouseDragJoint !== 0) {
    activeSample.world.destroyJoint(mouseDragJoint);
    mouseDragJoint = 0;
  }
  if (mouseDragBody !== 0) {
    activeSample.world.destroyBody(mouseDragBody);
    mouseDragBody = 0;
  }
}

function startFlyLook(e: PointerEvent): void {
  flyLook = true;
  flyPointerId = e.pointerId;
  orbit.enabled = false;
  camera.rotation.order = "YXZ";
  canvas!.setPointerCapture(e.pointerId);
}

function stopFlyLook(): void {
  flyLook = false;
  flyPointerId = -1;
  orbit.enabled = true;
}

function updateFlyTarget(): void {
  const forward = new THREE.Vector3();
  camera.getWorldDirection(forward);
  orbit.target.copy(camera.position).addScaledVector(forward, 10);
}

function updateFlyLook(e: PointerEvent): void {
  if (!flyLook || e.pointerId !== flyPointerId) return;
  camera.rotation.y -= e.movementX * 0.003;
  camera.rotation.x = Math.max(-Math.PI * 0.49, Math.min(Math.PI * 0.49, camera.rotation.x - e.movementY * 0.003));
  updateFlyTarget();
}

function updateFlyMovement(dt: number): void {
  if (!flyLook || flyKeys.size === 0) return;
  const forward = new THREE.Vector3();
  const right = new THREE.Vector3();
  camera.getWorldDirection(forward);
  right.setFromMatrixColumn(camera.matrix, 0);
  const delta = new THREE.Vector3();
  if (flyKeys.has("w")) delta.add(forward);
  if (flyKeys.has("s")) delta.sub(forward);
  if (flyKeys.has("d")) delta.add(right);
  if (flyKeys.has("a")) delta.sub(right);
  if (delta.lengthSq() === 0) return;
  delta.normalize().multiplyScalar(flySpeed * dt);
  camera.position.add(delta);
  orbit.target.add(delta);
}

function activateSample(index: number): void {
  if (runtime === null) return;
  clearScene();
  activeSampleIndex = index;
  activeSample = samples[index].create(runtime, scene);
  launchSpeed = activeSample.launchSpeed ?? 5.0;
  paused = false;
  singleStep = 0;
  renderControls(activeSample.controls);
  infoElement.textContent = "";
  updateStatus();
  renderSamples();
  const url = new URL(window.location.href);
  url.searchParams.set("sample", samples[index].id);
  history.replaceState(null, "", url);
}

function renderSamples(): void {
  const groups = new Map<string, { index: number; label: string }[]>();
  for (let i = 0; i < samples.length; i++) {
    const parts = samples[i].name.split(" / ");
    const group = parts.length > 1 ? parts[0] : "General";
    const label = parts.length > 1 ? parts.slice(1).join(" / ") : parts[0];
    if (!groups.has(group)) groups.set(group, []);
    groups.get(group)!.push({ index: i, label });
  }

  sampleListElement.innerHTML = "";

  let first = true;
  for (const [group, items] of groups) {
    if (!first) {
      const hr = document.createElement("div");
      hr.style.borderTop = "1px solid rgba(255,255,255,0.06)";
      hr.style.margin = "2px 8px";
      sampleListElement.appendChild(hr);
    }
    first = false;

    const header = document.createElement("div");
    header.className = "sample-menu-header";
    header.textContent = group;
    sampleListElement.appendChild(header);

    for (const item of items) {
      const btn = document.createElement("button");
      btn.className = "sample-menu-item" + (item.index === activeSampleIndex ? " active" : "");
      btn.textContent = item.label;
      btn.dataset.index = String(item.index);
      btn.addEventListener("click", () => {
        activateSample(item.index);
        sampleListElement.style.display = "none";
      });
      sampleListElement.appendChild(btn);
    }
  }
}

function resize(): void {
  renderer.setSize(window.innerWidth, window.innerHeight, false);
  camera.aspect = window.innerWidth / window.innerHeight;
  camera.updateProjectionMatrix();
}

function frame(time: number): void {
  stats.begin();
  const dt = lastTime === 0 ? 1 / 60 : Math.min((time - lastTime) / 1000, 1 / 30);
  lastTime = time;
  if (!paused || singleStep > 0) {
    activeSample?.step(dt);
    if (singleStep > 0) singleStep--;
  }
  updateMetrics();
  updateFlyMovement(dt);
  orbit.update();
  renderer.render(scene, camera);
  stats.end();
  rafId = window.requestAnimationFrame(frame);
}

window.addEventListener("resize", resize);
resize();
renderSamples();

function toggleSamples(): void {
  const shown = sampleListElement.classList.toggle("open");
  samplesBtn.classList.toggle("open", shown);
}

samplesBtn.addEventListener("click", toggleSamples);

statusLabel.addEventListener("click", toggleSamples);

controlsBtn.addEventListener("click", () => {
  controlsVisible = !controlsVisible;
  controlsElement.classList.toggle("hidden");
  controlsBtn.textContent = controlsVisible ? "Controls" : "Controls";
});

let dragStart = { x: 0, y: 0 };
let dragOffset = { x: 0, y: 0 };
let dragging = false;

controlsDialogHeader.addEventListener("pointerdown", (e) => {
  if (e.target === controlsDialogClose) return;
  dragging = true;
  dragStart.x = e.clientX;
  dragStart.y = e.clientY;
  const rect = controlsDialog.getBoundingClientRect();
  dragOffset.x = e.clientX - rect.left;
  dragOffset.y = e.clientY - rect.top;
  controlsDialogHeader.setPointerCapture(e.pointerId);
});

controlsDialogHeader.addEventListener("pointermove", (e) => {
  if (!dragging) return;
  controlsDialog.style.left = `${e.clientX - dragOffset.x}px`;
  controlsDialog.style.top = `${e.clientY - dragOffset.y}px`;
  controlsDialog.style.right = "auto";
  controlsDialog.style.bottom = "auto";
});

controlsDialogHeader.addEventListener("pointerup", () => {
  dragging = false;
});

controlsDialogClose.addEventListener("pointerdown", (e) => {
  e.stopPropagation();
});

controlsDialogClose.addEventListener("pointerup", (e) => {
  e.stopPropagation();
});

controlsDialogClose.addEventListener("click", () => {
  showControlsDialog = false;
  controlsDialog.style.display = "none";
});

function toggleControlsDialog(): void {
  if (!controlsDialog) return;
  showControlsDialog = !showControlsDialog;
  controlsDialog.style.display = showControlsDialog ? "block" : "none";
  if (showControlsDialog) {
    const halfW = controlsDialog.offsetWidth * 0.5;
    const halfH = controlsDialog.offsetHeight * 0.5;
    controlsDialog.style.left = `${window.innerWidth * 0.5 - halfW}px`;
    controlsDialog.style.top = `${window.innerHeight * 0.35 - halfH}px`;
    controlsDialog.style.right = "auto";
    controlsDialog.style.bottom = "auto";
    dragOffset.x = halfW;
    dragOffset.y = 0;
  }
}

toggleControlsDialog();

canvas.addEventListener("pointerdown", (e) => {
  lastPointerDown = e;
  if (e.shiftKey && e.button === 0) {
    e.preventDefault();
    e.stopImmediatePropagation();
    spawnProjectile(e.ctrlKey, e.altKey);
    return;
  }
  if (e.ctrlKey && e.button === 0) {
    if (startMouseDrag(e)) {
      e.preventDefault();
      e.stopImmediatePropagation();
    }
    return;
  }
  if (e.button === 0 && !e.altKey) {
    const hit = bodyFromPointer(e);
    setSelectedBody(hit?.body ?? null);
    e.preventDefault();
    e.stopImmediatePropagation();
    return;
  }
  if (e.button === 2 && !e.altKey) {
    e.preventDefault();
    e.stopImmediatePropagation();
    startFlyLook(e);
    return;
  }
  if (!e.altKey) {
    e.preventDefault();
    e.stopImmediatePropagation();
  }
}, true);

canvas.addEventListener("pointermove", (e) => {
  if (mouseDragBody !== 0) {
    e.preventDefault();
    e.stopImmediatePropagation();
    updateMouseDrag(e);
    return;
  }
  if (flyLook) {
    e.preventDefault();
    e.stopImmediatePropagation();
    updateFlyLook(e);
  }
}, true);

canvas.addEventListener("pointerup", (e) => {
  if (mouseDragBody !== 0) {
    e.preventDefault();
    e.stopImmediatePropagation();
    stopMouseDrag();
  }
  if (flyLook && e.pointerId === flyPointerId) {
    e.preventDefault();
    e.stopImmediatePropagation();
    stopFlyLook();
  }
}, true);

canvas.addEventListener("wheel", (e) => {
  if (!flyLook) return;
  e.preventDefault();
  e.stopImmediatePropagation();
  flySpeed = Math.max(1, flySpeed + Math.sign(e.deltaY) * -1);
}, { capture: true, passive: false });

canvas.addEventListener("contextmenu", (e) => {
  e.preventDefault();
});

window.addEventListener("keydown", (e) => {
  const key = e.key.toLowerCase();
  if (key === "w" || key === "a" || key === "s" || key === "d") {
    flyKeys.add(key);
  }
  if (e.key === "?") {
    e.preventDefault();
    toggleControlsDialog();
  } else if (e.key === "Escape") {
    if (showControlsDialog) {
      showControlsDialog = false;
      controlsDialog.style.display = "none";
    } else if (sampleListElement.classList.contains("open")) {
      sampleListElement.classList.remove("open");
      samplesBtn.classList.remove("open");
    }
  } else if (e.ctrlKey && e.key === "o") {
    e.preventDefault();
    toggleSamples();
  } else if (e.ctrlKey && e.key === "q") {
    e.preventDefault();
    window.close();
  } else if (e.key === "f" || e.key === "F") {
    if (!e.ctrlKey) {
      frameCamera();
    }
  } else if (e.key === "p" || e.key === "P") {
    paused = !paused;
    singleStep = 0;
    updateStatus();
  } else if (e.key === "o" || e.key === "O") {
    singleStep += e.shiftKey ? 5 : 1;
    if (paused) updateStatus();
  } else if (e.key === ".") {
    singleStep += e.shiftKey ? 5 : 1;
    if (paused) updateStatus();
  } else if (e.key === "r" || e.key === "R") {
    activateSample(activeSampleIndex);
  } else if (e.key === "[") {
    const prev = (activeSampleIndex - 1 + samples.length) % samples.length;
    activateSample(prev);
  } else if (e.key === "]") {
    const next = (activeSampleIndex + 1) % samples.length;
    activateSample(next);
  } else if (e.key === "Tab") {
    e.preventDefault();
    const topbar = document.querySelector<HTMLDivElement>(".topbar");
    const metricsBar = document.querySelector<HTMLDivElement>(".metrics-bar");
    if (topbar) topbar.style.display = topbar.style.display === "none" ? "" : "none";
    if (metricsBar) metricsBar.style.display = metricsBar.style.display === "none" ? "" : "none";
  } else if (e.key === "m" || e.key === "M") {
    metricsElement.style.display = metricsElement.style.display === "none" ? "" : "none";
  }
});

window.addEventListener("keyup", (e) => {
  flyKeys.delete(e.key.toLowerCase());
});

statusLabel.textContent = "Loading...";
runtime = await Box3DRuntime.load({ version: wasmBuildVersion });

const urlSampleId = new URL(window.location.href).searchParams.get("sample");
const initialIndex = urlSampleId ? samples.findIndex((s) => s.id === urlSampleId) : -1;
activateSample(initialIndex >= 0 ? initialIndex : 0);

rafId = window.requestAnimationFrame(frame);

window.addEventListener("beforeunload", () => {
  window.cancelAnimationFrame(rafId);
  clearScene();
  runtime?.destroy();
});
