import { contextBridge, ipcRenderer } from 'electron';
import api from '../types/api';

function send(controllerName: string) {
    return (methodName: string, ...args: any[]) => {
        return ipcRenderer.sendSync(controllerName + '.' + methodName, ...args);
    }
}

const s = send('dataTypeApi');

const controller: api.DataTypeController = {
    getDataTypeByName: (name) => s('getDataTypeByName', name)
};

contextBridge.exposeInMainWorld('dataTypeApi', controller);


/*
class Controller implements api.Controller {
    name: string;
    constructor(name: string) {
        this.name = name;
    }

    protected send(methodName: string, ...args: any[]) {
        return ipcRenderer.sendSync(this.name + '.' + methodName, ...args);
    }

    public expose() {
        contextBridge.exposeInMainWorld(this.name, this);
    }
}

class DataTypeController extends Controller implements api.DataTypeController {
    getDataTypeByName(name: string): api.DataType {
        return this.send('getDataTypeByName', name);
    }
}

new DataTypeController('dataTypeApi').expose();
*/