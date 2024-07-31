import { useEffect, useCallback } from 'react';
import PropTypes from 'prop-types';

function KeyPressHandler({ onPressCallback, onReleaseCallback }) {
  /* Handle key press */
  const handleKeyDown = useCallback(
    (event) => {
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
    (event) => {
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
}

KeyPressHandler.propTypes = {
  onPressCallback: PropTypes.func.isRequired,
  onReleaseCallback: PropTypes.func.isRequired
};

export default KeyPressHandler;
