/*
 * timer.c -- an example program to demonstrate lightweight
 * processes and time.
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
#include "lwp.h"

struct sem *s_end;

void f ()
{
	int i; char ch='O';
	for (i=1; i<1000; i++) {
		delayp (250*i);
		write (1, &ch, 1);
	}
	signals (s_end);
	suicidep ();
}

void g ()
{
	int i; char ch=' ';
	for (i=1; i<1000; i++) {
		delayp (350*i);
		write (1, &ch, 1);
	}
	signals (s_end);
	suicidep ();
}

int main ()
{
	initlp (1);
	s_end = creats (0);
	creatp (2, g, 4096, 0, 0, 0);
	creatp (2, f, 4096, 0, 0, 0);
	waits (s_end);
	waits (s_end);
	return 0;
}
