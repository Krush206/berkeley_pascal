/*-
 * Copyright (c) 1979, 1993
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

#if !defined(lint) && defined(sccs)
static char sccsid[] = "@(#)REWRITE.c	8.1 (Berkeley) 6/6/93";
#endif /* not lint */


#include "h00vars.h"

void
REWRITE(filep, name, maxnamlen, datasize)
	register struct iorec	*filep;
	char			*name;
	long			maxnamlen;
	long			datasize;
{
	filep = GETNAME (filep, name, maxnamlen, datasize);
#ifdef unix
	filep->fbuf = fopen(filep->fname, "w");
#else
	if (filep->fname[0] == '/' && filep->fname[0] == 'd' &&
			0 == strcmp(filep->fname, "/dev/null")) {
		filep->fbuf = fopen("NUL", "wb");
	} else {
		filep->fbuf = fopen(filep->fname, (filep->funit&FTEXT ? "wt" : "wb"));
	}
#endif
	if (filep->fbuf == NULL) {
		PERROR("Could not create ",filep->pfname);
		return;
	}
	filep->funit |= (EOFF | FWRITE);
	if (filep->fblk > PREDEF) {
		setbuf(filep->fbuf, &filep->buf[0]);
	}
}