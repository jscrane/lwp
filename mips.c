/*
 * mips.c -- lightweight process initialisation for mips.
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
 * fake setjmp() and longjmp() because builtin longjmp() checks the
 * stack.
 */
static int jmpresult;

int savep (jmp_buf jb)
{
	jmpresult = 0;
	/* save context */
	asm ("sw	$31, 8($4)");	/* return address */
	asm ("sw	$30, 4($4)");	/* frame pointer */
	asm ("sw	$sp, 0($4)");	/* stack pointer */
	asm ("sd	$16, 12($4)");	/* callee save registers */
	asm ("sd	$18, 20($4)");
	asm ("sd	$20, 28($4)");
	asm ("sd	$22, 36($4)");
	return (jmpresult);
}

void restorep (jmp_buf jb)
{
	jmpresult = 1;
	/* restore context */
	asm ("lw	$sp, 0($4)");
	asm ("lw	$30, 4($4)");
	asm ("lw	$31, 8($4)");

	asm ("ld	$16, 12($4)");
	asm ("ld	$18, 20($4)");
	asm ("ld	$20, 28($4)");
	asm ("ld	$22, 36($4)");
}

/*
 * initp -- initialise a new process's context.
 */
void initp (struct pcb *newp, void *sp)
{
	newp->context[1] = (int)sp;
	newp->context[2] = (int)wrapp;
}
