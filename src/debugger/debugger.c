#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "../cpu/cpu_types.h"
#include "../cpu/cpu_protos.h"
#include "debugger.h"


#define _DEBUGGER_ENABLED_ // Enables debugger thread support

#ifdef _DEBUGGER_ENABLED_

#ifdef _WIN32
    #include <windows.h>
    HANDLE p_debugger_thread;
#else
    #include <stdatomic.h>
	#include <pthread.h>
    pthread_t p_debugger_thread;
#endif

#ifdef _WIN32
DWORD WINAPI debugger_thread_win(LPVOID lpParam)
#else
void *debugger_thread(void *arg)
#endif
{
    while (1) {
        printf("Debugger thread running\n");
        sleep(10);  // Let the CPU run a bit

        set_debugger_requested_control(true);           
    
        // Wait for the CPU to pause
        while (get_cpu_run_mode() != CPU_PAUSED) {
            usleep(100000);  // 100 ms wait before checking again
        }    

        printf("Debugger: CPU is paused. Reading state...\n");


        printf("CPU STATE 1:\n");
        printf("  PC  = %06o\n", gPC);
        printf("  A = %06o\n", gA);
        printf("  X = %06o\n", gX);  


        printf(">>>>>>>>>>--------------------------------\n");
        printf("Debugger: Stepping CPU\n");  
        // Stepping 10
        cpu_run(1000);
        printf("------------------>>>>>>>>>>-------------\n");

        printf("  PC  = %06o\n", gPC);
        printf("  A = %06o\n", gA);
        printf("  X = %06o\n", gX);  

        



        printf("Debugger: resuming CPU\n");        
        printf("============================================\n");

        // Release control of the CPU, let the CPU run normal again
        debuggerReleaseControl();                    
    }

    THREAD_RETURN(0);
}


void start_debugger() {
    		// Start the debugger thread
    #ifdef _WIN32
        p_debugger_thread = CreateThread(
            NULL,                // default security attributes
            0,                   // default stack size
            debugger_thread_win, // thread function
            NULL,                // argument to thread function
            0,                   // default creation flags
            NULL);               // receive thread identifier (optional)
    #else
        pthread_create(&p_debugger_thread, NULL, debugger_thread, NULL);
    #endif
}

#endif // _DEBUGGER_ENABLED_
