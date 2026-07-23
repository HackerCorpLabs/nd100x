/*
 * nd100x - ND100 Virtual Machine
 *
 * Copyright (c) 2025 Ronny Hansen
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

#ifndef ND100X_SHELL_H
#define ND100X_SHELL_H

#include <stdint.h>
#include <stdbool.h>

/*
 * Interactive shell for loading and running BPUN/PROG files on the ND-100
 *
 * The shell provides a command-line interface for:
 * - Listing available BPUN/PROG files
 * - Loading programs into memory
 * - Running programs
 * - Basic file operations
 */

/**
 * Run the interactive shell
 *
 * @param nd100Root Directory to search for BPUN/PROG files (NULL = use current dir)
 * @param scriptPath Path to script file to execute (NULL = no script)
 *
 * @return 0 on success, -1 on error
 */
int nd100x_shell_run(const char *nd100Root, const char *scriptPath);

#endif /* ND100X_SHELL_H */
