import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';

const scriptDir = dirname(fileURLToPath(import.meta.url));
const repoRoot = join(scriptDir, '..', '..', '..');
const submoduleDir = join(repoRoot, 'box3d');

if (!(await Bun.file(join(submoduleDir, 'CMakeLists.txt')).exists())) {
  throw new Error(`Missing submodule checkout at ${submoduleDir}. Run git submodule update --init --recursive.`);
}
