import { Hash, IIdentifiable } from 'sda-core/utils';
import { ObjectId } from 'api/common';

export const toHash = (id: ObjectId): Hash => {
  return Number(id.key);
};

export const toId = (object: IIdentifiable): ObjectId => {
  return {
    key: object.hashId.toString(),
    className: object.className,
  };
};

export const binSearch = <T>(array: T[], compare: (mid: number) => number): number => {
  let left = 0;
  let right = array.length - 1;
  while (left <= right) {
    const mid = Math.floor((left + right) / 2);
    const cmp = compare(mid);
    if (cmp === 0) {
      return mid;
    }
    if (cmp < 0) {
      left = mid + 1;
    } else {
      right = mid - 1;
    }
  }
  return -1;
};
