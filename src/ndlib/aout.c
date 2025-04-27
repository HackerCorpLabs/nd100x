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



// Reference doc 2.11 BSD: https://www.retro11.de/ouxr/211bsd/usr/man/cat5/a.out.0.html

/*

+---------------------------------------+--------------------------------------+-----------------+-----------------------------------------------+
|  SEGMENT                              | CONTENT                              | Properties      | Purpose                                       |
+---------------------------------------+--------------------------------------+-----------------+-----------------------------------------------+
|  A.out header                         | Header information                   | Read-only       | Program identification, load address          |
+---------------------------------------+--------------------------------------+-----------------+-----------------------------------------------+
|  (Zero-page)                          | Reserved area before actual code     | N/A             | May be used to catch NULL pointer dereference |
+---------------------------------------+--------------------------------------+-----------------+-----------------------------------------------+
|  Text                                 | Executable machine instructions      | Read-only       | Memory sharing, security, efficiency          |
+---------------------------------------+--------------------------------------+-----------------+-----------------------------------------------+
|  Data                                 | Initialized global/static variables  | Read/Write      | Runtime data modification                     |
+---------------------------------------+--------------------------------------+-----------------+-----------------------------------------------+
|  Zero-page relocation                 | Relocation info for zero-page refs   | Read-only       | Fixes absolute addresses in zero-page region  |
+---------------------------------------+--------------------------------------+-----------------+-----------------------------------------------+
|  Text relocation                      | Relocation entries for TEXT segment  | Read-only       | Fixes addresses in code referencing data/syms |
+---------------------------------------+--------------------------------------+-----------------+-----------------------------------------------+
|  Data relocation                      | Relocation entries for DATA segment  | Read-only       | Fixes addresses in data referencing symbols   |
+---------------------------------------+--------------------------------------+-----------------+-----------------------------------------------+
|  Symbol table                         | Symbol names and their addresses     | Read-only       | Used for debugging, linking, symbol lookup    |
+---------------------------------------+--------------------------------------+-----------------+-----------------------------------------------+
|  String table                         | Names for symbols (longer than 8 ch) | Read-only       | Supports long symbol names in symbol table    |
+---------------------------------------+--------------------------------------+-----------------+-----------------------------------------------+


*/

#include "ndlib_types.h"
#include "ndlib_protos.h"
#include <stdlib.h>
#include <string.h>

// Physical memory functions in cpu_mms.c
extern int ReadPhysicalMemory(int physicalAddress, bool privileged);
extern void WritePhysicalMemory(int physicalAddress, uint16_t value, bool privileged);
extern void disasm_addword(uint16_t addr, uint16_t myword);
extern int DISASM;

// Reads a 16-bit word from file in little-endian order
static int read_word(FILE* f, uint16_t* out_word) {
    int byte1 = fgetc(f);
    int byte2 = fgetc(f);
    if (byte1 == EOF || byte2 == EOF) return 0;

    *out_word = ((byte2 & 0xFF) << 8) | (byte1 & 0xFF);
    return 1;
}

// Swaps a little-endian word to big-endian for memory storage
static uint16_t to_big_endian(uint16_t word) {
    return (word >> 8) | (word << 8);
}

/// @brief Magic number to string
/// @param magic 
/// @return
const char* magic2str(uint16_t magic) {
    switch (magic) {
        case 0407: return "normal";
        case 0410: return "read-only text";
        case 0411: return "separated I&D";
        case 0405: return "read-only shareable";
        case 0430: return "auto-overlay (nonseparate)";
        case 0431: return "auto-overlay (separate)";
        default:   return "Unknown magic";
    }
}

const char* get_symbol_type(uint16_t type) {
    static char buf[32];
    if (type & N_EXT) {
        type &= ~N_EXT;
        strcpy(buf, "EXTERNAL ");
    } else {
        buf[0] = '\0';
    }
    
    switch(type) {
        case N_UNDF: strcat(buf, "UNDEFINED"); break;
        case N_ABS: strcat(buf, "ABSOLUTE"); break;
        case N_TEXT: strcat(buf, "TEXT"); break;
        case N_DATA: strcat(buf, "DATA"); break;
        case N_BSS: strcat(buf, "BSS"); break;
        case N_ZREL: strcat(buf, "ZREL"); break;
        default: strcat(buf, "UNKNOWN"); break;
    }
    return buf;
}

// Placeholder for adding symbols 
// TODO: Implement symbol storage and management in the CPU
void add_symbol(const symbol_t* sym) {
    printf("  SYMBOL: %-6s  TYPE: 0x%02X  VALUE: 0%06o\n", sym->name, sym->type, sym->value);
}


