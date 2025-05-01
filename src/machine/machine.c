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
#include <pthread.h>
#include <unistd.h>

#include "machine_types.h"
#include "machine_protos.h"

#include "../devices/devices_types.h"
#include "../devices/devices_protos.h"

#include "../ndlib/ndlib_types.h"
#include "../ndlib/ndlib_protos.h"


const char* boot_type_str[] = {
    "none",
    "bpun",
    "aout",
    "bp",
    "floppy",
    "smd"
};


void 
machine_init (bool debuggerEnabled)
{
    
    // Initialize the CPU
    cpu_init(debuggerEnabled);
    // Initialize IO devices
    IO_Init();


    // Set the CPU to RUN mode
    set_cpu_run_mode(CPU_RUNNING);
}

void 
cleanup_machine (void)
{
    cleanup_cpu();
    IO_Destroy();
}


/// @brief Do NOT call from debugger thread
/// @param ticks Number of ticks to run the CPU. Use -1 for infinite.
void  machine_run (int ticks)
{    
    // Run the CPU until it stops but also handle debugger requests
    do 
    {    
        while (get_cpu_run_mode() == CPU_PAUSED) {
            //printf("Machine: CPU is paused, waiting for debugger to release. Sleeping 100ms\n");
            usleep(100000);
        }        
        ticks = cpu_run(ticks);  

        if (ticks == 0) break;
    } while (get_cpu_run_mode() != CPU_SHUTDOWN);

}

void machine_stop()
{
    // Stop the CPU
    set_cpu_run_mode(CPU_STOPPED);
}


/* 
 * 
 * 
 *  CONFIGURATION HANDLING
 * 
 * 
 */

 BOOT_TYPE	BootType; /* Variable holding the way we should boot up the emulator */
 
 void  setdefaultconfig (void)
 {
     // Set default configuration	
     BootType = BOOT_SMD;
     STARTADDR = 0;
     DISASM = 0;
 }
 


/* 
 * 
 * 
 *  BOOT HANDLING
 * 
 * 
 */


 void program_load(BOOT_TYPE bootType, char *imageFile, bool verbose)
 {
     int bootAddress;
     int result;
 
     switch (bootType)
     {
     case BOOT_BP:
         bootAddress = bp_load(imageFile);
         if (bootAddress < 0)
         {
             printf("Error loading BP file '%s'\n", imageFile);
             exit(1);
         }
         break;
 
     case BOOT_BPUN:		
         bootAddress = LoadBPUN(imageFile,verbose);
         if (bootAddress < 0)
         {
             printf("Error loading BPUN file '%s'\n", imageFile);
             exit(1);
         }
         STARTADDR = bootAddress;
         break;
    case BOOT_AOUT:
        bootAddress = load_aout(imageFile, verbose);
        if (bootAddress < 0)
        {
            printf("Error loading AOUT file '%s'\n", imageFile);
            exit(1);
        }
        STARTADDR = bootAddress;
        break;
     case BOOT_FLOPPY:
         bootAddress = LoadBPUN(imageFile, verbose);
         if (bootAddress < 0)
         {
             printf("Error loading BPUN file\n");
             exit(1);
         }
 
         STARTADDR = bootAddress;
 
         //gPC = (CONFIG_OK) ? bootaddress : 0;		
 
         /*
         result = sectorread(0, 0, 1, (ushort *)&VolatileMemory);
         if (result < 0) {
             printf("Error reading from floppy\n");
             exit(1);
         }
         gPC = 0;
         */
         break;
     case BOOT_SMD:
         bootAddress = DeviceManager_Boot(01540);
         if (bootAddress < 0)
         {
             exit(10);
         }
         STARTADDR = bootAddress;
         break;
     }
 }
 