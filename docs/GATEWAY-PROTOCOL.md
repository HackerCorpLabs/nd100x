# ND-100 Gateway WebSocket Protocol

Protocol specification for the WebSocket interface between the ND-100 emulator and gateway clients. This document describes the hybrid binary/JSON message protocol used over the WebSocket connection, enabling external applications to bridge remote terminal clients to emulated ND-100 terminal devices.

---

## Overview

The protocol supports two types of gateway clients:

1. **Gateway Server** (`gateway.js`) — Node.js bridge between TCP terminal clients (PuTTY/telnet) and the emulator
2. **RetroTerm** — Native terminal emulator that connects directly via WebSocket

Both use the same protocol over a single WebSocket connection. All terminal traffic is multiplexed using `identCode` as the terminal identifier.

```
Remote Clients          Gateway Server          Emulator
(PuTTY/telnet)          (Node.js)               (Browser/WASM)

  TCP :5001  ---------> WebSocket :8765 ------> Web Worker
  raw bytes              binary + JSON           WASM terminal devices
  (per terminal)         (multiplexed)           (identCode routing)

RetroTerm ------WebSocket :8765----------------> Web Worker
  native app             binary + JSON           WASM terminal devices
```

### Connection Model

- **One WebSocket connection** from the emulator to the gateway (additional connections are rejected with close code `4000`)
- **Multiple TCP connections** from remote clients to the gateway's terminal port (gateway.js only)
- **Multiple virtual connections** from RetroTerm tabs, each bound to one identCode
- All terminal I/O multiplexed over the single WebSocket using `identCode`

### Transport

- WebSocket (RFC 6455)
- **Binary frames** for high-frequency terminal I/O (`term-input`, `term-output`)
- **JSON text frames** for infrequent control messages (`register`, `client-connected`, `client-disconnected`)

---

## Configuration

The gateway server reads a JSON configuration file (`gateway.conf.json`):

```json
{
  "websocket": {
    "port": 8765
  },
  "terminals": {
    "port": 5001,
    "welcome": "ND-100/CX Terminal Server"
  },
  "hdlc": [
    { "name": "HDLC-0", "port": 5010, "enabled": false },
    { "name": "HDLC-1", "port": 5011, "enabled": false }
  ]
}
```

| Field | Default | Description |
|-------|---------|-------------|
| `websocket.port` | 8765 | WebSocket listen port for emulator connection |
| `terminals.port` | 5001 | TCP listen port for remote terminal clients |
| `terminals.welcome` | `"ND-100/CX Terminal Server"` | Banner text shown to TCP clients |
| `hdlc[]` | — | Future: HDLC controller TCP ports (not yet implemented) |

---

## Binary Frame Protocol (Terminal I/O)

High-frequency terminal input and output uses binary WebSocket frames for minimal overhead.

### Frame Format

```
[type: 1 byte] [identCode: 1 byte] [data: N bytes]
```

| Type | Direction | Description |
|------|-----------|-------------|
| `0x01` | Client → Emulator | **term-input** — keyboard data from terminal to host |
| `0x02` | Emulator → Client | **term-output** — display data from host to terminal |

The data length is implicit: `dataLength = frameLength - 2` (WebSocket provides message boundaries).

### Examples

Sending ESC key (0x1B) to identCode 43:
```
Binary frame: [0x01] [0x2B] [0x1B]
               type   id=43   ESC
               3 bytes total
```

Emulator sending "Hello" to identCode 43:
```
Binary frame: [0x02] [0x2B] [0x48] [0x65] [0x6C] [0x6C] [0x6F]
               type   id=43   'H'    'e'    'l'    'l'    'o'
               7 bytes total
```

A 100-byte terminal output = 102 bytes on the wire. No JSON parsing, no encoding overhead.

### term-input (0x01)

Keyboard input from a client, destined for a terminal device.

- Each byte in the data payload is delivered individually to the emulated terminal device's input queue via `SendKeyToTerminal(identCode, byte)`.
- Common input values: ESC = 0x1B (wakes SINTRAN), CR = 0x0D, printable ASCII = 0x20-0x7E.
- The gateway server strips telnet IAC sequences (0xFF prefix) before forwarding.

### term-output (0x02)

Terminal output data from the emulator, destined for a client.

- Data is batched per animation frame (~60 Hz). Each message may contain 1 to several hundred bytes.
- If no client is bound to the specified identCode, the data is silently discarded.
- The gateway server writes the raw bytes directly to the TCP socket.
- RetroTerm feeds the bytes to its TDV terminal emulator.

---

## JSON Text Protocol (Control Messages)

Infrequent control messages use JSON text WebSocket frames.

### Emulator to Client

#### `register`

Sent immediately after WebSocket connection is established. Declares the list of available remote terminals.

```json
{
  "type": "register",
  "terminals": [
    {
      "identCode": 43,
      "name": "TERMINAL 12",
      "logicalDevice": 51
    },
    {
      "identCode": 44,
      "name": "TERMINAL 13",
      "logicalDevice": 52
    }
  ]
}
```

