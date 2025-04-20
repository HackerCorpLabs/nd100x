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

void 
blocksignals (void)
{
	static sigset_t new_set;
	static sigset_t old_set;
	sigemptyset(&new_set);
	sigemptyset(&old_set);

	/* set signals now */
	if (false) // DEAMON is not used in this project
	{
		sigaddset(&new_set, SIGCHLD); /* ignore child */
		sigaddset(&new_set, SIGTSTP); /* ignore tty signals */
		sigaddset(&new_set, SIGTTOU);
		sigaddset(&new_set, SIGTTIN);
	}
	sigaddset(&new_set, SIGALRM); /* ignore timer for process.. this one we will install handler for later */
	sigaddset(&new_set, SIGINT);  /* kill signal we will catch in handles */
	sigaddset(&new_set, SIGHUP);  /* see above */
	sigaddset(&new_set, SIGTERM); /* see above */
}

void 
setsignalhandlers (void)
{
	// static sigset_t   new_set;
	// static sigset_t   old_set;

	// static struct sigaction act;
	// static struct sigaction act_alrm;

	/* set up handler for SIGINT, SIGHUP, SIGTERM */
	/*
	act.sa_handler = &shutdown;
	sigemptyset (&act.sa_mask);
	sigaction (SIGINT, &act, NULL);
	sigaction (SIGHUP, &act, NULL);
	sigaction (SIGTERM, &act, NULL);
	sigemptyset (&new_set);
	sigemptyset (&old_set);
	sigaddset (&new_set, SIGINT);
	sigaddset (&new_set, SIGHUP);
	sigaddset (&new_set, SIGTERM);
	*/

	/* set up handler for SIGALARM */
	/*
	act_alrm.sa_handler = &rtc_handler;
	sigemptyset (&act_alrm.sa_mask);
	sigaction (SIGALRM, &act_alrm, NULL);
	sigemptyset (&new_set);
	sigemptyset (&old_set);
	sigaddset (&new_set, SIGALRM);

	*/
	return;
}


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



