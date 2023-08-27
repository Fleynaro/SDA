import { useMemo } from 'react';
import { useKonvaTextSelection } from 'components/Konva';
import { useHtmlTextSelection } from './HtmlTextSelection';

export function useSelectedText() {
  const { selectedText: konvaSelectedText } = useKonvaTextSelection();
  const { selectedText: htmlSelectedText } = useHtmlTextSelection();
  return useMemo(() => konvaSelectedText + htmlSelectedText, [konvaSelectedText, htmlSelectedText]);
}

export function useSelectedObjects() {
  const { selectedObjects: konvaSelectedObjects } = useKonvaTextSelection();
  const { selectedObjects: htmlSelectedObjects } = useHtmlTextSelection();
  return useMemo(
    () => [...konvaSelectedObjects, ...htmlSelectedObjects],
    [konvaSelectedObjects, htmlSelectedObjects],
  );
}
