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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // For read() system call
#include <poll.h>   // For pollfd struct and POLLIN


#include "machine_types.h"
#include "machine_protos.h"

#include "../devices/devices_types.h"
#include "../devices/devices_protos.h"


Device *terminal;

void IO_Init(void)
{
    DeviceManager_Init(LOG_INFO);
    //DeviceManager_Init(LOG_DEBUG);
    DeviceManager_AddAllDevices();

    // Get a reference to the terminal device for keyboard input
    terminal = DeviceManager_GetDeviceByAddress(0300);
    if (!terminal)
    {
        printf("Failed to get terminal device\n");
        exit(1);
    }
}

void IO_Destroy(void)
{
    DeviceManager_Destroy();
}

uint16_t IO_Read(uint32_t address)
{
    return DeviceManager_Read(address);
}

void IO_Write(uint32_t address, uint16_t value)
{
    DeviceManager_Write(address, value);
}

int IO_Ident(uint16_t level)
{
    return DeviceManager_Ident(level);
}

void IO_Tick(void)
{
    // Check for input from console and queue it to terminal device
    char ch;
    static int tick_count = 0;

    // Check for input from console every 5000 ticks
    if (tick_count++ >= 5000)
    {

        tick_count = 0;

        if (terminal)
        {
            char recv_data[1024];
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
                ch = (char)recv_data[i];

                // ND doesnt like \n
                if (ch == '\n')
                {
                    ch = '\r';
                }
                Terminal_QueueKeyCode(terminal, ch);
            }
        }
    }
    // Tick all devices, and check for interrupts
    uint16_t interruptBits = DeviceManager_Tick();
    
    if (interruptBits)
        device_interrupt(interruptBits);
}


ushort io_op(ushort ioadd, ushort regA)
{
    // Even addresses are read operations, odd addresses are write operations
    if (ioadd & 1)
    {
        // Odd address - write operation
        IO_Write(ioadd, regA);
        return regA;
    }
    else
    {
        // Even address - read operation
        return IO_Read(ioadd);
    }
}
