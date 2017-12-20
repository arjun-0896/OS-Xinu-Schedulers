/* resched.c  -  resched */
#include<sched.h>
#include<math.h>
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>

unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */
int resched()
{
	register struct	pentry	*optr;	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */

        if(sched_class == 1)
    {
        int i;
        optr= &proctab[currpid];
        int a;
        double rb= expdev(0.1);
        int rn= (int) rb;
        int bn=-1;
        int newprocPID=0;
        struct pentry *p;
        for(i=0;i<5;i++)
        {
            a = rre[i];
            rre[i]=0;
            if(a != 0 && (proctab[a].pstate == PRREADY ))
            {
                if(optr->pstate == PRCURR)
                {
                    optr->pstate=PRREADY;
                    insert(currpid, rdyhead, optr->pprio);
                }
                nptr= &proctab[a];
                nptr->pstate=PRCURR;
                dequeue(a);
                currpid=a;
#ifdef    RTCLOCK
                preempt = QUANTUM;        //reset preemption counter
#endif
                ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
                //The OLD process returns here when resumed.
                return OK;
            }
        }
        for(i=1; i < NPROC ;i++)
        {
            p = &proctab[i];
            if(p-> pstate == PRREADY || p -> pstate == PRCURR )
            {
                if( p->pprio > rn && bn==-1)
                {
                    bn=p->pprio;
                    newprocPID = i;
                }
                else if( p->pprio > rn && p->pprio<bn)
                {
                    bn=p->pprio;
                    newprocPID=i;
                }
            }
        }
        if(bn == -1)
        {
            int sum = -100;
            for(i=1; i< NPROC; i++)
            {
                p = &proctab[i];
                if(p->pstate == PRREADY || p->pstate == PRCURR)
                {
                if(p->pprio>sum)
                {
                    sum=p->pprio;
                    newprocPID=i;
                }
                }
            }
        }
        int k=0;
        if(newprocPID == 0)
        {
            if (currpid == NULLPROC) {
                return OK;
            }
            else
            {
                newprocPID = NULLPROC;
                nptr = &proctab[newprocPID];
                nptr->pstate = PRCURR;
                dequeue(newprocPID);
                currpid = newprocPID;
#ifdef    RTCLOCK
                preempt = QUANTUM;
#endif
                ctxsw((int) &optr->pesp, (int) optr->pirmask, (int) &nptr->pesp, (int) nptr->pirmask);
                return OK;
            }
        }
        for(i=1; i< NPROC ; i++)
        {
            if( i!= newprocPID)
            {
                p = &proctab[i];
                if(p->pstate == PRREADY || p->pstate == PRCURR)
                {
                    if(p->pprio == proctab[newprocPID].pprio)
                    {
                        rre[k]= i;
                        k++;
                    }
                }
            }
        }
            if(optr->pstate==PRCURR && newprocPID == currpid)
            {
#ifdef    RTCLOCK
                preempt = QUANTUM;
#endif
                return OK;
            }
         else //(currpid != newprocPID || optr->pstate !=PRCURR)
                {
                    if(optr->pstate == PRCURR)
                    {
                        optr->pstate=PRREADY;
                        insert(currpid, rdyhead, optr->pprio);
                    }
                nptr= &proctab[newprocPID];
                nptr->pstate=PRCURR;
                dequeue(newprocPID);
                currpid=newprocPID;
#ifdef	RTCLOCK
                preempt = QUANTUM;		//reset preemption counter
#endif
                ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
                //The OLD process returns here when resumed.
                return OK;
                
                }
        //else
          //  return  SYSERR;
}
    
     else if(sched_class == 2)
    {
        optr = &proctab[currpid];
        if(count==1)
        {
            int optrOldPrio = optr->goodness - optr->counter; //Even If optr's prio has been changed, difference gives old priority
            optr->goodness = optrOldPrio + preempt + (optr->counter - QUANTUM);
            optr->counter = preempt + (optr->counter - QUANTUM);
            count=0;
        }
        else
        {
            int optrOldPrio = optr->goodness - optr->counter; //Even If optr's prio has been changed, difference gives old priority
            optr->goodness = optrOldPrio + preempt;
            optr->counter = preempt;
        }
        if (optr->counter <= 0 || currpid == NULLPROC) {
            optr->counter = 0;
            optr->goodness = 0;
        }
        //Find, of all runnable processes which has highest goodness
        int maxGoodness = 0;
        int k = 0;
        int newprocPID = 0;
        struct pentry *p;
        for (k = q[rdytail].qprev; k != rdyhead; k = q[k].qprev) {
                p = &proctab[k];
            if (p->goodness > maxGoodness) {
                newprocPID = k;
                maxGoodness = proctab[k].goodness;
            }
        }
        
        if (maxGoodness == 0 && (optr->pstate != PRCURR || optr->counter == 0))
        {
                int i;
                for (i = 0; i < NPROC; i++) {
                    p = &proctab[i];
                    if (p->pstate == PRFREE)
                        continue;
                    else if (p->counter == 0 || p->counter == p->quantum) { //Process either exhausted or not run at all;
                        p->quantum = p->pprio;
                    } else {
                        p->quantum = (p->counter) / 2 + p->pprio;
                    }
                    p->counter = p->quantum;
                    p->goodness = p->counter + p->pprio;
                }
                
                for (k = q[rdytail].qprev; k != rdyhead; k = q[k].qprev) {
                    p = &proctab[k];
                    if (p->goodness > maxGoodness) {
                        newprocPID = k;
                        maxGoodness = proctab[k].goodness;
                    }
                }
            }
        
            if (maxGoodness == 0) {
            if (currpid == NULLPROC) {
                return OK;
            } else {
                newprocPID = NULLPROC;
                if (optr->pstate == PRCURR) { //Just like in Xinu Scheduling
                    optr->pstate = PRREADY;
                    insert(currpid, rdyhead, optr->pprio);
                }
                nptr = &proctab[newprocPID];
                nptr->pstate = PRCURR;
                dequeue(newprocPID);
                currpid = newprocPID;
#ifdef	RTCLOCK
                preempt = QUANTUM;
#endif
                ctxsw((int) &optr->pesp, (int) optr->pirmask, (int) &nptr->pesp, (int) nptr->pirmask);
                return OK;
            }
        }
        else if (optr->goodness >= maxGoodness && optr->pstate == PRCURR) {
            preempt = optr->counter;
            for (k = q[rdytail].qprev; k != rdyhead; k = q[k].qprev) { //round robin implementation
                p = &proctab[k];
                if (p->goodness == optr -> goodness) {
                    count=1;
#ifdef    RTCLOCK
                    preempt = QUANTUM;        //reset preemption counter
#endif
                    break;
                }
            }
            return OK;
        }
        else //(optr->pstate != PRCURR || optr->goodness < maxGoodness){
        {
            if (optr->pstate == PRCURR) { //Just like in Xinu Scheduling
                optr->pstate = PRREADY;
                insert(currpid, rdyhead, optr->pprio);
            }
            nptr = &proctab[newprocPID];
            nptr->pstate = PRCURR;
            dequeue(newprocPID);
            currpid = newprocPID;
            preempt = nptr->counter;
            for (k = q[rdytail].qprev; k != rdyhead; k = q[k].qprev) { //round robin implementation
                p = &proctab[k];
                if (p->goodness == nptr -> goodness && k != newprocPID) {
                    count=1;
#ifdef    RTCLOCK
                    preempt = QUANTUM;        //reset preemption counter
#endif
                    break;
                }
            }
            ctxsw((int) &optr->pesp, (int) optr->pirmask, (int) &nptr->pesp, (int) nptr->pirmask);
            return OK;
        }
            //else
            //return SYSERR;
        
    }
    else
    {
        /* no switch needed if current process priority higher than next*/

	if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
	   (lastkey(rdytail)<optr->pprio)) {
		return(OK);
	}
	
	/* force context switch */

	if (optr->pstate == PRCURR) {
		optr->pstate = PRREADY;
		insert(currpid,rdyhead,optr->pprio);
	}

	/* remove highest priority process at end of ready list */

	nptr = &proctab[ (currpid = getlast(rdytail)) ];
	nptr->pstate = PRCURR;		/* mark it currently running	*/
#ifdef	RTCLOCK
	preempt = QUANTUM;		/* reset preemption counter	*/
#endif
	
	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	
	/* The OLD process returns here when resumed. */
	return OK;
    }
}
