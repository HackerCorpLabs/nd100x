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
#include <stdarg.h>
#include "ndlib_types.h"
#include "ndlib_protos.h"


const char *level_str[] = {
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR"};


LogLevel minLogLevel = LOG_DEBUG;


void Log_SetMinLevel(LogLevel level)
{
    minLogLevel = level;
}

// Logging function implementation
void Log(LogLevel level, const char *format, ...)
{
    // Skip if the message level is below the minimum level
    if (level < minLogLevel)
    {
        return;
    }

    va_list args;
    va_start(args, format);
    printf("[%s] ", level_str[level]);
    vprintf(format, args);
    va_end(args);
}