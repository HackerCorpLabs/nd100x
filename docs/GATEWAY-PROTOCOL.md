# ND-100 Gateway WebSocket Protocol

Protocol specification for the WebSocket interface between the ND-100 emulator and the Gateway Server. This document describes the JSON message protocol used over the WebSocket connection, enabling external applications to bridge remote terminal clients to emulated ND-100 terminal devices.

---

## Overview

The Gateway Server acts as a bridge between remote terminal clients (TCP) and the ND-100 emulator (WebSocket). The emulator connects to the gateway via a single WebSocket connection. All terminal traffic is multiplexed over this connection using `identCode` as the terminal identifier.

```
Remote Clients          Gateway Server          Emulator
(PuTTY/telnet)          (Node.js)               (Browser/WASM)

  TCP :5001  ---------> WebSocket :8765 ------> Web Worker
  raw bytes              JSON messages           WASM terminal devices
  (per terminal)         (multiplexed)           (identCode routing)
```

### Connection Model

- **One WebSocket connection** from the emulator to the gateway (additional connections are rejected with close code `4000`)
- **Multiple TCP connections** from remote clients to the gateway's terminal port
- All terminal I/O multiplexed over the single WebSocket using `identCode`

### Transport

- WebSocket (RFC 6455), text frames only
- All messages are JSON objects with a `type` field
- No binary frames are used

---

## Configuration

The gateway reads a JSON configuration file (`gateway.conf.json`):

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

## Message Reference

### Emulator to Gateway

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
| `terminals[].name` | String | Human-readable terminal name (shown in TCP menu) |
| `terminals[].logicalDevice` | Integer | SINTRAN logical device number (-1 if unknown) |

**Notes:**
- The `identCode` is the ND-100 hardware device identification code. For terminals with thumbwheels 12-19, identCodes are 43-50 (decimal), corresponding to octal 053-062.
- This message can be sent again to update the terminal list (e.g., after reconfiguration).
- The gateway replaces its stored terminal list each time a `register` message is received.

---

#### `term-output`

Terminal output data from the emulator, destined for a remote TCP client.

```json
{
  "type": "term-output",
  "identCode": 43,
  "data": [72, 101, 108, 108, 111]
}
```

| Field | Type | Description |
|-------|------|-------------|
| `identCode` | Integer | Target terminal identifier |
| `data` | Array of Integer | Output bytes (0-255). Typically 7-bit ASCII from SINTRAN. |

**Notes:**
- Data is batched: multiple characters may be sent in a single message.
- In the reference implementation, output is batched per animation frame (~60 Hz), so each message may contain anywhere from 1 to several hundred bytes.
- If no TCP client is bound to the specified `identCode`, the data is silently discarded by the gateway.

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

### Gateway to Emulator

#### `term-input`

Keyboard input from a remote TCP client, destined for a terminal device.

```json
{
  "type": "term-input",
  "identCode": 44,
  "data": [27]
}
```

| Field | Type | Description |
|-------|------|-------------|
| `identCode` | Integer | Source terminal identifier (the terminal this TCP client is connected to) |
| `data` | Array of Integer | Input bytes (0-255). Raw keyboard data. |

**Notes:**
- Each byte is delivered individually to the emulated terminal device's input queue.
- Telnet IAC sequences (0xFF prefix) are stripped by the gateway before forwarding.
- Common input values: ESC = 27 (0x1B), CR = 13 (0x0D), printable ASCII = 32-126.

---

#### `client-connected`

Sent when a TCP client selects a terminal and the connection is established.

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
| `clientAddr` | String | TCP client's IP address and port |

**Notes:**
- The emulator should restore carrier on this terminal (clear the carrier-missing flag).
- On the ND-100, this is equivalent to a modem establishing carrier detect (DCD).
- SINTRAN will then service the terminal: an ESC keypress triggers the login prompt.

---

#### `client-disconnected`

Sent when a TCP client disconnects (socket close, network drop, or quit command).

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
- The terminal becomes available for a new TCP client connection.

---

## Connection Lifecycle

### Startup Sequence

```
1. Gateway starts, listens on WebSocket port and TCP port
2. Emulator connects via WebSocket
3. Emulator sends "register" message with terminal list
4. Gateway stores terminal list, ready for TCP clients
```

### TCP Client Session

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
7. Gateway sends "client-connected" to emulator
8. Raw byte pass-through begins:
   - TCP bytes -> "term-input" JSON -> emulator
   - "term-output" JSON -> raw bytes -> TCP
9. On TCP disconnect:
   - Gateway sends "client-disconnected" to emulator
   - Terminal becomes available for new client
```

### Telnet Negotiation

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
- The terminal list is cleared
- New TCP clients see "No terminals available" until the emulator reconnects

### Auto-Reconnect

The reference emulator implementation (emu-worker.js) automatically reconnects to the gateway 3 seconds after a WebSocket disconnection, and re-sends the `register` message on reconnect.

---

## Implementing a Custom Client

To implement the emulator side of this protocol (e.g., for a different emulator or test harness):

### Minimal Implementation

```javascript
const ws = new WebSocket('ws://localhost:8765');

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
  var msg = JSON.parse(ev.data);

  switch (msg.type) {
    case 'term-input':
      // Deliver each byte to the terminal device
      for (var i = 0; i < msg.data.length; i++) {
        myEmulator.sendKeyToTerminal(msg.identCode, msg.data[i]);
      }
      break;

    case 'client-connected':
      // Restore carrier (modem DCD) on this terminal
      myEmulator.setCarrier(msg.identCode, true);
      break;

    case 'client-disconnected':
      // Set carrier missing (modem hangup)
      myEmulator.setCarrier(msg.identCode, false);
      break;
  }
};

// 3. Send terminal output (call periodically or on output)
function sendOutput(identCode, bytes) {
  ws.send(JSON.stringify({
    type: 'term-output',
    identCode: identCode,
    data: bytes  // Array of integers 0-255
  }));
}
```

### Performance Considerations

- **Batch output**: Send one `term-output` message per terminal per frame (~60 Hz), not per character. The reference implementation accumulates output bytes across multiple CPU step batches within a single 16ms frame window.
- **Input latency**: Each `term-input` byte should be delivered to the terminal device immediately. Don't batch input.
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

The protocol is designed to be extended for HDLC controller bridging:

```json
{ "type": "hdlc-frame", "controller": 0, "data": [1, 2, 3] }
{ "type": "hdlc-status", "controller": 0, "dcd": true }
```

Each HDLC controller would have its own TCP port in the gateway configuration. This is not yet implemented.