void load_symbols_with_string_table(FILE *f,long sym_offset, uint32_t num_bytes_syms, bool verbose)
{
        // Seek to symbol table
        fseek(f, sym_offset, SEEK_SET);

        if (verbose) {
            // Read and print symbols        
            printf("%-20s %-12s %s\n", "NAME", "TYPE", "VALUE");
            printf("----------------------------------------\n");
        }

        // Calculate number of symbol entries
        int num_symbols = (num_bytes_syms * 2) / sizeof(aout_nlist_t);
        
        // Remember current position for string table
        long str_table_pos = sym_offset + (num_bytes_syms * 2);
    
        for (int i = 0; i < num_symbols; i++) {
            aout_nlist_t sym;
            if (fread(&sym, sizeof(sym), 1, f) != 1) break;
    
            // Save current position
            long cur_pos = ftell(f);

            // Read symbol name from string table
            fseek(f, str_table_pos + sym.n_strx, SEEK_SET);
            char name[64] = {0};
            fgets(name, sizeof(name), f);
    
            //add_symbol((symbol_t*)&sym);


            if (verbose) {
                // Print symbol information|                            
                printf("%-20s %-12s 0x%06o\n", 
                    name, 
                    get_symbol_type(sym.n_type),
                    sym.n_value);    
            }
            
            // Restore position for next symbol
            fseek(f, cur_pos, SEEK_SET);
        }
}

/// @brief Loads a PDP-11 a.out file and writes the text/data segments to memory.
/// @param filename The filename of the a.out file to load
/// @return -1 on error, else the entry point address
int load_aout(const char* filename, bool verbose) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        perror("Failed to open file");
        return -1;
    }

    uint16_t dataLoadAddress;

    aout_header_t header;

    // Read and parse all 8 header fields from the file
    for (int i = 0; i < sizeof(header) / 2; i++) {
        uint16_t* field = ((uint16_t*)&header) + i;
        if (!read_word(f, field)) {
            fprintf(stderr, "Failed to read a.out header field %d\n", i);
            fclose(f);
            return -1;
        }
    }

    if (verbose)
    {
        // Print header information for debugging
        printf("=== Loaded a.out Header ===\n");
        printf("  Magic     : 0x%04X (%s)\n", header.a_magic, magic2str(header.a_magic));
        printf("  Text size : %u words\n", header.a_text);
        printf("  Data size : %u word\n", header.a_data);
        printf("  BSS size  : %u words (will be zero-filled if needed)\n", header.a_bss);
        printf("  Symbols   : %u bytes\n", header.a_syms);
        printf("  Entry     : 0%06o\n", header.a_entry);
        printf("  Zero Page : %u words\n", header.a_zp);
        printf("  Flags     : 0%06o\n", header.a_flag);    
        printf("===========================\n");
    }
    // Skip zero page if present
    fseek(f, 16 + header.a_zp * 2, SEEK_SET);

    uint16_t memoryPtr = 0;    
    // Load the text segment
    if (verbose)
        printf("Loading text segment at 0%06o (%u bytes)\n", TEXT_START, header.a_text);

    for (uint16_t i = 0; i < header.a_text; i++) {
        uint16_t word;
        if (!read_word(f, &word)) {
            fprintf(stderr, "Unexpected EOF while reading text segment\n");
            break;
        }        

        dataLoadAddress = TEXT_START + memoryPtr;
        //printf("Writing TEXT %06o to %06o\n", word, dataLoadAddress);
                
        WritePhysicalMemory(dataLoadAddress,word, false);        
        if (DISASM) disasm_addword(dataLoadAddress, word);

        memoryPtr ++;
    }

  

    // Load the data segment
    uint16_t data_addr = DATA_START(header.a_text);
    if (verbose)
        printf("Loading data segment at 0%06o (%u bytes)\n", data_addr, header.a_data);

    for (uint16_t i = 0; i < header.a_data; i ++) {
        uint16_t word;
        if (!read_word(f, &word)) {
            fprintf(stderr, "Unexpected EOF while reading data segment\n");
            break;
        }        

        dataLoadAddress = data_addr + memoryPtr;
        //printf("Writing DATA %06o to %06o\n", word, dataLoadAddress);
        WritePhysicalMemory(dataLoadAddress,word, false);
        if (DISASM) disasm_addword(dataLoadAddress, word);
        
        memoryPtr ++;
    }
    
    // Calculate symbol table offset
    long sym_offset = 16 +          // Header size
        (header.a_zp * 2) +         // Zero page size
        (header.a_text * 2) +       // Text segment
        (header.a_data * 2) +       // Data segment
        (header.a_zp * 2) +         // Zero page relocation
        (header.a_text * 2) +       // Text relocation
        (header.a_data * 2);        // Data relocation

    if (verbose)
        printf("Loading symbols(%u bytes)\n", header.a_syms);    

    // Load symbols
    load_symbols_with_string_table(f,sym_offset, header.a_syms, verbose);

    fclose(f);    

    if (verbose)
        printf("\n\n");
    return header.a_entry; // return entry address for execution
}