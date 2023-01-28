import { PopperProps, Popper, PopperHook, usePopper } from 'components/Popper';
import { MenuNode } from './MenuNode';

export type ContextMenuProps = PopperProps;

export type ContextMenuHook = PopperHook;

export const useContextMenu = (): ContextMenuHook => {
  return usePopper();
};

export const ContextMenu = (props: ContextMenuProps) => {
  return (
    <Popper {...props} closeOnContentClick>
      <MenuNode>{props.children}</MenuNode>
    </Popper>
  );
};
