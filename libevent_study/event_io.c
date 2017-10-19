#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef LIBEVENT-1.4
	#include <event.h>
#else
	#include <event2/event.h>
#endif
static const char fifo[] = {"/tmp/fifo"};
void fifo_read(int fd,short event,void *arg)
{
	struct event *ev;
	char buf[256];
	int len;
	ev = (struct event*)arg;
#ifdef LIBEVENT-1.4
	/*libevent-1.4版本需要重载事件集才能持续循环*/
	event_add(ev,NULL);
	fprintf(stdout,"libevent-1.4\n");
#endif
	memset(buf,0,sizeof(buf));
	len = read(fd,buf,sizeof(buf));
	if (len <= 0) 
	{
		if(len == -1)
			perror("read");
		else if(len == 0)
			fprintf(stderr, "Connection closed\n");
#ifndef LIBEVENT-1.4
		event_del(ev);
		event_base_loopbreak(event_get_base(ev));
#endif
		return;
	}
	fprintf(stdout,"%-5s:%s\n","buf",buf);
	return;
}
int main(int argc, const char *argv[])
{
	int fd;	
	int i = 0;
#ifdef LIBEVENT-1.4
	struct event evfifo;
#else
	struct event_base *base;
	struct event *evfifo;
#endif
	unlink(fifo);

	if(-1 == mkfifo(fifo,0644))
	{
		perror("mkfifo");
		return -1;
	}
	
	fd = open(fifo,O_RDONLY);
	if(-1 == fd)
	{
		perror("open");
		return -1;
	}
#ifdef LIBEVENT-1.4
	event_init();
	event_set(&evfifo,fd,EV_READ,fifo_read,&evfifo);
	event_add(&evfifo,NULL);
	event_dispatch();
#else
	base = event_base_new();
	evfifo = event_new(base,fd,EV_READ|EV_PERSIST,fifo_read,event_self_cbarg());
	event_add(evfifo,NULL);
	event_base_dispatch(base);
	event_base_free(base);
#endif
	close(fd);
	unlink(fifo);
#ifndef LIBEVENT-1.4
	libevent_global_shutdown();
#endif
	return 0;
}
