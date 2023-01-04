import { Theme, emphasize } from '@mui/material';
import { makeStyles } from '@mui/styles';

export const useProjectWindowStyles = makeStyles((theme: Theme) => ({
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
