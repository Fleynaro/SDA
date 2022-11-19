import { toId } from '../../utils/common';
import { ContextObject } from 'sda-core/object';
import { ContextObject as ContextObjectDTO } from '../../api/context';
// import { ObjectId } from '../../api/common';
// import { AddressSpace } from 'sda-core/address-space';
// import { toAddressSpaceId } from './address-space';
// import { Image } from 'sda-core/image';
// import { toImageId } from './image';

// const ContextObjectToId: [any, (obj: any) => ObjectId][] = [
//     [AddressSpace, toAddressSpaceId],
//     [Image, toImageId],
// ];

// export const toContextObjectId = (obj: ContextObject): ObjectId => {
//     const pair = ContextObjectToId.find(([cls]) => obj instanceof cls);
//     if (!pair) {
//         throw new Error(`Cannot find id for object ${obj}`);
//     }
//     const [_, toId] = pair;
//     return toId(obj);
// };

export const toContextObjectDTO = (obj: ContextObject): ContextObjectDTO => {
    return {
        id: toId(obj),
        name: obj.name,
        comment: obj.comment,
    };
};

export const changeContextObject = (obj: ContextObject, dto: ContextObjectDTO): void => {
    obj.name = dto.name;
    obj.comment = dto.comment;
};
