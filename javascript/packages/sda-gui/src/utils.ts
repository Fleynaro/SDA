export const strContains = (str: string, search: string): boolean => {
  return str.toLowerCase().includes(search.toLowerCase());
};

export const animate = (updater: (progress: number) => void, duration: number) => {
  const start = performance.now();
  const update = (now: number) => {
    const progress = Math.max(Math.min((now - start) / duration, 1), 0);
    updater(progress);
    if (progress < 1) {
      requestAnimationFrame(update);
    }
  };
  requestAnimationFrame(update);
};

export const nullFunction = () => {
  throw new Error('Function is not set');
};
