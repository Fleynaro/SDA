import BaseController from './base-controller';
import { ObjectId } from '../api/common';
import { ContextObject as ContextObjectDTO } from '../api/context';
import { toHash } from '../utils/common';
import { Context } from 'sda-core/context';
import { ContextObject } from 'sda-core/object';

class ContextObjectController extends BaseController {

    constructor(name: string) {
        super(name);
    }

    protected toContext(id: ObjectId): Context {
        const context = Context.Get(toHash(id));
        if (!context) {
            throw new Error(`Context ${id.key} does not exist`);
        }
        return context;
    }

    protected changeContextObject(obj: ContextObject, dto: ContextObjectDTO): void {
        obj.name = dto.name;
        obj.comment = dto.comment;
    }
}

export default ContextObjectController;