import React, { useState } from 'react';
import { ProjectWindowPayload } from 'sda-electron/api/window';
import { getProjectApi } from 'sda-electron/api/project';
import { useWindowTitle, useObject } from 'hooks';
import { Box, useTheme, Theme, emphasize } from '@mui/material';
import { makeStyles } from '@mui/styles';
import SegmentIcon from '@mui/icons-material/Segment';
import SearchIcon from '@mui/icons-material/Search';
import { Resizable } from 're-resizable';
import { useTabs, Tabs } from 'components/Tabs';
import { LeftNavBar, Images } from './LeftNav';
import { ProjectMenuBar } from './ProjectMenuBar';
import { ProjectProvider } from 'providers/ProjectProvider';
import { SdaContextProvider } from 'providers/SdaContextProvider';
import { ObjectId } from 'sda-electron/api/common';
import { getImageApi } from 'sda-electron/api/image';
import { ImageContent } from 'components/ImageContent';
import { BoxSwitch, BoxSwitchCase } from 'components/BoxSwitch';
import { KonvaStage } from 'components/Konva';

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

const ImageLabel = ({ imageId }: { imageId: ObjectId }) => {
  const image = useObject(() => getImageApi().getImage(imageId));
  return <>{image && image.name}</>;
};

export default function ProjectWindow({ projectId }: ProjectWindowPayload) {
  const project = useObject(() => getProjectApi().getActiveProject(projectId), [projectId.key]);
  useWindowTitle(`Project: ${project?.path}`);
  const theme = useTheme();
  const classes = useStyles();
  const [leftNavItem, setLeftNavItem] = useState('images');
  const tabs = useTabs<{
    imageId: ObjectId;
  }>();

  if (!project) {
    return null;
  }

  return (
    <ProjectProvider projectId={project.id}>
      <SdaContextProvider contextId={project.contextId}>
        <Box sx={{ display: 'flex', flexDirection: 'column', height: '100vh' }}>
          <ProjectMenuBar />
          <Box sx={{ display: 'flex', flexGrow: 1 }}>
            <Box sx={{ display: 'flex' }}>
              <Box
                sx={{
                  width: 40,
                  backgroundColor: emphasize(theme.palette.background.default, 0.1),
                }}
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
                  <BoxSwitch value={leftNavItem}>
                    <BoxSwitchCase value="images">
                      <Images
                        onSelect={(image) =>
                          tabs.open({
                            key: image.id.key,
                            label: <ImageLabel imageId={image.id} />,
                            value: { imageId: image.id },
                          })
                        }
                      />
                    </BoxSwitchCase>
                    <BoxSwitchCase value="search">
                      <Box sx={{ p: 2 }}>Search</Box>
                    </BoxSwitchCase>
                  </BoxSwitch>
                </Box>
              </Resizable>
            </Box>
            <Box sx={{ display: 'flex', flexGrow: 1 }}>
              <Box sx={{ display: 'flex', flexDirection: 'column', flexGrow: 1 }}>
                <Tabs {...tabs.props} />
                <BoxSwitch value={tabs.activeTab?.key} sx={{ flexGrow: 1 }}>
                  {tabs.tabs.map((tab) => (
                    <BoxSwitchCase key={tab.key} value={tab.key}>
                      <KonvaStage sx={{ width: '100%', height: '100%' }}>
                        <ImageContent imageId={tab.value.imageId} />
                      </KonvaStage>
                    </BoxSwitchCase>
                  ))}
                </BoxSwitch>
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
        </Box>
      </SdaContextProvider>
    </ProjectProvider>
  );
}
