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
machine_init (void)
{
    
    // Initialize the CPU
    cpu_init();
    // Initialize IO devices
    IO_Init();

    // Set the CPU to RUN mode
    CurrentCPURunMode = RUN;

}

void 
cleanup_machine (void)
{
    cleanup_cpu();
    IO_Destroy();
}


void 
machine_run (void)
{
    // Run the CPU until it stops
    cpu_run(-1);
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
 