/*
 * clk.c -- timer routines for lightweight processes.
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
 * (fleiner@icsi.berkeley.edu). New features include locks, the possibility
 * to interrupt threads, thread local memory, definitions of
 * functions to execute whenever a thread is rescheduled or killed, and more.
 * These features have been tested under sun 4.1.3 and linux 1.1.59.
 */

#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include "lwp.h"

extern int getitimer(int,struct itimerval *value);
extern int setitimer (int,const struct itimerval*,struct itimerval*);
extern int gettimeofday(struct timeval *,struct timezone *);



/* delta queue for the timer */
static struct lpq delayq;

/*
 * tvsub calculate to=a-b; for timevals.  assumes a>b
 */
static void tvsub (struct timeval *to, struct timeval *a, struct timeval *b)
{
	if (a->tv_usec < b->tv_usec) {
		a->tv_usec += 1000000;
		a->tv_sec--;
	}
	to->tv_usec = a->tv_usec - b->tv_usec;
	to->tv_sec = a->tv_sec - b->tv_sec;
}

static struct timeval tstart,last_timer;

/*
 * delay -- block the invoker for >t microseconds.  If t==0, reschedule.
 */
#define cmptmvi(a,b) 	(((a)>(b))?1:((a)<(b)?-1:0))
#define cmptmv(a,b)  	((a).tv_sec==(b).tv_sec?cmptmvi((a).tv_usec,(b).tv_usec):cmptmvi((a).tv_sec,(b).tv_sec))
void delayp (long ut)
{
	extern struct pcb *currp;
	struct pcb *p=delayq.head, *q=0;
	struct itimerval itime;
	struct timeval t, *left = &itime.it_value;

	if (!ut) {
		yieldp ();
		return;
	}
	t.tv_sec = ut/1000000;
	t.tv_usec = ut%1000000;
	if (p) {
		getitimer (ITIMER_REAL, &itime);
		if(cmptmv(itime.it_value,last_timer)>0) itime.it_value=last_timer;
		if (cmptmv(*left, t)>0) {
			/* new delay less than old */
			delayq.head = currp;
			tvsub (&p->dt, left, &t);
		} else {
			/* insert delay in body of queue */
			p->dt = *left;
			do {
				tvsub (&t, &t, &p->dt);
				q=p; p=p->next;
			} while (p && cmptmv(t, p->dt)>=0);
			q->next = currp;
			if (p)	/* adjust ensuing delta */
				tvsub (&p->dt, &p->dt, &t);
		}
	} else	/* the queue is empty */
		toq (&delayq, currp);

	/* time to wait has changed */
	currp->next = p;
	currp->dt = t;
	*left = delayq.head->dt;
	timerclear(&itime.it_interval);	/* one-shot */
	if (!timerisset(left)) left->tv_usec++;
	last_timer=itime.it_value;
	setitimer (ITIMER_REAL, &itime, (struct itimerval *)0);
	gettimeofday (&tstart, (struct timezone *)0);
	reschedp ();
}

/*
 * onalarm -- process alarm signals
 * this routine contains an adjustment for any current timer over-run
 */
void onalarm ()
{
	struct pcb *p;
	struct itimerval itime;
	struct timeval tnow, dt;

	gettimeofday (&tnow, (struct timezone *)0);
	tvsub (&dt, &tnow, &tstart);
	tstart = tnow;
	while ((p = delayq.head))
		if (cmptmv(dt, p->dt) >0) {	
			/* reduce the delta and ready the process */
			tvsub (&dt, &dt, &p->dt);
			p->dt.tv_sec = p->dt.tv_usec = 0;
			readyp (hoq (&delayq));
		} else {
			/* reduce time-to-wait */
			tvsub (&p->dt, &p->dt, &dt);
			break;
		}
	if (p) {
		/* start a new one-shot timer */
		timerclear(&itime.it_interval);
		itime.it_value = p->dt;
		last_timer=itime.it_value;
		setitimer (ITIMER_REAL, &itime, (struct itimerval *)0);
	}
}
