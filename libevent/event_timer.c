#include <stdio.h>
#include <string.h>
#include <event.h>
struct timeval tv;

void timer_proc(int fd,short event,void *arg)
{
	static int i = 0;
	struct event *ev;
	tv.tv_sec = 2;
	tv.tv_usec = 0;
	ev = arg;
	fprintf(stdout,"Hello world %d, fd = %d,event=%d\n",i++,fd,event);
	evtimer_add(ev,&tv);
	return;
}
int main(int argc, const char *argv[])
{ 
	struct event ev;
	tv.tv_sec = 2;
	tv.tv_usec = 0;

	event_init();
	evtimer_set(&ev,timer_proc,&ev);
	evtimer_add(&ev,&tv);
	event_dispatch();
	return 0;
}
