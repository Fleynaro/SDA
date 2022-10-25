import { useState, useEffect, useCallback } from 'react';
import { getEventApi } from 'sda-electron/api/event';
import { Identifiable, ObjectId, CmpObjectIds, ObjectChangeType } from 'sda-electron/api/common';

export default function useList<T extends Identifiable>(
  dataSource: () => Promise<T[]>,
  className?: string,
) {
  const [items, setItems] = useState<T[]>([]);

  const updateItems = useCallback(async () => {
    const newItems = await dataSource();
    setItems(newItems);
  }, [dataSource]);

  const update = useCallback(
    () => (changedItemId: ObjectId, changeType: ObjectChangeType) => {
      if (className && changedItemId.className !== className) return;
      if (changeType === ObjectChangeType.Update || changeType === ObjectChangeType.Delete) {
        if (!items.some((item) => CmpObjectIds(changedItemId, item.id))) return;
      }
      updateItems();
    },
    [className, items, updateItems],
  );

  useEffect(() => {
    updateItems();
    const unsubscribe = getEventApi().subscribeToObjectChangeEvent(update);
    return () => {
      unsubscribe();
    };
  }, [update, updateItems]);
  return items;
}
