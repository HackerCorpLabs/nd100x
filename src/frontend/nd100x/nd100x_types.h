
/*
 * nd100x - ND100 Virtual Machine
 *
 * Copyright (c) 2025 Ronny Hansen
 *
 * This file is originated from the nd100x project and the RetroCore project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the nd100em
 * distribution in the file COPYING); if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ND100X_TYPES_H
#define ND100X_TYPES_H

#include <stdbool.h>
#include <stdint.h>


#include "../../machine/machine_types.h"
#include "../ndlib/ndlib_types.h"

// Printer emulation type (--printer= option)
typedef enum {
    PRINTER_TEXT,      // Simple line printer (plain ASCII)
    PRINTER_ESCP,      // Epson ESC/P interpreter
    PRINTER_LASER      // Color laser (future, not yet implemented)
} PrinterType_t;

// Printer output format (--printformat= option)
typedef enum {
    PRINT_FORMAT_TXT,  // Plain text output (.txt)
    PRINT_FORMAT_PDF   // PDF output (.pdf)
} PrintFormat_t;

// Configuration structure
typedef struct {
    BOOT_TYPE bootType;
    char *imageFile;
    uint32_t startAddress;
    bool disasmEnabled;
    bool verbose;
    bool showHelp;
    bool debuggerEnabled;
    int debuggerPort;
    bool smdDebug;
    bool traceEnabled;
    uint64_t maxInstructions;
    bool breakpointEnabled;
    uint32_t breakpointAddr;
    bool textStartSet;
    uint32_t textStart;
    char *printDir;      // Output directory for print jobs (default: ./prints/)
    char *tapeDir;       // Output directory for punched tape (default: ./tapes/)
    char *tapeFile;      // Input file for paper tape reader
    bool telnetEnabled;  // --telnet flag
    int telnetPort;      // Default: 9000
    PrinterType_t printerType;   // --printer= option (default: PRINTER_TEXT)
    PrintFormat_t printFormat;    // --printformat= option (default: PRINT_FORMAT_TXT)
    // HDLC configuration
    bool hdlcEnabled;
    char *hdlcAddress;   // IP address or hostname for client mode
    int hdlcPort;        // Port number for server or client mode
    bool hdlcServer;     // true = server mode, false = client mode
} Config_t;

#endif // CONFIG_H 