/*
 * sun4.c -- lightweight process initialisation for sun4.
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

#include "lwp.h"

/*
 * initp -- initialise a new process's context.
 */
void initp (volatile struct pcb *volatile newp, void *sp)
{
	static jmp_buf *cpp;
	extern struct pcb *currp;
	
	newp->context[0] = (int)sp;

	/* preserve cpp for new context */
	cpp = (jmp_buf *) &newp->context;
	if (!savep (currp->context)) {
		/* create new context */		
		/* flush registers */
		asm volatile ("ta	0x03");
		/* %o0 <- newp */
		asm volatile ("ld	[%fp+0x44], %o0");
		/* %o1 <- newp->context[0] */
		asm volatile ("ld	[%o0], %o1");
		/* create min frame on new stack */
		asm volatile ("save	%o1,-96, %sp");
		if (!savep (*cpp))
			restorep (currp->context);
		wrapp ();
	}
}
