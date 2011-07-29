/*
 * linux.c -- lightweight process initialisation for linux
 * Copyright (C) 1992-3 Stephen Crane.
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
#include <sys/param.h>

#include "linux.h"
#include "lwp.h"

/*
 * initp -- initialise a new process's context.
 */
void initp (volatile struct pcb *volatile newp, void *sp)
{
  newp->context->__jmpbuf[0] = 0;
  newp->context->__jmpbuf[1] = 0;
  newp->context->__jmpbuf[2] = 0;
  newp->context->__jmpbuf[3] = (unsigned int)sp;
  newp->context->__jmpbuf[4] = (unsigned int)sp;
  newp->context->__jmpbuf[5] = (unsigned int)(void *)wrapp;
}

/*
 * we can't use setjmp and longjmp anymore because of this new-fangled
 * pointer mangling (this breaks the initialisation routine above).
 * these two are copies of the glibc routines with mangling removed...
 */
int savep (jmp_buf jb)
{
  int r = setjmp(jb);
  if (!r) {
    /* fixup mangled sp and pc */
    asm("movl 0(%esp), %eax");
    asm("leal 0(%esp), %ecx");
    asm("movl %ecx, 16(%eax)");
    asm("movl 4(%ebp), %ecx");
    asm("movl %ecx, 20(%eax)");
  }
  return r;
}

void restorep (jmp_buf jb)
{
  asm("movl -4(%esp), %ecx");
  asm("movl 0(%ecx), %ebx");
  asm("movl 4(%ecx), %esi");
  asm("movl 8(%ecx), %edi");
  asm("movl 12(%ecx), %ebp");
  asm("movl 16(%ecx), %esp");
  asm("movl 20(%ecx), %edx");
  asm("movl $1, %eax");
  asm("jmp *%edx");
}
