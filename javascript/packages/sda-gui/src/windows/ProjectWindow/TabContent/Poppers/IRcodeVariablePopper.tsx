import { Paper } from '@mui/material';
import { Popper, usePopper } from 'components/Popper';
import { withCrash_ } from 'providers/CrashProvider';
import { useCallback, useEffect, useState } from 'react';
import { IRcodeVariableId } from 'sda-electron/api/ir-code';
import { SemanticsObject, StructureLink, getResearcherApi } from 'sda-electron/api/researcher';
import { StructurePopper } from './StructurePopper';
import { SemanticsObjectPopper } from './SemanticsObjectPopper';

export const IRcodeVariablePopper = ({ variableId }: { variableId: IRcodeVariableId }) => {
  const [link, setLink] = useState<StructureLink | undefined>();
  const [semanticsObject, setSemanticsObject] = useState<SemanticsObject | undefined>();
  const popper = usePopper();

  useEffect(
    withCrash_(async () => {
      const link = await getResearcherApi().findStructureByVariableId(variableId);
      setLink(link);
      const semanticsObject = await getResearcherApi().findSemanticsObjectByVariableId(variableId);
      setSemanticsObject(semanticsObject);
    }),
    [variableId],
  );

  const onMouseEnterStructure = useCallback(
    (e: React.MouseEvent<HTMLSpanElement, MouseEvent>) => {
      popper.withTimer(async () => {
        if (!link) return;
        popper.openAtPos(e.clientX, e.clientY + 10);
        popper.setContent(<StructurePopper structure={link.structure} link={link} />);
      }, 100);
    },
    [popper, link],
  );

  const onMouseEnterSemantics = useCallback(
    (e: React.MouseEvent<HTMLSpanElement, MouseEvent>) => {
      popper.withTimer(async () => {
        if (!semanticsObject) return;
        popper.openAtPos(e.clientX, e.clientY + 10);
        popper.setContent(<SemanticsObjectPopper object={semanticsObject} />);
      }, 100);
    },
    [popper, semanticsObject],
  );

  const onMouseLeave = useCallback(() => {
    popper.withTimer(() => {
      popper.close();
    }, 300);
  }, [popper]);

  return (
    <Paper sx={{ p: 5 }}>
      {semanticsObject && (
        <span
          onMouseEnter={onMouseEnterSemantics}
          onMouseLeave={onMouseLeave}
          style={{ color: '#76b0e3' }}
        >
          Semantics
        </span>
      )}
      {link && (
        <span
          onMouseEnter={onMouseEnterStructure}
          onMouseLeave={onMouseLeave}
          style={{ color: '#c8cbe6', marginLeft: semanticsObject && 5 }}
        >
          Structure
        </span>
      )}
      <Popper {...popper.props} closeOnMouseLeave />
    </Paper>
  );
};
