import { useRef } from 'react';
import { Button } from '@mui/material';
import { TreeView, TreeItem } from '@mui/lab';
import ExpandMoreIcon from '@mui/icons-material/ExpandMore';
import ChevronRightIcon from '@mui/icons-material/ChevronRight';
import { useList, useObject } from 'hooks';
import { useSdaContextId } from 'providers/SdaContextProvider';
import {
  ContextMenuRef,
  ContextMenu,
  ContextMenuList,
  ContextMenuItem,
} from 'components/ContextMenu';
import {
  getAddressSpaceApi,
  AddressSpaceClassName,
  AddressSpace,
} from 'sda-electron/api/address-space';
import { getImageApi, Image } from 'sda-electron/api/image';

const AddressSpaceContextMenuBody = ({ addressSpace }: { addressSpace: AddressSpace }) => {
  return (
    <ContextMenuList>
      <ContextMenuItem
        onClick={() => {
          console.log('Create');
        }}
      >
        Create Image
      </ContextMenuItem>
    </ContextMenuList>
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
  const addressSpaceContextMenu = useRef<ContextMenuRef>(null);
  return (
    <>
      <Button
        onClick={() => {
          getAddressSpaceApi().createAddressSpace(contextId, 'test');
        }}
      >
        Add
      </Button>
      <TreeView defaultCollapseIcon={<ExpandMoreIcon />} defaultExpandIcon={<ChevronRightIcon />}>
        {addressSpaces.map((addressSpace) => (
          <TreeItem
            key={addressSpace.id.key}
            nodeId={addressSpace.id.key}
            label={addressSpace.name}
            onContextMenu={(event) => {
              event.preventDefault();
              addressSpaceContextMenu.current?.open(
                event,
                <AddressSpaceContextMenuBody addressSpace={addressSpace} />,
              );
            }}
          >
            {addressSpace.imageIds.map((imageId) => {
              const image = useObject(() => getImageApi().getImage(imageId), imageId);
              if (!image) return <></>;
              return (
                <TreeItem
                  key={image.id.key}
                  nodeId={image.id.key}
                  label={image.name}
                  onClick={() => onSelect(image)}
                />
              );
            })}
          </TreeItem>
        ))}
      </TreeView>
      <ContextMenu ref={addressSpaceContextMenu} />
    </>
  );
}
