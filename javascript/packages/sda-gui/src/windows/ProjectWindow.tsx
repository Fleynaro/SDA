import React, { useState } from 'react';
import { ProjectWindowPayload } from 'sda-electron/api/window';
import { getProjectApi } from 'sda-electron/api/project';
import useWindowTitle from '../hooks/useWindowTitile';
import useObject from '../hooks/useObject';
import {
  Box,
  List,
  ListItem,
  ListItemButton,
  ListItemIcon,
  useTheme,
  Theme,
  emphasize,
} from '@mui/material';
import { makeStyles } from '@mui/styles';
import InboxIcon from '@mui/icons-material/MoveToInbox';
import { Resizable } from 're-resizable';
import { Tabs, TabPanel } from '../components/Tabs';

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

// https://codesandbox.io/s/mmrp44okvj

export default function ProjectWindow({ projectId }: ProjectWindowPayload) {
  const project = useObject(getProjectApi().getActiveProject, projectId);
  useWindowTitle(`Project: ${project?.path}`);
  const theme = useTheme();
  const classes = useStyles();
  const [activeTabId, setActiveTabId] = useState(1);
  const [tabs, setTabs] = useState([
    { id: 1, label: 'Core' },
    { id: 2, label: 'SomeLibrary' },
    { id: 3, label: 'Three' },
    { id: 4, label: 'Four' },
  ]);

  return (
    <>
      <Box sx={{ display: 'flex', height: '100vh' }}>
        <Box sx={{ display: 'flex' }}>
          <Box
            sx={{ width: 40, backgroundColor: emphasize(theme.palette.background.default, 0.1) }}
          >
            <List disablePadding>
              {['Inbox', 'Starred'].map((text) => (
                <ListItem key={text} disablePadding>
                  <ListItemButton
                    sx={{
                      minHeight: 40,
                      justifyContent: 'center',
                    }}
                  >
                    <ListItemIcon
                      sx={{
                        minWidth: 0,
                        '& svg': {
                          fontSize: 25,
                        },
                      }}
                    >
                      <InboxIcon />
                    </ListItemIcon>
                  </ListItemButton>
                </ListItem>
              ))}
            </List>
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
              Left panel
            </Box>
          </Resizable>
        </Box>
        <Box sx={{ display: 'flex', flexGrow: 1 }}>
          <Box sx={{ flexGrow: 1 }}>
            <Tabs
              tabs={tabs}
              onChange={(tabs) => setTabs(tabs)}
              activeTabId={activeTabId}
              onSelect={(tab) => setActiveTabId(tab.id)}
            />
            <TabPanel value={activeTabId} id={1}>
              Item One
            </TabPanel>
            <TabPanel value={activeTabId} id={2}>
              Item Two
            </TabPanel>
            <TabPanel value={activeTabId} id={3}>
              Item Three
            </TabPanel>
            <TabPanel value={activeTabId} id={4}>
              Item Four
            </TabPanel>
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
    </>
  );
}
