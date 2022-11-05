import { useSdaContextId } from 'providers/SdaContextProvider';

interface ImagesProps {
  onSelect: (key: string) => void;
}

export default function Images({ onSelect }: ImagesProps) {
  const contextId = useSdaContextId();
  return <>Hi!</>;
}