| Field | Type | Description |
|-------|------|-------------|
| `terminals` | Array | List of terminal descriptors |
| `terminals[].identCode` | Integer | Unique device identifier (decimal). Used in all subsequent messages to route I/O. |
| `terminals[].name` | String | Human-readable terminal name (shown in TCP menu / connection dialog) |
| `terminals[].logicalDevice` | Integer | SINTRAN logical device number (-1 if unknown) |

**Notes:**
- The `identCode` is the ND-100 hardware device identification code. For terminals with thumbwheels 12-19, identCodes are 43-50 (decimal), corresponding to octal 053-062.
- This message can be sent again to update the terminal list (e.g., after reconfiguration).
- The gateway replaces its stored terminal list each time a `register` message is received.

---

#### `carrier`

Reports carrier status changes on a terminal device.

```json
{
  "type": "carrier",
  "identCode": 43,
  "missing": true
}
```

| Field | Type | Description |
|-------|------|-------------|
| `identCode` | Integer | Terminal identifier |
| `missing` | Boolean | `true` = carrier lost, `false` = carrier present |

**Notes:**
- This message is informational. The gateway logs it but takes no action.
- Carrier status is managed by the emulator's terminal device logic.

---

### Client to Emulator

#### `client-connected`

Sent when a client connects to a terminal (TCP client selects terminal, or RetroTerm tab connects).

```json
{
  "type": "client-connected",
  "identCode": 43,
  "clientAddr": "192.168.1.5:54321"
}
```

| Field | Type | Description |
|-------|------|-------------|
| `identCode` | Integer | Terminal identifier that the client connected to |
| `clientAddr` | String | (Optional) TCP client's IP address and port |

**Notes:**
- The emulator should restore carrier on this terminal (clear the carrier-missing flag).
- On the ND-100, this is equivalent to a modem establishing carrier detect (DCD).
- SINTRAN will then service the terminal: an ESC keypress triggers the login prompt.

---

#### `client-disconnected`

Sent when a client disconnects (socket close, network drop, tab close, or quit command).

```json
{
  "type": "client-disconnected",
  "identCode": 43
}
```

| Field | Type | Description |
|-------|------|-------------|
| `identCode` | Integer | Terminal identifier that the client was connected to |

**Notes:**
- The emulator should set carrier missing on this terminal.
- On the ND-100, this signals modem hangup. SINTRAN detects the carrier loss via the terminal input status register (bit 11) and cleans up the user session.
- The terminal becomes available for a new client connection.

---

## Connection Lifecycle

### Startup Sequence

```
1. Gateway/RetroTerm starts, listens on WebSocket port (or connects to emulator)
2. Emulator connects via WebSocket
3. Emulator sends "register" message with terminal list
4. Gateway stores terminal list, ready for clients
```

### TCP Client Session (gateway.js)

```
1. Client connects to TCP terminal port
2. Gateway sends welcome banner + numbered terminal menu
3. Client sends terminal number (line-buffered, CR/LF terminated)
4. Gateway validates selection:
   - 0 = disconnect
   - Invalid/in-use = error message, re-show menu
   - Valid = bind client to terminal
5. Gateway sends "Connected to TERMINAL N" confirmation
6. Gateway sends telnet negotiation (IAC WILL ECHO, WILL/DO SGA)
7. Gateway sends "client-connected" JSON to emulator
8. Raw byte pass-through begins:
   - TCP bytes -> binary term-input frame -> emulator
   - Binary term-output frame -> raw bytes -> TCP
9. On TCP disconnect:
   - Gateway sends "client-disconnected" JSON to emulator
   - Terminal becomes available for new client
```

### RetroTerm Session

```
1. RetroTerm hosts WebSocket listener on configured port
2. Emulator connects and sends "register" message
3. User opens connection dialog, selects Gateway protocol, picks terminal
4. RetroTerm sends "client-connected" JSON to emulator
5. Terminal I/O flows as binary frames:
   - Keyboard input -> binary term-input (0x01) -> emulator
   - Binary term-output (0x02) -> TDV terminal emulator -> screen
6. On disconnect:
   - RetroTerm sends "client-disconnected" JSON to emulator
   - Terminal becomes available
```

### Telnet Negotiation (gateway.js only)

When a TCP client enters connected mode, the gateway sends:

| Sequence | Meaning |
|----------|---------|
| `FF FB 01` | IAC WILL ECHO — server handles echo |
| `FF FB 03` | IAC WILL SUPPRESS-GO-AHEAD — character mode |
| `FF FD 03` | IAC DO SUPPRESS-GO-AHEAD — client should also use character mode |

This switches telnet clients from line mode (buffered) to character-at-a-time mode with remote echo. Raw TCP clients (PuTTY raw mode, netcat) ignore these bytes.

Inbound telnet IAC sequences from the client are stripped and not forwarded to the emulator.

### Emulator Disconnect

When the WebSocket connection closes:
- All active TCP client sessions receive "Emulator disconnected." and are closed
- All active RetroTerm gateway connections transition to Disconnected state
- The terminal list is cleared
- New clients see "No terminals available" until the emulator reconnects

### Auto-Reconnect

