import * as THREE from "three";
import { OrbitControls } from "three/addons/controls/OrbitControls.js";
import Stats from "stats.js";
import { BodyType, Box3DRuntime, type Quat, type Vec3 } from "box3d-wasm";
import { wasmBuildVersion } from "virtual:wasm-version";
import "./style.css";

type BodyEntry = { handle: number; mesh: THREE.Mesh; isStatic: boolean };

type AppState = {
  renderer: THREE.WebGLRenderer;
  scene: THREE.Scene;
  camera: THREE.PerspectiveCamera;
  controls: OrbitControls;
  runtime: Box3DRuntime;
  world: ReturnType<Box3DRuntime["createWorld"]>;
  stats: Stats;
  bodies: BodyEntry[];
  dragBody: BodyEntry | null;
  dragMouseHandle: number | null;
  dragJointHandle: number | null;
  rafId: number;
  clock: THREE.Clock;
  accumulator: number;
};

type WasmUpdatePayload = {
  version: string;
};

const app = document.querySelector<HTMLDivElement>("#app");
if (app === null) throw new Error("App container not found");

app.innerHTML = `
  <div class="hud">
    <div class="title">Box3D in the browser</div>
    <div class="copy">Drag dynamic bodies. Orbit empty space. Space fires spheres.</div>
    <div class="status" id="status">Loading wasm...</div>
  </div>
  <canvas id="view"></canvas>
`;

const canvas = document.querySelector<HTMLCanvasElement>("#view");
const status = document.querySelector<HTMLDivElement>("#status");
if (canvas === null || status === null) throw new Error("Required demo elements are missing");
const viewCanvas = canvas;
const statusLabel = status;

const raycaster = new THREE.Raycaster();
const pointer = new THREE.Vector2();
const hoverMarker = new THREE.Mesh(
  new THREE.SphereGeometry(0.075, 16, 12),
  new THREE.MeshBasicMaterial({ color: 0xffb703 }),
);
const grabMarker = new THREE.Mesh(
  new THREE.SphereGeometry(0.04, 16, 12),
  new THREE.MeshBasicMaterial({ color: 0xffffff, transparent: true, opacity: 0.8 }),
);
hoverMarker.visible = false;
grabMarker.visible = false;

const dragPlane = new THREE.Plane();
const dragPoint = new THREE.Vector3();
const grabLocalPoint = new THREE.Vector3();
const hiddenGrabPoint = new THREE.Vector3(0, -1000, 0);

const stats = new Stats();
stats.showPanel(0);
stats.dom.style.position = "fixed";
stats.dom.style.right = "0";
stats.dom.style.top = "0";
stats.dom.style.left = "auto";
stats.dom.style.zIndex = "20";
stats.dom.style.pointerEvents = "none";
stats.dom.querySelectorAll("div").forEach((node) => {
  const element = node as HTMLElement;
  element.style.left = "auto";
  element.style.right = "0";
});
document.body.appendChild(stats.dom);

let state: AppState | null = null;

function updatePointer(clientX: number, clientY: number): void {
  const rect = viewCanvas.getBoundingClientRect();
  pointer.x = ((clientX - rect.left) / rect.width) * 2 - 1;
  pointer.y = -(((clientY - rect.top) / rect.height) * 2 - 1);
}

function clearHover(): void {
  hoverMarker.visible = false;
}

function pickBody(): { body: BodyEntry; point: THREE.Vector3; normal: THREE.Vector3 } | null {
  const current = state;
  if (current === null) return null;
  raycaster.setFromCamera(pointer, current.camera);
  const hit = raycaster.intersectObjects(
    current.bodies.map((entry) => entry.mesh),
    false,
  )[0];
  if (hit === undefined) return null;
  const body = current.bodies.find((entry) => entry.mesh === hit.object) ?? null;
  if (body === null) return null;
  const normal =
    hit.face?.normal?.clone().transformDirection(hit.object.matrixWorld).normalize() ??
    new THREE.Vector3(0, 1, 0);
  return { body, point: hit.point.clone(), normal };
}

function updateHover(): void {
  const hit = pickBody();
  if (hit === null) {
    clearHover();
    return;
  }
  hoverMarker.visible = true;
  hoverMarker.position.copy(hit.point).addScaledVector(hit.normal, 0.02);
}

function addBox(
  stateValue: AppState,
  size: Vec3,
  position: Vec3,
  isStatic = false,
  color = 0xd1d5db,
): BodyEntry {
  const handle = stateValue.world.createBox({
    size,
    position,
    static: isStatic,
    density: isStatic ? 0 : 1,
  });
  const mesh = new THREE.Mesh(
    new THREE.BoxGeometry(size[0] * 2, size[1] * 2, size[2] * 2),
    new THREE.MeshStandardMaterial({ color, roughness: 0.75, metalness: 0.05 }),
  );
  mesh.castShadow = !isStatic;
  mesh.receiveShadow = true;
  stateValue.scene.add(mesh);
  const entry = { handle, mesh, isStatic };
  stateValue.bodies.push(entry);
  return entry;
}

