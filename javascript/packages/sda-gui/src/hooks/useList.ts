import { useState, useEffect, useCallback } from 'react';
import { getEventApi } from 'sda-electron/api/event';
import { Identifiable, ObjectId, CmpObjectIds, ObjectChangeType } from 'sda-electron/api/common';

export default function useList<T extends Identifiable>(
  dataSource: () => Promise<T[]>,
  className: string,
) {
  const [items, setItems] = useState<T[]>([]);

  const updateItems = useCallback(async () => {
    const newItems = await dataSource();
    setItems(newItems);
  }, [dataSource]);

  useEffect(() => {
    updateItems();

    const unsubscribe = getEventApi().subscribeToObjectChangeEvent(
      (changedItemId: ObjectId, changeType: ObjectChangeType) => {
        if (changeType === ObjectChangeType.Create) {
          if (changedItemId.className !== className) return;
        } else if (
          changeType === ObjectChangeType.Update ||
          changeType === ObjectChangeType.Delete
        ) {
          if (!items.some((item) => CmpObjectIds(changedItemId, item.id))) return;
        }
        updateItems();
      },
    );

    return () => {
      unsubscribe();
    };
  }, [updateItems]); // eslint-disable-line
  return items;
}
