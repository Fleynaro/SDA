import { useState, forwardRef } from 'react';
import { useImperativeHandle } from 'hooks';
import { useSdaContextId } from 'providers/SdaContextProvider';
import { Stack, FormControl, FormHelperText, TextField } from '@mui/material';
import { FilePicker } from '../FilePicker';
import { getImageApi } from 'sda-electron/api/image';
import { AddressSpace } from 'sda-electron/api/address-space';

interface CreateImageFormProps {
  addressSpace: AddressSpace;
}

export interface CreateImageFormRef {
  create: () => void;
}

export const CreateImageForm = forwardRef(
  (props: CreateImageFormProps, ref: React.Ref<CreateImageFormRef>) => {
    const contextId = useSdaContextId();
    const [name, setName] = useState('');
    const [path, setPath] = useState('');

    useImperativeHandle(ref, () => ({
      create: async () => {
        console.log('Create Image', path, name);
        await getImageApi().createImage(contextId, name, 'PEImageAnalyser', path);
      },
    }));

    return (
      <>
        <Stack spacing={5}>
          <FormControl fullWidth>
            <FormHelperText>Enter a name</FormHelperText>
            <TextField placeholder="Name" value={name} onChange={(e) => setName(e.target.value)} />
          </FormControl>
          <FormControl fullWidth>
            <FormHelperText>Select an image file</FormHelperText>
            <FilePicker onFileSelected={(path) => setPath(path)} directory />
          </FormControl>
        </Stack>
      </>
    );
  },
);
