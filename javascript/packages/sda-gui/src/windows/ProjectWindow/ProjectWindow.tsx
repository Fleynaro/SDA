import React, { useCallback, useState } from 'react';
import { ProjectWindowPayload } from 'sda-electron/api/window';
import { getProjectApi, Project } from 'sda-electron/api/project';
import { useWindowTitle, useObject } from 'hooks';
import { Box, useTheme, emphasize } from '@mui/material';
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
import { BoxSwitch, BoxSwitchCase } from 'components/BoxSwitch';
import { KonvaFormatTextSelectionProvider, useKonvaFormatTextSelection } from 'components/Konva';
import { useProjectWindowStyles } from './style';
import { TabContent } from './TabContent';
import { ImageContentStyleProvider } from 'components/ImageContent';

const ImageLabel = ({ imageId }: { imageId: ObjectId }) => {
  const image = useObject(() => getImageApi().getImage(imageId));
  return <>{image && image.name}</>;
};

function ProjectPanel({ project }: { project: Project }) {
  const theme = useTheme();
  const classes = useProjectWindowStyles();
  const textSelection = useKonvaFormatTextSelection();
  const [leftNavItem, setLeftNavItem] = useState('images');
  const tabs = useTabs<{
    imageId: ObjectId;
  }>();

  const onMouseUp = useCallback(() => {
    textSelection.stopSelecting();
  }, [textSelection]);

  return (
    <Box sx={{ display: 'flex', flexDirection: 'column', height: '100vh' }} onMouseUp={onMouseUp}>
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
              width: 170,
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
                <BoxSwitchCase key={tab.key} value={tab.key} sx={{ display: 'flex' }}>
                  <TabContent imageId={tab.value.imageId} />
                </BoxSwitchCase>
              ))}
            </BoxSwitch>
          </Box>
        </Box>
      </Box>
    </Box>
  );
}

export default function ProjectWindow({ projectId }: ProjectWindowPayload) {
  const project = useObject(() => getProjectApi().getActiveProject(projectId), [projectId.key]);
  useWindowTitle(`Project: ${project?.path}`);

  if (!project) {
    return null;
  }

  return (
    <ProjectProvider projectId={project.id}>
      <SdaContextProvider contextId={project.contextId}>
        <KonvaFormatTextSelectionProvider>
          <ImageContentStyleProvider>
            <ProjectPanel project={project} />
          </ImageContentStyleProvider>
        </KonvaFormatTextSelectionProvider>
      </SdaContextProvider>
    </ProjectProvider>
  );
}
