import { useEffect } from 'react';

export default function useWindowTitle(title: string) {
  useEffect(() => {
    document.title = title;
  }, [title]);
}
