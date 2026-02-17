# ND-100 Gateway WebSocket Protocol

Protocol specification for the WebSocket interface between the ND-100 emulator and gateway clients. This document describes the hybrid binary/JSON message protocol used over the WebSocket connection, enabling external applications to bridge remote terminal clients, disk block I/O, and HDLC frames to emulated ND-100 devices.

---

## Overview

The gateway server (`gateway.js`) is a Node.js bridge providing:

1. **Terminal I/O** -- TCP terminal clients (PuTTY/telnet) bridged to emulated terminal devices
2. **Disk block I/O** -- SMD and floppy disk images served from the host filesystem
3. **HDLC frame bridging** -- TCP endpoints bridged to emulated HDLC controllers

All traffic flows over a **single WebSocket server** on one port. The emulator's main Worker and disk I/O sub-worker each open a WebSocket connection. Message types are distinguished by frame type byte.

```
Browser                                     Gateway Server (Node.js)
+-----------------------------------+       +---------------------------+
| Main Worker (emu-worker.js)       |       |                           |
|   WebSocket -------- terminal I/O ------> | TCP :5001 <-> PuTTY/telnet|
|                +----- HDLC frames ------> | TCP :5010 <-> HDLC client |
|                +----- control JSON -----> |                           |
|                                   |       |                           |
| Disk Sub-Worker (disk-io-worker)  |       |                           |
|   WebSocket -------- block R/W ---------> | fs.readSync/writeSync     |
|   SharedArrayBuffer + Atomics     |       | (SMD0.IMG, FLOPPY.IMG)    |
+-----------------------------------+       +---------------------------+
```

### Connection Model

- **Two WebSocket connections** from the browser to the gateway:
  1. **Emulator** (first to connect) -- terminal I/O, HDLC frames, control messages
  2. **Disk sub-worker** (second to connect) -- disk block read/write
- Additional WebSocket connections are rejected with close code `4000`
- **Multiple TCP connections** from remote terminal clients to the gateway's terminal port
- All terminal I/O multiplexed over the emulator WebSocket using `identCode`

### Transport

- WebSocket (RFC 6455), single server, no path filtering
- **Binary frames** for high-frequency I/O (terminal, HDLC, disk block)
- **JSON text frames** for infrequent control messages (`register`, `client-connected`, etc.)

---

## Configuration

The gateway server reads a JSON configuration file (`gateway.conf.json`):

```json
{
  "websocket": { "port": 8765 },
  "staticDir": "",
  "terminals": { "port": 5001, "welcome": "ND-100/CX Terminal Server" },
  "hdlc": [
    { "name": "HDLC-0", "channel": 0, "port": 5010, "enabled": false },
    { "name": "HDLC-1", "channel": 1, "port": 5011, "enabled": false }
  ],
  "smd": {
    "images": [
      {
        "path": "../../SMD0.IMG",
        "name": "SINTRAN K",
        "description": "SINTRAN III/VSE version K, standard ND-100 system disk"
      },
      "../../SMD1.IMG"
    ]
  },
  "floppy": {
    "images": [
      "../../FLOPPY.IMG"
    ]
  }
}
```

Disk image entries support two formats: a simple string path (`"../../SMD1.IMG"`) or an object with metadata. Object entries provide `name` and `description` which are used in the dynamic catalog endpoint and the Glass UI Disk Manager.

| Field | Default | Description |
|-------|---------|-------------|
| `websocket.port` | 8765 | HTTP + WebSocket listen port |
| `staticDir` | `""` | Serve static files from this directory (with COOP/COEP headers) |
| `terminals.port` | 5001 | TCP listen port for remote terminal clients |
| `terminals.welcome` | `"ND-100/CX Terminal Server"` | Banner text shown to TCP clients |
| `hdlc[]` | -- | HDLC controller TCP port definitions |
| `hdlc[].channel` | -- | HDLC channel number (0-based) |
| `hdlc[].enabled` | `false` | Enable this HDLC TCP server |
| `smd.images` | `[]` | Array of SMD disk image entries (index = unit number). Each entry is either a string path or an object `{ path, name, description }`. |
| `floppy.images` | `[]` | Array of floppy disk image entries (index = unit number). Same format as `smd.images`. |

### CLI Options

```
node gateway.js [--config path] [--static dir] [--verbose]
```