The emulator (emu-worker.js) automatically reconnects to the gateway 3 seconds after a WebSocket disconnection, and re-sends the `register` message on reconnect.

---

## Implementing a Custom Client

To implement the emulator side of this protocol (e.g., for a different emulator or test harness):

### Minimal Implementation

```javascript
const ws = new WebSocket('ws://localhost:8765');
ws.binaryType = 'arraybuffer';

// 1. Register terminals on connect
ws.onopen = function() {
  ws.send(JSON.stringify({
    type: 'register',
    terminals: [
      { identCode: 43, name: 'TERMINAL 12', logicalDevice: -1 },
      { identCode: 44, name: 'TERMINAL 13', logicalDevice: -1 }
    ]
  }));
};

// 2. Handle incoming messages
ws.onmessage = function(ev) {
  // Binary frame: term-input
  if (ev.data instanceof ArrayBuffer) {
    var buf = new Uint8Array(ev.data);
    if (buf.length >= 2 && buf[0] === 0x01) {
      var identCode = buf[1];
      for (var i = 2; i < buf.length; i++) {
        myEmulator.sendKeyToTerminal(identCode, buf[i]);
      }
    }
    return;
  }

  // JSON text frame: control messages
  var msg = JSON.parse(ev.data);
  switch (msg.type) {
    case 'client-connected':
      myEmulator.setCarrier(msg.identCode, true);
      break;
    case 'client-disconnected':
      myEmulator.setCarrier(msg.identCode, false);
      break;
  }
};

// 3. Send terminal output as binary frame
function sendOutput(identCode, bytes) {
  var frame = new Uint8Array(2 + bytes.length);
  frame[0] = 0x02;  // term-output
  frame[1] = identCode & 0xFF;
  for (var i = 0; i < bytes.length; i++) {
    frame[2 + i] = bytes[i];
  }
  ws.send(frame.buffer);
}
```

### Performance Considerations

- **Batch output**: Send one `term-output` binary frame per terminal per frame (~60 Hz), not per character. The reference implementation accumulates output bytes across multiple CPU step batches within a single 16ms frame window.
- **Input latency**: Each `term-input` byte should be delivered to the terminal device immediately. Don't batch input.
- **Binary frames**: Always use binary WebSocket frames for `term-input` and `term-output`. JSON text frames are only for control messages.
- **identCode range**: identCodes are ND-100 hardware device codes. The standard range for terminals is 1-62 (octal 001-076). Your implementation defines which identCodes to register.

---

## ND-100 Terminal Device Details

For implementors working with the ND-100 terminal hardware model:

### Terminal Identification

| Thumbwheel | Terminal Name | identCode (oct) | identCode (dec) | I/O Address (oct) |
|------------|--------------|------------------|------------------|--------------------|
| 0 | Console (TERMINAL 1) | 001 | 1 | 300-307 |
| 5 | TERMINAL 5 | 044 | 36 | 340-347 |
| 6 | TERMINAL 6 | 045 | 37 | 350-357 |
| 7 | TERMINAL 7 | 046 | 38 | 360-367 |
| 8 | TERMINAL 8 | 047 | 39 | 370-377 |
| 9 | TERMINAL 9 | 050 | 40 | 1300-1307 |
| 10 | TERMINAL 10 | 051 | 41 | 1310-1317 |
| 11 | TERMINAL 11 | 052 | 42 | 1320-1327 |
| 12 | TERMINAL 12 | 053 | 43 | 1330-1337 |
| 13 | TERMINAL 13 | 054 | 44 | 1340-1347 |
| 14 | TERMINAL 14 | 055 | 45 | 1350-1357 |
| 15 | TERMINAL 15 | 056 | 46 | 1360-1367 |
| 16 | TERMINAL 16 | 057 | 47 | 1370-1377 |
| 17 | TERMINAL 17 | 060 | 48 | 200-207 |
| 18 | TERMINAL 18 | 061 | 49 | 210-217 |
| 19 | TERMINAL 19 | 062 | 50 | 220-227 |

### Carrier Detection

The ND-100 terminal input status register (IOX address + 1) has bit 11 as the carrier-missing flag. SINTRAN polls this bit when processing terminal input. Setting carrier missing causes SINTRAN to log out the user and release the terminal.

### Input Status Register (Read, offset +1)

| Bit | Name | Description |
|-----|------|-------------|
| 0 | Interrupt Enabled | Data-available interrupt enabled |
| 2 | Device Activated | Terminal is active |
| 3 | Ready For Transfer | Input data available |
| 4 | Error OR | OR of bits 5-7 |
| 5 | Framing Error | UART framing error |
| 6 | Parity Error | Parity check failed |
| 7 | Overrun | Input buffer overrun |
| 11 | Carrier Missing | Modem carrier detect lost |

---

## Future: HDLC Extension

The protocol is designed to be extended for HDLC controller bridging. Binary frame types could be assigned for HDLC data:

| Type | Description |
|------|-------------|
| `0x10` | HDLC frame data (controller ID in identCode byte) |
| `0x11` | HDLC status/control |

Each HDLC controller would have its own TCP port in the gateway configuration. This is not yet implemented.
