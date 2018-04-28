#include <stdio.h>
#include <string.h>
#include <errno.h>
int main(int argc, const char *argv[])
{
	FILE *f = NULL,*f2 = NULL;
	char buf[128];
	char brname[32];
	char ifname[32];
	f = popen("brctl show| grep -v bridge |awk 'NF>2 {print $1}'","r");
	if(NULL == f)
	{
		fprintf(stdout,"popen err:%s\n",strerror(errno));
		return -1;
	}
	memset(buf,0,sizeof(buf));
	while(NULL != fgets(buf,sizeof(buf),f))
	{
		buf[strlen(buf)-1] = '\0';
		if(0 == strlen(buf)) continue;
		memset(brname,0,sizeof(brname));
		strncpy(brname,buf,strlen(buf));

		memset(buf,0,sizeof(buf));
		sprintf(buf,"ls -l /sys/devices/virtual/net/%s/brif/ | awk '{print $9}'",brname);
		f2 = popen(buf,"r");
		if(NULL == f2)
		{
			fprintf(stdout,"popen err:%s\n",strerror(errno));
			continue;
		}
		memset(buf,0,sizeof(buf));
		while(NULL != fgets(buf,sizeof(buf),f2))
		{
			memset(ifname,0,sizeof(ifname));
			buf[strlen(buf)-1] = '\0';
			if(0 == strlen(buf)) continue;
			strncpy(ifname,buf,strlen(buf));
			fprintf(stdout,"%s-%s\n",brname,ifname);
		}
		pclose(f2);
		memset(buf,0,sizeof(buf));
	}
	pclose(f);
	return 0;
}
