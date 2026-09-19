// Bun is already required by the workspace scripts. Windows Python normally
// installs as python/py, while Unix installations generally provide python3.
import { spawnSync } from "node:child_process";
import { fileURLToPath } from "node:url";
const candidates = process.platform === "win32" ? ["python", "py", "python3"] : ["python3", "python"];
const python = candidates.find((command) => {
  const result = spawnSync(command, ["-c", "import sys; sys.exit(sys.version_info < (3, 10))"], { stdio: "ignore" });
  return result.status === 0;
});
if (!python) throw new Error("Native samples require Python 3.10 or newer on PATH.");
const result = spawnSync(python, [fileURLToPath(new URL("./run-native-samples.py", import.meta.url)), ...process.argv.slice(2)], { stdio: "inherit" });
if (result.error) throw result.error;
process.exit(result.status ?? 1);
