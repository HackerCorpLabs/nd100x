# The :BPUN file format (Bootable Punched Tape)

Authoritative format description (supplied 2026-07, previously living only
in session history). Implemented in `src/ndlib/load_bpun.c`. The costly
mistake this document exists to prevent: the program ENTRY POINT is the
**Start** field - entering at the Boot field (often 0) sends the CPU on a
runaway (the MAC "hang" was exactly this, not a missing MON call).

## Layout

```
Preamble  Start  Boot  !  Address  Count  Data  Checksum  Action
```

| Field | Meaning |
|---|---|
| Preamble | May contain any characters except `!` (41 octal, 0x21). Its original use was a bootstrap loader for stand-alone CPUs. This is no longer used, but various tools still generate the bootstrap loader, including the BPUN command in NRL. |
| Start | ASCII encoded octal START ADDRESS for the program. Terminated by carriage return (15 octal) and optionally a line feed (12 octal). |
| Boot | ASCII encoded octal value giving the start address of the bootstrap loader (see Preamble), terminated by the `!` character (i.e. the next field). No longer used, but still created by tools that generate the bootstrap loader. |
| `!` | Exclamation mark: start-of-data signal. |
| Address | Address where the binary load of the data will start. Two-byte word, big endian. |
| Count | Number of 16-bit words in the following Data field. Two-byte word, big endian. |
| Data | `Count` 16-bit words, big endian. |
| Checksum | Arithmetic sum of all the words in the Data field, modulo 2^16. Two-byte word, big endian. |
| Action | Two-byte field. If ZERO, execution starts at the address in the Start field. If NON-ZERO, the CPU remains in OPCOM mode with the P register set to the Start field value. |

## Encoding notes

- Everything before the `!` mark is 7-bit ASCII with EVEN PARITY, except
  that there may be a string of binary zeroes before the actual preamble
  starts. CR/LF also use parity, so CR is encoded as 0x8D.
- The preamble (bootstrap) is code encoded as octal values of the
  instructions, with carriage return and line feed between each
  instruction. It starts with a command that instructs the hardware
  loader where in memory to load the following code: an address followed
  by the `/` character.

## Loader rules (what load_bpun.c must honor)

- Entry point = Start field. The Boot field is NOT the entry point;
  Boot = 0 is valid and common.
- Action == 0: autostart at Start. Action != 0: stop in OPCOM with
  P = Start (the shell maps this to CPU_STOPPED).
- Verify Checksum against the summed Data words.
