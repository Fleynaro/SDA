import { Context } from 'sda-core/context';
import { Context as ContextDTO } from '@api/context';

export const toContextDTO = (context: Context): ContextDTO => {
    return {
        hashId: context.hashId,
    };
};