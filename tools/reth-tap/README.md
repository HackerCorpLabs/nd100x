# reth-tap - put a gateway ethernet segment on the host network

The gateway (`tools/nd100-gateway/gateway.js`) runs an ethernet segment per
entry in its config, and everything on that segment sees everything else: a
browser NDIX, a native `nd500x`, another gateway. What was missing was a way
for **the host itself** to be on that wire. This is that bridge: a TAP device
on one side, an ordinary RETH member on the other.

Neither half is new. `nd500x` already contains both - `uplink_tap.c` opens the
TAP, `uplink_tcp.c` speaks RETH - but they live *inside* the emulator, wired to
`nd500_xmsg`. A browser machine has no such process to borrow them from, so
they are re-joined here as a program of their own.

**The gateway needs no changes and no configuration for this.** Any RETH client
may join a segment.

## Build

```sh
make
```

Linux only, libc only.

## Use

Create the TAP device once - this needs root, and it is deliberately not done
by the bridge (see below):

```sh
sudo <nd500x>/tools/ndix-tap.sh up      # creates nd0, 223.255.254.1/24
```

Then, with the gateway running:

```sh
./reth-tap                              # nd0 <-> 127.0.0.1:3094
./reth-tap --dev nd0 --host 127.0.0.1 --port 3094
```

| option | default | meaning |
| --- | --- | --- |
| `--dev` | `nd0` | TAP device to attach to. Must already exist. |
| `--host` | `127.0.0.1` | gateway address |
| `--port` | `3094` | ethernet segment port (the ND Ethernet II PCB number) |
| `--quiet` | off | suppress the periodic frame counters |

In the guest, give NDIX an address on the same subnet:

```sh
/etc/etconfig et0 0x08 0x00 0x26 0xF4 0x01 0x00
/etc/ifconfig et0 inet 223.255.254.8 -trailers up
```

`-trailers` is **not** optional - NDIX's ARP layer advertises trailer
encapsulation that its own `et` driver refuses to send, and without the flag
every IP packet dies inside `etoutput` with `EPROTONOSUPPORT`. See nd500x's
`docs/NDIX-NETWORKING.md`.

Then from the host: `ping 223.255.254.8`.

## What it will not do

Create the TAP device, give it an address, or bring it up. Those are root
operations that change host networking, and a program doing them silently is
hard to undo and easy not to notice - the same reasoning as `nd500x`'s.

The bridge goes further than nd500x here and **refuses** a device name that is
not already an interface. `TUNSETIFF` will happily create one when the name is
free, and on a box where `/dev/net/tun` is world-writable it may well succeed -
which is the worst outcome available: the bridge would attach, join the segment
and report frames moving, while the device it made has no address, no route and
vanishes on exit. So the host would not be on the wire, and nothing would say
so.

## Tests

```sh
make test      # framing only: no privilege, no network, no gateway
make test-gw   # end-to-end against a throwaway gateway (needs node)
make check     # both
```

`test-framing` drives the real pump functions over socketpairs: the big-endian
length prefix, several frames arriving in one read, one frame split across two
reads, an impossible length, a peer going away, a maximum-size frame.

`test-gateway` starts a real `gateway.js` on ports nothing else uses, joins it
with the bridge's own `reth_connect()` **and** a separate plain RETH client, and
sends a frame each way through the gateway's real repeat logic - including the
check that a member never receives its own frame back.

Neither test creates a TAP device. The TAP fd is a datagram socketpair, which
has the one property the code depends on: a read returns exactly one frame.
Everything except the literal `TUNSETIFF` is exercised.
