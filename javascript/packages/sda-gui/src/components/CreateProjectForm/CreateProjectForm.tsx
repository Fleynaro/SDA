import { useState, forwardRef } from 'react';
import { useEffect, useImperativeHandle } from '../../hooks/reactWrappers';
import { Stack, FormControl, FormHelperText, Select, MenuItem } from '@mui/material';
import { FilePicker } from '../FilePicker';
import { getProjectApi } from 'sda-electron/api/project';
import { getPlatformApi } from 'sda-electron/api/platform';

export interface CreateProjectFormRef {
  create: () => void;
}

export const CreateProjectForm = forwardRef((props, ref: React.Ref<CreateProjectFormRef>) => {
  const [projectPath, setProjectPath] = useState('');
  const [platformName, setPlatformName] = useState('');
  const [availablePlatforms, setAvailablePlatforms] = useState<string[]>([]);

  useEffect(async () => {
    const platforms = await getPlatformApi().getPlatforms();
    setAvailablePlatforms(platforms.map((p) => p.name));
  }, []);

  useImperativeHandle(ref, () => ({
    create: async () => {
      await getProjectApi().createProject(projectPath, platformName);
    },
  }));

  return (
    <>
      <Stack spacing={5}>
        <FormControl fullWidth>
          <FormHelperText>Select platform</FormHelperText>
          <Select
            value={platformName}
            onChange={(event) => setPlatformName(event.target.value)}
            displayEmpty
          >
            <MenuItem disabled value="">
              <em>None</em>
            </MenuItem>
            {availablePlatforms.map((platform) => (
              <MenuItem key={platform} value={platform}>
                {platform}
              </MenuItem>
            ))}
          </Select>
        </FormControl>
        <FormControl fullWidth>
          <FormHelperText>Select project directory</FormHelperText>
          <FilePicker onFileSelected={(path) => setProjectPath(path)} directory />
        </FormControl>
      </Stack>
    </>
  );
});