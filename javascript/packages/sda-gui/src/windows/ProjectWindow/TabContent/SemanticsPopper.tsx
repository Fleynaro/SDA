import { Paper } from '@mui/material';
import { Popper, usePopper } from 'components/Popper';
import { withCrash_ } from 'providers/CrashProvider';
import { useCallback, useEffect, useState } from 'react';
import { Semantics, getResearcherApi } from 'sda-electron/api/researcher';
import { SemanticsObjectPopper } from './SemanticsObjectPopper';

export const SemanticsPopper = ({ semantics }: { semantics: Semantics }) => {
  const [predecessors, setPredecessors] = useState<Semantics[]>([]);
  const [successors, setSuccessors] = useState<Semantics[]>([]);
  const popper = usePopper();

  useEffect(
    withCrash_(async () => {
      setPredecessors(
        await Promise.all(
          semantics.predecessors.map(async (id) => getResearcherApi().getSemanticsById(id)),
        ),
      );
      setSuccessors(
        await Promise.all(
          semantics.successors.map(async (id) => getResearcherApi().getSemanticsById(id)),
        ),
      );
    }),
    [semantics],
  );

  const onMouseEnterSemantics = useCallback(
    (e: React.MouseEvent<HTMLSpanElement, MouseEvent>, semantics: Semantics) => {
      popper.withTimer(async () => {
        popper.openAtPos(e.clientX, e.clientY + 10);
        popper.setContent(<SemanticsPopper semantics={semantics} />);
      }, 500);
    },
    [popper],
  );

  const onMouseEnterSemanticsObject = useCallback(
    (e: React.MouseEvent<HTMLSpanElement, MouseEvent>) => {
      popper.withTimer(async () => {
        if (!semantics.holder?.object) return;
        popper.openAtPos(e.clientX, e.clientY + 10);
        popper.setContent(<SemanticsObjectPopper object={semantics.holder?.object} />);
      }, 100);
    },
    [popper, semantics],
  );

  const onMouseLeave = useCallback(() => {
    popper.withTimer(() => {
      popper.close();
    }, 300);
  }, [popper]);

  const renderSemanticsName = useCallback(
    (semantics: Semantics) => (
      <span
        onMouseEnter={(e) => onMouseEnterSemantics(e, semantics)}
        onMouseLeave={onMouseLeave}
        style={{ color: '#76b0e3' }}
      >
        {semantics.name}
      </span>
    ),
    [onMouseEnterSemantics, onMouseLeave],
  );

  const renderSemanticsList = useCallback(
    (semantics: Semantics[]) =>
      semantics.map((sem) => (
        <>
          <br />- {renderSemanticsName(sem)}
        </>
      )),
    [renderSemanticsName],
  );

  return (
    <Paper sx={{ p: 5 }}>
      {semantics.holder ? (
        <>
          {'holder of'} {renderSemanticsName(semantics.holder.semantics)}
          <span
            onMouseEnter={onMouseEnterSemanticsObject}
            onMouseLeave={onMouseLeave}
            style={{ color: '#7d7d7d' }}
          >
            {' (holder)'}
          </span>
        </>
      ) : (
        semantics.name
      )}
      <br />
      {predecessors.length > 0 && (
        <>
          Predecessors: {renderSemanticsList(predecessors)}
          <br />
        </>
      )}
      {successors.length > 0 && <>Successors: {renderSemanticsList(successors)}</>}
      <Popper {...popper.props} />
    </Paper>
  );
};
