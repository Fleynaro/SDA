import { Paper } from '@mui/material';
import { Popper, usePopper } from 'components/Popper';
import { withCrash_ } from 'providers/CrashProvider';
import { useCallback, useEffect, useState } from 'react';
import { Semantics, SemanticsObject, getResearcherApi } from 'sda-electron/api/researcher';
import { SemanticsPopper } from './SemanticsPopper';

export const SemanticsObjectPopper = ({ object }: { object: SemanticsObject }) => {
  const [semantics, setSemantics] = useState<Semantics[]>([]);
  const popper = usePopper();

  useEffect(
    withCrash_(async () => {
      setSemantics(
        await Promise.all(
          object.semantics.map(async (id) => getResearcherApi().getSemanticsById(id)),
        ),
      );
    }),
    [object],
  );

  const onMouseEnterVariables = useCallback(
    (e: React.MouseEvent<HTMLSpanElement, MouseEvent>) => {
      popper.withTimer(async () => {
        popper.openAtPos(e.clientX, e.clientY + 10);
        popper.setContent(
          <Paper sx={{ p: 5 }}>
            {object.variables
              .sort((a, b) => a.variableId - b.variableId)
              .map((variableId) => (
                <>
                  f{variableId.functionId.offset}:var{variableId.variableId}
                  <br />
                </>
              ))}
          </Paper>,
        );
      }, 500);
    },
    [popper, object],
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

  const onMouseLeave = useCallback(() => {
    popper.withTimer(() => {
      popper.close();
    }, 300);
  }, [popper]);

  return (
    <Paper sx={{ p: 5 }}>
      <span
        onMouseEnter={onMouseEnterVariables}
        onMouseLeave={onMouseLeave}
        style={{ color: '#4287f5' }}
      >
        All Variables
      </span>
      {semantics.length > 0 &&
        semantics.map((sem) => (
          <>
            <br />
            <span
              onMouseEnter={(e) => onMouseEnterSemantics(e, sem)}
              onMouseLeave={onMouseLeave}
              style={{ color: '#76b0e3' }}
            >
              - {sem.name}
            </span>
          </>
        ))}
      <Popper {...popper.props} />
    </Paper>
  );
};
