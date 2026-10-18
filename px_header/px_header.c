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
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * pxheader - program to sit in front of interpreter code to make shell mods
 *	      unnecessary to make Pascal obj's look like real programs.
 *
 * Unix/Linux port: instead of the vintage a.out self-overlay (which read
 * the object from a fixed virtual address in its own text segment -- not
 * possible under ELF/ASLR), the Pascal obj file is appended to this binary
 * followed by an 8-byte little-endian trailer giving the appended size.
 * At startup we read /proc/self/exe, extract the trailer, load the obj and
 * either pipe it to px (fd 3) or stage it to a temp file and exec pxd.
 *
 * Bill Joy UCB February 6, 1978; Linux port 2026.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "config.h"
#include "whoami.h"
#include "objfmt.h"

#ifndef ETXTBSY
#define	ETXTBSY		26
#endif

#define TRAIL_SIZE	8	/* appended-size trailer, uint64 little-endian */
#define MAXARGS		512

static void		writen(int, const char *, int);
static void		writeobj(int, const char *, int);
static void		run(const char *prog, char **args);
static void		error(int, const char *cp);
static unsigned long long	rd_ulong_le(const unsigned char *p);

/* Interpreter/debugger paths (default; overridden by config at install time). */
const char *px_intrp = "/usr/local/bin/px";
const char *px_debug = "/usr/local/bin/pdx";
static void		onintr(void);

int
main(argc, argv)
	register int argc;
	register char *argv[];
{
	int i;
	int	codesiz, symtabsiz;
	register char *cp;
	char *	largv[MAXARGS];
	int	fd, pv[2], pid;
	struct stat st;
	char *	objdata;
	struct pxhdr *pxhd;
	char *	codeptr;
	unsigned long long apsz;
	off_t	fsize;

	fd = open("/proc/self/exe", O_RDONLY);
	if (fd < 0)
		error(2, "px_header: cannot open /proc/self/exe\n");
	if (fstat(fd, &st) < 0)
		error(2, "px_header: cannot stat /proc/self/exe\n");
	fsize = st.st_size;
	if (fsize < (off_t)TRAIL_SIZE)
		error(2, "px_header: executable too small\n");

	/* Read the appended-size trailer from the end of the file. */
	if (lseek(fd, (off_t)(fsize - TRAIL_SIZE), SEEK_SET) == (off_t)-1)
		error(2, "px_header: lseek trailer\n");
	if (read(fd, (char *)&apsz, TRAIL_SIZE) != TRAIL_SIZE)
		error(2, "px_header: read trailer\n");
	/* trailer was written little-endian */
	apsz = rd_ulong_le((const unsigned char *)&apsz);

	if (apsz == 0 || (unsigned long)fsize < apsz)
		error(2, "px_header: bad appended object size\n");

	objdata = malloc((int)apsz);
	if (objdata == (char *)NULL)
		error(2, "px_header: out of memory\n");
	if (lseek(fd, (off_t)(fsize - (off_t)apsz), SEEK_SET) == (off_t)-1)
		error(2, "px_header: lseek object\n");
	if (read(fd, objdata, (int)apsz) != (int)apsz)
		error(2, "px_header: read object\n");
	(void) close(fd);

	/* The pxhdr lives at HEADER_BYTES - sizeof(pxhdr) inside the obj. */
	pxhd = (struct pxhdr *)(objdata + HEADER_BYTES - sizeof(struct pxhdr));
	codesiz = (int)pxhd->objsize;
	symtabsiz = (int)pxhd->symtabsize;
	codeptr = objdata + HEADER_BYTES;

	if (pxhd->magicnum != MAGICNUM)
		error(2, "px_header: not a Pascal interpreter object\n");

	if (argc > MAXARGS - 3)
		error(2, "Too many arguments.\n");

	if (symtabsiz != 0) {
		largv[0] = "pxhdr";
		largv[1] = "/tmp/px00000";
		cp = &largv[1][11];
		for (i = (int)getpid(); i > 0; i /= 10)
			*cp-- = '0' + i % 10;
		fd = creat(largv[1], 0444);
		if (fd < 0)
			error(3, "Cannot create /tmp file\n");
		writeobj(fd, objdata, (int)apsz);
		(void) close(fd);
		largv[2] = 0;
		for (i = 0; i < argc; i++)
			largv[i + 2] = argv[i];
		largv[argc + 2] = 0;
		run(px_debug, largv);
		/* no return */
	}

	largv[0] = "pipe";
	for (i = 0; i < argc; i++)
		largv[i + 1] = argv[i];
	largv[argc + 1] = 0;
	pipe(pv);
	pid = fork();
	if (pid == -1) {
		write(2, "No more processes\n", 18);
		onintr();
	}
	if (pid == 0) {
		if (pv[0] != 3) {
			close(3);
			dup(pv[0]);
			close(pv[0]);
		}
		close(pv[1]);
		run(px_intrp, largv);
		/* no return */
	}
	(void) close(pv[0]);
	/* px (PIPE mode) reads the pxhdr, then objsize bytes of code. */
	writen(pv[1], (char *)pxhd, (int)sizeof(struct pxhdr));
	writen(pv[1], codeptr, codesiz);
	(void) close(pv[1]);

	i = 0;
	while (wait(&i) != pid && i != -1)
		continue;
	if (i == -1)
		onintr();
	if (i & 0377)
		onintr();
	free(objdata);
	exit(0);
}

static void
writen(fd, buf, len)
	int fd;
	const char *buf;
	int len;
{
	int n;
	while (len > 0) {
		n = write(fd, buf, len < BUFSIZ ? len : BUFSIZ);
		if (n <= 0)
			break;
		buf += n;
		len -= n;
	}
}

static void
writeobj(fd, objdata, appended_size)
	int fd;
	const char *objdata;
	int appended_size;
{
	writen(fd, objdata, appended_size);
	(void) close(fd);
}

static void
run(prog, args)
	const char *prog;
	char **args;
{
	for (;;) {
		execv(prog, args);
		if (errno != ETXTBSY)
			break;
		sleep(2);
	}
	error(0, prog);
	error(1, " not found.\n");
}

static void
error(errcode, cp)
	int errcode;
	register const char *cp;
{
	register int i;
	register const char *dp;

	dp = cp;
	i = 0;
	while (*dp++)
		i++;
	write(2, cp, i);
	if (errcode)
		exit(errcode);
}

/* ARGSUSED */
void
onintr()
{
	write(2, "interrupt\n", 10);
	_exit(1);
}

static unsigned long long
rd_ulong_le(const unsigned char *p)
{
	return ((unsigned long long)p[0])
	     | ((unsigned long long)p[1] << 8)
	     | ((unsigned long long)p[2] << 16)
	     | ((unsigned long long)p[3] << 24)
	     | ((unsigned long long)p[4] << 32)
	     | ((unsigned long long)p[5] << 40)
	     | ((unsigned long long)p[6] << 48)
	     | ((unsigned long long)p[7] << 56);
}
