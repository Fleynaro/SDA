namespace api {
    interface ContextObject {
        name: string;
    }
    
    interface DataType extends ContextObject {
        isVoid: boolean;
    }
    
    interface DataTypeController {
        getDataTypeByName(name: string): DataType;
    }
}

declare global {
    interface Window {
        dataTypeApi: api.DataTypeController;
    }
}

export default api;