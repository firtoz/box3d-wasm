import * as THREE from "three";
import { OrbitControls } from "three/addons/controls/OrbitControls.js";
import Stats from "stats.js";
import { BodyType, Box3DRuntime, type Quat, type Vec3 } from "box3d-wasm";
import { samples, type ControlSpec, type DemoBody, type DemoSampleInstance, type SolverParams } from "./samples";
import { getWasmVariant, getWorkerCounts } from "./samples/shared";
import "./style.css";

const app = document.querySelector<HTMLDivElement>("#app");
if (app === null) throw new Error("App container not found");

const wasmVariant = getWasmVariant();
const urlParams = new URL(window.location.href).searchParams;
const benchRunnerMode = window.location.pathname === "/bench";
const benchmarkMode = benchRunnerMode || urlParams.get("bench") === "1";
const SETTINGS_STORAGE_KEY = "box3d-demo-settings";

type PersistedSettings = {
  solver?: Partial<SolverParams>;
  chartsEnabled?: boolean;
  chartHz?: number;
  shadowsEnabled?: boolean;
};

function loadPersistedSettings(): PersistedSettings {
  try {
    const raw = localStorage.getItem(SETTINGS_STORAGE_KEY);
    if (raw === null) return {};
    const parsed = JSON.parse(raw) as PersistedSettings;
    return parsed !== null && typeof parsed === "object" ? parsed : {};
  } catch {
    return {};
  }
}

function savePersistedSettings(settings: PersistedSettings): void {
  try { localStorage.setItem(SETTINGS_STORAGE_KEY, JSON.stringify(settings)); } catch { /* ignore */ }
}

const persistedSettings = loadPersistedSettings();
const defaultChartHz = 15;
const defaultChartsEnabled = !benchmarkMode;
const initialChartsEnabled = urlParams.has("charts") ? urlParams.get("charts") !== "0" : (persistedSettings.chartsEnabled ?? defaultChartsEnabled);
const statsEnabled = urlParams.has("stats") ? urlParams.get("stats") !== "0" : !benchmarkMode;
const metricsEnabled = urlParams.has("metrics") ? urlParams.get("metrics") !== "0" : statsEnabled;
const defaultShadowsEnabled = !benchmarkMode;
let shadowsEnabled = urlParams.has("shadows") ? urlParams.get("shadows") !== "0" : persistedSettings.shadowsEnabled ?? defaultShadowsEnabled;
const timingsEnabled = urlParams.get("timings") === "1";
const chartHzParam = urlParams.get("chartHz");
let chartsEnabled = initialChartsEnabled;
let currentChartHz = chartHzParam !== null ? Math.max(0, Number(chartHzParam) || defaultChartHz) : persistedSettings.chartHz ?? defaultChartHz;
let chartIntervalMs = chartsEnabled && currentChartHz > 0 ? 1000 / currentChartHz : 0;

app.innerHTML = benchRunnerMode ? `<canvas id="view"></canvas>` : `
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
          <tr><td class="cd-key">C</td><td>Toggle simple / detailed color mode</td></tr>
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

function detachedElement<K extends keyof HTMLElementTagNameMap>(tagName: K): HTMLElementTagNameMap[K] {
  return document.createElement(tagName);
}

const canvas = document.querySelector<HTMLCanvasElement>("#view");
if (!canvas) throw new Error("Required demo canvas is missing");
const statusLabel = document.querySelector<HTMLDivElement>("#status") ?? detachedElement("div");
const sampleListElement = document.querySelector<HTMLDivElement>("#sample-list") ?? detachedElement("div");
const controlsElement = document.querySelector<HTMLDivElement>("#controls") ?? detachedElement("div");
const metricsElement = document.querySelector<HTMLDivElement>("#metrics") ?? detachedElement("div");
const infoElement = document.querySelector<HTMLDivElement>("#info") ?? detachedElement("div");
const samplesToggle = document.querySelector<HTMLButtonElement>("#samples-toggle") ?? detachedElement("button");
const controlsToggle = document.querySelector<HTMLButtonElement>("#controls-toggle") ?? detachedElement("button");
const controlsDialog = document.querySelector<HTMLDivElement>("#controls-dialog") ?? detachedElement("div");
const controlsDialogHeader = document.querySelector<HTMLDivElement>("#controls-dialog-header") ?? detachedElement("div");
const controlsDialogClose = document.querySelector<HTMLSpanElement>("#controls-dialog-close") ?? detachedElement("span");
const samplesBtn = samplesToggle;
const controlsBtn = controlsToggle;

const renderer = new THREE.WebGLRenderer({ canvas, antialias: true });
renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
renderer.shadowMap.enabled = shadowsEnabled;
renderer.shadowMap.type = THREE.PCFSoftShadowMap;

const scene = new THREE.Scene();
scene.background = new THREE.Color(benchRunnerMode ? 0x000000 : 0x0b1220);

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
if (statsEnabled) document.body.appendChild(stats.dom);
if (!metricsEnabled) metricsElement.style.display = "none";

const CHART_W = 240;
const CHART_H = 120;
const CHART_LEN = 600;
const CHART_GAP = 8;

const chartDefs = [
  { key: "step" as const, color: "#00ff88", label: "step", desc: "Time Box3D took to run one physics step (ms). Higher = more CPU work." },
  { key: "publish" as const, color: "#88ccff", label: "pub", desc: "Time to copy transforms from wasm heap to shared buffers (ms). Overhead of the JS bridge." },
  { key: "lag" as const, color: "#ffaa00", label: "lag", desc: "Accumulated time debt after catching up (ms). Rises when real-time outruns physics." },
  { key: "dropped" as const, color: "#ff4444", label: "drop", desc: "Time discarded when debt exceeded the max catchup limit (ms). Sim skipped this much wall-clock time." },
];

const chartTooltip = document.createElement("div");
chartTooltip.style.cssText = "position:fixed;z-index:30;pointer-events:none;background:rgba(0,0,0,0.85);color:#eee;font:11px sans-serif;padding:6px 10px;border-radius:4px;display:none;max-width:300px;line-height:1.4";

const chartCtxs: CanvasRenderingContext2D[] = [];
const chartLabels: HTMLSpanElement[] = [];
const chartElements: HTMLElement[] = [];

function showChartTooltip(el: HTMLElement, text: string): void {
  chartTooltip.textContent = text;
  chartTooltip.style.display = "block";
  const r = el.getBoundingClientRect();
  const tw = chartTooltip.offsetWidth;
  chartTooltip.style.left = `${Math.min(r.left, window.innerWidth - tw - 8)}px`;
  chartTooltip.style.top = `${r.bottom + 4}px`;
}
function hideChartTooltip(): void { chartTooltip.style.display = "none"; }

function ensurePhysCharts(): void {
  if (chartCtxs.length > 0) return;
  document.body.appendChild(chartTooltip);
  for (let ci = 0; ci < chartDefs.length; ci++) {
    const def = chartDefs[ci];
    const right = 10 + ci * (CHART_W + CHART_GAP);

    const cv = document.createElement("canvas");
    cv.width = CHART_W;
    cv.height = CHART_H;
    cv.style.cssText = `position:fixed;right:${right}px;bottom:76px;z-index:20;pointer-events:none;opacity:0.7;width:${CHART_W}px;height:${CHART_H}px`;
    document.body.appendChild(cv);
    chartElements.push(cv);
    chartCtxs.push(cv.getContext("2d")!);

    const lb = document.createElement("span");
    lb.style.cssText = `position:fixed;right:${right}px;bottom:210px;z-index:20;pointer-events:none;color:${def.color};font:11px monospace;text-shadow:0 0 3px #000`;
    lb.textContent = `0.0`;
    document.body.appendChild(lb);
    chartElements.push(lb);

    const infoBtn = document.createElement("span");
    infoBtn.textContent = "?";
    infoBtn.style.cssText = `position:fixed;right:${right}px;bottom:228px;z-index:25;color:${def.color};font:10px monospace;cursor:help;opacity:0.6`;
    infoBtn.addEventListener("mouseenter", () => showChartTooltip(infoBtn, def.desc));
    infoBtn.addEventListener("mouseleave", hideChartTooltip);
    document.body.appendChild(infoBtn);
    chartElements.push(infoBtn);
    chartLabels.push(lb);
  }
}

function setPhysChartsVisible(visible: boolean): void {
  if (visible) ensurePhysCharts();
  for (const el of chartElements) el.style.display = visible ? "" : "none";
  if (!visible) hideChartTooltip();
}

function updatePhysChartVisibility(): void {
  setPhysChartsVisible(chartsEnabled && (activeSample?.profile ?? false));
}

