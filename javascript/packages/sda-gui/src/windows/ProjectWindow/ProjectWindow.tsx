import React, { useCallback, useState } from 'react';
import { ProjectWindowPayload } from 'sda-electron/api/window';
import { getProjectApi, Project } from 'sda-electron/api/project';
import { useWindowTitle, useObject } from 'hooks';
import { Box, useTheme, emphasize, Grid } from '@mui/material';
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
import { KonvaTextSelectionProvider, useKonvaTextSelection } from 'components/Konva';
import { HtmlTextSelectionProvider } from 'components/Text';
import { ImageContentStyleProvider } from 'components/ImageContent';
import { useProjectWindowStyles } from './style';
import { TabContent } from './TabContent';
import { VFileSystem } from 'components/VFileSystem';

const ImageLabel = ({ imageId }: { imageId: ObjectId }) => {
  const image = useObject(() => getImageApi().getImage(imageId));
  return <>{image && image.name}</>;
};

function ProjectPanel({ project }: { project: Project }) {
  const theme = useTheme();
  const classes = useProjectWindowStyles();
  const textSelection = useKonvaTextSelection();
  const [leftNavItem, setLeftNavItem] = useState('images');
  const tabs = useTabs<{
    imageId: ObjectId;
  }>();

  const onMouseUp = useCallback(() => {
    textSelection.stopSelecting();
  }, [textSelection]);

  return (
    <Grid container direction="column" height="100vh" wrap="nowrap" onMouseUp={onMouseUp}>
      <Grid item aria-label="project-menu-bar">
        <ProjectMenuBar />
      </Grid>
      <Grid
        item
        container
        flex={1}
        minHeight={0}
        direction="row"
        wrap="nowrap"
        aria-label="project-content"
      >
        <Grid item container width="auto" wrap="nowrap" aria-label="project-content-left">
          <Grid
            item
            width={40}
            sx={{
              backgroundColor: emphasize(theme.palette.background.default, 0.1),
            }}
            aria-label="left-nav-bar"
          >
            <LeftNavBar
              items={[
                { key: 'images', icon: <SegmentIcon /> },
                { key: 'search', icon: <SearchIcon /> },
              ]}
              selected={leftNavItem}
              onSelect={(key) => setLeftNavItem(key)}
            />
          </Grid>
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
            <Grid
              item
              height="100%"
              sx={{
                backgroundColor: emphasize(theme.palette.background.default, 0.05),
              }}
              aria-label="left-nav-content"
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
                  <VFileSystem />
                </BoxSwitchCase>
                <BoxSwitchCase value="search">
                  <Box sx={{ p: 2 }}>Search</Box>
                </BoxSwitchCase>
              </BoxSwitch>
            </Grid>
          </Resizable>
        </Grid>
        <Grid item container direction="column" wrap="nowrap" aria-label="project-content-right">
          <Grid item aria-label="project-tab-bar">
            <Tabs {...tabs.props} />
          </Grid>
          <Grid item container flex={1} minHeight={0} aria-label="project-tabs">
            {tabs.tabs.map((tab) => (
              <Grid
                item
                container
                height="100%"
                key={tab.key}
                sx={tab.key !== tabs.activeTab?.key ? { display: 'none' } : {}}
                aria-label="project-tab"
              >
                <TabContent imageId={tab.value.imageId} />
              </Grid>
            ))}
          </Grid>
        </Grid>
      </Grid>
    </Grid>
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
        <HtmlTextSelectionProvider>
          <KonvaTextSelectionProvider>
            <ImageContentStyleProvider>
              <ProjectPanel project={project} />
            </ImageContentStyleProvider>
          </KonvaTextSelectionProvider>
        </HtmlTextSelectionProvider>
      </SdaContextProvider>
    </ProjectProvider>
  );
}
