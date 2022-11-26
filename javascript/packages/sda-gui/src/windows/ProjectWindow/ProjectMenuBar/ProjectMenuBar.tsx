import { MenuBar, MenuBarItem, MenuNode } from 'components/Menu';
import { useProjectId } from 'providers/ProjectProvider';
import FileOpenIcon from '@mui/icons-material/FileOpen';

interface ProjectMenuBarProps {
  f?: number;
}

export default function ProjectMenuBar(props: ProjectMenuBarProps) {
  const projectId = useProjectId();
  return (
    <MenuBar>
      <MenuBarItem label="File">
        <MenuNode label="Open" icon={<FileOpenIcon />} hotkey="Ctrl + O" />
        <MenuNode label="List 1">
          <MenuNode label="Open 1" icon={<FileOpenIcon />} hotkey="Ctrl + O" />
          <MenuNode label="Open 2" icon={<FileOpenIcon />} hotkey="Ctrl + O" />
        </MenuNode>
        <MenuNode label="List 2">
          <MenuNode label="List 2-1">
            <MenuNode label="Open 10" icon={<FileOpenIcon />} />
          </MenuNode>
          <MenuNode label="Open 3" icon={<FileOpenIcon />} hotkey="Ctrl + O" />
        </MenuNode>
      </MenuBarItem>
      <MenuBarItem label="Edit">6</MenuBarItem>
      <MenuBarItem label="View">7</MenuBarItem>
      <MenuBarItem label="Help">8</MenuBarItem>
    </MenuBar>
  );
}