if (chartsEnabled) ensurePhysCharts();

let lastChartDrawTime = 0;
let lastProfileSample: { step: number; solve: number; pairs: number; collide: number; solverSetup: number } | null = null;
let timingLastLog = 0;
let timingFrames = 0;
let timingStepMs = 0;
let timingChartsMs = 0;
let timingRenderMs = 0;
let benchTimingActive = false;
const physHistory: { step: number; steps: number; lag: number; dropped: number; publish: number }[] = [];
let physHistoryHead = 0;
let physHistoryFilled = 0;

function pushPhysProfile(p: { step: number; solve: number; pairs: number; collide: number; solverSetup: number }): void {
  if (physHistory.length < CHART_LEN) physHistory.length = CHART_LEN;
  physHistory[physHistoryHead] = { step: p.step, steps: p.solve, lag: p.pairs, dropped: p.collide, publish: p.solverSetup };
  physHistoryHead = (physHistoryHead + 1) % CHART_LEN;
  if (physHistoryFilled < CHART_LEN) physHistoryFilled++;
}

function drawPhysCharts(): void {
  const n = physHistoryFilled;
  if (n < 2) return;
  const pad = 14;
  const bottomPad = 8;
  const leftPad = 4;
  const rightPad = 4;
  const plotW = CHART_W - leftPad - rightPad;
  const plotH = CHART_H - pad - bottomPad;
  const count = Math.min(n, CHART_LEN);
  const xScale = plotW / (CHART_LEN - 1);

  for (let ci = 0; ci < chartDefs.length; ci++) {
    const { key, color, label } = chartDefs[ci];
    const ctx = chartCtxs[ci];
    ctx.clearRect(0, 0, CHART_W, CHART_H);
    ctx.fillStyle = "#000";
    ctx.fillRect(0, 0, CHART_W, CHART_H);

    let minVal = Infinity;
    let maxVal = -Infinity;
    for (let i = 0; i < count; i++) {
      const idx = (physHistoryHead - count + i + CHART_LEN) % CHART_LEN;
      const v = physHistory[idx][key];
      if (v < minVal) minVal = v;
      if (v > maxVal) maxVal = v;
    }
    const range = Math.max(maxVal - minVal, 0.001);
    const padRatio = 0.1;

    ctx.strokeStyle = color;
    ctx.lineWidth = 0.5;
    ctx.beginPath();
    for (let i = 0; i < count; i++) {
      const idx = (physHistoryHead - count + i + CHART_LEN) % CHART_LEN;
      const val = physHistory[idx][key];
      const x = leftPad + i * xScale;
      const y = pad + plotH - ((val - minVal) / range) * (1 - padRatio * 2) * plotH - padRatio * plotH;
      if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
    }
    ctx.stroke();
    ctx.fillStyle = color;
    ctx.font = "10px monospace";
    ctx.fillText(label, 6, 9);
  }
}

const VISIBLE_STORAGE_KEY = "controlsDialogVisible";
const MOUSE_FORCE_SCALE = 100;
const GRAVITY_MAGNITUDE = 9.81;
 
let runtime: Box3DRuntime | null = null;
const workerRuntimePlaceholder = null as unknown as Box3DRuntime;
let activeSampleIndex = 0;
let activeSample: DemoSampleInstance | null = null;
let rafId = 0;
let lastTime = 0;
let launchSpeed = 5.0;
const solverParams: SolverParams = {
  subSteps: 4,
  hertz: 60,
  recycleDistance: 0.05,
  sleep: true,
  warmStart: true,
  continuous: true,
};
const defaultSolverParams: Required<Omit<SolverParams, "workerCount">> = {
  subSteps: 4,
  hertz: 60,
  recycleDistance: 0.05,
  sleep: true,
  warmStart: true,
  continuous: true,
};
Object.assign(solverParams, persistedSettings.solver);
let paused = false;
let singleStep = 0;
let controlsVisible = true;
let showControlsDialog = (() => { try { return localStorage.getItem(VISIBLE_STORAGE_KEY) === "1"; } catch { return false; } })();
let colorMode = localStorage.getItem("box3d:color-mode") === "light" ? "light" : "full";
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
let metricsTick = 0;
let benchCancelRequested = false;
type RagdollBone = {
  name: string;
  parent: number;
  position: Vec3;
  rotation: Quat;
  a: Vec3;
  b: Vec3;
  radius: number;
  color: number;
  joint?:
    | { kind: "spherical"; localFrameA: { position: Vec3; rotation: Quat }; localFrameB: { position: Vec3; rotation: Quat }; coneAngle: number; lowerTwist: number; upperTwist: number; hertz: number; damping: number; maxTorque: number }
    | { kind: "revolute"; localFrameA: { position: Vec3; rotation: Quat }; localFrameB: { position: Vec3; rotation: Quat }; lowerAngle: number; upperAngle: number; hertz: number; damping: number; maxTorque: number };
};

