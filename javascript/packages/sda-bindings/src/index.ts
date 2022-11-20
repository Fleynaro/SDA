import bindings from 'bindings';
import path from 'path';

export const import_module = (packageName: string) => {
  return bindings({
    bindings: packageName,
    module_root: path.join(__dirname, '..', '..', '..', '..'),
  });
};
