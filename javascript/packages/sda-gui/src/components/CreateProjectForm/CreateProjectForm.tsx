import { useState, useImperativeHandle, forwardRef } from 'react';
import {
  FormControl,
  InputLabel,
  Select,
  MenuItem
} from '@mui/material';

export interface CreateProjectFormRef {
  create: () => void
}

export const CreateProjectForm = forwardRef((props, ref: React.Ref<CreateProjectFormRef>) => {
  const [projectPath, setProjectPath] = useState('');
  const [platformName, setPlatformName] = useState('');

  useImperativeHandle(ref, () => ({
    create: () => {
      console.log(projectPath, platformName);
    }
  }));

  return (
    <FormControl fullWidth>
      <InputLabel variant="standard">
          Platform
      </InputLabel>
      <Select
        label="Platform"
        value={platformName}
        onChange={(event) => setPlatformName(event.target.value)}
      >
        <MenuItem value='x86'>x86</MenuItem>
      </Select>
    </FormControl>
  );
});