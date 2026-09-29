Run the native input tests from the repository root:

```sh
sh tests/run-controls.sh
```

This compiles the production input queue and DOOM event adapter with the host C
compiler. It checks packet validation, key ordering, modifiers, resets, queue
overflow, the engine event buffer, and concurrent input.

Run the browser input tests after installing the web dependencies:

```sh
node --test web/tests/key-input.test.mjs
```

These exercise the keyboard handler with simulated focus, keyboard, connection,
and visibility events. They do not run the video player or a real browser.

The control endpoint remains `/local/acap_doom/control`. Binary protocol version
1 accepts two packet types:

- Key: version byte `1`, type byte `1`, a two-byte little-endian key identifier,
  then state byte `1` for pressed or `0` for released.
- Reset: version byte `1`, type byte `5`.

Key identifiers match Quake II for values 1 through 51. DOOM adds comma as 52
and period as 53. Other message types are rejected. Update the web interface and
ACAP binary together when deploying this protocol.
