import { Grid, Paper } from '@mui/material';

export const ConstantValuePopper = ({ value }: { value: number }) => {
  return (
    <Paper sx={{ p: 5 }}>
      <Grid container direction="column">
        <Grid item>Hex: 0x{value.toString(16)}</Grid>
        <Grid item>Dec: {value}</Grid>
        <Grid item>Bin: 0b{value.toString(2)}</Grid>
      </Grid>
    </Paper>
  );
};
