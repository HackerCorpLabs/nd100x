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

#include "nd100x_types.h"
#include "nd100x_protos.h"


struct rusage *used;
//extern double instr_counter;  // likely from cpu.h

double usertime;
double systemtime;
double totaltime;
Config_t config;


void initialize()
{
   srand ( time(NULL) ); /* Generate PRNG Seed */
   used=calloc(1,sizeof(struct rusage)); /* Perf counter stuff */
	

	blocksignals();

	
	if (DISASM) disasm_init();

	machine_init();

	program_load(config.bootType, config.imageFile, config.verbose);	
	gPC = STARTADDR;

	/* Direct input/output enabled */
	setcbreak ();
	setvbuf(stdout, NULL, _IONBF, 0);
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

void cleanup()
{
	cleanup_machine();
	unsetcbreak ();
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
    
    // Set global variables from config
    DISASM = config.disasmEnabled;
    STARTADDR = config.startAddress;

	initialize();

	// Run the machine
	machine_run();

	if (DISASM)
		disasm_dump();

	dump_stats();
	cleanup();

	// exit
	return(0);
}
