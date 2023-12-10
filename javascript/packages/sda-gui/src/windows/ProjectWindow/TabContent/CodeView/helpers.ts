import { useMemo } from 'react';
import { TokenGroup, TokenizedText } from 'sda-electron/api/common';
import { PcodeLocatableByOffset } from 'sda-electron/api/p-code';
import { useSelectedObjects } from 'components/Text';

export const useHighlightedGroupIndexes = (text: TokenizedText | null) => {
  const selectedObjects = useSelectedObjects();
  return useMemo(() => {
    if (!text) return [];
    const result: number[] = [];
    for (const selObject of selectedObjects) {
      const selGroup = selObject as TokenGroup;
      if (!('locatableByOffset' in selGroup.action)) continue;
      const { offset: selOffset } = selGroup.action as PcodeLocatableByOffset;
      for (const group of text.groups) {
        if (!('locatableByOffset' in group.action)) continue;
        const { offset } = group.action as PcodeLocatableByOffset;
        if (offset === selOffset) {
          result.push(group.idx);
        }
      }
    }
    return result;
  }, [selectedObjects, text]);
};
