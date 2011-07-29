/*
 * lwp.c -- lightweight process creation, destruction and manipulation.
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

#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <signal.h>
#include <malloc.h>
#include <assert.h>
#include <signal.h>
#include <syscall.h>
#include "lwp.h"

extern int sigpause(int);
extern int sigsetmask(int);

struct lpq schedq[MAXTPRI], deadq;
struct pcb *currp;
void (*lwp_sched_func)(void)=NULL;
int maxpri=0;		/* maximum priority so far */

static int oldmask;

/*
 * growsdown -- check stack direction
 */
static int growsdown (void *x) { int y; return x > (void *)&y; }

/*
 * reschedp -- schedule another process.  we also check for dead
 * processes here and free them.
 */
void reschedp (void)
{
	extern struct lpq schedq[];
	struct pcb *volatile nextp;
	long i;

	for (i=maxpri+1; i--; )
		while ((nextp = hoq (&schedq[i])))
			if (nextp->dead) {
				if (nextp!=currp) {
					free (nextp->sbtm);
					free (nextp);
				} else {
					currp = 0;
				}
			} else
				goto change;
change:
	/* change context? ... save current context */
	if (currp != nextp) {
		if (currp!=0) {
			if (savep (currp->context)==0) {
				/* restore previous context */
				currp = nextp;
				restorep (currp->context);
			}
		} else {
			currp = nextp;
			restorep (currp->context);
		}
	}
}

/*
 * wrapp -- process entry point.
 */
void wrapp (void)
{
	extern struct pcb *currp;

	sigsetmask (SIGNALS);
	(*currp->entry) (currp->argc, currp->argv, currp->envp);
	suicidep ();
}

/*
 * nullp -- a null process, always ready to run.
 * it (1) sets its priority to maximum to prevent a signal doing a
 * reschedule (2) enables signals, waits for one and handles it
 * and (3) resets its priority causing a reschedule.
 */
struct timeval idle_time;
static void nullp (void)
{
	struct timeval st,en;
	extern int gettimeofday(struct timeval *,struct timezone *);
	for (;;) {
		int p = prisetp (MAXTPRI-1);
		gettimeofday(&st,NULL);
		sigpause (oldmask);
		gettimeofday(&en,NULL);
		idle_time.tv_sec+=en.tv_sec-st.tv_sec;
		if (en.tv_usec>=st.tv_usec) {
			idle_time.tv_usec+=en.tv_usec-st.tv_usec;
		} else {
			idle_time.tv_usec+=1000000+en.tv_usec-st.tv_usec;
			idle_time.tv_sec--;
		}
		while(idle_time.tv_usec>=1000000) {
			idle_time.tv_sec++;
			idle_time.tv_usec-=1000000;
		}
		prisetp (p);
	}
}

/*
 * creatp -- create a process.
 */
struct pcb *creatp (int priority, void (*entry)(), int size, int argc, char *argv[], char *envp)
{
	struct pcb *newp;
	int *s, x;
	void *sp;
	extern struct pcb *currp;

	if (!(newp = (struct pcb *)calloc (sizeof(struct pcb),1)))
		return (0);
	size += sizeof(stkalign_t);
	if (!(s = (int *)malloc (size)))
		return (0);
	newp->entry = entry;
	newp->argc = argc;
	newp->argv = argv;
	newp->envp = envp;

	sp = (void *)(growsdown (&x)? (size+(int)s)&-sizeof(stkalign_t): (int)s);
	if (MAXTPRI <= priority)
		priority = MAXTPRI-1;
	if (maxpri < (newp->pri = priority))
		maxpri = priority;
	newp->sbtm = s;
	newp->size = size;
	newp->dead = 0;
	readyp (newp);
	readyp (currp);
	initp (newp, sp);	/* architecture-dependent: from $(ARCH).c */
	reschedp ();
	return (newp);
}

/*
 * readyp -- put process on ready queue.  if null, assume current.
 */
void readyp (struct pcb *p)
{
	extern struct pcb *currp;
	extern struct lpq schedq[];

	if (!p)
		p = currp;
	toq (&schedq[p->pri], p);
}

/*
 * getenvp -- return back-pointer to user's data
 */
void *getenvp (struct pcb *p)
{
	if (!p) p = currp;
	return (p->envp);
}

/*
 * setenvp -- set back-pointer to user's data
 */
void setenvp (struct pcb *p, void *ud)
{
	if (!p) p = currp;
	p->envp = ud;
}

/*
 * yieldp -- yield the processor to another thread.
 */
void yieldp (void)
{
	readyp (currp);
	reschedp ();
}

/*
 * suicidep -- cause the current process to be scheduled for deletion.
 */
void suicidep (void)
{
	currp->dead = 1;
	yieldp ();
}

/*
 * destroyp -- mark a process as dead, so it will never be rescheduled.
 */
void destroyp (struct pcb *p) { p->dead = 1; }

/*
 * prisetp -- set the thread's priority, returning the old.
 * if the new priority is lower than the old, we reschedule.
 */
int prisetp (int new)
{
	int old = currp->pri;

	if (MAXTPRI <= new)
		new = MAXTPRI-1;
	if (maxpri < new)
		maxpri = new;
	currp->pri = new;
	if (new < old)
		yieldp ();
	return (old);
}

/*
 * prisetp -- set the thread's priority, returning the old.
 * Does not do any rescheduling. The thread will run with
 * this priority until the next yieldp()
 */
int prichangep (int new)
{
	int old = currp->pri;

	if (MAXTPRI <= new)
		new = MAXTPRI-1;
	if (maxpri < new)
		maxpri = new;
	currp->pri = new;
	return (old);
}

/*
 * initlp -- initialise the coroutine structures
 */
struct pcb *initlp (int pri)
{
	extern struct lpq schedq[];
	extern struct pcb *currp;
	extern void onalarm ();
	struct lpq *q;
	int i, *stack;
	struct sigaction s;

	if (!(currp = (struct pcb *)calloc (sizeof (struct pcb),1)))
		return (0);
	if (!(stack = (int *)malloc (64)))
		return (0);
	if (MAXTPRI <= pri)
		pri = MAXTPRI-1;
	if (maxpri < pri)
		maxpri = pri;

	currp->next = 0;
	currp->sbtm = stack;	/* dummy stack */
	currp->pri = pri;
	currp->dead = 0;
	currp->size = 0; /* dummy size */

	for (i=MAXTPRI, q=schedq; i--; q++)
		q->head = q->tail = 0;
	deadq.head = deadq.tail = 0;

	s.sa_handler = &onalarm;
	s.sa_flags = SA_INTERRUPT;
	sigemptyset(&s.sa_mask);

	sigaction (SIGALRM, &s, (struct sigaction *)0);

	oldmask = sigsetmask (SIGNALS);

	creatp (0, nullp, 8192, 0, 0, 0);
	return (currp);
}
