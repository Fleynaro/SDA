import bindings from 'bindings';
import { fileURLToPath } from 'url';
import path from 'path';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

export default (packageName: string) => {
    return bindings({
        bindings: packageName,
        module_root: path.join(__dirname, '..', '..', '..', '..')
    });
};