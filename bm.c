/*
 * bm.c -- benchmark the lwp implementation.
 * Copyright (C) 1991-3 Stephen Crane.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 1, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * author: Stephen Crane, (jsc@doc.ic.ac.uk), Department of Computing,
 * Imperial College of Science, Technology and Medicine, 180 Queen's
 * Gate, London SW7 2BZ, England.
 */

#include <sys/time.h>

#include "lwp.h"
struct sem *s;

void f (int n)
{
	while (n--)
		yieldp ();
	signals (s);
	suicidep ();
}

void g (void)
{
	suicidep ();
}

main ()
{
	int n, t;
	struct timeval ts, te;

	initlp (1);
	s = creats (0);
	creatp (1, f, 4096, 5000, 0, 0);
	creatp (1, f, 4096, 5000, 0, 0);
	gettimeofday (&ts, (struct timezone *)0);
	waits (s);
	waits (s);
	gettimeofday (&te, (struct timezone *)0);
	t = 1000*(te.tv_sec-ts.tv_sec) + (te.tv_usec-ts.tv_usec)/1000;
	free (s);
	printf ("Context switching: %d ms /10000\n", t);

	gettimeofday (&ts, (struct timezone *)0);
	for (n=10000; n--; )
		creatp (2, g, 4096, 0, 0, 0);
	gettimeofday (&te, (struct timezone *)0);
	t = 1000*(te.tv_sec-ts.tv_sec) + (te.tv_usec-ts.tv_usec)/1000;
	printf ("Process creation: %d ms /10000\n", t);
}