| Option | Description |
|--------|-------------|
| `--config path` | Config file path (default: `gateway.conf.json`) |
| `--static dir` | Serve static files from directory (overrides config `staticDir`) |
| `--verbose` | Enable verbose logging |

When `--static` is used, the gateway serves the WASM build directory with COOP/COEP headers (`Cross-Origin-Opener-Policy: same-origin`, `Cross-Origin-Embedder-Policy: credentialless`), enabling `SharedArrayBuffer` for the disk I/O sub-worker.

### Makefile Targets

```bash
make gateway              # Start gateway with default config
make gateway-test         # Run 14 unit tests
make wasm-glass-gateway   # Build Glass UI + start unified gateway server
```

The `wasm-glass-gateway` target builds the WASM Glass UI and starts the gateway serving both static files and WebSocket on port 8765. Open `http://localhost:8765/?worker=1` in the browser.

---

## Binary Frame Type Map

All binary WebSocket frames use the first byte as a type discriminator:

| Type | Direction | Category | Description |
|------|-----------|----------|-------------|
| `0x01` | Gateway -> Emulator | Terminal | **term-input** -- keyboard data to terminal |
| `0x02` | Emulator -> Gateway | Terminal | **term-output** -- display data from terminal |
| `0x10` | Gateway -> Emulator | HDLC | **HDLC RX frame** -- data received from TCP |
| `0x11` | Emulator -> Gateway | HDLC | **HDLC TX frame** -- data to send to TCP |
| `0x12` | Gateway -> Emulator | HDLC | **HDLC carrier status** |
| `0x20` | Disk Worker -> Gateway | Disk | **Block read request** |
| `0x21` | Gateway -> Disk Worker | Disk | **Block read response** |
| `0x22` | Disk Worker -> Gateway | Disk | **Block write request** |
| `0x23` | Gateway -> Disk Worker | Disk | **Block write ack** |

---

## Terminal I/O Protocol (0x01, 0x02)

High-frequency terminal input and output uses binary WebSocket frames for minimal overhead.

### Frame Format

```
[type: 1 byte] [identCode: 1 byte] [data: N bytes]
```

The data length is implicit: `dataLength = frameLength - 2` (WebSocket provides message boundaries).

### term-input (0x01)

Keyboard input from a client, destined for a terminal device.

```
Binary frame: [0x01] [0x2B] [0x1B]
               type   id=43   ESC
```

- Each byte in the data payload is delivered individually to the emulated terminal device's input queue via `SendKeyToTerminal(identCode, byte)`.
- Common input values: ESC = 0x1B (wakes SINTRAN), CR = 0x0D, printable ASCII = 0x20-0x7E.
- The gateway server strips telnet IAC sequences (0xFF prefix) before forwarding.

### term-output (0x02)

Terminal output data from the emulator, destined for a client.

```
Binary frame: [0x02] [0x2B] [0x48] [0x65] [0x6C] [0x6C] [0x6F]
               type   id=43   'H'    'e'    'l'    'l'    'o'
```

- Data is batched per animation frame (~60 Hz). Each message may contain 1 to several hundred bytes.
- If no client is bound to the specified identCode, the data is silently discarded.
- The gateway server writes the raw bytes directly to the TCP socket.

---

## HDLC Frame Protocol (0x10, 0x11, 0x12)

HDLC frames are bridged between TCP endpoints and the emulator's HDLC device stubs.

### HDLC RX Frame (0x10) -- Gateway to Emulator

Data received from a TCP client, destined for an HDLC controller.

```
[0x10] [channel: 1] [lenHi: 1] [lenLo: 1] [data: N bytes]
```

| Offset | Size | Description |
|--------|------|-------------|
| 0 | 1 | Type = 0x10 |
| 1 | 1 | HDLC channel number |
| 2-3 | 2 | Data length (big-endian) |
| 4+ | N | Frame data bytes |

### HDLC TX Frame (0x11) -- Emulator to Gateway

Data from an HDLC controller, destined for a TCP client.

```
[0x11] [channel: 1] [lenHi: 1] [lenLo: 1] [data: N bytes]
```

Same format as 0x10 but in the opposite direction.

### HDLC Carrier Status (0x12) -- Gateway to Emulator

```
[0x12] [channel: 1] [present: 1]
```

