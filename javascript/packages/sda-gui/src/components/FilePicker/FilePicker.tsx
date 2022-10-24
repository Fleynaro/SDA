import { useState, useCallback, useEffect } from 'react';
import { Stack, IconButton, TextField } from '@mui/material';
import FolderIcon from '@mui/icons-material/Folder';
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
          <FolderIcon />
        </IconButton>
      </Stack>
    </>
  );
}
