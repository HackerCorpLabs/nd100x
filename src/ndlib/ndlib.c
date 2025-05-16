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
#include <signal.h>
#include "ndlib_types.h"
#include "ndlib_protos.h"


struct termios savetty; 
struct config_t *pCFG = NULL;

/* 
 * 
 * 
 *  TERMINAL HANDLING
 * 
 * 
 */

void unsetcbreak(void)
{ /* prepare to exit this program. */
	tcsetattr(0, TCSADRAIN, &savetty);
}

void setcbreak(void)
{ /* set console input to raw mode. */
	struct termios tty;
	tcgetattr(0, &savetty);
	tcgetattr(0, &tty);
	tty.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN);
	tty.c_cc[VTIME] = (cc_t)0; /* inter-character timer unused */
	tty.c_cc[VMIN] = (cc_t)0;  /* Dont wait for chars - non-blocking read */
	tcsetattr(0, TCSADRAIN, &tty);

	/* After this is set:
	   - Use read() or getchar_unlocked() for non-blocking reads
	   - read() will return -1 with errno=EAGAIN if no data
	   - getchar_unlocked() will return EOF if no data
	   - Both functions return immediately without waiting
	*/
}



