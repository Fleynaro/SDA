import { useCallback, useMemo } from 'react';
import { Tabs as MuiTabs, Tab, IconButton } from '@mui/material';
import CloseIcon from '@mui/icons-material/Close';
import { DragDropContext, Draggable, DragStart, DropResult } from 'react-beautiful-dnd';
import Droppable from '../Droppable';

type TabInfo = {
  key: string;
  label: string;
};

interface TabsProps {
  tabs: TabInfo[];
  onChange: (tabs: TabInfo[]) => void;
  selected: string;
  onSelect: (tab: TabInfo) => void;
}

export default function Tabs({ tabs, onChange, selected, onSelect }: TabsProps) {
  const selectedTabIdx = useMemo(
    () => tabs.findIndex((tab) => tab.key === selected),
    [tabs, selected],
  );

  const onDragStart = useCallback(
    (initial: DragStart) => {
      onSelect(tabs[initial.source.index]);
    },
    [tabs, onSelect],
  );

  const onDragEnd = useCallback(
    (result: DropResult) => {
      if (!result.destination) return;
      if (result.destination.index === result.source.index) return;

      const newTabList = [...tabs];
      const [removed] = newTabList.splice(result.source.index, 1);
      newTabList.splice(result.destination.index, 0, removed);
      onChange(newTabList);
      onSelect(newTabList[result.destination.index]);
    },
    [tabs, onChange, onSelect],
  );

  const onTabRemove = useCallback(
    (index: number) => {
      const newTabList = [...tabs];
      newTabList.splice(index, 1);
      onChange(newTabList);
      if (index > 0 && selectedTabIdx === index) {
        onSelect(tabs[index - 1]);
      }
    },
    [tabs, selectedTabIdx, onChange],
  );

  return (
    <DragDropContext onDragStart={onDragStart} onDragEnd={onDragEnd}>
      <Droppable droppableId="tabs" direction="horizontal">
        {(provider) => (
          <MuiTabs ref={provider.innerRef} {...provider.droppableProps} value={selectedTabIdx}>
            {tabs.map(({ key, label }, index) => (
              <Draggable
                key={key}
                draggableId={`id-${key}`}
                index={index}
                disableInteractiveElementBlocking={true}
              >
                {(provider) => (
                  <Tab
                    ref={provider.innerRef}
                    {...provider.draggableProps}
                    {...provider.dragHandleProps}
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
                          onClick={(e) => {
                            e.stopPropagation();
                            onTabRemove(index);
                          }}
                        >
                          <CloseIcon />
                        </IconButton>
                      </span>
                    }
                    onClick={() => onSelect(tabs[index])}
                  />
                )}
              </Draggable>
            ))}
            <div style={{ display: 'none' }}>{provider.placeholder}</div>
          </MuiTabs>
        )}
      </Droppable>
    </DragDropContext>
  );
}