const RAGDOLL_BONES: RagdollBone[] = [
  { name: "pelvis", parent: -1, position: [0, 0.932087, -0.051708], rotation: [0.739169, 0, 0, 0.67352], a: [0.07, 0, -0.08], b: [-0.07, 0, -0.08], radius: 0.13, color: 0x1e90ff },
  { name: "spine_01", parent: 0, position: [0, 1.113505, -0.03481], rotation: [0.739973, 0, 0, 0.672637], a: [0.06, 0, -0.052264], b: [-0.06, 0, -0.052264], radius: 0.12, color: 0x48d1cc, joint: { kind: "spherical", localFrameA: { position: [0, 0, -0.182204], rotation: [-0.999999, 0, 0, 0.001194] }, localFrameB: { position: [0, 0, -0.007736], rotation: [-1, 0, 0, 0] }, coneAngle: 25 * Math.PI / 180, lowerTwist: -15 * Math.PI / 180, upperTwist: 15 * Math.PI / 180, hertz: 1, damping: 1, maxTorque: 1 } },
  { name: "spine_02", parent: 1, position: [0, 1.194336, -0.027087], rotation: [0.703611, 0, 0, 0.710586], a: [0.08, -0.015133, -0.091801], b: [-0.08, -0.015133, -0.091801], radius: 0.1, color: 0x48d1cc, joint: { kind: "spherical", localFrameA: { position: [0, 0, -0.088935], rotation: [-0.998619, 0, 0, -0.05254] }, localFrameB: { position: [0, 0, -0.008199], rotation: [-1, 0, 0, 0] }, coneAngle: 25 * Math.PI / 180, lowerTwist: -15 * Math.PI / 180, upperTwist: 15 * Math.PI / 180, hertz: 1, damping: 1, maxTorque: 1 } },
  { name: "spine_03", parent: 2, position: [0, 1.31043, -0.028232], rotation: [0.669856, 0.000001, -0.000001, 0.742491], a: [0.11, -0.039753, -0.13], b: [-0.11, -0.039753, -0.13], radius: 0.145, color: 0x48d1cc, joint: { kind: "spherical", localFrameA: { position: [0, 0, -0.124298], rotation: [-0.998921, 0.000001, -0.000001, -0.046434] }, localFrameB: { position: [0, 0, 0], rotation: [-1, 0, 0, 0] }, coneAngle: 15 * Math.PI / 180, lowerTwist: -10 * Math.PI / 180, upperTwist: 10 * Math.PI / 180, hertz: 1, damping: 1, maxTorque: 1 } },
  { name: "neck", parent: 3, position: [0, 1.575582, -0.055837], rotation: [0.879922, 0, 0, 0.475118], a: [-0.000001, 0, -0.02], b: [0, -0.005, -0.08], radius: 0.07, color: 0xffdead, joint: { kind: "spherical", localFrameA: { position: [0.000001, -0.000259, -0.266585], rotation: [-0.942192, -0.000001, 0, 0.335074] }, localFrameB: { position: [0, 0, 0], rotation: [-1, 0, 0, 0] }, coneAngle: 45 * Math.PI / 180, lowerTwist: -15 * Math.PI / 180, upperTwist: 15 * Math.PI / 180, hertz: 1, damping: 1, maxTorque: 0.8 } },
  { name: "head", parent: 4, position: [0, 1.653348, -0.003241], rotation: [0.750288, 0, 0, 0.661111], a: [-0.000001, 0.016892, -0.05869], b: [0, -0.003629, -0.115072], radius: 0.0975, color: 0xffdead, joint: { kind: "spherical", localFrameA: { position: [0, 0.001321, -0.093873], rotation: [-0.974301, 0, 0, -0.225251] }, localFrameB: { position: [0, 0.001268, -0.005104], rotation: [-1, 0, 0, 0] }, coneAngle: 15 * Math.PI / 180, lowerTwist: -15 * Math.PI / 180, upperTwist: 15 * Math.PI / 180, hertz: 1, damping: 1, maxTorque: 0.4 } },
  { name: "thigh_l", parent: 0, position: [0.090416, 0.986104, -0.03509], rotation: [-0.703287, -0.070715, 0.053866, 0.705327], a: [0.023719, 0.006008, -0.039068], b: [-0.064492, -0.004664, -0.424718], radius: 0.09, color: 0x1e90ff, joint: { kind: "spherical", localFrameA: { position: [0.05, 0.011537, -0.055325], rotation: [-0.714896, -0.022305, -0.698361, -0.02679] }, localFrameB: { position: [0, 0, 0], rotation: [-0.002064, 0.758987, 0.017046, 0.65088] }, coneAngle: 10 * Math.PI / 180, lowerTwist: -60 * Math.PI / 180, upperTwist: 40 * Math.PI / 180, hertz: 1, damping: 1, maxTorque: 1 } },
  { name: "calf_l", parent: 6, position: [0.101198, 0.527027, -0.037374], rotation: [-0.653328, -0.06686, 0.058582, 0.751838], a: [0.001778, 0, 0.009841], b: [-0.078577, 0.014707, -0.41816], radius: 0.075, color: 0x1e90ff, joint: { kind: "revolute", localFrameA: { position: [-0.069989, 0.000253, -0.453844], rotation: [-0.000677, 0.760087, 0.105674, 0.641171] }, localFrameB: { position: [0, 0, 0], rotation: [-0.044589, 0.76554, 0.053368, 0.639619] }, lowerAngle: -5 * Math.PI / 180, upperAngle: 45 * Math.PI / 180, hertz: 1, damping: 1, maxTorque: 1 } },
  { name: "thigh_r", parent: 0, position: [-0.090416, 0.986104, -0.03509], rotation: [-0.703287, 0.070715, -0.053865, 0.705326], a: [-0.023719, 0.006008, -0.039068], b: [0.064492, -0.004664, -0.424718], radius: 0.09, color: 0x1e90ff, joint: { kind: "spherical", localFrameA: { position: [-0.05, 0.011537, -0.055326], rotation: [-0.039089, -0.714094, 0.043177, 0.697623] }, localFrameB: { position: [0, 0, 0], rotation: [0.758805, -0.019886, -0.651012, -0.001759] }, coneAngle: 10 * Math.PI / 180, lowerTwist: -30 * Math.PI / 180, upperTwist: 60 * Math.PI / 180, hertz: 1, damping: 1, maxTorque: 1 } },
  { name: "calf_r", parent: 8, position: [-0.101198, 0.527027, -0.037373], rotation: [-0.653327, 0.06686, -0.058582, 0.751839], a: [-0.00182, 0, 0.010071], b: [0.077883, 0.014825, -0.418047], radius: 0.075, color: 0x1e90ff, joint: { kind: "revolute", localFrameA: { position: [0.069988, 0.000253, -0.453844], rotation: [0.760086, -0.000675, -0.641171, -0.105676] }, localFrameB: { position: [0, 0, 0], rotation: [0.76554, -0.044589, -0.639619, -0.053368] }, lowerAngle: -45 * Math.PI / 180, upperAngle: 5 * Math.PI / 180, hertz: 1, damping: 1, maxTorque: 1 } },
  { name: "upper_arm_l", parent: 3, position: [0.20378, 1.484275, -0.115897], rotation: [0.143082, 0.69598, -0.69013, 0.13733], a: [0, 0, 0], b: [-0.091118, 0.037775, 0.229719], radius: 0.075, color: 0x48d1cc, joint: { kind: "spherical", localFrameA: { position: [0.20378, -0.069369, -0.181921], rotation: [-0.278486, 0.4456, -0.097014, 0.845266] }, localFrameB: { position: [0, 0, 0], rotation: [-0.201396, -0.001586, 0.90185, 0.382234] }, coneAngle: 60 * Math.PI / 180, lowerTwist: -5 * Math.PI / 180, upperTwist: 5 * Math.PI / 180, hertz: 1, damping: 1, maxTorque: 1 } },
  { name: "lower_arm_l", parent: 10, position: [0.305614, 1.242908, -0.117599], rotation: [0.165048, 0.563437, -0.802002, 0.109959], a: [0, 0, 0], b: [-0.142406, 0.039392, 0.261092], radius: 0.05, color: 0xffdead, joint: { kind: "revolute", localFrameA: { position: [-0.095482, 0.039584, 0.240723], rotation: [0.512487, -0.180629, 0.839474, 0.003742] }, localFrameB: { position: [0, 0, 0], rotation: [0.503803, -0.029831, 0.858168, 0.094017] }, lowerAngle: -5 * Math.PI / 180, upperAngle: 60 * Math.PI / 180, hertz: 1, damping: 1, maxTorque: 1 } },
  { name: "upper_arm_r", parent: 3, position: [-0.20378, 1.484276, -0.115899], rotation: [0.143083, -0.695978, 0.690132, 0.137329], a: [0, 0, 0], b: [0.091118, 0.037775, 0.229718], radius: 0.075, color: 0x48d1cc, joint: { kind: "spherical", localFrameA: { position: [-0.203779, -0.069371, -0.181922], rotation: [-0.253621, -0.414842, 0.106962, 0.867261] }, localFrameB: { position: [0, 0, 0], rotation: [-0.201397, 0.001587, -0.90185, 0.382233] }, coneAngle: 60 * Math.PI / 180, lowerTwist: -5 * Math.PI / 180, upperTwist: 5 * Math.PI / 180, hertz: 1, damping: 1, maxTorque: 1 } },
  { name: "lower_arm_r", parent: 12, position: [-0.305614, 1.242907, -0.117599], rotation: [0.165048, -0.563437, 0.802002, 0.109959], a: [0, 0, 0], b: [0.142406, 0.039392, 0.261092], radius: 0.05, color: 0xffdead, joint: { kind: "revolute", localFrameA: { position: [0.095484, 0.039585, 0.240723], rotation: [-0.180627, 0.512487, -0.003744, -0.839474] }, localFrameB: { position: [0, 0, 0], rotation: [-0.029831, 0.503803, -0.094017, -0.858169] }, lowerAngle: -60 * Math.PI / 180, upperAngle: 5 * Math.PI / 180, hertz: 1, damping: 1, maxTorque: 1 } },
];

function ragdollCapsuleMesh(a: Vec3, b: Vec3, radius: number, color: number): THREE.Mesh {
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
    activeSample.bodies.push({ handle, mesh, type: BodyType.Dynamic, preserveColor: true });
  }
}

function spawnProjectile(spin = false, ragdoll = false): void {
  if (activeSample === null) return;
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

  const projectileSpeed = ragdoll ? 10 * launchSpeed : 20 * launchSpeed;
  const velocity: [number, number, number] = [dir.x * projectileSpeed, dir.y * projectileSpeed, dir.z * projectileSpeed];
  const originTuple: [number, number, number] = [origin.x, origin.y, origin.z];

  if (activeSample.spawnProjectile !== undefined) {
    activeSample.spawnProjectile(originTuple, velocity, spin, ragdoll);
    return;
  }

  if (runtime === null) return;

  if (ragdoll) {
    spawnRagdoll(origin, dir.clone().multiplyScalar(10 * launchSpeed));
    return;
  }

  const color = spin ? 0x8b5cf6 : 0xf59e0b;

  const bodyHandle = runtime.createSphere(activeSample.world.handle, {
    radius: 0.25,
    position: originTuple,
    velocity,
    density: 4000,
    isBullet: true,
  });
  const mesh = new THREE.Mesh(
    new THREE.SphereGeometry(0.25, 16, 12),
    new THREE.MeshStandardMaterial({ color, roughness: 0.6 }),
  );
  mesh.castShadow = true;
  scene.add(mesh);
  activeSample.bodies.push({ handle: bodyHandle, mesh, type: BodyType.Dynamic });
}

function clearScene(): void {
  activeSample?.dispose();
  activeSample = null;
  updatePhysChartVisibility();
}

