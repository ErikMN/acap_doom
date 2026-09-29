import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import test from 'node:test';
import vm from 'node:vm';
import ts from 'typescript';

const source = readFileSync(
  new URL('../src/components/KeyPressHandler.tsx', import.meta.url),
  'utf8'
);
const compiled = ts.transpileModule(source, {
  compilerOptions: { module: ts.ModuleKind.CommonJS }
}).outputText;

function setup() {
  const window = new EventTarget();
  const document = new EventTarget();
  const target = new EventTarget();
  const packets = [];
  const messages = [];
  let cleanup;
  document.activeElement = null;
  document.visibilityState = 'visible';
  target.closest = () => null;
  target.focus = () => {
    document.activeElement = target;
  };
  const socketRef = {
    current: {
      readyState: 1,
      send(packet) {
        packets.push([...new Uint8Array(packet)]);
      }
    }
  };
  const context = {
    exports: {},
    window,
    document,
    WebSocket: { OPEN: 1 },
    console: { error() {} },
    require(name) {
      assert.equal(name, 'react');
      return {
        useEffect(effect) {
          cleanup = effect();
        }
      };
    }
  };
  vm.runInNewContext(compiled, context);
  const component = context.exports.default;
  const mount = (connected = true) =>
    component({
      socketRef,
      target,
      connected,
      onInput: (message) => messages.push(message)
    });
  mount();
  assert.deepEqual(packets.shift(), [1, 5]);
  const key = (type, code, options = {}) => {
    const event = new Event(type, { cancelable: true });
    Object.assign(event, { code, repeat: false }, options);
    window.dispatchEvent(event);
    return event;
  };
  return {
    window,
    document,
    target,
    packets,
    messages,
    socketRef,
    key,
    mount,
    cleanup: () => cleanup()
  };
}

test('game focus, physical keys, repeat suppression and release packets', () => {
  const env = setup();
  env.key('keydown', 'KeyA');
  assert.equal(env.packets.length, 0);
  env.target.dispatchEvent(new Event('click'));
  assert.equal(env.document.activeElement, env.target);
  assert.equal(env.key('keydown', 'KeyA', { key: 'A' }).defaultPrevented, true);
  env.key('keydown', 'KeyA', { repeat: true });
  env.key('keydown', 'KeyA');
  env.key('keyup', 'KeyA', { key: 'a' });
  env.key('keyup', 'KeyA');
  env.key('keydown', 'Unidentified');
  assert.deepEqual(env.packets, [
    [1, 1, 1, 0, 1],
    [1, 1, 1, 0, 0]
  ]);
  assert.deepEqual(env.messages, ['Sent: KeyA', 'Sent: KeyA_release']);
  env.cleanup();
});

test('player buttons do not capture keyboard focus', () => {
  const env = setup();
  const click = new Event('click');
  Object.defineProperty(click, 'target', { value: { closest: () => ({}) } });
  env.target.dispatchEvent(click);
  assert.equal(env.document.activeElement, null);
  env.cleanup();
});

test('blur, hidden tab, cleanup and reconnect release controls', () => {
  const env = setup();
  env.target.focus();
  for (const surface of [env.target, env.window]) {
    env.key('keydown', 'ArrowUp');
    surface.dispatchEvent(new Event('blur'));
    env.key('keyup', 'ArrowUp');
    assert.deepEqual(env.packets.splice(0), [
      [1, 1, 48, 0, 1],
      [1, 5]
    ]);
  }
  env.key('keydown', 'ControlLeft');
  env.document.visibilityState = 'hidden';
  env.document.dispatchEvent(new Event('visibilitychange'));
  assert.deepEqual(env.packets.splice(0), [
    [1, 1, 44, 0, 1],
    [1, 5]
  ]);
  env.cleanup();
  assert.deepEqual(env.packets.splice(0), [[1, 5]]);
  env.key('keydown', 'KeyA');
  assert.equal(env.packets.length, 0);
  env.socketRef.current.readyState = 3;
  env.mount(false);
  env.key('keydown', 'KeyA');
  assert.equal(env.packets.length, 0);
  env.cleanup();
  env.socketRef.current.readyState = 1;
  env.mount();
  env.key('keydown', 'KeyA');
  assert.deepEqual(env.packets, [
    [1, 5],
    [1, 1, 1, 0, 1]
  ]);
  env.cleanup();
});

test('modifier sides and strafe keys retain distinct protocol identifiers', () => {
  const env = setup();
  env.target.focus();
  const codes = [
    'ShiftLeft',
    'ShiftRight',
    'ControlLeft',
    'ControlRight',
    'AltLeft',
    'AltRight',
    'Comma',
    'Period'
  ];
  const ids = [42, 43, 44, 45, 46, 47, 52, 53];
  for (const [index, code] of codes.entries()) {
    env.key('keydown', code);
    env.key('keyup', code);
    assert.deepEqual(env.packets.splice(0), [
      [1, 1, ids[index], 0, 1],
      [1, 1, ids[index], 0, 0]
    ]);
  }
  env.cleanup();
});

test('failed sends do not mark a key as held', () => {
  const env = setup();
  env.target.focus();
  const send = env.socketRef.current.send;
  env.socketRef.current.send = () => {
    throw new Error('closed');
  };
  env.key('keydown', 'KeyA');
  env.socketRef.current.send = send;
  env.key('keydown', 'KeyA');
  assert.deepEqual(env.packets, [[1, 1, 1, 0, 1]]);
  env.cleanup();
});