| Offset | Size | Description |
|--------|------|-------------|
| 0 | 1 | Type = 0x12 |
| 1 | 1 | HDLC channel number |
| 2 | 1 | 0x01 = carrier present (TCP client connected), 0x00 = carrier lost |

### HDLC TCP Server

Each enabled HDLC entry in the config creates a TCP server. One TCP client per channel. Data flows as raw bytes over TCP, framed as HDLC binary messages over WebSocket.

---

## Disk Block I/O Protocol (0x20-0x23)

The disk sub-worker uses a dedicated WebSocket connection (second connection to the gateway) for synchronous block read/write of SMD and floppy disk images.

### Why a Separate Connection

The C code calls `machine_block_read()` synchronously -- it must return data immediately. But the main Worker's event loop is blocked by `Atomics.wait()` during these calls. The disk sub-worker has its own event loop and WebSocket, communicating with the main Worker via SharedArrayBuffer + Atomics.

```
C: machine_block_read()
  -> EM_JS: gateway_block_read_js()
    -> Atomics.wait() [main Worker BLOCKED]
       ...
Disk Sub-Worker: Atomics.waitAsync() fires
  -> WebSocket send [0x20 read request]
  -> WebSocket recv [0x21 read response]
  -> Copy data to SharedArrayBuffer
  -> Atomics.notify() [wakes main Worker]
       ...
Main Worker: reads data from SharedArrayBuffer
  -> returns to C code
```

### Drive Types

| Value | Type |
|-------|------|
| 0 | SMD |
| 1 | Floppy |

### Block Read Request (0x20) -- Disk Worker to Gateway

```
[0x20] [driveType: 1] [unit: 1] [offset: 4 BE] [size: 2 BE]
```

| Offset | Size | Description |
|--------|------|-------------|
| 0 | 1 | Type = 0x20 |
| 1 | 1 | Drive type (0=SMD, 1=Floppy) |
| 2 | 1 | Unit number (0-3) |
| 3-6 | 4 | Byte offset into image (big-endian) |
| 7-8 | 2 | Bytes to read (big-endian) |

Total: 9 bytes.

### Block Read Response (0x21) -- Gateway to Disk Worker

```
[0x21] [driveType: 1] [unit: 1] [data: N bytes]
```

| Offset | Size | Description |
|--------|------|-------------|
| 0 | 1 | Type = 0x21 |
| 1 | 1 | Drive type |
| 2 | 1 | Unit number |
| 3+ | N | Read data (N = requested size) |

On error: response has 4 bytes with `data[0] = 0xFF`.

### Block Write Request (0x22) -- Disk Worker to Gateway

```
[0x22] [driveType: 1] [unit: 1] [offset: 4 BE] [size: 2 BE] [data: N bytes]
```

| Offset | Size | Description |
|--------|------|-------------|
| 0 | 1 | Type = 0x22 |
| 1 | 1 | Drive type |
| 2 | 1 | Unit number |
| 3-6 | 4 | Byte offset into image (big-endian) |
| 7-8 | 2 | Bytes to write (big-endian) |
| 9+ | N | Data to write |

Total: 9 + N bytes.

### Block Write Ack (0x23) -- Gateway to Disk Worker

```
[0x23] [driveType: 1] [unit: 1] [status: 1]
```

| Offset | Size | Description |
|--------|------|-------------|
| 0 | 1 | Type = 0x23 |
| 1 | 1 | Drive type |
| 2 | 1 | Unit number |
| 3 | 1 | Status: 0x00 = OK, 0xFF = error |

### Disk Listing (JSON)

On connect, the gateway sends a JSON text frame listing available disk images:

```json
{
  "type": "disk-list",
  "smd": [
    { "unit": 0, "name": "SMD0.IMG", "size": 33554432 }
  ],
  "floppy": []
}
```

This is forwarded to the main thread and displayed in the SMD Disk Manager's "Remote Images" section.

### SharedArrayBuffer Layout

The main Worker and disk sub-worker share a 65568-byte SharedArrayBuffer (32 bytes control + 64 KB data):

```
Offset  Type         Purpose
0       Int32        control: 0=idle, 1=request pending, 2=response ready
4       Int32        driveType (0=SMD, 1=Floppy)
8       Int32        unit number
12      Int32        byte offset into image
16      Int32        size (bytes to read/write)
20      Int32        response status (0=ok, -1=error)
24      Int32        response data length
28      Int32        isWrite (0=read, 1=write)
32+     Uint8[65536] data area (64 KB -- sized for largest possible SMD block read)
```

