#ifndef _DEBUGGER_H_
#define _DEBUGGER_H_

#include <stdint.h>

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
    uint16_t pc;
    uint16_t operand;
    uint16_t return_address;
} StackFrame;

typedef struct {
    StackFrame frames[MAX_STACK_FRAMES];
    int current_frame;
    int frame_count;
} StackTrace;


/// @brief Step types for the debugger. Maps to DAP
typedef enum  {

    /// @brief DAP "next" command
    STEP_OVER,

    /// @brief DAP "stepIn" command
    STEP_IN,

    /// @brief DAP "stepOut" command
    STEP_OUT,
} StepType;

// Function declarations
void start_debugger();
int ndx_server_init(int port);
int ndx_server_stop();
void debugger_kbd_input(char c);

#endif
