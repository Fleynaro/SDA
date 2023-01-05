import { useState, forwardRef, useImperativeHandle } from 'react';
import { withCrash } from 'hooks';
import { useSdaContextId } from 'providers/SdaContextProvider';
import { Stack, FormControl, FormHelperText, TextField } from '@mui/material';
import { FilePicker } from '../FilePicker';
import { getImageApi } from 'sda-electron/api/image';
import { AddressSpace, getAddressSpaceApi } from 'sda-electron/api/address-space';

interface CreateImageFormProps {
  addressSpace: AddressSpace;
}

export interface CreateImageFormRef {
  create: () => void;
}

export const CreateImageForm = forwardRef(
  ({ addressSpace }: CreateImageFormProps, ref: React.Ref<CreateImageFormRef>) => {
    const contextId = useSdaContextId();
    const [name, setName] = useState('');
    const [path, setPath] = useState('');

    const methods = {
      create: withCrash(async () => {
        console.log('Create Image', path, name);
        const image = await getImageApi().createImage(contextId, name, 'PEImageAnalyser', path);
        addressSpace.imageIds.push(image.id);
        await getAddressSpaceApi().changeAddressSpace(addressSpace);
      }),
    };
    useImperativeHandle(ref, () => methods);

    return (
      <>
        <Stack spacing={5}>
          <FormControl fullWidth>
            <FormHelperText>Enter a name</FormHelperText>
            <TextField placeholder="Name" value={name} onChange={(e) => setName(e.target.value)} />
          </FormControl>
          <FormControl fullWidth>
            <FormHelperText>Select an image file</FormHelperText>
            <FilePicker onFileSelected={(path) => setPath(path)} />
          </FormControl>
        </Stack>
      </>
    );
  },
);
