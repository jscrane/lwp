/*
 * sig.c -- share signals among threads.
 * Copyright (C) 1991-3 Stephen Crane.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * author: Stephen Crane, (jsc@doc.ic.ac.uk), Department of Computing,
 * Imperial College of Science, Technology and Medicine, 180 Queen's
 * Gate, London SW7 2BZ, England.
 */

#include <malloc.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include "lwp.h"

extern int select(int,fd_set *,fd_set *,fd_set *,struct timeval *);
extern int getdtablesize(void);
#ifndef linux
extern void bzero(char *,int);
#endif

static struct siginf *info;
static int w, n;
static fd_set fds;
static struct sigaction os;

/*
 * iohan -- main handler for sigio.  dispatches signal to
 * first ready descriptor.
 */
static void iohan ()
{
        fd_set fdtmp;
	struct timeval t;
	struct siginf *p;

	fdtmp = fds;
	t.tv_sec = t.tv_usec = 0;
	select (w, &fdtmp, (fd_set *)0, (fd_set *)0, &t);
	for (p=info; p; p=p->next)
		if (FD_ISSET(p->des, &fdtmp)) {
			p->han (p->ctx, p->des);
			break;
		}
}

/*
 * sigioset -- install handler for SIGIO
 */
int sigioset (int fd, void (*handler) (void *, int), void *context)
{
	struct siginf *p;
	struct sigaction s;

	if (!n) {
		w = getdtablesize ();
		FD_ZERO(&fds);
		s.sa_handler = &iohan;
		sigemptyset(&s.sa_mask);
		s.sa_flags = SA_INTERRUPT;
		sigaction (SIGIO, &s, &os);
	}
	if (n++ == w)
		return (-1);
	p = (struct siginf *)malloc (sizeof(struct siginf));
	p->next = info;
	p->han = handler;
	p->des = fd;
	p->ctx = context;
	info = p;
	FD_SET(fd, &fds);
	return (0);
}

/*
 * sigioclr -- remove handler for SIGIO
 */
int sigioclr (int fd)
{
	struct siginf *p, *q;

	for (p=info, q=0; p; q=p, p=p->next)
		if (p->des == fd)
			break;
	if (!p) return (-1);

	if (q) q->next = p->next;
	else info = p->next;
	FD_CLR(fd, &fds);

	free (p);
	if (!--n)
		sigaction (SIGIO, &os, 0);
	return (0);
}
