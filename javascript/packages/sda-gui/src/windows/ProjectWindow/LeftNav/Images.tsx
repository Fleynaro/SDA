import { useState, useRef } from 'react';
import { Box, Button, IconButton, Input } from '@mui/material';
import { TreeView, TreeItem } from '@mui/lab';
import AddIcon from '@mui/icons-material/Add';
import ExpandMoreIcon from '@mui/icons-material/ExpandMore';
import ChevronRightIcon from '@mui/icons-material/ChevronRight';
import { useList, useObject } from 'hooks';
import { useSdaContextId } from 'providers/SdaContextProvider';
import {
  useContextMenu,
  ContextMenuProps,
  ContextMenu,
  ContextMenuList,
  ContextMenuItem,
} from 'components/ContextMenu';
import { DialogRef, Dialog } from 'components/Dialog';
import { CreateImageForm, CreateImageFormRef } from 'components/CreateImageForm';
import {
  getAddressSpaceApi,
  AddressSpaceClassName,
  AddressSpace,
} from 'sda-electron/api/address-space';
import { getImageApi, Image } from 'sda-electron/api/image';

const AddressSpaceContextMenu = ({
  addressSpace,
  ...props
}: { addressSpace: AddressSpace } & ContextMenuProps) => {
  const createImageDialogRef = useRef<DialogRef>(null);
  const createImageFormRef = useRef<CreateImageFormRef>(null);
  return (
    <>
      <ContextMenu {...props}>
        <ContextMenuList>
          <ContextMenuItem
            onClick={() => {
              createImageDialogRef.current?.open(
                <Box component="form" noValidate>
                  <CreateImageForm ref={createImageFormRef} addressSpace={addressSpace} />
                </Box>,
                <Button
                  onClick={() => {
                    createImageFormRef.current?.create();
                    createImageDialogRef.current?.close();
                  }}
                >
                  Create
                </Button>,
              );
            }}
          >
            Create Image
          </ContextMenuItem>
        </ContextMenuList>
      </ContextMenu>
      <Dialog title="New Image" showCancelButton={true} ref={createImageDialogRef} />
    </>
  );
};

interface ImagesProps {
  onSelect: (image: Image) => void;
}

export default function Images({ onSelect }: ImagesProps) {
  const contextId = useSdaContextId();
  const addressSpaces = useList(
    () => getAddressSpaceApi().getAddressSpaces(contextId),
    AddressSpaceClassName,
  );
  const [addressSpace, setAddressSpace] = useState<AddressSpace>();
  const addressSpaceContextMenu = useContextMenu();
  const [filterName, setFilterName] = useState('');
  return (
    <>
      <Box sx={{ display: 'flex', flexDirection: 'row', height: '25px' }}>
        <IconButton
          onClick={() => {
            getAddressSpaceApi().createAddressSpace(contextId, 'test');
          }}
          sx={{ width: '25px' }}
        >
          <AddIcon />
        </IconButton>
        <Input
          placeholder="Filter by name"
          value={filterName}
          onChange={(e) => setFilterName(e.target.value)}
          sx={{ width: '100%', height: '25px' }}
        />
      </Box>
      <TreeView defaultCollapseIcon={<ExpandMoreIcon />} defaultExpandIcon={<ChevronRightIcon />}>
        {addressSpaces
          .map((addressSpace) => {
            const images = addressSpace.imageIds
              .map((imageId) => useObject(() => getImageApi().getImage(imageId), imageId))
              .filter((image) => image !== null) as Image[];
            return { addressSpace, images };
          })
          .filter(({ addressSpace, images }) => {
            const names = addressSpace.name + images.map((image) => image.name).join('');
            return names.includes(filterName);
          })
          .map(({ addressSpace, images }) => (
            <TreeItem
              key={addressSpace.id.key}
              nodeId={addressSpace.id.key}
              label={addressSpace.name}
              onContextMenu={(event) => {
                event.preventDefault();
                addressSpaceContextMenu.open(event);
                setAddressSpace(addressSpace);
              }}
            >
              {images.map((image) => (
                <TreeItem
                  key={image.id.key}
                  nodeId={image.id.key}
                  label={image.name}
                  onClick={() => onSelect(image)}
                />
              ))}
            </TreeItem>
          ))}
      </TreeView>
      {addressSpace && (
        <AddressSpaceContextMenu {...addressSpaceContextMenu.props} addressSpace={addressSpace} />
      )}
    </>
  );
}
