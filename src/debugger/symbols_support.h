#ifndef _SYMBOLS_SUPPORT_H_
#define _SYMBOLS_SUPPORT_H_

#include "../../external/libsymbols/include/symbols.h"
#include "../../external/libdap/libdap/include/dap_server.h"

// Symbol loading and management
int init_symbol_support(const char* filename);
const char* find_symbol_by_address(symbol_table_t* symtab, uint16_t address);

// Helper functions for common symbol operations
const char* get_symbol_for_address(uint16_t address);
const char* get_source_location(uint16_t address, int* line);

// DAP integration
int register_symbol_file_with_dap(DAPServer *server, const char* symbol_file);

#endif // _SYMBOLS_SUPPORT_H_ 