import { useCallback, useMemo, useState } from 'react';
import { Tabs as MuiTabs, Tab, IconButton } from '@mui/material';
import CloseIcon from '@mui/icons-material/Close';
import { DragDropContext, Draggable, DragStart, DropResult } from 'react-beautiful-dnd';
import Droppable from '../Droppable';

export interface TabInfo {
  key: string;
  label: string | React.ReactNode;
}

export interface TabInfoWithValue<T> extends TabInfo {
  value: T;
}

export interface TabsProps {
  tabs: TabInfo[];
  onChange: (tabs: TabInfo[]) => void;
  selected: string;
  onSelect: (tab: TabInfo) => void;
}

export interface TabsPropsHook<T> {
  open: (tab: TabInfoWithValue<T>) => void;
  rename: (key: string, label: string) => void;
  tabs: TabInfoWithValue<T>[];
  activeTab?: TabInfoWithValue<T>;
  props: TabsProps;
}

export const useTabs = <T,>(): TabsPropsHook<T> => {
  const [activeTab, setActiveTab] = useState<TabInfoWithValue<T>>();
  const [tabs, setTabs] = useState<TabInfoWithValue<T>[]>([]);

  const open = useCallback((tab: TabInfoWithValue<T>) => {
    setTabs((prevTabs) => {
      if (prevTabs.find((t) => t.key === tab.key)) {
        return prevTabs;
      }
      return [...prevTabs, tab];
    });
    setActiveTab(tab);
  }, []);

  const rename = useCallback((key: string, label: string) => {
    setTabs((prevTabs) => {
      return prevTabs.map((t) => (t.key === key ? { ...t, label } : t));
    });
  }, []);

  const onChange = useCallback((tabs: TabInfo[]) => {
    setTabs(tabs as TabInfoWithValue<T>[]);
    if (tabs.length === 0) {
      setActiveTab(undefined);
    }
  }, []);

  const onSelect = useCallback((tab: TabInfo) => {
    setActiveTab(tab as TabInfoWithValue<T>);
  }, []);

  const props: TabsProps = {
    tabs,
    onChange,
    selected: activeTab?.key || '',
    onSelect,
  };

  return {
    open,
    rename,
    tabs,
    activeTab,
    props,
  };
};

export const Tabs = ({ tabs, onChange, selected, onSelect }: TabsProps) => {
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
};
