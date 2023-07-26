import bindings from 'bindings';
import path from 'path';

export const import_module = (packageName: string) => {
  return bindings({
    bindings: packageName,
    module_root: path.join(__dirname, '..', '..', '..', '..'),
  });
};

// eslint-disable-next-line @typescript-eslint/ban-types
export const instance_of = (obj: Object, classObj: Function) => {
  while (obj) {
    if (obj.constructor.name === classObj.name) {
      return true;
    }
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    obj = (obj as any).__proto__;
  }
  return false;
};
