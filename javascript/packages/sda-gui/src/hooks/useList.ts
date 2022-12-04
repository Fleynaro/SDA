import { useState, useEffect } from 'react';
import { useCallback } from './reactWrappedHooks';
import { getEventApi } from 'sda-electron/api/event';
import { Identifiable, ObjectId, CmpObjectIds, ObjectChangeType } from 'sda-electron/api/common';

export default function useList<T extends Identifiable>(
  dataSource: () => Promise<T[]>,
  className?: string,
  deps: React.DependencyList = [],
) {
  const [items, setItems] = useState<T[]>([]);

  const loadItems = useCallback(async () => {
    const newItems = await dataSource();
    setItems(newItems);
  }, deps);

  const updateItems = useCallback(
    async (changedItemId: ObjectId, changeType: ObjectChangeType) => {
      if (className && changedItemId.className !== className) return;
      if (changeType === ObjectChangeType.Update || changeType === ObjectChangeType.Delete) {
        if (!items.some((item) => CmpObjectIds(changedItemId, item.id))) return;
      }
      await loadItems();
    },
    [className, items, loadItems],
  );

  useEffect(() => {
    loadItems();
  }, [loadItems]);

  useEffect(() => {
    const unsubscribe = getEventApi().subscribeToObjectChangeEvent(updateItems);
    return () => {
      unsubscribe();
    };
  }, [updateItems]);
  return items;
}
