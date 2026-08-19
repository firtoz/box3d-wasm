/**
 * Tear down a Box3D physics DedicatedWorker without killing it mid-`Atomics.wait`.
 *
 * Host `Worker.terminate()` during a threaded `world.step()` leaves Emscripten
 * pthread children alive and can freeze the next WASM instantiate. Let the worker
 * finish the current step, `b3DestroyWorld`, and `terminatePthreads` first.
 */

const DISPOSE_TIMEOUT_MS = 2500;

let pendingShutdown: Promise<void> = Promise.resolve();

export function pendingPhysicsWorkerShutdown(): Promise<void> {
  return pendingShutdown;
}

export function shutdownPhysicsWorker(worker: Worker): Promise<void> {
  const next = shutdownOne(worker);
  pendingShutdown = Promise.all([pendingShutdown, next]).then(() => undefined);
  return next;
}

function shutdownOne(worker: Worker): Promise<void> {
  return new Promise((resolve) => {
    let settled = false;
    const settle = (): void => {
      if (settled) return;
      settled = true;
      worker.removeEventListener("message", onMessage);
      worker.removeEventListener("error", onError);
      try {
        worker.terminate();
      } catch {
        /* already closed */
      }
      resolve();
    };
    const onMessage = (event: MessageEvent<{ type?: string }>): void => {
      if (event.data?.type === "disposed") settle();
    };
    const onError = (): void => {
      settle();
    };
    worker.addEventListener("message", onMessage);
    worker.addEventListener("error", onError);
    try {
      worker.postMessage({ type: "set-paused", paused: true });
      worker.postMessage({ type: "dispose" });
    } catch {
      settle();
      return;
    }
    setTimeout(settle, DISPOSE_TIMEOUT_MS);
  });
}