**Requires COOP/COEP headers** on the serving page for SharedArrayBuffer support.

---

## JSON Text Protocol (Control Messages)

Infrequent control messages use JSON text WebSocket frames.

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

---

### Gateway to Emulator

#### `client-connected`

Sent when a TCP client connects to a terminal.

```json
{
  "type": "client-connected",
  "identCode": 43,
  "clientAddr": "192.168.1.5:54321"
}
```

- The emulator should restore carrier on this terminal (clear the carrier-missing flag).
- On the ND-100, this is equivalent to a modem establishing carrier detect (DCD).
- SINTRAN will then service the terminal: an ESC keypress triggers the login prompt.

#### `client-disconnected`

Sent when a TCP client disconnects.

```json
{
  "type": "client-disconnected",
  "identCode": 43
}
```

- The emulator should set carrier missing on this terminal.
- SINTRAN detects the carrier loss and cleans up the user session.

---

## Connection Lifecycle

### Startup Sequence

```
1. Gateway starts, listens on HTTP/WebSocket port and TCP terminal port
2. Emulator Worker connects via WebSocket (becomes emulator connection)
3. Emulator sends "register" message with terminal list
4. Disk sub-worker connects via WebSocket (becomes disk I/O connection)
5. Gateway sends "disk-list" JSON to disk sub-worker
6. System ready for TCP terminal clients and disk block I/O
```

### TCP Client Session

```
1. Client connects to TCP terminal port (default 5001)
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
   - TCP bytes -> binary term-input (0x01) -> emulator
   - Binary term-output (0x02) -> raw bytes -> TCP
9. On TCP disconnect:
   - Gateway sends "client-disconnected" JSON to emulator
   - Terminal becomes available for new client
```

### Telnet Negotiation

When a TCP client enters connected mode, the gateway sends:

| Sequence | Meaning |
|----------|---------|
| `FF FB 01` | IAC WILL ECHO -- server handles echo |
| `FF FB 03` | IAC WILL SUPPRESS-GO-AHEAD -- character mode |
| `FF FD 03` | IAC DO SUPPRESS-GO-AHEAD -- client should also use character mode |

Inbound telnet IAC sequences from the client are stripped and not forwarded to the emulator.

### Emulator Disconnect

When the emulator WebSocket connection closes:
- All active TCP client sessions receive "Emulator disconnected." and are closed
- The disk sub-worker WebSocket is also closed
- The terminal list is cleared
- New clients see "No terminals available" until the emulator reconnects

### Auto-Reconnect

The emulator (emu-worker.js) automatically reconnects to the gateway 3 seconds after a WebSocket disconnection, and re-sends the `register` message on reconnect.

---

## HTTP Endpoints

The gateway serves two dynamic HTTP endpoints in addition to static files:

### `/smd-catalog.json` -- Disk Image Catalog

Returns a JSON array describing all configured SMD images. Generated dynamically from `gateway.conf.json` at request time. The Glass UI fetches this to populate the "Copy to Library" dialog in the SMD Disk Manager.

**Response:**

```json
[
  {
    "name": "SINTRAN K",
    "description": "SINTRAN III/VSE version K, standard ND-100 system disk",
    "size": 33554432,
    "url": "smd-images/0",
    "available": true
  }
]
```

| Field | Type | Description |
|-------|------|-------------|
| `name` | String | Display name (from config object `name` field, or filename without extension) |
| `description` | String | Description text (from config object, or empty) |
| `size` | Number | File size in bytes (0 if file not found) |
| `url` | String | Relative URL for downloading the image |
| `available` | Boolean | `true` if the file exists on disk, `false` otherwise |

### `/smd-images/{index}` -- Image Streaming

Streams a raw SMD disk image file. The index corresponds to the position in the `smd.images` config array. Returns `Content-Type: application/octet-stream` with `Content-Length` header. Used by the Glass UI to copy gateway images into the local OPFS library.

Returns 404 if the index is out of range or the file does not exist.

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
  // Binary frame
  if (ev.data instanceof ArrayBuffer) {
    var buf = new Uint8Array(ev.data);
    if (buf.length >= 2 && buf[0] === 0x01) {
      // term-input
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