function applySolverParams(): void {
  if (activeSample === null) return;
  if (activeSample.sendSolverParams !== undefined) {
    activeSample.sendSolverParams(solverParams);
    return;
  }
  if (runtime === null) return;
  const w = activeSample.world;
  if (solverParams.recycleDistance !== undefined) runtime.setWorldContactRecycleDistance(w.handle, solverParams.recycleDistance);
  if (solverParams.sleep !== undefined) runtime.enableWorldSleeping(w.handle, solverParams.sleep);
  if (solverParams.continuous !== undefined) runtime.enableWorldContinuous(w.handle, solverParams.continuous);
  if (solverParams.warmStart !== undefined) runtime.enableWorldWarmStarting(w.handle, solverParams.warmStart);
  if (solverParams.workerCount !== undefined) w.setWorkerCount(solverParams.workerCount);
}

function persistSettings(): void {
  savePersistedSettings({ solver: { ...solverParams }, chartsEnabled, chartHz: currentChartHz, shadowsEnabled });
}

function setShadowsEnabled(enabled: boolean): void {
  shadowsEnabled = enabled;
  renderer.shadowMap.enabled = shadowsEnabled;
  persistSettings();
}

function setChartsEnabled(enabled: boolean): void {
  chartsEnabled = enabled;
  chartIntervalMs = chartsEnabled && currentChartHz > 0 ? 1000 / currentChartHz : 0;
  updatePhysChartVisibility();
  persistSettings();
}

function setChartHz(hz: number): void {
  currentChartHz = Math.max(0, hz);
  chartIntervalMs = chartsEnabled && currentChartHz > 0 ? 1000 / currentChartHz : 0;
  lastChartDrawTime = 0;
  persistSettings();
}

function resetSolverParam(key: keyof SolverParams, value: number | boolean | undefined): void {
  if (value === undefined) delete solverParams[key];
  else (solverParams as Record<string, number | boolean>)[key] = value;
  persistSettings();
  if (key === "workerCount") resetScene();
  else applySolverParams();
  renderControls(activeSample?.controls ?? []);
}

function controlNote(text: string): HTMLSpanElement {
  const note = document.createElement("span");
  note.className = "ctrl-note";
  note.textContent = "?";
  note.title = text;
  return note;
}

function addResetButton(row: HTMLElement, show: boolean, onClick: () => void): void {
  if (!show) return;
  const btn = document.createElement("button");
  btn.className = "ctrl-reset";
  btn.type = "button";
  btn.textContent = "Reset";
  btn.addEventListener("click", onClick);
  row.appendChild(btn);
}

function renderSolverControls(): void {
  const { defaultWorkerCount, maxWorkerCount } = getWorkerCounts();
  const sections: { label: string; key: keyof SolverParams; min: number; max: number; step: number; value: number; defaultValue: number; note: string; toParam: (v: number) => void; resetValue?: number | boolean }[] = [
    { label: "Sub-steps", key: "subSteps", min: 1, max: 50, step: 1, value: solverParams.subSteps ?? defaultSolverParams.subSteps, defaultValue: defaultSolverParams.subSteps, note: "Higher values can improve stability but directly increase physics work. For performance, keep this near the default unless a scene needs more stability.", toParam: (v) => { solverParams.subSteps = v; }, resetValue: defaultSolverParams.subSteps },
    { label: "Hertz", key: "hertz", min: 5, max: 240, step: 1, value: solverParams.hertz ?? defaultSolverParams.hertz, defaultValue: defaultSolverParams.hertz, note: "Higher hertz runs physics more often and can cost a lot. 60 Hz is the performance-oriented default.", toParam: (v) => { solverParams.hertz = v; }, resetValue: defaultSolverParams.hertz },
    { label: "Recycle", key: "recycleDistance", min: 0, max: 10, step: 0.1, value: (solverParams.recycleDistance ?? defaultSolverParams.recycleDistance) * 100, defaultValue: defaultSolverParams.recycleDistance * 100, note: "Contact recycling can reduce churn. Very different values may change contact behavior and performance.", toParam: (v) => { solverParams.recycleDistance = v * 0.01; }, resetValue: defaultSolverParams.recycleDistance },
    { label: "Workers", key: "workerCount", min: 1, max: maxWorkerCount, step: 1, value: Math.min(maxWorkerCount, solverParams.workerCount ?? (activeSample?.world.getWorkerCount() ?? defaultWorkerCount)), defaultValue: defaultWorkerCount, note: "More workers can speed heavy scenes, but too many can add scheduling/publish overhead. Best value depends on CPU and scene.", toParam: (v) => { solverParams.workerCount = v; } },
  ];

  const toggles: { label: string; key: "sleep" | "warmStart" | "continuous"; value: boolean; onChange: (v: boolean) => void }[] = [
    { label: "Sleep", key: "sleep", value: solverParams.sleep ?? defaultSolverParams.sleep, onChange: (v) => { solverParams.sleep = v; applySolverParams(); } },
    { label: "Warm Starting", key: "warmStart", value: solverParams.warmStart ?? defaultSolverParams.warmStart, onChange: (v) => { solverParams.warmStart = v; applySolverParams(); } },
    { label: "Continuous", key: "continuous", value: solverParams.continuous ?? defaultSolverParams.continuous, onChange: (v) => { solverParams.continuous = v; applySolverParams(); } },
  ];

  const el = controlsElement;

  const chartTitle = document.createElement("div");
  chartTitle.className = "ctrl-section-title";
  chartTitle.textContent = "Charts";
  el.appendChild(chartTitle);

  const chartToggleRow = document.createElement("div");
  chartToggleRow.className = "ctrl-toggle-row";
  const chartLabel = document.createElement("label");
  chartLabel.className = "ctrl-toggle-label";
  const chartCb = document.createElement("input");
  chartCb.type = "checkbox";
  chartCb.checked = chartsEnabled;
  chartCb.addEventListener("change", () => setChartsEnabled(chartCb.checked));
  chartLabel.appendChild(chartCb);
  chartLabel.appendChild(document.createTextNode(" Physics charts"));
  chartLabel.appendChild(controlNote("Physics charts poll profile data and draw canvases. Turn them off for the cleanest performance; 15 Hz is the default compromise."));
  chartToggleRow.appendChild(chartLabel);
  addResetButton(chartToggleRow, chartsEnabled !== defaultChartsEnabled, () => { setChartsEnabled(defaultChartsEnabled); renderControls(activeSample?.controls ?? []); });
  el.appendChild(chartToggleRow);

  const chartRateRow = document.createElement("div");
  chartRateRow.className = "ctrl-row";
  chartRateRow.innerHTML = `<div class="ctrl-header"><span>Chart Rate</span><span class="ctrl-value">${currentChartHz === 0 ? "Full" : `${currentChartHz} Hz`}</span></div>`;
  chartRateRow.querySelector(".ctrl-header span")?.appendChild(controlNote("Full-rate charting has measurable overhead. 10-15 Hz is usually a good compromise if you want the physics charts visible."));
  const rateSelect = document.createElement("select");
  rateSelect.className = "ctrl-select";
  for (const hz of [0, 30, 15, 10, 5, 1]) {
    const option = document.createElement("option");
    option.value = String(hz);
    option.textContent = hz === 0 ? "Full-rate" : `${hz} Hz`;
    option.selected = currentChartHz === hz;
    rateSelect.appendChild(option);
  }
  rateSelect.addEventListener("change", () => {
    setChartHz(Number(rateSelect.value));
    renderControls(activeSample?.controls ?? []);
  });
  chartRateRow.appendChild(rateSelect);
  addResetButton(chartRateRow, currentChartHz !== defaultChartHz, () => { setChartHz(defaultChartHz); renderControls(activeSample?.controls ?? []); });
  el.appendChild(chartRateRow);

  const wasmRow = document.createElement("div");
  wasmRow.className = "ctrl-row";
  wasmRow.innerHTML = `<div class="ctrl-header"><span>WASM Variant</span><span class="ctrl-value">${wasmVariant}</span></div>`;
  const wasmSelect = document.createElement("select");
  wasmSelect.className = "ctrl-select";
  wasmSelect.id = "wasm-variant";
  for (const v of ["release", "profile"]) {
    const option = document.createElement("option");
    option.value = v;
    option.textContent = v;
    option.selected = wasmVariant === v;
    wasmSelect.appendChild(option);
  }
  wasmSelect.addEventListener("change", () => {
    const url = new URL(window.location.href);
    if (wasmSelect.value === "profile") url.searchParams.set("wasm", "profile");
    else url.searchParams.delete("wasm");
    window.location.href = url.href;
  });
  wasmRow.appendChild(wasmSelect);
  el.appendChild(wasmRow);

  const renderTitle = document.createElement("div");
  renderTitle.className = "ctrl-section-title";
  renderTitle.textContent = "Rendering";
  el.appendChild(renderTitle);

  const shadowsRow = document.createElement("div");
  shadowsRow.className = "ctrl-toggle-row";
  const shadowsLabel = document.createElement("label");
  shadowsLabel.className = "ctrl-toggle-label";
  const shadowsCb = document.createElement("input");
  shadowsCb.type = "checkbox";
  shadowsCb.checked = shadowsEnabled;
  shadowsCb.addEventListener("change", () => {
    setShadowsEnabled(shadowsCb.checked);
    renderControls(activeSample?.controls ?? []);
  });
  shadowsLabel.appendChild(shadowsCb);
  shadowsLabel.appendChild(document.createTextNode(" Shadows"));
  shadowsLabel.appendChild(controlNote("Shadows increase render/GPU work. Turn them off for cleaner performance measurements."));
  shadowsRow.appendChild(shadowsLabel);
  addResetButton(shadowsRow, shadowsEnabled !== defaultShadowsEnabled, () => { setShadowsEnabled(defaultShadowsEnabled); renderControls(activeSample?.controls ?? []); });
  el.appendChild(shadowsRow);

  const colorModeRow = document.createElement("div");
  colorModeRow.className = "ctrl-toggle-row";
  const colorModeLabel = document.createElement("label");
  colorModeLabel.className = "ctrl-toggle-label";
  const colorModeCb = document.createElement("input");
  colorModeCb.type = "checkbox";
  colorModeCb.id = "color-mode-cb";
  colorModeCb.checked = colorMode === "light";
  colorModeCb.addEventListener("change", () => {
    colorMode = colorModeCb.checked ? "light" : "full";
    localStorage.setItem("box3d:color-mode", colorMode);
    activeSample?.onKey?.("c");
  });
  colorModeLabel.appendChild(colorModeCb);
  colorModeLabel.appendChild(document.createTextNode(" Simple colors"));
  colorModeLabel.appendChild(controlNote("Uses simple transform transfers to reduce native WASM calls. Toggle with the C key."));
  colorModeRow.appendChild(colorModeLabel);
  el.appendChild(colorModeRow);

  const title = document.createElement("div");
  title.className = "ctrl-section-title";
  title.textContent = "Solver";
  el.appendChild(title);

  const stepDecimals = (step: number) => Math.max(0, (String(step).split('.')[1]?.length ?? 0));
  for (const s of sections) {
    const row = document.createElement("div");
    row.className = "ctrl-row";
    const isWorkers = s.key === "workerCount";
    const dec = stepDecimals(s.step);
    row.innerHTML = `<div class="ctrl-header"><span>${s.label}</span><span class="ctrl-value">${s.value.toFixed(dec)}</span></div>`;
    row.querySelector(".ctrl-header span")?.appendChild(controlNote(s.note));
    const input = document.createElement("input");
    input.type = "range";
    input.min = String(s.min);
    input.max = String(s.max);
    input.step = String(s.step);
    input.value = String(s.value);
    input.addEventListener("input", () => {
      const v = Number(input.value);
      const vl = row.querySelector<HTMLSpanElement>(".ctrl-value");
      if (vl !== null) vl.textContent = v.toFixed(dec);
      s.toParam(v);
      persistSettings();
      if (!isWorkers) applySolverParams();
    });
    if (isWorkers) {
      input.addEventListener("change", () => resetScene());
    } else {
      input.addEventListener("change", () => renderControls(activeSample?.controls ?? []));
    }
    row.appendChild(input);
    addResetButton(row, s.value !== s.defaultValue, () => resetSolverParam(s.key, s.resetValue));
    el.appendChild(row);
  }

  for (const t of toggles) {
    const row = document.createElement("div");
    row.className = "ctrl-toggle-row";
    const label = document.createElement("label");
    label.className = "ctrl-toggle-label";
    const cb = document.createElement("input");
    cb.type = "checkbox";
    cb.checked = t.value;
    cb.addEventListener("change", () => {
      const newVal = cb.checked;
      solverParams[t.key] = newVal;
      t.onChange(newVal);
      persistSettings();
      renderControls(activeSample?.controls ?? []);
    });
    label.appendChild(cb);
    label.appendChild(document.createTextNode(" " + t.label));
    const note = t.key === "sleep" ? "Sleeping usually helps performance in settled scenes. Turning it off can keep more bodies active." : t.key === "warmStart" ? "Warm starting usually improves solver convergence. Turning it off was slower in washer traces." : "Continuous collision can cost extra. Turning it off may help some scenes but can alter collision quality.";
    label.appendChild(controlNote(note));
    row.appendChild(label);
    addResetButton(row, t.value !== defaultSolverParams[t.key], () => resetSolverParam(t.key, defaultSolverParams[t.key]));
    el.appendChild(row);
  }

  const restartRow = document.createElement("div");
  restartRow.className = "ctrl-row";
  const restartBtn = document.createElement("button");
  restartBtn.className = "ctrl-btn";
  restartBtn.textContent = "Restart";
  restartBtn.addEventListener("click", () => resetScene());
  restartRow.appendChild(restartBtn);
  el.appendChild(restartRow);
}