function addSphere(
  stateValue: AppState,
  radius: number,
  position: Vec3,
  velocity: Vec3,
  density = 1,
): void {
  const handle = stateValue.world.createSphere({ radius, position, velocity, density });
  const mesh = new THREE.Mesh(
    new THREE.SphereGeometry(radius, 24, 16),
    new THREE.MeshStandardMaterial({ color: 0xf8fafc, roughness: 0.55, metalness: 0.12 }),
  );
  mesh.castShadow = true;
  mesh.receiveShadow = true;
  stateValue.scene.add(mesh);
  stateValue.bodies.push({ handle, mesh, isStatic: false });
  stateValue.world.setBodyDamping(handle, 0.35, 0.2);
}

function syncBodies(stateValue: AppState): void {
  for (const entry of stateValue.bodies) {
    const transform: { position: Vec3; rotation: Quat } = stateValue.world.getBodyTransform(
      entry.handle,
    );
    entry.mesh.position.set(transform.position[0], transform.position[1], transform.position[2]);
    entry.mesh.quaternion.set(
      transform.rotation[0],
      transform.rotation[1],
      transform.rotation[2],
      transform.rotation[3],
    );
  }
}

function resize(stateValue: AppState): void {
  stateValue.renderer.setSize(window.innerWidth, window.innerHeight, false);
  stateValue.camera.aspect = window.innerWidth / window.innerHeight;
  stateValue.camera.updateProjectionMatrix();
}

function endDrag(): void {
  const current = state;
  if (current === null) return;
  if (current.dragJointHandle !== null) {
    current.world.destroyJoint(current.dragJointHandle);
    current.dragJointHandle = null;
  }
  if (current.dragMouseHandle !== null) {
    current.world.destroyBody(current.dragMouseHandle);
    current.dragMouseHandle = null;
  }
  current.dragBody = null;
  grabMarker.visible = false;
  grabMarker.position.copy(hiddenGrabPoint);
  current.controls.enabled = true;
  statusLabel.textContent = "Ready";
  clearHover();
}

function onPointerMove(event: PointerEvent): void {
  const current = state;
  if (current === null) return;
  updatePointer(event.clientX, event.clientY);
  updateHover();
  if (current.dragBody === null) return;
  raycaster.setFromCamera(pointer, current.camera);
  if (raycaster.ray.intersectPlane(dragPlane, dragPoint) && current.dragMouseHandle !== null) {
    current.world.setBodyTransform(current.dragMouseHandle, [
      dragPoint.x,
      dragPoint.y,
      dragPoint.z,
    ]);
  }
  grabMarker.position.copy(current.dragBody.mesh.localToWorld(grabLocalPoint.clone()));
}

function onPointerDown(event: PointerEvent): void {
  const current = state;
  if (current === null) return;
  updatePointer(event.clientX, event.clientY);
  const hit = pickBody();
  if (hit === null || hit.body.isStatic) return;
  event.preventDefault();
  event.stopImmediatePropagation();
  current.controls.enabled = false;
  current.dragBody = hit.body;
  dragPlane.setFromNormalAndCoplanarPoint(
    current.camera.getWorldDirection(new THREE.Vector3()),
    hit.point,
  );
  raycaster.setFromCamera(pointer, current.camera);
  raycaster.ray.intersectPlane(dragPlane, dragPoint);

  const dragPointVec: Vec3 = [dragPoint.x, dragPoint.y, dragPoint.z];
  current.dragMouseHandle = current.world.createBody({
    type: BodyType.Kinematic,
    position: dragPointVec,
    enableSleep: false,
    awake: true,
  });
  const localPoint = current.world.getBodyLocalPoint(hit.body.handle, dragPointVec);
  grabLocalPoint.set(localPoint[0], localPoint[1], localPoint[2]);
  grabMarker.visible = true;
  current.dragJointHandle = current.world.createMotorJoint(
    current.dragMouseHandle,
    hit.body.handle,
    {
      localFrameA: [0, 0, 0],
      localFrameB: localPoint,
      linearHertz: 7.5,
      linearDampingRatio: 1.0,
      maxSpringForce: 500.0,
    },
  );
  current.world.setBodyAwake(hit.body.handle, true);
  statusLabel.textContent = "Dragging";
}

function onKeyDown(event: KeyboardEvent): void {
  if (event.code !== "Space") return;
  event.preventDefault();
  const current = state;
  if (current === null) return;
  raycaster.setFromCamera(pointer, current.camera);
  const origin = raycaster.ray.origin;
  const direction = raycaster.ray.direction.clone().normalize();
  const speed = 22;
  const spawnOffset = 1.6;
  addSphere(
    current,
    0.35,
    [
      origin.x + direction.x * spawnOffset,
      origin.y + direction.y * spawnOffset,
      origin.z + direction.z * spawnOffset,
    ],
    [direction.x * speed, direction.y * speed, direction.z * speed],
    10,
  );
}

