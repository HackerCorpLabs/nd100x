# nd100x
ND-100/CX Emulator written in C

For more information about the ND-100 series of minicomputers: https://www.ndwiki.org/wiki/ND-100

# Origins

This project (nd100x) is a fork of nd100em, started in 2025 by Ronny Hansen.
It remains under the GNU General Public License (GPL v2 or later).

This project is based upon the source code from the nd100em project.
Their latest version was version 0.2.4, which can be found here https://github.com/tingox/nd100em
Read more about the nd100em project here https://www.ndwiki.org/wiki/ND100_emulator_project

The original authors of the nd100em are:
* Per-Olof Åström
* Roger Abrahamsson
* Zdravko Dimitrov
* Göran Axelsson

## Status
The emulator is under active development. Current work in progress includes:

* All test programs validate the CPU, Memory Management and Devices.
* Missing some opcodes around BCD
* Boots SINTRAN L from SMD in 6-7 seconds on my machine.

## Improvements

This project continues from nd100em version 0.2.4 and includes significant enhancements:

- **Opcode Improvements**
  - Added support for missing opcodes
  - Fixed bugs in several instructions
- **Memory Management**
  - Fully re-implemented with support for MMS1 and MMS2
  - Proper TRAP generation for Page Faults and Access Violations
- **IO Subsystem**
  - Complete rewrite: now single-threaded and simplified
  - Old IO code has been removed
- **Supported IO Devices**
  - Real-Time Clock (20ms tick emulation pending for full SINTRAN compatibility)
  - Console
  - Floppy (PIO and DMA) for 8" and 5.25" formats
  - SMD Hard Disk (75MB; multi-drive support in progress)
- **Codebase Modernization**
  - Removed requirement for libconfig and the supporting .conf file. All options are now given on the command line.
  - Refactored folder structure
  - Automatic function header generation via [mkptypes](https://github.com/OrangeTide/mkptypes) (by Eric R. Smith)

 ## Project Goals

* Focus on ND-100 CPU and controllers. 
  * The newer CPU's (ND-110 and ND-120) is mostly hardware/performance improvements, with some new opcodes to support SINTRAN and COBOL running with better performance. 
  * Focus on getting these opcodes into this emulator has very low priority, as SINTRAN doesnt require an ND-110 or ND-120 CPU.
* Refactorand, re-structure and modernize code so its easier to extend
* Adding support for building different frontends
  * The only frontend currently is the nd100x emulator 
  * Other frond ends planned
    * Web Assembly version (WASM) for running emulator in browser and in Visual Studio Code as plugin
    * Arduino, ESP and RISC-V device support.
    * Emulator with debug interface
    * Windows version
* Adding even more devices (like HDLC, Ethernet, SCSI)



## Project Structure
The project is organized into several key components:

- `src/cpu/` - CPU emulation implementation
- `src/devices/` - Device emulation (I/O, peripherals)
- `src/machine/` - Machine state and main emulation loop
- `src/ndlib/` - Supporting library functions (loading BPUN and a.out formats++)
- `src/frontend/` - User interface and emulator frontend(s)
- `tools/` - Development and build tools
- `images/` - Norsk-Data SMD disk, floppy and BPUN files.
- `build/` - Build output directory


## Build Requirements
- Linux (I am using Ubuntu 22.04) under WSL
- GCC compiler (I am using GCC 11.4.0)
- Make build system
- mkptypes tool (automatically built during compilation)

## Building the Project

1. Clone the repository:
```bash
git clone https://github.com/HackerCorpLabs/nd100x.git
cd nd100x
```

2. Build the project:
- For debug build (default):
```bash
make debug
```
- For release build:
```bash
make release
```
- For build with sanitizers:
```bash
make sanitize
```

The build system supports three different build types:
- `debug`: Includes debug symbols and disables optimizations
- `release`: Enables high-level optimizations for best performance
- `sanitize`: Includes Address Sanitizer for debugging memory issues

### Command Line Options for nd100x

The emulator supports the following command line options:

```bash
Usage: nd100x [options]

Options:
  -b, --boot=TYPE    Boot type (bp, bpun, aout, floppy, smd)
  -i, --image=FILE   Image file to load
  -s, --start=ADDR   Start address (default: 0)
  -d, --disasm       Enable disassembly output (dump after emulator stops)
  -v, --verbose      Enable verbose output
  -h, --help         Show this help message

Examples:

  build/bin/nd100x -b aout -i a.out -v -d    # Loads an a.out file from the current directory in verbose mode and enables disassembly

  build/bin/nd100x -b bpun -i images/FILSYS-INV-Q04.BPUN      # Loads the latest version of FILSYSTEM INVESTIGATOR (Version: Q04 - 1987-10-10)
  build/bin/nd100x -b bpun -i images/CONFIGURATIO-C08.BPUN    # Loads an old version of CONFIGURATION-C08
  build/bin/nd100x -b bpun -i images/INSTRUCTION-B.BPUN       # Loads an old version of INSTRUCTION VERIFIER

  build/bin/nd100x -b floppy                                  # Boots from a floppy file named FLOPPY.IMG
  
  
```

Boot Types:
- `smd`: SMD disk boot (default)
- `bp`: Boot program
- `bpun`: Boot program unprotected
- `aout`: BSD 2.11 a.out format
- `floppy`: Floppy disk boot

## Block devices (Floppy and SMD)

Currently the file names used for floppy and SMD are hard coded.
And the files are expected to be in the current folder.

* Floppy drives uses FLOPPY.IMG 
* SMD  drives uses SMD0.IMG, SMD1.IMG, SMD2.IMG and SMD3.IMG. 

## Running SINTRAN in the Emulator

The emulator requires a system image file (SMD0.IMG) to run. Place the image file in the project root directory.

To boot a SINTRAN image from an SMD disk
```bash
build/bin/nd100x --boot=smd 
```


Read more about [how to boot sintran](SINTRAN.md)



## Booting TPE-MON from floppy and running test programs
The emulator requires a floppy image to boot from. Place the image file in the current directory.


```bash
cp images/Nd-210523I01-XX-01D.img FLOPPY.IMG
build/bin/nd100x --boot=floppy
```

Now you have access to CONFIG, PAGING, INSTRUCTION and more.




## Assembling your own programs

Using the assembly tool from Ragge [nd100-as](https://github.com/ragge0/norsk_data/tree/main/nd100-as) you can compile ND assembly to a.out format and load into the emulator.

This assembler is following the AT&T syntax so it differs a bit from the ND-100 MAC assembler.

```asm
lda foo
sta bar
opcom


bar: .word 11
foo: .word 22
```

Remember to end your code with 'opcom' to make the emulator stop executing your code.

Overview of all [assembly instructions ](docs/cpu_documentation.md)

## License

See the [LICENSE](LICENSE) file for detailed licensing information.

## Contributing

See the [CONTRIBUTING.md](CONTRIBUTING.md) file for more information


## TODO

- Refactor IO access to request BLOCKs from Machine instead of direct file access
  - Opens up for running WASM in the browser
- OPCOM implementation
  - Emulation of OPCOM for memory inspection when CPU is in STOP mode.
  - Currently STOP mode exits the emulator 
- Clean up code and standardise on 8,16,32 and 64 bit signed and unsigned names for types