function renderControls(specs: ControlSpec[]): void {
  controlsElement.innerHTML = "";

  renderSolverControls();

  if (specs.length > 0) {
    const divider = document.createElement("hr");
    divider.className = "ctrl-divider";
    controlsElement.appendChild(divider);
  }

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
    const t = spec.type ?? "range";
    if (t === "button") {
      const row = document.createElement("div");
      row.className = "ctrl-row";
      const btn = document.createElement("button");
      btn.className = "ctrl-btn";
      btn.textContent = spec.label;
      btn.addEventListener("click", () => spec.onClick?.());
      row.appendChild(btn);
      controlsElement.appendChild(row);
      continue;
    }
    if (t === "toggle") {
      const row = document.createElement("div");
      row.className = "ctrl-toggle-row";
      const label = document.createElement("label");
      label.className = "ctrl-toggle-label";
      const cb = document.createElement("input");
      cb.type = "checkbox";
      cb.checked = !!spec.value;
      cb.addEventListener("change", () => {
        const newVal = cb.checked;
        spec.value = newVal;
        spec.onChange?.(newVal);
      });
      label.appendChild(cb);
      label.appendChild(document.createTextNode(" " + spec.label));
      row.appendChild(label);
      controlsElement.appendChild(row);
      continue;
    }
    const row = document.createElement("div");
    row.className = "ctrl-row";
    const val = Number(spec.value);
    row.innerHTML = `<div class="ctrl-header"><span>${spec.label}</span><span class="ctrl-value">${val.toFixed(2)}</span></div>`;
    const input = document.createElement("input");
    input.type = "range";
    input.min = String(spec.min ?? 0);
    input.max = String(spec.max ?? 1);
    input.step = String(spec.step ?? 0.01);
    input.value = String(val);
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
  if (activeSampleIndex < 0) {
    statusLabel.textContent = "Box3D / Washer Bench Runner";
    statusLabel.className = "sample-name";
    return;
  }
  const name = samples[activeSampleIndex].name;
  const icon = paused ? "\u23f8" : "";
  const parts = name.split(" / ");
  const crumbs = ["Box3D", ...parts];
  statusLabel.innerHTML = icon + crumbs.map((c) => `<span class="crumb">${c}</span>`).join('<span class="sep">/</span>');
  statusLabel.className = "sample-name" + (paused ? " paused" : "");
}

