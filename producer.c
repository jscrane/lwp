/*
 * producer.c -- an example program to demonstrate lightweight
 * processes, semaphores and time.
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

#include <stdio.h>
#include <unistd.h>
#include <stddef.h>
#include <math.h>
#include <signal.h>
#include <string.h>
#include "lwp.h"

struct sem *s_empty, *s_full, *s_end;
char c;

void f (int n, char *p)
{
	while (n--) {
		waits (s_empty);
		c = *p++;
		signals (s_full);
		delayp (1000000);
	}
	signals (s_end);
	suicidep ();
}

void g (int n)
{
	while (n--) {
		waits (s_full);
		putchar (c);
		fflush (stdout);
		signals (s_empty);
	}
	signals (s_end);
	suicidep ();
}

int main (int argc, char **argv)
{
	char *str = "Hello world\n";
	int n = strlen (str);

	initlp (4);
	s_empty = creats (1);
	s_full = creats (0);
	s_end = creats (0);
	creatp (2, g, 2048, n, (char **)0, 0);
	creatp (2, f, 2048, n, (char **)str, 0);
	for (n=2; n--; )
		waits (s_end);
	return 0;
}
