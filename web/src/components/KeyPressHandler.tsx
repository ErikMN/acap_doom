import { RefObject, useEffect } from 'react';

interface KeyPressHandlerProps {
  socketRef: RefObject<WebSocket | null>;
  target: HTMLDivElement | null;
  connected: boolean;
  onInput: (message: string) => void;
}

const PROTOCOL_VERSION = 1;

enum MessageType {
  Key = 1,
  Reset = 5
}

/* Protocol identifiers match acap_key_code in controls.h */
enum AcapKey {
  A = 1,
  B = 2,
  C = 3,
  D = 4,
  E = 5,
  F = 6,
  G = 7,
  H = 8,
  I = 9,
  J = 10,
  K = 11,
  L = 12,
  M = 13,
  N = 14,
  O = 15,
  P = 16,
  Q = 17,
  R = 18,
  S = 19,
  T = 20,
  U = 21,
  V = 22,
  W = 23,
  X = 24,
  Y = 25,
  Z = 26,
  Digit0 = 27,
  Digit1 = 28,
  Digit2 = 29,
  Digit3 = 30,
  Digit4 = 31,
  Digit5 = 32,
  Digit6 = 33,
  Digit7 = 34,
  Digit8 = 35,
  Digit9 = 36,
  Escape = 37,
  Tab = 38,
  Enter = 39,
  Space = 40,
  Backspace = 41,
  ShiftLeft = 42,
  ShiftRight = 43,
  ControlLeft = 44,
  ControlRight = 45,
  AltLeft = 46,
  AltRight = 47,
  ArrowUp = 48,
  ArrowDown = 49,
  ArrowLeft = 50,
  ArrowRight = 51,
  Comma = 52,
  Period = 53
}

const keyCodes: Record<string, AcapKey> = {
  KeyA: AcapKey.A,
  KeyB: AcapKey.B,
  KeyC: AcapKey.C,
  KeyD: AcapKey.D,
  KeyE: AcapKey.E,
  KeyF: AcapKey.F,
  KeyG: AcapKey.G,
  KeyH: AcapKey.H,
  KeyI: AcapKey.I,
  KeyJ: AcapKey.J,
  KeyK: AcapKey.K,
  KeyL: AcapKey.L,
  KeyM: AcapKey.M,
  KeyN: AcapKey.N,
  KeyO: AcapKey.O,
  KeyP: AcapKey.P,
  KeyQ: AcapKey.Q,
  KeyR: AcapKey.R,
  KeyS: AcapKey.S,
  KeyT: AcapKey.T,
  KeyU: AcapKey.U,
  KeyV: AcapKey.V,
  KeyW: AcapKey.W,
  KeyX: AcapKey.X,
  KeyY: AcapKey.Y,
  KeyZ: AcapKey.Z,
  Digit0: AcapKey.Digit0,
  Digit1: AcapKey.Digit1,
  Digit2: AcapKey.Digit2,
  Digit3: AcapKey.Digit3,
  Digit4: AcapKey.Digit4,
  Digit5: AcapKey.Digit5,
  Digit6: AcapKey.Digit6,
  Digit7: AcapKey.Digit7,
  Digit8: AcapKey.Digit8,
  Digit9: AcapKey.Digit9,
  Escape: AcapKey.Escape,
  Tab: AcapKey.Tab,
  Enter: AcapKey.Enter,
  Space: AcapKey.Space,
  Backspace: AcapKey.Backspace,
  ShiftLeft: AcapKey.ShiftLeft,
  ShiftRight: AcapKey.ShiftRight,
  ControlLeft: AcapKey.ControlLeft,
  ControlRight: AcapKey.ControlRight,
  AltLeft: AcapKey.AltLeft,
  AltRight: AcapKey.AltRight,
  ArrowUp: AcapKey.ArrowUp,
  ArrowDown: AcapKey.ArrowDown,
  ArrowLeft: AcapKey.ArrowLeft,
  ArrowRight: AcapKey.ArrowRight,
  Comma: AcapKey.Comma,
  Period: AcapKey.Period
};

function KeyPressHandler({
  socketRef,
  target,
  connected,
  onInput
}: KeyPressHandlerProps) {
  useEffect(() => {
    if (!target) {
      return;
    }

    const pressedKeys = new Set<string>();

    const send = (packet: ArrayBuffer): boolean => {
      const socket = socketRef.current;
      if (!socket || socket.readyState !== WebSocket.OPEN) {
        return false;
      }

      try {
        socket.send(packet);
        return true;
      } catch (error) {
        console.error('Failed to send input:', error);
        return false;
      }
    };

    /* Release all controls when focus or the connection changes */
    const resetInput = () => {
      send(new Uint8Array([PROTOCOL_VERSION, MessageType.Reset]).buffer);
      pressedKeys.clear();
    };

    const sendKey = (key: AcapKey, down: boolean): boolean => {
      const packet = new ArrayBuffer(5);
      const view = new DataView(packet);
      view.setUint8(0, PROTOCOL_VERSION);
      view.setUint8(1, MessageType.Key);
      view.setUint16(2, key, true);
      view.setUint8(4, down ? 1 : 0);
      return send(packet);
    };

    const handleClick = (event: MouseEvent) => {
      const element = event.target as HTMLElement | null;
      if (
        element?.closest(
          'button, input, select, textarea, a, [role="button"], [role="menuitem"], [role="slider"], [role="combobox"], [role="listbox"]'
        )
      ) {
        return;
      }
      target.focus({ preventScroll: true });
    };

    /* Handle key press */
    const handleKeyDown = (event: KeyboardEvent) => {
      if (!connected || document.activeElement !== target) {
        return;
      }
      const key = keyCodes[event.code];
      if (key === undefined) {
        return;
      }
      event.preventDefault();

      /* Ignore repeating keydown events (held down key) */
      if (event.repeat || pressedKeys.has(event.code)) {
        return;
      }
      if (sendKey(key, true)) {
        pressedKeys.add(event.code);
        onInput(`Sent: ${event.code}`);
      }
    };

    /* Handle key release */
    const handleKeyUp = (event: KeyboardEvent) => {
      if (!pressedKeys.delete(event.code)) {
        return;
      }
      event.preventDefault();
      if (sendKey(keyCodes[event.code], false)) {
        onInput(`Sent: ${event.code}_release`);
      }
    };

    const handleVisibilityChange = () => {
      if (document.visibilityState === 'hidden') {
        resetInput();
      }
    };

    /* Listen to key events */
    target.addEventListener('click', handleClick);
    target.addEventListener('blur', resetInput);
    window.addEventListener('keydown', handleKeyDown);
    window.addEventListener('keyup', handleKeyUp);
    window.addEventListener('blur', resetInput);
    document.addEventListener('visibilitychange', handleVisibilityChange);
    resetInput();

    /* Cleanup event listeners */
    return () => {
      target.removeEventListener('click', handleClick);
      target.removeEventListener('blur', resetInput);
      window.removeEventListener('keydown', handleKeyDown);
      window.removeEventListener('keyup', handleKeyUp);
      window.removeEventListener('blur', resetInput);
      document.removeEventListener('visibilitychange', handleVisibilityChange);
      resetInput();
    };
  }, [socketRef, target, connected, onInput]);

  return null;
}

export default KeyPressHandler;
