/*
 * nd100x - ND100 Virtual Machine
 *
 * Copyright (c) 2006 Per-Olof Astrom
 * Copyright (c) 2006-2008 Roger Abrahamsson
 * Copyright (c) 2008 Zdravko
 * Copyright (c) 2025 Ronny Hansen
 *
 * This file is originated from the nd100em project.
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

#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h> // For read() system call
#include <poll.h>   // For pollfd struct and POLLIN

#include <signal.h>
#include <unistd.h>

#include <sys/resource.h>
#include <limits.h>
#include <string.h>

#include <time.h>       // for time()
#include <sys/time.h>   // for timeval
#include <sys/resource.h> // for struct rusage


#include "../ndlib/ndlib_types.h"
#include "../ndlib/ndlib_protos.h"

#include "../machine/machine_types.h"
#include "../machine/machine_protos.h"
#include "devices_types.h"
#include "../../devices/devices_protos.h"

#ifdef WITH_DEBUGGER
void stop_debugger_thread();
#endif

#include "nd100x_types.h"
#include "nd100x_protos.h"


// dont include debugger.h here, it will cause other problems, but we define the function here
void debugger_kbd_input(char c) ;

struct rusage *used;
//extern double instr_counter;  // likely from cpu.h

double usertime;
double systemtime;
double totaltime;
Config_t config;


void handle_sigint(int sig) {
    printf("\nCaught signal %d (Ctrl-C). Cleaning up...\n", sig);
        
#ifdef WITH_DEBUGGER    
    // Stop the debugger server gracefully
    stop_debugger_thread();
#endif
    
    // Stop the machine
    machine_stop();
    
    // Exit the program
    exit(0);
}


void register_signals()
{
#ifdef _WIN32
    // Windows signal handling
    signal(SIGINT, handle_sigint);
    signal(SIGTERM, handle_sigint);
#else
    // POSIX signal handling using sigaction
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;  // Restart interrupted system calls
    
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    if (sigaction(SIGABRT, &sa, NULL) == -1) {
        perror("sigabrt");
        exit(1);
    }
#endif
}

void dump_stats()
{
	getrusage(RUSAGE_SELF, used);	/* Read how much resources we used */

	usertime=used->ru_utime.tv_sec+((float)used->ru_utime.tv_usec/1000000);
	systemtime=used->ru_stime.tv_sec+((float)used->ru_stime.tv_usec/1000000);
	totaltime=(float)usertime + (float)systemtime;

	printf("Number of instructions run: %lu, time used: %f\n",instr_counter,totaltime); // maybe us macro PRIu64  for instr_counter?
	printf("usertime: %f  systemtime: %f\n",usertime,systemtime);
	printf("Current cpu cycle time is:%f microsecs\n",(totaltime/((double)instr_counter/1000000.0)));
}

/// @brief Initialize the emulator. Add devices and load program

void initialize()
{
   srand ( time(NULL) ); /* Generate PRNG Seed */
   used=calloc(1,sizeof(struct rusage)); /* Perf counter stuff */
	

	//blocksignals();
	register_signals();
	

	
	if (DISASM) disasm_init();

	machine_init(config.debuggerEnabled);



    //     {0340, 044, 044, "TERMINAL 5/ TET12"},
    DeviceManager_AddDevice(DEVICE_TYPE_TERMINAL, 5);
    
    // {0350, 045, 045, "TERMINAL 6/ TET11"},
    DeviceManager_AddDevice(DEVICE_TYPE_TERMINAL, 6);
    

    // {0360, 046, 046, "TERMINAL 7/ TET10"},
    DeviceManager_AddDevice(DEVICE_TYPE_TERMINAL, 7);
    
    
	program_load(config.bootType, config.imageFile, config.verbose);	
	gPC = STARTADDR;

	/* Direct input/output enabled */
	setcbreak ();
	setvbuf(stdout, NULL, _IONBF, 0);
}




void cleanup()
{
	cleanup_machine();
	unsetcbreak ();
}


// Character device output handler for terminals
static void TerminalOutputHandler(Device *device, char c) 
{
    if (!device)
        return;
   
   printf("%c",c);   
}


int main(int argc, char *argv[]) 
{

	// Initialize the configuration
    Config_Init(&config);

	 // Parse command line arguments
    if (!Config_ParseCommandLine(&config, argc, argv)) {
        Config_PrintHelp(argv[0]);
        return EXIT_FAILURE;
    }
    
    // Show help if requested
    if (config.showHelp) {
        Config_PrintHelp(argv[0]);
        return EXIT_SUCCESS;
    }
    
    if (config.debuggerEnabled) {
        printf("DAP Debugger enabled on port %d\n", config.debuggerPort);
    }

    // Set global variables from config
    DISASM = config.disasmEnabled;
    STARTADDR = config.startAddress;

    initialize();

	Device *terminal = DeviceManager_GetDeviceByAddress(0300);
	Device_SetCharacterOutput(terminal, TerminalOutputHandler);

	if (!terminal)
	{
		printf("Terminal device not found\n");
		return EXIT_FAILURE;
	}

    if (config.debuggerEnabled) {
        set_cpu_run_mode(CPU_PAUSED);
    }

	// Run the machine until it stops	
	CPURunMode runMode = get_cpu_run_mode();

	while (runMode != CPU_SHUTDOWN)
	{



        if(get_debugger_control_granted())
        {
            // DAP adapter has control, so we need to wait for it to release control
            sleep_ms(100); // sleep 100ms
        }
        else
        {
            // Check if DAP adapter has requested a pause
            if (get_debugger_request_pause())
            {
                printf("Pausing CPU...\n");
                set_debugger_control_granted(true);
                continue;
            }


            // ND100x has control, so we need to run the machine
            runMode = get_cpu_run_mode();
            if (runMode == CPU_RUNNING)
            {
                machine_run(5000);  
            }
            else
            {
                // CPU is paused, so we need to wait for the debugger to release control
                sleep_ms(100); // sleep 100ms
            }
        }

		runMode = get_cpu_run_mode();

        // Handle keyboard input to console
		if (runMode != CPU_SHUTDOWN)
		{
			// Check for keyboard input
	
            char recv_data[20];
            int numbytes = 10;
            int numread;

            // Try to read from stdin
            struct pollfd fds;
            fds.fd = 0; // stdin
            fds.events = POLLIN;
            if (poll(&fds, 1, 0) > 0 && (fds.revents & POLLIN)) {
                numread = read(0, recv_data, numbytes);                
            } else {
                numread = 0;
            }
            
                        
            for (int i = 0; i < numread; i++)
            {
                char ch = (char)recv_data[i];

                // ND doesnt like \n
                if (ch == '\n')
                {
                    ch = '\r';
                }

                if ((runMode == CPU_PAUSED)||(runMode == CPU_BREAKPOINT)) {
                    debugger_kbd_input(ch);
                }
                else
                {
                    Terminal_QueueKeyCode(terminal, ch);
                }
            }
        }
    }




	if (DISASM)
		disasm_dump();

	dump_stats();
	cleanup();

	// exit
	return(0);
}
