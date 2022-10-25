import { useState, useCallback, useEffect } from 'react';
import { Stack, IconButton, TextField } from '@mui/material';
import FileOpenIcon from '@mui/icons-material/FileOpen';
import FolderOpenIcon from '@mui/icons-material/FolderOpen';
import { getWindowApi } from 'sda-electron/api/window';

export interface FilePickerProps {
  directory?: boolean;
  onFileSelected?: (path: string) => void;
}

export function FilePicker({ directory, onFileSelected }: FilePickerProps) {
  const [filePath, setFilePath] = useState('');

  useEffect(() => {
    if (onFileSelected) {
      onFileSelected(filePath);
    }
  }, [filePath]);

  const onOpenFilePickerDialog = useCallback(async () => {
    const paths = await getWindowApi().openFilePickerDialog(Boolean(directory), false);
    if (paths.length > 0) {
      setFilePath(paths[0]);
    }
  }, []);

  return (
    <>
      <Stack direction="row" justifyContent="flex-end" spacing={2}>
        <TextField
          required
          value={filePath}
          onChange={(event) => setFilePath(event.target.value)}
          placeholder="Path"
          fullWidth
        />
        <IconButton component="label" onClick={onOpenFilePickerDialog}>
          {directory ? <FolderOpenIcon /> : <FileOpenIcon />}
        </IconButton>
      </Stack>
    </>
  );
}
