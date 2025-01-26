import { useEffect, useCallback } from 'react';

interface KeyPressHandlerProps {
  onPressCallback: (key: string) => void;
  onReleaseCallback: (key: string) => void;
}

function KeyPressHandler({
  onPressCallback,
  onReleaseCallback
}: KeyPressHandlerProps) {
  /* Handle key press */
  const handleKeyDown = useCallback(
    (event: KeyboardEvent) => {
      /* Ignore repeating keydown events (held down key) */
      if (event.repeat) {
        return;
      }
      onPressCallback(event.key);
    },
    [onPressCallback]
  );

  /* Handle key release */
  const handleKeyUp = useCallback(
    (event: KeyboardEvent) => {
      onReleaseCallback(`${event.key}_release`);
    },
    [onReleaseCallback]
  );

  /* Listen to key events */
  useEffect(() => {
    window.addEventListener('keydown', handleKeyDown);
    window.addEventListener('keyup', handleKeyUp);
    /* Cleanup event listeners */
    return () => {
      window.removeEventListener('keydown', handleKeyDown);
      window.removeEventListener('keyup', handleKeyUp);
    };
  }, [handleKeyDown, handleKeyUp]);

  return null;
}

export default KeyPressHandler;
