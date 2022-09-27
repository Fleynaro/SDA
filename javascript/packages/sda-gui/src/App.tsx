import React, { useEffect } from 'react';
import Button from '@mui/material/Button';
import api from 'sda-electron/api';

export default function App() {
  const [count, setCount] = React.useState(0);
  const [dataType, setDataType] = React.useState<api.DataType | undefined>(undefined);

  useEffect(() => {
    setDataType(window.dataTypeApi.getDataTypeByName('lol'));
  }, []);

  const handleClick = () => {
    setCount(count + 1);
  };

  return (
    <div>
      <Button variant="outlined" onClick={handleClick}>Click</Button>
      {count > 0 && <p>Clicked {count} times</p>}
      {dataType?.name}
    </div>
  );
}