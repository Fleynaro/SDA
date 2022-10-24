import { useState, useImperativeHandle, forwardRef } from 'react';
import { Stack, FormControl, FormHelperText, Select, MenuItem } from '@mui/material';
import { FilePicker } from '../FilePicker';
import { getProjectApi } from 'sda-electron/api/project';

export interface CreateProjectFormRef {
  create: () => void;
}

export const CreateProjectForm = forwardRef((props, ref: React.Ref<CreateProjectFormRef>) => {
  const [projectPath, setProjectPath] = useState('');
  const [platformName, setPlatformName] = useState('');

  useImperativeHandle(ref, () => ({
    create: () => {
      getProjectApi().createProject(projectPath, platformName);
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
            <MenuItem value="x86">x86</MenuItem>
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
