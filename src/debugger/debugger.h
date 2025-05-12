#ifndef _DEBUGGER_H_
#define _DEBUGGER_H_

#include <stdint.h>
#include "symbols.h"

#ifdef _WIN32
    #define THREAD_FUNC DWORD WINAPI
    
    #define THREAD_RETURN(val) return val
    #define WIN_RESULT DWORD WINAPI 
#else
    #define THREAD_FUNC void *
    #define LPVOID void *
    #define THREAD_RETURN(val) return (void *)(val)
    #define WIN_RESULT void*
#endif

// Define the maximum number of stack frames we'll track
#define MAX_STACK_FRAMES 20

    // Static variables to maintain stack frame state
typedef struct {
    /// @brief Program counter of the stack frame
    uint16_t pc;
    /// @brief Operand that made the call 
    uint16_t operand;
    /// @brief Return address of the stack frame (where it was called from)
    uint16_t return_address;
    /// @brief Entry point of the stack frame
    uint16_t entry_point;
} StackFrame;

typedef struct {
    StackFrame frames[MAX_STACK_FRAMES];
    int current_frame;
    int frame_count;
} StackTrace;


typedef struct {
    symbol_table_t *symbol_table_map;
    symbol_table_t *symbol_table_aout;
    symbol_table_t *symbol_table_stabs;
} SymbolTables;



/// @brief Step types for the debugger. Maps to DAP
typedef enum  {

    /// @brief DAP "next" command
    STEP_OVER,

    /// @brief DAP "stepIn" command
    STEP_IN,

    /// @brief DAP "stepOut" command
    STEP_OUT,
} StepType;


typedef enum {
    SYMBOL_TYPE_MAP,
    SYMBOL_TYPE_AOUT,
    SYMBOL_TYPE_STABS,
} SymbolType;

// Function declarations
void start_debugger();
int ndx_server_init(int port);
int ndx_server_stop();
void debugger_kbd_input(char c);

#endif
