/*-
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
static char copyright[] =
"@(#) Copyright (c) 1980, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)pix.c	8.2 (Berkeley) 5/27/94";
#endif /* not lint */

/*
 * pix - pi then px
 *
 * Bill Joy UCB August 26, 1977
 */

#include "whoami.h"
#include "objfmt.h"
#include "config.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/wait.h>
#define	ERRS	1

char	argname[] = "-o/tmp/pixaXXXXXX";
char	*name = &argname[2];

void	onintr(int sig);
int	dotted();

#define ETXTBSY	26

int
main(argc, argv)
	int argc;
	char *argv[];
{
	register int ac;
	int i, io, pid, status;
//	extern errno;

#ifndef lint
	(void) copyright;
	(void) sccsid;
#endif

	do
		io = open("/dev/null", 0);
	while (io >= 0 && io < 3);
	for (io = 3; io < 15; io++)
		close(io);
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, onintr);
	for (ac = 1; ac < argc; ac++)
		if (dotted(argv[ac], 'p')) {
			ac++;
			break;
		}
	io = mkstemp(name);
	if (io < 0) {
		perror(name);
		exit(1);
	}
	pid = fork();
	if (pid == -1) {
		write(2, "No more processes\n", 18);
		onintr(0);
	}
	if (pid == 0) {
		if (io != 3) {
			write(2, "Impossible error in pix\n", 24);
			onintr(0);
		}
		argv[ac] = 0;
		argv[0] = name - 2;
		do
			execv(pi_comp, argv);
		while (errno == ETXTBSY);
		write(2, "Can't find pi\n", 14);
		onintr(0);
	}
	close(io);
	do
		i = wait(&status);
	while (i != pid && i != -1);
	if (i == -1 || (status & 0377))
		onintr(0);
	if (status != 0) {
		if ((status >> 8) == ERRS)
			write(2, "Execution suppressed due to compilation errors\n", 47);
		onintr(0);
	}
	ac--;
	argv[ac] = name;
	ac--;
	argv[ac] = "pix";
	argv[argc] = 0;
	do
		execv(px_debug, &argv[ac]);
	while (errno == ETXTBSY);
	write(2, "Can't find px\n", 14);
	onintr(0);
}

int
dotted(cp, ch)
	char *cp, ch;
{
	register int i;

	i = strlen(cp);
	return (i > 1 && cp[i - 2] == '.' && cp[i - 1] == ch);
}

void
onintr(int sig)
{
	(void) sig;
	signal(SIGINT, SIG_IGN);
	unlink(name);
	exit(1);
}