function updateMetrics(): void {
  if (!metricsEnabled || metricsElement.style.display === "none") return;
  if (activeSample === null) return;
  metricsTick = (metricsTick + 1) % 15;
  if (metricsTick !== 0) return;
  const c = activeSample.world.getCounters();
  const awake = activeSample.world.getAwakeBodyCount();
  const wc = activeSample.world.getWorkerCount();
  let text = `Body:${c.bodyCount} Awake:${awake} Shape:${c.shapeCount} Contact:${c.contactCount} Joint:${c.jointCount} Island:${c.islandCount} Tree:${c.treeHeight} Static:${c.staticTreeHeight}`;
  if (chartsEnabled && activeSample.profile && lastProfileSample !== null) {
    const p = lastProfileSample;
    text += ` | Phys:${p.step.toFixed(2)}ms Pub:${p.solverSetup.toFixed(2)}ms Steps:${p.solve.toFixed(0)} Lag:${p.pairs.toFixed(1)}ms Drop:${p.collide.toFixed(1)}ms`;
  }
  text += ` | Workers:${wc}`;
  metricsElement.textContent = text;
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
  const origin = [raycaster.ray.origin.x, raycaster.ray.origin.y, raycaster.ray.origin.z] as [number, number, number];
  const translation = [raycaster.ray.direction.x * 1000, raycaster.ray.direction.y * 1000, raycaster.ray.direction.z * 1000] as [number, number, number];
  const hit = activeSample.world.rayCastClosest(origin, translation);
  if (hit === null || hit.bodyHandle === 0) return null;
  const body = activeSample.bodies.find((b) => b.handle === hit.bodyHandle);
  if (body === undefined) return null;
  return { body, point: new THREE.Vector3(hit.point[0], hit.point[1], hit.point[2]) };
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
  if (activeSample.startMouseDragRay !== undefined) {
    setPointerFromEvent(e);
    raycaster.setFromCamera(pointerNdc, camera);
    const origin = [raycaster.ray.origin.x, raycaster.ray.origin.y, raycaster.ray.origin.z] as [number, number, number];
    const translation = [raycaster.ray.direction.x * 1000, raycaster.ray.direction.y * 1000, raycaster.ray.direction.z * 1000] as [number, number, number];
    const started = activeSample.startMouseDragRay(origin, translation);
    if (started) {
      mouseDragBody = -1;
      canvas!.setPointerCapture(e.pointerId);
    }
    return started;
  }
  const hit = bodyFromPointer(e);
  if (hit === null || hit.body.type !== 2) return false;
  setSelectedBody(hit.body);
  mouseDragDistance = camera.position.distanceTo(hit.point);
  mouseDragBody = activeSample.world.createBody({ type: BodyType.Kinematic, position: [hit.point.x, hit.point.y, hit.point.z], enableSleep: false });
  const localBodyPoint = activeSample.world.getBodyLocalPoint(hit.body.handle, [hit.point.x, hit.point.y, hit.point.z]);
  const massData = activeSample.world.getBodyMassData(hit.body.handle);
  const mg = massData.mass * GRAVITY_MAGNITUDE;
  const lever = massData.mass > 0 ? Math.sqrt(massData.inertiaTrace / (3 * massData.mass)) : 0;
  mouseDragJoint = activeSample.world.createMotorJoint(mouseDragBody, hit.body.handle, {
    localFrameA: [0, 0, 0],
    localFrameB: localBodyPoint,
    linearHertz: 7.5,
    linearDampingRatio: 1,
    maxSpringForce: MOUSE_FORCE_SCALE * mg,
    maxVelocityTorque: 0.5 * lever * mg,
  });
  activeSample.world.setBodyAwake(hit.body.handle, true);
  canvas!.setPointerCapture(e.pointerId);
  return true;
}

function updateMouseDrag(e: PointerEvent): void {
  if (activeSample === null || mouseDragBody === 0) return;
  if (mouseDragBody === -1 && activeSample.updateMouseDragRay !== undefined) {
    setPointerFromEvent(e);
    raycaster.setFromCamera(pointerNdc, camera);
    activeSample.updateMouseDragRay(
      [raycaster.ray.origin.x, raycaster.ray.origin.y, raycaster.ray.origin.z],
      [raycaster.ray.direction.x * 1000, raycaster.ray.direction.y * 1000, raycaster.ray.direction.z * 1000],
    );
    return;
  }
  const p = pointOnPickRay(e, mouseDragDistance);
  activeSample.world.setBodyTransform(mouseDragBody, [p.x, p.y, p.z]);
}

