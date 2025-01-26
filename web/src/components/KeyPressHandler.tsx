import { useEffect, useCallback } from 'react';

interface KeyPressHandlerProps {
  onPressCallback: (key: string) => void;
  onReleaseCallback: (key: string) => void;
  joystickDevice?: HIDDevice | null;
}

function KeyPressHandler({
  onPressCallback,
  onReleaseCallback,
  joystickDevice
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

  /****************************************************************************/
  /* Joystick inputreport listener */

  useEffect(() => {
    /* If there's no device, do nothing */
    if (!joystickDevice) {
      return;
    }

    const handleInputReport = (e: HIDInputReportEvent) => {
      const { data } = e;
      const xAxis = data.getUint8(0);
      const yAxis = data.getUint8(1);
      // const buttons = data.getUint8(2);

      if (xAxis < 100) {
        onPressCallback('ArrowLeft');
      } else if (xAxis > 156) {
        onPressCallback('ArrowRight');
      } else {
        onReleaseCallback('ArrowLeft_release');
        onReleaseCallback('ArrowRight_release');
      }

      if (yAxis < 100) {
        onPressCallback('ArrowUp');
      } else if (yAxis > 156) {
        onPressCallback('ArrowDown');
      } else {
        onReleaseCallback('ArrowUp_release');
        onReleaseCallback('ArrowDown_release');
      }

      // if (buttons & 0x01) {
      //   onPressCallback('Control');
      // } else {
      //   onReleaseCallback('Control_release');
      // }

      // if (buttons & 0x02) {
      //   onPressCallback(' ');
      // } else {
      //   onReleaseCallback(' _release');
      // }
    };

    joystickDevice.addEventListener('inputreport', handleInputReport);

    return () => {
      joystickDevice.removeEventListener(
        'inputreport',
        handleInputReport as EventListener
      );
    };
  }, [joystickDevice, onPressCallback, onReleaseCallback]);

  /****************************************************************************/

  return null;
}

export default KeyPressHandler;