async function createApp(version: string): Promise<AppState> {
  statusLabel.textContent = `Loading wasm ${version}...`;

  const renderer = new THREE.WebGLRenderer({ canvas: viewCanvas, antialias: true });
  renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
  renderer.shadowMap.enabled = true;
  renderer.shadowMap.type = THREE.PCFSoftShadowMap;

  const scene = new THREE.Scene();
  scene.background = new THREE.Color(0x0b1220);
  scene.add(new THREE.HemisphereLight(0xbcdcff, 0x172033, 1.2));

  const sun = new THREE.DirectionalLight(0xffffff, 3.0);
  sun.position.set(8, 12, 6);
  sun.castShadow = true;
  sun.shadow.mapSize.set(1024, 1024);
  scene.add(sun, new THREE.GridHelper(24, 24, 0x334155, 0x1f2937));

  const groundVisual = new THREE.Mesh(
    new THREE.BoxGeometry(24, 1, 24),
    new THREE.MeshStandardMaterial({ color: 0x111827, roughness: 1 }),
  );
  groundVisual.position.set(0, -1.5, 0);
  groundVisual.receiveShadow = true;
  scene.add(groundVisual, hoverMarker, grabMarker);

  const camera = new THREE.PerspectiveCamera(50, 1, 0.1, 100);
  camera.position.set(9, 7, 10);
  camera.lookAt(0, 1.2, 0);

  const controls = new OrbitControls(camera, viewCanvas);
  controls.enableDamping = true;
  controls.dampingFactor = 0.08;
  controls.target.set(0, 1.2, 0);
  controls.update();

  const runtime = await Box3DRuntime.load({ version });
  const world = runtime.createWorld({ gravity: [0, -9.81, 0] });
  const bodies: BodyEntry[] = [];
  const appState: AppState = {
    renderer,
    scene,
    camera,
    controls,
    runtime,
    world,
    stats,
    bodies,
    dragBody: null,
    dragMouseHandle: null,
    dragJointHandle: null,
    rafId: 0,
    clock: new THREE.Clock(),
    accumulator: 0,
  };

  addBox(appState, [12, 0.5, 12], [0, -0.5, 0], true, 0x1f2937);
  for (let y = 0; y < 3; ++y) {
    for (let x = 0; x < 3; ++x) {
      addBox(
        appState,
        [0.5, 0.5, 0.5],
        [x * 1.05 - 1.05, 1 + y * 1.05, 0],
        false,
        0xf59e0b + x * 0x080808 + y * 0x050505,
      );
    }
  }
  addBox(appState, [0.75, 0.75, 0.75], [-2.5, 7, 0.5], false, 0x7dd3fc);
  addBox(appState, [0.75, 0.75, 0.75], [2.2, 9, -1.5], false, 0xa78bfa);

  resize(appState);
  statusLabel.textContent = "Ready";
  return appState;
}

function destroyApp(): void {
  const current = state;
  if (current === null) return;
  window.removeEventListener("resize", onResize);
  viewCanvas.removeEventListener("pointermove", onPointerMove);
  viewCanvas.removeEventListener("pointerdown", onPointerDown);
  viewCanvas.removeEventListener("pointerup", endDrag);
  viewCanvas.removeEventListener("pointercancel", endDrag);
  window.removeEventListener("keydown", onKeyDown);
  cancelAnimationFrame(current.rafId);
  endDrag();
  current.controls.dispose();
  current.renderer.dispose();
  for (const body of current.bodies) {
    body.mesh.geometry.dispose();
    if (Array.isArray(body.mesh.material))
      body.mesh.material.forEach((material) => material.dispose());
    else body.mesh.material.dispose();
  }
  current.scene.clear();
  state = null;
}

function onResize(): void {
  if (state === null) return;
  resize(state);
}

async function boot(version: string): Promise<void> {
  destroyApp();
  state = await createApp(version);
  window.addEventListener("resize", onResize);
  viewCanvas.addEventListener("pointermove", onPointerMove);
  viewCanvas.addEventListener("pointerdown", onPointerDown);
  viewCanvas.addEventListener("pointerup", endDrag);
  viewCanvas.addEventListener("pointercancel", endDrag);
  window.addEventListener("keydown", onKeyDown);

  const frame = (): void => {
    const current = state;
    if (current === null) return;
    stats.begin();
    const delta = Math.min(current.clock.getDelta(), 0.05);
    current.accumulator += delta;
    while (current.accumulator >= 1 / 60) {
      current.world.step(1 / 60, 4);
      current.accumulator -= 1 / 60;
    }
    syncBodies(current);
    current.controls.update();
    current.renderer.render(current.scene, current.camera);
    stats.end();
    current.rafId = requestAnimationFrame(frame);
  };

  state.rafId = requestAnimationFrame(frame);
}

void boot(wasmBuildVersion);

if (import.meta.hot) {
  import.meta.hot.accept("virtual:wasm-version", async (mod) => {
    await boot(mod?.wasmBuildVersion ?? wasmBuildVersion);
  });
  import.meta.hot.on("box3d-wasm:update", async (payload: WasmUpdatePayload) => {
    await boot(payload.version);
  });
  import.meta.hot.dispose(destroyApp);
}
