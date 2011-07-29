/*
 * sem.c -- semaphore manipulation.
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
#include <assert.h>

#include "lwp.h"

/*
 * creats -- create a semaphore.
 */
struct sem *creats (int count)
{
	struct sem *new;

	if (!(new = (struct sem *)malloc (sizeof(struct sem))))
		return 0;
	new->count = count;
	new->q.head = new->q.tail = 0;
	return new;
}

/*
 * signals -- signal a semaphore.  We only yield here if
 * the blocked process has a higher priority than ours'.
 */
void signals (struct sem *s)
{
	extern struct pcb *currp;
	struct pcb *p = hoq (&s->q);

	if (p) {
		assert(s->count==0);
		readyp (p);
		if (currp->pri < p->pri)
			yieldp ();
	} else
	  s->count++;
}

/*
 * tests -- if the counter is zero, it returns zero,
 * otherwise it decrements it and returns one
 */
int tests(struct sem *s)
{
	if (s->count > 0) {
		s->count--;
		return 1;
	} 
	return 0;
}

/*
 * waits -- wait on a semaphore
 */
int waits (struct sem *s)
{
	extern struct pcb *currp;

	if (s->count == 0) {
		toq (&s->q, currp);
		reschedp ();
	} else
	  s->count--;
	return 0;
}
