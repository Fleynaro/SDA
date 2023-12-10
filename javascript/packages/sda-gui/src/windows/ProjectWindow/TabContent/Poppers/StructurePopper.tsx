import { Paper } from '@mui/material';
import { Popper, usePopper } from 'components/Popper';
import { withCrash_ } from 'providers/CrashProvider';
import { useCallback, useEffect, useState } from 'react';
import {
  ConstantSet,
  Structure,
  StructureLink,
  getResearcherApi,
} from 'sda-electron/api/researcher';

export const StructurePopper = ({
  structure,
  link,
}: {
  structure: Structure;
  link?: StructureLink;
}) => {
  const [parents, setParents] = useState<Structure[]>([]);
  const [children, setChildren] = useState<Structure[]>([]);
  const [fields, setFields] = useState<{
    [offset: string]: Structure;
  }>({});
  const popper = usePopper();
  const popperGroup = usePopper();

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

  const onMouseEnterStructure = useCallback(
    (e: React.MouseEvent<HTMLSpanElement, MouseEvent>, structure: Structure) => {
      popper.withTimer(async () => {
        popper.openAtPos(e.clientX, e.clientY + 10);
        popper.setContent(<StructurePopper structure={structure} />);
      }, 300);
    },
    [popper],
  );

  const onMouseEnterConstantSet = useCallback(
    (e: React.MouseEvent<HTMLSpanElement, MouseEvent>, set?: ConstantSet) => {
      if (!set || Object.keys(set).length === 0) return;
      popper.withTimer(async () => {
        popper.openAtPos(e.clientX, e.clientY + 10);
        popper.setContent(
          <Paper sx={{ p: 5 }}>
            {Object.entries(set).map(([offset, value]) => (
              <>
                <span style={{ color: '#d9d59c' }}>0x{Number(offset).toString(16)}</span>
                {': '}
                {value.join(', ')}
                <br />
              </>
            ))}
          </Paper>,
        );
      }, 300);
    },
    [popper],
  );

  const onMouseEnterGroup = useCallback(
    async (e: React.MouseEvent<HTMLSpanElement, MouseEvent>, structure: Structure) => {
      if (!structure.classInfo) return;
      const structuresInGroup = await Promise.all(
        structure.classInfo.structuresInGroup.map(async (id) =>
          getResearcherApi().getStructureById(id),
        ),
      );
      popperGroup.withTimer(async () => {
        popperGroup.openAtPos(e.clientX, e.clientY + 10);
        popperGroup.setContent(
          <Paper sx={{ p: 5 }}>
            {structuresInGroup.map((s) => (
              <>
                {renderStructureName(s)}
                <br />
              </>
            ))}
          </Paper>,
        );
      }, 300);
    },
    [popperGroup],
  );

  const onMouseLeaveGroup = useCallback(() => {
    popperGroup.withTimer(() => {
      popperGroup.close();
    }, 300);
  }, [popperGroup]);

  const onMouseLeave = useCallback(() => {
    popper.withTimer(() => {
      popper.close();
    }, 300);
  }, [popper]);

  const renderStructureName = useCallback(
    (structure: Structure) => (
      <span onMouseEnter={(e) => onMouseEnterStructure(e, structure)} onMouseLeave={onMouseLeave}>
        {structure.name}
      </span>
    ),
    [onMouseEnterStructure, onMouseLeave],
  );

  return (
    <Paper sx={{ p: 5 }}>
      {link && (
        <>
          <span style={{ color: '#bfbfbf' }}>
            At offset: 0x{link.offset.toString(16)}
            {link.own && ' (own)'}
          </span>
          <br />
        </>
      )}
      {structure.classInfo && (
        <>
          <span
            onMouseEnter={(e) => onMouseEnterConstantSet(e, structure.classInfo?.labelSet)}
            onMouseLeave={onMouseLeave}
            style={{ color: '#3277a8' }}
          >
            Labels: {structure.classInfo.labels.map((l) => (l > 1000 ? -1 : 0)).join(', ')} (at 0x
            {structure.classInfo.labelOffset.toString(16)})
          </span>
          {structure.classInfo.structuresInGroup.length > 0 && (
            <>
              <br />
              <span
                onMouseEnter={(e) => onMouseEnterGroup(e, structure)}
                onMouseLeave={onMouseLeaveGroup}
                style={{ color: '#e3b268' }}
              >
                Group ({structure.classInfo.structuresInGroup.length} structures)
              </span>
            </>
          )}
          <br />
        </>
      )}
      <span style={{ color: '#93bbd9' }}>struct</span> {structure.name} {'{'}
      {Object.keys(fields).length > 0 && (
        <>
          {Object.entries(fields).map(([offset, fieldStructure]) => (
            <>
              <br />
              &nbsp;&nbsp;
              <span style={{ color: link?.offset === Number(offset) ? '#faee23' : '#d9d59c' }}>
                0x{Number(offset).toString(16)}
              </span>
              {': '}
              {renderStructureName(fieldStructure)}
            </>
          ))}
          <br />
        </>
      )}
      {'}'}
      {Object.keys(structure.conditions).length + Object.keys(structure.constants).length > 0 && (
        <>
          <br />
          <br />
          {Object.keys(structure.constants).length > 0 && (
            <span
              onMouseEnter={(e) => onMouseEnterConstantSet(e, structure.constants)}
              onMouseLeave={onMouseLeave}
              style={{ color: '#e364e3' }}
            >
              Constants
            </span>
          )}
          {Object.keys(structure.conditions).length > 0 && (
            <span
              onMouseEnter={(e) => onMouseEnterConstantSet(e, structure.conditions)}
              onMouseLeave={onMouseLeave}
              style={{ color: '#e364e3', marginLeft: 5 }}
            >
              Conditions
            </span>
          )}
        </>
      )}
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
      <Popper {...popperGroup.props} />
    </Paper>
  );
};
