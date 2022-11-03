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
  Tabs,
  Tab,
  IconButton,
} from '@mui/material';
import { makeStyles } from '@mui/styles';
import InboxIcon from '@mui/icons-material/MoveToInbox';
import CloseIcon from '@mui/icons-material/Close';
import { Resizable } from 're-resizable';
import { DragDropContext, Draggable, DragStart, DropResult } from 'react-beautiful-dnd';
import Droppable from '../components/Droppable';

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

  const [activeTab, setActiveTab] = useState(0);
  const [tabs, setTabs] = useState([
    { id: 1, label: 'Core' },
    { id: 2, label: 'SomeLibrary' },
    { id: 3, label: 'Three' },
    { id: 4, label: 'Four' },
  ]);
  const onDragStart = (initial: DragStart) => {
    setActiveTab(initial.source.index);
  };
  const onDragEnd = (result: DropResult) => {
    if (!result.destination) return;
    if (result.destination.index === result.source.index) return;

    const newTabs = [...tabs];
    const [removed] = newTabs.splice(result.source.index, 1);
    newTabs.splice(result.destination.index, 0, removed);
    setTabs(newTabs);
    setActiveTab(result.destination.index);
  };

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
            <DragDropContext onDragStart={onDragStart} onDragEnd={onDragEnd}>
              <Droppable droppableId="tabs" direction="horizontal">
                {(props) => (
                  <Tabs ref={props.innerRef} {...props.droppableProps} value={activeTab}>
                    {tabs.map(({ id, label }, index) => (
                      <Draggable
                        key={id}
                        draggableId={`id-${id}`}
                        index={index}
                        disableInteractiveElementBlocking={true}
                      >
                        {(props) => (
                          <Tab
                            ref={props.innerRef}
                            {...props.draggableProps}
                            {...props.dragHandleProps}
                            label={
                              <span>
                                {label}
                                <IconButton
                                  sx={{
                                    ml: 2,
                                    mb: 1,
                                    width: 0,
                                    height: 0,
                                    transform: 'scale(0.8)',
                                  }}
                                  onClick={() => console.log('close')}
                                >
                                  <CloseIcon />
                                </IconButton>
                              </span>
                            }
                            onClick={() => setActiveTab(index)}
                          />
                        )}
                      </Draggable>
                    ))}
                    <div style={{ display: 'none' }}>{props.placeholder}</div>
                  </Tabs>
                )}
              </Droppable>
            </DragDropContext>
            Content
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
