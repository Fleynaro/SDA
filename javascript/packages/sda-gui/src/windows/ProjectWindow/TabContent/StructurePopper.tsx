import { Paper } from '@mui/material';
import { withCrash_ } from 'providers/CrashProvider';
import { useEffect, useState } from 'react';
import { Structure, getResearcherApi } from 'sda-electron/api/researcher';

export const StructurePopper = ({ structure }: { structure: Structure }) => {
  const [text, setText] = useState<string>('');
  useEffect(
    withCrash_(async () => {
      const text = await getResearcherApi().printStructure(structure.id);
      setText(text);
    }),
    [structure],
  );
  return <Paper sx={{ p: 5, whiteSpace: 'pre-line' }}>{text}</Paper>;
};
