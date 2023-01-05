import { useState, useEffect, useCallback } from 'react';
import { getEventApi } from 'sda-electron/api/event';
import { Identifiable, ObjectId, CmpObjectIds, ObjectChangeType } from 'sda-electron/api/common';
import { withCrash } from './useCrash';

export default function useObject<T extends Identifiable>(
  dataSource: () => Promise<T>,
  deps: React.DependencyList = [],
) {
  const [object, setObject] = useState<T | null>(null);

  const loadObject = useCallback(
    withCrash(async () => {
      const data = await dataSource();
      setObject(data);
    }),
    deps,
  );

  const updateObject = useCallback(
    withCrash(async (changedObjId: ObjectId, changeType: ObjectChangeType) => {
      if (!object) return;
      if (CmpObjectIds(changedObjId, object.id)) {
        if (changeType === ObjectChangeType.Update) {
          await loadObject();
        } else if (changeType === ObjectChangeType.Delete) {
          setObject(null);
        }
      }
    }),
    [object, loadObject],
  );

  useEffect(() => {
    loadObject();
  }, [loadObject]);

  useEffect(() => {
    const unsubscribe = getEventApi().subscribeToObjectChangeEvent(updateObject);
    return () => {
      unsubscribe();
    };
  }, [updateObject]);
  return object;
}