function stopMouseDrag(): void {
  if (activeSample === null) return;
  if (mouseDragBody === -1 && activeSample.stopMouseDrag !== undefined) {
    activeSample.stopMouseDrag();
    mouseDragBody = 0;
    return;
  }
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

function resetFrameTimings(): void {
  timingFrames = 0;
  timingStepMs = 0;
  timingChartsMs = 0;
  timingRenderMs = 0;
}

function frameTimingSummary(): { frames: number; sampleStepMs: number; chartsMs: number; renderMs: number } {
  const inv = timingFrames > 0 ? 1 / timingFrames : 0;
  return {
    frames: timingFrames,
    sampleStepMs: timingStepMs * inv,
    chartsMs: timingChartsMs * inv,
    renderMs: timingRenderMs * inv,
  };
}

function resetScene(): void {
  if (activeSampleIndex < 0) return;
  const camPos = camera.position.clone();
  const camTarget = orbit.target.clone();
  clearScene();
  activeSample = samples[activeSampleIndex].create(runtime ?? workerRuntimePlaceholder, scene, solverParams);
  launchSpeed = activeSample.launchSpeed ?? 5.0;
  camera.position.copy(camPos);
  orbit.target.copy(camTarget);
  orbit.update();
  paused = false;
  singleStep = 0;
  lastProfileSample = null;
  renderControls(activeSample.controls);
  if (!benchRunnerMode) applySolverParams();
  infoElement.textContent = "";
  updatePhysChartVisibility();
  updateStatus();
}

function activateSample(index: number): void {
  clearScene();
  activeSampleIndex = index;
  activeSample = samples[index].create(runtime ?? workerRuntimePlaceholder, scene, solverParams);
  launchSpeed = activeSample.launchSpeed ?? 5.0;
  if (activeSample.camera) {
    camera.position.set(activeSample.camera.position[0], activeSample.camera.position[1], activeSample.camera.position[2]);
    orbit.target.set(activeSample.camera.target[0], activeSample.camera.target[1], activeSample.camera.target[2]);
    orbit.update();
  }
  paused = false;
  singleStep = 0;
  lastProfileSample = null;
  if (!benchRunnerMode) renderControls(activeSample.controls);
  if (!benchRunnerMode) applySolverParams();
  activeSample?.setPaused?.(paused);
  infoElement.textContent = "";
  updatePhysChartVisibility();
  if (!benchRunnerMode) {
    updateStatus();
    renderSamples();
    const url = new URL(window.location.href);
    url.searchParams.set("sample", samples[index].id);
    history.replaceState(null, "", url);
  }
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
        sampleListElement.classList.remove("open");
        samplesBtn.classList.remove("open");
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
  if (statsEnabled) stats.begin();
  const collectTimings = timingsEnabled || benchTimingActive;
  const dt = lastTime === 0 ? 1 / 60 : Math.min((time - lastTime) / 1000, 1 / 30);
  lastTime = time;
  const stepStart = collectTimings ? performance.now() : 0;
  let didStep = false;
  let wasSingleStep = false;
  if (!paused) {
    const physDt = 1 / (solverParams.hertz ?? 60);
    activeSample?.step(physDt, solverParams.subSteps ?? 4);
    didStep = true;
  } else if (singleStep > 0) {
    const physDt = 1 / (solverParams.hertz ?? 60);
    if (activeSample?.stepOnce !== undefined) {
      activeSample.stepOnce();
      activeSample.step(physDt, solverParams.subSteps ?? 4);
    } else {
      activeSample?.step(physDt, solverParams.subSteps ?? 4);
    }
    singleStep--;
    didStep = true;
    wasSingleStep = true;
  } else if (activeSample?.stepOnce !== undefined) {
    activeSample.step();
  }
  if (collectTimings) timingStepMs += performance.now() - stepStart;
  const chartStart = collectTimings ? performance.now() : 0;
  const shouldSample = didStep && (wasSingleStep || chartIntervalMs === 0 || time - lastChartDrawTime >= chartIntervalMs);
  if (shouldSample && chartsEnabled && activeSample?.profile) {
    lastProfileSample = activeSample.world.getProfile();
    pushPhysProfile(lastProfileSample);
    ensurePhysCharts();
    chartLabels[0].textContent = `${lastProfileSample.step.toFixed(1)}ms`;
    chartLabels[1].textContent = `${lastProfileSample.solverSetup.toFixed(1)}ms`;
    chartLabels[2].textContent = `${lastProfileSample.pairs.toFixed(1)}ms`;
    chartLabels[3].textContent = `${lastProfileSample.collide.toFixed(1)}ms`;
    drawPhysCharts();
    lastChartDrawTime = time;
  }
  if (collectTimings) timingChartsMs += performance.now() - chartStart;
  if (didStep || !paused) updateMetrics();
  updateFlyMovement(dt);
  orbit.update();
  const renderStart = collectTimings ? performance.now() : 0;
  renderer.render(scene, camera);
  if (collectTimings) {
    timingRenderMs += performance.now() - renderStart;
    timingFrames++;
    if (timingsEnabled && !benchTimingActive && time - timingLastLog >= 1000) {
      const inv = timingFrames > 0 ? 1 / timingFrames : 0;
      console.log(`[perf] frames=${timingFrames} step=${(timingStepMs * inv).toFixed(2)}ms charts=${(timingChartsMs * inv).toFixed(2)}ms render=${(timingRenderMs * inv).toFixed(2)}ms`);
      timingLastLog = time;
      timingFrames = 0;
      timingStepMs = 0;
      timingChartsMs = 0;
      timingRenderMs = 0;
    }
  }
  if (statsEnabled) stats.end();
  rafId = window.requestAnimationFrame(frame);
}

window.addEventListener("resize", resize);
resize();
if (!benchRunnerMode) renderSamples();

type BenchVariant = {
  id: string;
  label: string;
  params?: Partial<SolverParams>;
  shadows?: boolean;
  renderMode?: "matrix" | "shader";
  physicsCharts?: boolean;
  chartHz?: number | null;
};

function benchStamp(label: string): void {
  console.log(`[bench] ${label}`);
  console.timeStamp?.(`[bench] ${label}`);
  performance.mark(`bench:${label}`);
}

function uniqueNumbers(values: number[]): number[] {
  return [...new Set(values.filter((v) => Number.isFinite(v) && v >= 1))];
}

function createBenchVariants(): BenchVariant[] {
  const { defaultWorkerCount, maxWorkerCount } = getWorkerCounts();
  const workerCounts = uniqueNumbers([1, 2, 4, 6, 8, 10, 12, 14, 16, maxWorkerCount]).filter((v) => v <= maxWorkerCount && v !== defaultWorkerCount);
  const baseParams: SolverParams = { workerCount: defaultWorkerCount, subSteps: 4, hertz: 60, recycleDistance: 0.05, continuous: true, warmStart: true, sleep: true };
  return [
    { id: "default-shader", label: `Default shader (${defaultWorkerCount} workers)`, params: { ...baseParams }, shadows: false, renderMode: "shader" },
    { id: "charts-full", label: `Physics charts full-rate (${defaultWorkerCount} workers)`, params: { ...baseParams }, shadows: false, renderMode: "shader", physicsCharts: true, chartHz: null },
    { id: "charts-30hz", label: `Physics charts 30 Hz (${defaultWorkerCount} workers)`, params: { ...baseParams }, shadows: false, renderMode: "shader", physicsCharts: true, chartHz: 30 },
    { id: "charts-15hz", label: `Physics charts 15 Hz (${defaultWorkerCount} workers)`, params: { ...baseParams }, shadows: false, renderMode: "shader", physicsCharts: true, chartHz: 15 },
    { id: "charts-10hz", label: `Physics charts 10 Hz (${defaultWorkerCount} workers)`, params: { ...baseParams }, shadows: false, renderMode: "shader", physicsCharts: true, chartHz: 10 },
    { id: "charts-5hz", label: `Physics charts 5 Hz (${defaultWorkerCount} workers)`, params: { ...baseParams }, shadows: false, renderMode: "shader", physicsCharts: true, chartHz: 5 },
    { id: "default-matrix", label: `Default matrix (${defaultWorkerCount} workers)`, params: { ...baseParams }, shadows: false, renderMode: "matrix" },
    ...workerCounts.map((workerCount) => ({ id: `workers-${workerCount}`, label: `${workerCount} worker${workerCount === 1 ? "" : "s"}`, params: { ...baseParams, workerCount }, shadows: false, renderMode: "shader" as const })),
    { id: "continuous-off", label: "Continuous collision off", params: { ...baseParams, continuous: false }, shadows: false, renderMode: "shader" },
    { id: "shadows-on", label: "Shadows on", params: { ...baseParams }, shadows: true, renderMode: "shader" },
  ];
}

function delay(ms: number): Promise<void> {
  return new Promise((resolve) => window.setTimeout(resolve, ms));
}

function applyBenchVariant(variant: BenchVariant): void {
  if (variant.params !== undefined) Object.assign(solverParams, variant.params);
  renderer.shadowMap.enabled = variant.shadows ?? false;
  chartsEnabled = variant.physicsCharts ?? false;
  chartIntervalMs = chartsEnabled && variant.chartHz !== undefined && variant.chartHz !== null ? 1000 / Math.max(1, variant.chartHz) : 0;
  (globalThis as { __BOX3D_WASHER_RENDER_MODE?: "matrix" | "shader" }).__BOX3D_WASHER_RENDER_MODE = variant.renderMode ?? "shader";
  const washerIndex = samples.findIndex((sample) => sample.id === "washer");
  if (washerIndex >= 0 && activeSampleIndex !== washerIndex) {
    activateSample(washerIndex);
  } else {
    resetScene();
  }
  if (activeSample !== null) activeSample.profile = chartsEnabled;
  updatePhysChartVisibility();
}

function unloadBenchVariant(): void {
  clearScene();
  activeSampleIndex = -1;
  updateStatus();
}

function setupBenchRunner(): void {
  if (!benchRunnerMode) return;

  controlsVisible = false;
  controlsElement.classList.add("hidden");
  controlsDialog!.style.display = "none";
  showControlsDialog = false;
  document.querySelector<HTMLElement>(".topbar")?.remove();
  document.querySelector<HTMLElement>(".metrics-bar")?.remove();
  sampleListElement.classList.remove("open");
  sampleListElement.style.display = "none";
  controlsElement.style.display = "none";

  const variants = createBenchVariants();
  const panel = document.createElement("div");
  panel.style.cssText = "position:fixed;left:16px;top:64px;z-index:40;width:min(420px,calc(100vw - 32px));background:rgba(3,7,18,0.92);border:1px solid rgba(148,163,184,0.35);border-radius:12px;color:#e5e7eb;font:13px system-ui,sans-serif;padding:14px;box-shadow:0 20px 60px rgba(0,0,0,0.35)";
  panel.innerHTML = `
    <div style="font-weight:700;font-size:16px;margin-bottom:8px">Washer Bench Runner</div>
    <div style="color:#a5b4fc;line-height:1.45;margin-bottom:10px">Start Chrome performance recording, then press Start. The runner emits User Timing marks and console timestamps for each variant.</div>
    <div style="display:grid;grid-template-columns:1fr 1fr;gap:8px;margin-bottom:10px">
      <label>Settle seconds<br><input id="bench-settle" type="number" min="0" step="0.5" value="1" style="width:100%"></label>
      <label>Capture seconds<br><input id="bench-run" type="number" min="1" step="0.5" value="3" style="width:100%"></label>
    </div>
    <label style="display:block;margin-bottom:10px;color:#cbd5e1"><input id="bench-hide-ui" type="checkbox"> Hide runner UI while sequence runs</label>
    <div style="display:flex;gap:8px;margin-bottom:10px">
      <button id="bench-start" style="flex:1;padding:8px 10px;border-radius:8px;border:0;background:#22c55e;color:#03110a;font-weight:700;cursor:pointer">Start Sequence</button>
      <button id="bench-stop" style="padding:8px 10px;border-radius:8px;border:1px solid rgba(248,113,113,0.55);background:transparent;color:#fecaca;cursor:pointer">Stop</button>
    </div>
    <div id="bench-status" style="white-space:pre-wrap;line-height:1.45;color:#cbd5e1">Idle. Variants: ${variants.length}</div>
    <ol id="bench-list" style="margin:10px 0 0 20px;padding:0;max-height:260px;overflow:auto;color:#94a3b8"></ol>
  `;
  document.body.appendChild(panel);

  const list = panel.querySelector<HTMLOListElement>("#bench-list")!;
  for (const variant of variants) {
    const item = document.createElement("li");
    item.textContent = `${variant.id}: ${variant.label}`;
    item.dataset.variant = variant.id;
    list.appendChild(item);
  }

  const status = panel.querySelector<HTMLDivElement>("#bench-status")!;
  const startButton = panel.querySelector<HTMLButtonElement>("#bench-start")!;
  const stopButton = panel.querySelector<HTMLButtonElement>("#bench-stop")!;
  const settleInput = panel.querySelector<HTMLInputElement>("#bench-settle")!;
  const runInput = panel.querySelector<HTMLInputElement>("#bench-run")!;
  const hideUiInput = panel.querySelector<HTMLInputElement>("#bench-hide-ui")!;

  stopButton.addEventListener("click", () => {
    benchCancelRequested = true;
    status.textContent = "Stop requested. Current variant will finish shortly.";
  });

  startButton.addEventListener("click", () => {
    if (startButton.disabled) return;
    benchCancelRequested = false;
    startButton.disabled = true;
    void (async () => {
      const settleMs = Math.max(0, Number(settleInput.value) || 0) * 1000;
      const runMs = Math.max(1, Number(runInput.value) || 8) * 1000;
      benchStamp("sequence-start");
      status.textContent = `Running ${variants.length} variants...`;
      if (hideUiInput.checked) panel.style.display = "none";

      for (let i = 0; i < variants.length; i++) {
        if (benchCancelRequested) break;
        const variant = variants[i];
        for (const item of list.querySelectorAll("li")) item.style.color = item.getAttribute("data-variant") === variant.id ? "#fbbf24" : "#64748b";
        status.textContent = `Variant ${i + 1}/${variants.length}: ${variant.label}\nConfiguring...`;
        benchStamp(`${variant.id}:config-start`);
        console.log("[bench] variant config", variant.id, { params: variant.params, shadows: variant.shadows ?? false, renderMode: variant.renderMode ?? "shader" });
        applyBenchVariant(variant);
        benchStamp(`${variant.id}:config-end`);

        status.textContent = `Variant ${i + 1}/${variants.length}: ${variant.label}\nSettling ${Math.round(settleMs / 100) / 10}s...`;
        await delay(settleMs);
        if (benchCancelRequested) break;

        const captureStartMark = `bench:${variant.id}:capture-start`;
        const captureEndMark = `bench:${variant.id}:capture-end`;
        status.textContent = `Variant ${i + 1}/${variants.length}: ${variant.label}\nCAPTURING ${Math.round(runMs / 100) / 10}s...`;
        resetFrameTimings();
        benchTimingActive = true;
        benchStamp(`${variant.id}:capture-start`);
        await delay(runMs);
        benchStamp(`${variant.id}:capture-end`);
        benchTimingActive = false;
        performance.measure(`bench:${variant.id}:capture`, captureStartMark, captureEndMark);

        const p = activeSample?.world.getProfile();
        const frameStats = frameTimingSummary();
        const summary = p === undefined ? "" : ` step=${p.step.toFixed(2)}ms publish=${p.solverSetup.toFixed(2)}ms lag=${p.pairs.toFixed(1)}ms dropped=${p.collide.toFixed(1)}ms`;
        const frameSummary = ` frames=${frameStats.frames} sampleStep=${frameStats.sampleStepMs.toFixed(2)}ms render=${frameStats.renderMs.toFixed(2)}ms charts=${frameStats.chartsMs.toFixed(2)}ms`;
        console.log(`[bench] ${variant.id} summary${summary}${frameSummary}`);
        console.timeStamp?.(`[bench] ${variant.id} summary${summary}${frameSummary}`);
        benchStamp(`${variant.id}:unload-start`);
        unloadBenchVariant();
        benchStamp(`${variant.id}:unload-end`);
      }

      unloadBenchVariant();
      benchStamp(benchCancelRequested ? "sequence-cancelled" : "sequence-end");
      panel.style.display = "block";
      for (const item of list.querySelectorAll("li")) item.style.color = "#94a3b8";
      status.textContent = benchCancelRequested ? "Cancelled. Stop the Chrome recording and save the trace if useful." : "Done. Stop the Chrome recording and save/export the trace.";
      startButton.disabled = false;
    })().catch((error) => {
      panel.style.display = "block";
      console.error("[bench] sequence failed", error);
      status.textContent = `Failed: ${error instanceof Error ? error.message : String(error)}`;
      startButton.disabled = false;
    });
  });
}

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

const DIALOG_WIDTH = 340;
const POS_STORAGE_KEY = "controlsDialogPos";

function saveControlsDialogPos(): void {
  const r = controlsDialog!.getBoundingClientRect();
  const cr = canvas!.getBoundingClientRect();
  try {
    localStorage.setItem(POS_STORAGE_KEY, JSON.stringify({
      left: Math.round(r.left - cr.left),
      top: Math.round(r.top - cr.top),
    }));
  } catch { /* storage unavailable */ }
}

function loadControlsDialogPos(): { left: number; top: number } | null {
  try {
    const v = localStorage.getItem(POS_STORAGE_KEY);
    if (v) return JSON.parse(v);
  } catch { /* ignore */ }
  return null;
}

function constrainControlsDialog(): void {
  const cr = canvas!.getBoundingClientRect();
  const naturalW = Math.min(DIALOG_WIDTH, window.innerWidth - 32);
  const fullW = naturalW > cr.width;

  if (fullW) {
    controlsDialog!.style.left = `${cr.left}px`;
    controlsDialog!.style.width = `${cr.width}px`;
  } else {
    controlsDialog!.style.width = "";
    let l = parseFloat(controlsDialog!.style.left) || 0;
    l = Math.max(cr.left, Math.min(l, cr.right - naturalW));
    controlsDialog!.style.left = `${l}px`;
  }
  controlsDialog!.style.right = "auto";

  const dh = controlsDialog!.offsetHeight;
  if (dh > cr.height) {
    controlsDialog!.style.top = `${cr.top}px`;
    controlsDialog!.style.maxHeight = `${cr.height}px`;
    controlsDialog!.style.bottom = "auto";
  } else {
    controlsDialog!.style.maxHeight = "";
    let t = parseFloat(controlsDialog!.style.top) || 0;
    t = Math.max(cr.top, Math.min(t, cr.bottom - dh));
    controlsDialog!.style.top = `${t}px`;
    controlsDialog!.style.bottom = "auto";
  }
}

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
  if (!dragging) return;
  dragging = false;
  constrainControlsDialog();
  saveControlsDialogPos();
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
  try { localStorage.setItem(VISIBLE_STORAGE_KEY, "0"); } catch { /* ignore */ }
});

