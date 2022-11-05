import { List, ListItem, ListItemButton, ListItemIcon } from '@mui/material';

interface LeftMenuBarProps {
  items: { key: string; icon: React.ReactNode }[];
  selected: string;
  onSelect: (key: string) => void;
}

export default function LeftNavBar({ items, selected, onSelect }: LeftMenuBarProps) {
  return (
    <List disablePadding>
      {items.map(({ key, icon }) => (
        <ListItem key={key} disablePadding>
          <ListItemButton
            sx={{
              minHeight: 40,
              justifyContent: 'center',
            }}
            selected={selected === key}
            onClick={() => onSelect(key)}
          >
            <ListItemIcon
              sx={{
                minWidth: 0,
                '& svg': {
                  fontSize: 25,
                },
              }}
            >
              {icon}
            </ListItemIcon>
          </ListItemButton>
        </ListItem>
      ))}
    </List>
  );
}
