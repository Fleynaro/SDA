import { useState } from 'react';
import { useCallback } from 'hooks';
import { MenuBar, MenuBarItem, MenuNode } from 'components/Menu';
import { useProjectId } from 'providers/ProjectProvider';
import SaveIcon from '@mui/icons-material/Save';
import { getProjectApi } from 'sda-electron/api/project';

export default function ProjectMenuBar() {
  const projectId = useProjectId();
  const [canBeSaved, setCanBeSaved] = useState(false);

  const onMenuOpen = useCallback(async () => {
    setCanBeSaved(await getProjectApi().canProjectBeSaved(projectId));
  }, [projectId]);

  const fileSave = useCallback(() => {
    getProjectApi().saveProject(projectId);
  }, [projectId]);

  return (
    <MenuBar onMenuOpen={onMenuOpen}>
      <MenuBarItem label="File">
        <MenuNode
          label="Save"
          icon={<SaveIcon />}
          hotkey="Ctrl + S"
          onClick={fileSave}
          disabled={!canBeSaved}
        />
      </MenuBarItem>
    </MenuBar>
  );
}

/*
  <MenuBarItem label="File 2">
      <MenuNode label="Open" hotkey="Ctrl + O" />
      <MenuNode label="List 1">
        <MenuNode label="Open 1" hotkey="Ctrl + O" />
        <MenuNode label="Open 2" hotkey="Ctrl + O" />
      </MenuNode>
      <MenuNode label="List 2">
        <MenuNode label="List 2-1">
          <MenuNode label="Open 10" />
        </MenuNode>
        <MenuNode label="Open 3" hotkey="Ctrl + O" />
      </MenuNode>
    </MenuBarItem>
    <MenuBarItem label="Edit">6</MenuBarItem>
    <MenuBarItem label="View">7</MenuBarItem>
    <MenuBarItem label="Help">8</MenuBarItem>
  </MenuBar>
*/
