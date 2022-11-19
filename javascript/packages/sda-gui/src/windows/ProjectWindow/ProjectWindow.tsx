import React, { useState } from 'react';
import { ProjectWindowPayload } from 'sda-electron/api/window';
import { getProjectApi } from 'sda-electron/api/project';
import { useWindowTitle, useObject } from 'hooks';
import { Box, useTheme, Theme, emphasize } from '@mui/material';
import { makeStyles } from '@mui/styles';
import SegmentIcon from '@mui/icons-material/Segment';
import SearchIcon from '@mui/icons-material/Search';
import { Resizable } from 're-resizable';
import Tabs from 'components/Tabs';
import { LeftNavBar, Images } from './LeftNav';
import { SdaContextProvider } from 'providers/SdaContextProvider';

const useStyles = makeStyles((theme: Theme) => ({
  resizable: {
    width: '5px!important',
    right: '-2.5px!important',
    cursor: 'ew-resize!important',
    '&:hover, &:active': {
      backgroundColor: emphasize(theme.palette.background.default, 0.2),
      transitionDelay: '0.2s',
      transition: 'background-color 0.4s',
    },
  },
}));

export default function ProjectWindow({ projectId }: ProjectWindowPayload) {
  const project = useObject(() => getProjectApi().getActiveProject(projectId));
  useWindowTitle(`Project: ${project?.path}`);
  const theme = useTheme();
  const classes = useStyles();
  const [leftNavItem, setLeftNavItem] = useState('images');
  const [activeTab, setActiveTab] = useState('1');
  const [tabs, setTabs] = useState([
    { key: '1', label: 'Core' },
    { key: '2', label: 'Some Library' },
    { key: '3', label: 'Three' },
    { key: '4', label: 'Four' },
  ]);

  if (!project) {
    return null;
  }

  return (
    <SdaContextProvider contextId={project.contextId}>
      <Box sx={{ display: 'flex', height: '100vh' }}>
        <Box sx={{ display: 'flex' }}>
          <Box
            sx={{ width: 40, backgroundColor: emphasize(theme.palette.background.default, 0.1) }}
          >
            <LeftNavBar
              items={[
                { key: 'images', icon: <SegmentIcon /> },
                { key: 'search', icon: <SearchIcon /> },
              ]}
              selected={leftNavItem}
              onSelect={(key) => setLeftNavItem(key)}
            />
          </Box>
          <Resizable
            defaultSize={{
              width: 150,
              height: 'auto',
            }}
            enable={{ right: true }}
            handleClasses={{
              right: classes.resizable,
            }}
          >
            <Box
              sx={{
                backgroundColor: emphasize(theme.palette.background.default, 0.05),
                height: '100%',
              }}
            >
              {leftNavItem === 'images' && <Images onSelect={() => console.log('hi!')} />}
              {leftNavItem === 'search' && <div>Search</div>}
            </Box>
          </Resizable>
        </Box>
        <Box sx={{ display: 'flex', flexGrow: 1 }}>
          <Box sx={{ flexGrow: 1 }}>
            <Tabs
              tabs={tabs}
              onChange={(tabs) => setTabs(tabs)}
              selected={activeTab}
              onSelect={(tab) => setActiveTab(tab.key)}
            />
            {activeTab === '1' && <div>Core</div>}
            {activeTab === '2' && <div>SomeLibrary</div>}
          </Box>
          <Resizable
            defaultSize={{
              width: 200,
              height: 'auto',
            }}
            enable={{ left: true }}
            handleClasses={{
              left: classes.resizable,
            }}
          >
            <Box
              sx={{
                backgroundColor: emphasize(theme.palette.background.default, 0.05),
                height: '100%',
              }}
            >
              Right panel
            </Box>
          </Resizable>
        </Box>
      </Box>
    </SdaContextProvider>
  );
}