window.addEventListener("resize", () => {
  if (showControlsDialog) {
    constrainControlsDialog();
  }
});

function toggleControlsDialog(force?: boolean): void {
  if (!controlsDialog) return;
  showControlsDialog = force ?? !showControlsDialog;
  controlsDialog.style.display = showControlsDialog ? "block" : "none";
  try { localStorage.setItem(VISIBLE_STORAGE_KEY, showControlsDialog ? "1" : "0"); } catch { /* ignore */ }
  if (showControlsDialog) {
    const saved = loadControlsDialogPos();
    if (saved) {
      controlsDialog.style.left = `${saved.left}px`;
      controlsDialog.style.top = `${saved.top}px`;
      controlsDialog.style.right = "auto";
      controlsDialog.style.bottom = "auto";
      dragOffset.x = 0;
      dragOffset.y = 0;
    } else {
      const halfW = controlsDialog.offsetWidth * 0.5;
      const halfH = controlsDialog.offsetHeight * 0.5;
      controlsDialog.style.left = `${window.innerWidth * 0.5 - halfW}px`;
      controlsDialog.style.top = `${window.innerHeight * 0.35 - halfH}px`;
      controlsDialog.style.right = "auto";
      controlsDialog.style.bottom = "auto";
      dragOffset.x = halfW;
      dragOffset.y = 0;
    }
    constrainControlsDialog();
  }
}

toggleControlsDialog(showControlsDialog);

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
      try { localStorage.setItem(VISIBLE_STORAGE_KEY, "0"); } catch { /* ignore */ }
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
    activeSample?.setPaused?.(paused);
    updateStatus();
  } else if (e.key === "o" || e.key === "O") {
    singleStep += e.shiftKey ? 5 : 1;
    if (paused) updateStatus();
  } else if (e.key === ".") {
    singleStep += e.shiftKey ? 5 : 1;
    if (paused) updateStatus();
  } else if (e.key === "r" || e.key === "R") {
    resetScene();
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
  } else if (e.key === "c" || e.key === "C") {
    colorMode = colorMode === "full" ? "light" : "full";
    localStorage.setItem("box3d:color-mode", colorMode);
    activeSample?.onKey?.("c");
    const colorCb = document.querySelector<HTMLInputElement>("#color-mode-cb");
    if (colorCb) colorCb.checked = colorMode === "light";
  }
  if (!e.defaultPrevented && activeSample?.onKey) {
    activeSample.onKey(e.key);
  }
});

window.addEventListener("keyup", (e) => {
  flyKeys.delete(e.key.toLowerCase());
});

statusLabel.textContent = "Loading...";

const urlSampleId = new URL(window.location.href).searchParams.get("sample");
const initialIndex = urlSampleId ? samples.findIndex((s) => s.id === urlSampleId) : -1;
if (benchRunnerMode) {
  activeSampleIndex = -1;
  statusLabel.textContent = "Box3D / Washer Bench Runner";
  setupBenchRunner();
} else {
  activateSample(initialIndex >= 0 ? initialIndex : 0);
}

rafId = window.requestAnimationFrame(frame);

window.addEventListener("beforeunload", () => {
  window.cancelAnimationFrame(rafId);
  clearScene();
});
