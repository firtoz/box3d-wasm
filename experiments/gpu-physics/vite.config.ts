import { createReadStream, statSync } from "node:fs";
import { dirname, extname, resolve, sep } from "node:path";
import { fileURLToPath } from "node:url";
import { defineConfig, type Connect } from "vite";

const gpuPhysicsRoot = dirname(fileURLToPath(import.meta.url));
const compareRoot = resolve(gpuPhysicsRoot, "compare");
const recordingsRoot = resolve(gpuPhysicsRoot, "recordings");

const MIME: Record<string, string> = {
  ".mp4": "video/mp4",
  ".json": "application/json",
};

function serveRecordings(): Connect.NextHandleFunction {
  return (req, res, next) => {
    const pathOnly = (req.url ?? "").split("?")[0] ?? "";
    if (!pathOnly.startsWith("/recordings/")) {
      next();
      return;
    }
    const rel = decodeURIComponent(pathOnly.slice("/recordings/".length));
    const file = resolve(recordingsRoot, rel);
    if (file !== recordingsRoot && !file.startsWith(recordingsRoot + sep)) {
      res.statusCode = 403;
      res.end();
      return;
    }
    let size = 0;
    try {
      const st = statSync(file);
      if (!st.isFile()) {
        next();
        return;
      }
      size = st.size;
    } catch {
      next();
      return;
    }
    const mime = MIME[extname(file).toLowerCase()] ?? "application/octet-stream";
    res.setHeader("Content-Type", mime);
    res.setHeader("Accept-Ranges", "bytes");
    const range = req.headers.range;
    const m = range ? /bytes=(\d*)-(\d*)/.exec(range) : null;
    if (m) {
      const start = m[1] ? Number(m[1]) : 0;
      const end = m[2] ? Number(m[2]) : size - 1;
      res.statusCode = 206;
      res.setHeader("Content-Range", `bytes ${start}-${end}/${size}`);
      res.setHeader("Content-Length", String(end - start + 1));
      createReadStream(file, { start, end }).pipe(res);
      return;
    }
    res.setHeader("Content-Length", String(size));
    createReadStream(file).pipe(res);
  };
}

export default defineConfig({
  root: compareRoot,
  appType: "mpa",
  publicDir: false,
  server: {
    host: "0.0.0.0",
    port: 8766,
    strictPort: true,
    open: "/",
    fs: { allow: [gpuPhysicsRoot] },
  },
  plugins: [
    {
      name: "serve-recordings",
      configureServer(server) {
        server.middlewares.use(serveRecordings());
      },
    },
  ],
});
