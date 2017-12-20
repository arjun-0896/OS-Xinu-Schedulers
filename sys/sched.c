#include <sched.h>
#include <conf.h>
#include <kernel.h>
#include <proc.h>
int sched_class = 0;
int count=0;
int rre[5]={0,0,0,0,0};
void setschedclass(int sched) {
    sched_class = sched;
}
int getschedclass(){
    return sched_class;
}


