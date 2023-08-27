import { useCallback } from 'react';
import { getImageApi } from 'sda-electron/api/image';
import { MenuNode } from 'components/Menu';
import { useImageContent } from './context';
import { withCrash } from 'providers/CrashProvider';

export const ImageContentContextMenu = () => {
  const {
    imageId,
    view,
    setView,
    rowSelection: { selectedRows },
  } = useImageContent();

  const onPCodeAnalysis = useCallback(
    withCrash(async () => {
      if (selectedRows.length === 0) return;
      const startOffset = selectedRows[0];
      await getImageApi().analyzePcode(imageId, [startOffset]);
    }),
    [imageId, selectedRows],
  );

  const onShowPCode = useCallback(() => {
    setView({ ...view, showPcode: !view.showPcode });
  }, [view]);

  return (
    <>
      <MenuNode label="P-Code Analysis" onClick={onPCodeAnalysis} />
      <MenuNode label={view.showPcode ? 'Hide P-Code' : 'Show P-Code'} onClick={onShowPCode} />
    </>
  );
};
