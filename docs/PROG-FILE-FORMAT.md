# The :PROG file format

Authoritative format description (supplied 2026-07, previously living only
in session history). Implemented in `src/ndlib/load_prog.c`.

## File layout

```
Header 1 : 1 256-word (512-byte) header block
N        : N 256-word blocks, up to a maximum of 255 blocks
Header 2 : 1 256-word (512-byte) header block, starting at
             file byte offset 0x20000 (or 131072)
M        : M 256-word blocks, starting at file byte offset 0x20200,
             up to a maximum of either 255 or 256 blocks (TBD).
```

The Header 2 and M sections are optional (used for 2-bank programs only).

## Header block layout

All values are 16-bit words, big endian:

| Field |
|---|
| Start address |
| Restart address |
| First address Bank 1 |
| Last address Bank 1 |
| First address Bank 2 |
| Last address Bank 2 |

First address is where in the memory map of the (bank1/bank2) segment the
image data (N or M) should be loaded. The number of words to load is:

    (Last address - First address) + 1

For 1-bank programs, First/Last address Bank 2 are always 0xFFFF / 0x0000,
indicating no data; there is then no Header 2 / M section in the file.

## The hole in 2-bank files (FTP warning)

2-bank programs will often have a HOLE (unallocated disk pages) in the
:PROG file between the end of N and Header 2, because Header 2 always
starts at byte offset 0x20000. Because of this (and the header block), a
:PROG file cannot have a bank 1 larger than 256 words less than the
maximum ND-100 virtual memory size (64Kw); bank 2 can be a full 64Kw.

Care must be taken when copying 2-bank files off a Norsk Data system:
the FTP program, for example, will mishandle the hole.

## Example: 2-bank program (a version of the BRF-LINKER)

```
Start address        : 026111
Restart address      : 026111
First address Bank 1 : 0
Last address  Bank 1 : 071560
First address Bank 2 : 0
Last address  Bank 2 : 024263
```

To load: prepare two 64-kiloword areas (the first accessed through the
normal page table, the second through the ALTERNATIVE page table; the
application calls MON ALTON early on to enable it). Copy
((071560 - 0) + 1) * 2 bytes from byte offset 512 into area 1 at offset 0.
Copy ((024263 - 0) + 1) * 2 bytes from byte offset 131584 (131072 + 512)
into area 2 at offset 0. Start execution at address 026111.

## Example: 1-bank program

```
Start address        : 0177777
Restart address      : 0177775
First address Bank 1 : 0145000
Last address  Bank 1 : 0177777
First address Bank 2 : 0177777
Last address  Bank 2 : 0
```

Prepare a 64-kiloword area. Copy ((0177777 - 0145000) + 1) * 2 bytes from
byte offset 512 into the area at offset 0145000 and onwards; nothing is
loaded below 0145000. Start execution at address 0177777 - which must be a
JMP instruction with negative displacement, as it is the last word in the
address range.
