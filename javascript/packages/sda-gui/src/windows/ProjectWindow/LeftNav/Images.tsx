import { useState, useRef, useCallback } from 'react';
import { Box, Button, IconButton, Input, Link, Tooltip, Typography } from '@mui/material';
import { TreeView, TreeItem } from '@mui/lab';
import AddIcon from '@mui/icons-material/Add';
import ExpandMoreIcon from '@mui/icons-material/ExpandMore';
import ChevronRightIcon from '@mui/icons-material/ChevronRight';
import { useEffect, useList, useObject, useToggleList } from 'hooks';
import { useSdaContextId } from 'providers/SdaContextProvider';
import { useContextMenu, ContextMenuProps, ContextMenu, MenuNode } from 'components/Menu';
import { DialogRef, Dialog } from 'components/Dialog';
import { CreateImageForm, CreateImageFormRef } from 'components/CreateImageForm';
import {
  getAddressSpaceApi,
  AddressSpaceClassName,
  AddressSpace,
} from 'sda-electron/api/address-space';
import { getImageApi, Image } from 'sda-electron/api/image';
import { ObjectId } from 'sda-electron/api/common';
import { strContains } from 'utils';

const AddressSpaceContextMenu = ({
  addressSpace,
  ...props
}: { addressSpace: AddressSpace } & ContextMenuProps) => {
  const createImageDialogRef = useRef<DialogRef>(null);
  const createImageFormRef = useRef<CreateImageFormRef>(null);
  return (
    <>
      <ContextMenu {...props}>
        <MenuNode
          label="Create Image"
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
        />
      </ContextMenu>
      <Dialog
        title={`New Image for ${addressSpace.name}`}
        showCancelButton={true}
        ref={createImageDialogRef}
      />
    </>
  );
};

interface ImageTreeItemProps {
  imageId: ObjectId;
  onSelect: (image: Image) => void;
}

const ImageTreeItem = ({ imageId, onSelect }: ImageTreeItemProps) => {
  const image = useObject(() => getImageApi().getImage(imageId));
  if (!image) return <></>;
  return <TreeItem nodeId={image.id.key} label={image.name} onClick={() => onSelect(image)} />;
};
interface ImagesProps {
  onSelect: (image: Image) => void;
}

export default function Images({ onSelect }: ImagesProps) {
  const contextId = useSdaContextId();
  const [filterName, setFilterName] = useState('');
  const addressSpaces = useList(
    () => getAddressSpaceApi().getAddressSpaces(contextId),
    AddressSpaceClassName,
  );
  const [addressSpacesWithImages, setAddressSpacesWithImages] = useState<
    {
      addressSpace: AddressSpace;
      images: Image[];
    }[]
  >([]);
  const [expandedAddressSpaces, setExpandedAddressSpaces, toggleExpandedAddressSpaces] =
    useToggleList<string>([]);
  useEffect(async () => {
    let result = await Promise.all(
      addressSpaces.map(async (addressSpace) => {
        let images = await Promise.all(addressSpace.imageIds.map(getImageApi().getImage));
        if (filterName && !strContains(addressSpace.name, filterName)) {
          images = images.filter((image) => strContains(image.name, filterName));
        }
        images = images.sort((a, b) => a.name.localeCompare(b.name));
        return { addressSpace, images };
      }),
    );
    if (filterName) {
      result = result.filter(
        ({ addressSpace, images }) =>
          strContains(addressSpace.name, filterName) || images.length > 0,
      );
    }
    result = result.sort((a, b) => a.addressSpace.name.localeCompare(b.addressSpace.name));
    setAddressSpacesWithImages(result);
    setExpandedAddressSpaces(filterName ? result.map((r) => r.addressSpace.id.key) : []);
  }, [addressSpaces, filterName]);
  const [newAddressSpaceName, setNewAddressSpaceName] = useState<string | null>(null);
  const [addressSpace, setAddressSpace] = useState<AddressSpace>();
  const addressSpaceContextMenu = useContextMenu();
  const showCreateAddressSpace = useCallback(() => {
    setNewAddressSpaceName('');
  }, []);
  return (
    <>
      <Box sx={{ display: 'flex', flexDirection: 'row', height: '25px' }}>
        <Tooltip title="Create a new address space" placement="bottom-end">
          <IconButton onClick={showCreateAddressSpace} sx={{ width: '25px' }}>
            <AddIcon />
          </IconButton>
        </Tooltip>
        <Input
          placeholder="Filter by name"
          value={filterName}
          onChange={(e) => setFilterName(e.target.value)}
          sx={{ width: '100%', height: '25px' }}
        />
      </Box>
      {addressSpacesWithImages.length > 0 || newAddressSpaceName !== null ? (
        <>
          <TreeView
            defaultCollapseIcon={<ExpandMoreIcon />}
            defaultExpandIcon={<ChevronRightIcon />}
            expanded={expandedAddressSpaces}
            defaultSelected={['new']}
          >
            {newAddressSpaceName !== null && (
              <TreeItem
                nodeId="new"
                label={
                  <Input
                    autoFocus
                    placeholder="Enter name"
                    value={newAddressSpaceName}
                    sx={{ width: '100%', height: '20px' }}
                    onChange={(e) => setNewAddressSpaceName(e.target.value)}
                    onKeyPress={(e) => {
                      if (e.key === 'Enter') {
                        getAddressSpaceApi().createAddressSpace(contextId, newAddressSpaceName);
                        setNewAddressSpaceName(null);
                      }
                    }}
                    onBlur={() => setNewAddressSpaceName(null)}
                  />
                }
              />
            )}
            {addressSpacesWithImages.map(({ addressSpace, images }) => (
              <TreeItem
                key={addressSpace.id.key}
                nodeId={addressSpace.id.key}
                label={addressSpace.name}
                onContextMenu={(event) => {
                  event.preventDefault();
                  addressSpaceContextMenu.open(event);
                  setAddressSpace(addressSpace);
                }}
                onClick={() => toggleExpandedAddressSpaces(addressSpace.id.key)}
              >
                {images.length > 0 ? (
                  images.map((image) => (
                    <ImageTreeItem key={image.id.key} imageId={image.id} onSelect={onSelect} />
                  ))
                ) : (
                  <Tooltip title="Right click to create an image">
                    <Typography variant="body1" sx={{ textAlign: 'center' }}>
                      No images here.
                    </Typography>
                  </Tooltip>
                )}
              </TreeItem>
            ))}
          </TreeView>
          {addressSpace && (
            <AddressSpaceContextMenu
              {...addressSpaceContextMenu.props}
              addressSpace={addressSpace}
            />
          )}
        </>
      ) : (
        filterName.length === 0 && (
          <Typography variant="body1" sx={{ textAlign: 'center' }}>
            No address spaces.{' '}
            <Link href="#" onClick={showCreateAddressSpace}>
              Create
            </Link>{' '}
            one.
          </Typography>
        )
      )}
    </>
  );
}
