#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <event.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
static const char fifo[] = {"/tmp/fifo"};
int main(int argc, const char *argv[])
{
	int fd;	
	char buf[256];
	int i = 0;

	fd = open(fifo,O_RDWR);
	if(-1 == fd)
	{
		perror("open");
		return -1;
	}
	memset(buf,0,sizeof(buf));
	while(NULL != fgets(buf,sizeof(buf),stdin))
	{
		write(fd,buf,strlen(buf)-1);
		fprintf(stdout,"%-5s:%s\n","buf",buf);
		memset(buf,0,sizeof(buf));
	}
	close(fd);
	fprintf(stdout,"man over!\n");
	return 0;
}
