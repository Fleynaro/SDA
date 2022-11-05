import { useState, useEffect } from 'react';
import { useCallback } from './reactWrappers';
import { getEventApi } from 'sda-electron/api/event';
import { Identifiable, ObjectId, CmpObjectIds, ObjectChangeType } from 'sda-electron/api/common';

export default function useObject<T extends Identifiable, U extends unknown[]>(
  dataSource: (objId: ObjectId, ...args: U) => Promise<T>,
  objId: ObjectId,
  ...args: U
) {
  const [object, setObject] = useState<T | null>(null);

  const loadObject = useCallback(
    async (objId: ObjectId, ...args: U) => {
      const data = await dataSource(objId, ...args);
      setObject(data);
    },
    [dataSource],
  );

  const updateObject = useCallback(
    async (changedObjId: ObjectId, changeType: ObjectChangeType) => {
      if (CmpObjectIds(changedObjId, objId)) {
        if (changeType === ObjectChangeType.Update) {
          await loadObject(objId, ...args);
        } else if (changeType === ObjectChangeType.Delete) {
          setObject(null);
        }
      }
    },
    [objId, ...args, loadObject],
  );

  useEffect(() => {
    loadObject(objId, ...args);
  }, []);

  useEffect(() => {
    const unsubscribe = getEventApi().subscribeToObjectChangeEvent(updateObject);
    return () => {
      unsubscribe();
    };
  }, [updateObject]);
  return object;
}
