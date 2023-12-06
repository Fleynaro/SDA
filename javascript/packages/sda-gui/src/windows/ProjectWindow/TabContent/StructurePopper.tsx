import { Paper } from '@mui/material';
import { Popper, usePopper } from 'components/Popper';
import { withCrash_ } from 'providers/CrashProvider';
import { useCallback, useEffect, useState } from 'react';
import { Structure, getResearcherApi } from 'sda-electron/api/researcher';

export const StructurePopper = ({ structure }: { structure: Structure }) => {
  const [parents, setParents] = useState<Structure[]>([]);
  const [children, setChildren] = useState<Structure[]>([]);
  const [fields, setFields] = useState<{
    [offset: string]: Structure;
  }>({});
  const popper = usePopper();

  useEffect(
    withCrash_(async () => {
      setParents(
        await Promise.all(
          structure.parents.map(async (id) => getResearcherApi().getStructureById(id)),
        ),
      );
      setChildren(
        await Promise.all(
          structure.children.map(async (id) => getResearcherApi().getStructureById(id)),
        ),
      );
      const fields: { [offset: string]: Structure } = {};
      await Promise.all(
        Object.entries(structure.fields).map(async ([offset, id]) => {
          fields[offset] = await getResearcherApi().getStructureById(id);
        }),
      );
      setFields(fields);
    }),
    [structure],
  );

  const onMouseEnter = useCallback(
    (e: React.MouseEvent<HTMLSpanElement, MouseEvent>, structure: Structure) => {
      popper.withTimer(async () => {
        popper.openAtPos(e.clientX, e.clientY + 10);
        popper.setContent(<StructurePopper structure={structure} />);
      }, 300);
    },
    [popper],
  );

  const onMouseLeave = useCallback(() => {
    popper.withTimer(() => {
      popper.close();
    }, 300);
  }, [popper]);

  const renderStructureName = useCallback(
    (structure: Structure) => (
      <span onMouseEnter={(e) => onMouseEnter(e, structure)} onMouseLeave={onMouseLeave}>
        {structure.name}
      </span>
    ),
    [onMouseEnter, onMouseLeave],
  );

  return (
    <Paper sx={{ p: 5 }}>
      <span style={{ color: '#93bbd9' }}>struct</span> {structure.name} {'{'}
      {Object.keys(fields).length > 0 && (
        <>
          {Object.entries(fields).map(([offset, fieldStructure]) => (
            <>
              <br />
              &nbsp;&nbsp;<span style={{ color: '#d9d59c' }}>0x{Number(offset).toString(16)}</span>
              {': '}
              {renderStructureName(fieldStructure)}
            </>
          ))}
          <br />
        </>
      )}
      {'}'}
      {parents.length > 0 && (
        <>
          <br />
          <br />
          <span style={{ color: '#99cfd1' }}>Parents: </span>
          {parents.map((parent) => (
            <>
              <br />
              {renderStructureName(parent)}
            </>
          ))}
        </>
      )}
      {children.length > 0 && (
        <>
          <br />
          <br />
          <span style={{ color: '#99cfd1' }}>Children: </span>
          {children.map((parent) => (
            <>
              <br />
              {renderStructureName(parent)}
            </>
          ))}
        </>
      )}
      <Popper {...popper.props} />
    </Paper>
  );
};
