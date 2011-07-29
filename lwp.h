/*
 * lwp.h -- prototypes and structures for lightweight processes
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
 *
 * This version of lwp has been significantly changed by Claudio Fleiner
 * (fleiner@icsi.berkeley.edu) in 1995. New features include locks, 
 * the possibility to interrupt threads, thread local memory, definitions of
 * functions to execute whenever a thread is rescheduled or killed, and more.
 * These features have been tested under sun 4.1.3 and linux 1.1.59.
 */
#ifndef _LWP_H
#define _LWP_H

/* maximum thread priority: number of priority levels */
#define MAXTPRI	8

#ifndef _SIGNAL_H
#include <signal.h>
#endif
#define SIGNALS (sigmask(SIGIO)|sigmask(SIGALRM))

#include "arch.h"

#ifndef OWN_CONTEXT_SWITCH
#ifndef _SETJMP_H
#include <setjmp.h>
#define savep(x)	setjmp(x)
#define restorep(x)	longjmp(x, 1)
#endif
#endif

#ifndef _sys_time_h
#include <sys/time.h>
#endif

/* process control block.  do *not* change the position of context */
struct pcb {
	jmp_buf	context;	/* processor context area */
	void	*sbtm;		/* bottom of stack attached to it */
	int     size;           /* size of stack */
	void	(*entry)();	/* entry point */
	unsigned dead:1;	/* whether the process can be rescheduled */
	int	pri;		/* which scheduling queue we're on */
	struct timeval	dt;
	int	argc;		/* initial arguments */
	char	**argv;
	void	*envp;		/* user structure pointer */
	struct pcb	*next;
};
extern struct pcb *currp;

/* queue */
struct lpq {
	struct pcb *head, *tail;
};

/* semaphore */
struct sem {
	int count;
	struct lpq q;
};

/* signal struct */
struct siginf {
	struct siginf *next;
	int des;
	void (*han) (void *, int);
	void *ctx;
};

typedef double stkalign_t;

#ifdef __cplusplus
extern "C" {
#endif

/* semaphore operations */
struct sem *creats (int);
void signals (struct sem *);
int waits (struct sem *); /* returns 0 on succes, -1 if interrupted */
int tests(struct sem *);
#define deletes(x) free((x))

/* queue operations */
void toq (struct lpq *, struct pcb *);
struct pcb *hoq (struct lpq *);

/* process operations */
struct pcb *creatp (int prio, void (*entry)(), int size, int argc, char *argv[], char *envp);
void suicidep (void);
void destroyp (struct pcb *);
void readyp (struct pcb *);
void yieldp (void);
void *getenvp (struct pcb *);
void setenvp (struct pcb *, void *);
int prisetp (int);
int prichangep (int);
void reschedp (void);

/* timer operations */
void delayp (long);
void onalarm ();

/* initialisation */
struct pcb *initlp (int);
void initp (volatile struct pcb *volatile newp, void *sp);
void wrapp (void);

/* signal stuff */
int sigioset (int, void (*)(void *, int), void *);
int sigioclr (int);
#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif	/* _LWP_H */
