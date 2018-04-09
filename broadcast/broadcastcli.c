#include <stdio.h>
#include <sys/types.h>	       /* See NOTES */
#include <sys/socket.h>
#include <linux/in.h>
#include <string.h>
#include <errno.h>
//#include <arpa/inet.h>
	
int sockfd;

#define UDP_PORT 5008
#define BROADCAST_IP 	"224.1.1.1"
int create_socket(void)
{
#ifdef SERVER
	sockfd = socket(AF_INET,SOCK_DGRAM,0);
	if(-1 == sockfd)
	{
		fprintf(stderr,"sockfd err:%s\n",strerror(errno));
		return -1;
	}
#else
	sockfd = socket(AF_INET,SOCK_DGRAM,0);
	if(-1 == sockfd)
	{
		fprintf(stderr,"sockfd err:%s\n",strerror(errno));
		return -1;
	}
#endif
	return 0;
}
int main(int argc, const char *argv[])
{
	int ret = -1;
	unsigned int addrlen;
	char recvbuf[1024];
	struct sockaddr_in sockaddr,peeraddr;
	ret = create_socket();
	if(-1 == ret)
	{
		fprintf(stderr,"create_socket failed\n");
		return -1;	
	}
	memset(&sockaddr,0,sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = INADDR_ANY;
	sockaddr.sin_port = htons(UDP_PORT);
#ifdef SERVER
	ret = bind(sockfd,(struct sockaddr *)&sockaddr,sizeof(sockaddr));	
	if(-1 == ret)
	{
		fprintf(stderr,"bind err:%s\n",strerror(errno));
		close(sockfd);
		return -1;
	}
	struct ip_mreq mreq;
	inet_pton(AF_INET,BROADCAST_IP,&(mreq.imr_multiaddr.s_addr));
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	int loop = 1;
	ret = setsockopt(sockfd,IPPROTO_IP,IP_MULTICAST_LOOP,&loop,sizeof(loop));
	if(-1 == ret)
	{
		fprintf(stderr,"setsockopt set loop err:%s\n",strerror(errno));
	}

	ret = setsockopt(sockfd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq));
	if(-1 == ret)
	{
		fprintf(stderr,"setsockopt set cast err:%s\n",strerror(errno));
	}

	for(;;)
	{
		memset(&peeraddr,0,sizeof(peeraddr));
		addrlen = sizeof(peeraddr);
		memset(recvbuf,0,sizeof(recvbuf));
		ret = recvfrom(sockfd,recvbuf,sizeof(recvbuf),0,(struct sockaddr *)&peeraddr,(socklen_t*)&addrlen);
		if(-1 == ret)
		{
			fprintf(stderr,"recvfrom err:%s\n",strerror(errno));
			continue;
		}
		fprintf(stdout,"recvmsg:\t%s\n",recvbuf);
		if(strcmp(recvbuf,"quit"))
		{
			fprintf(stdout,"quit...\n");
			break;
		}
	}
#else
	struct sockaddr_in mcastaddr;
	memset(&mcastaddr,0,sizeof(mcastaddr));
	mcastaddr.sin_family =AF_INET;
	mcastaddr.sin_port = htons(UDP_PORT);	
	inet_pton(AF_INET,BROADCAST_IP,&(mcastaddr.sin_addr.s_addr));
	for(;;)
	{
		memset(&peeraddr,0,sizeof(peeraddr));
		addrlen = sizeof(peeraddr);
		memset(recvbuf,0,sizeof(recvbuf));
		if(NULL != fgets(recvbuf,sizeof(recvbuf),stdin))
		{
			//去掉回车
			recvbuf[strlen(recvbuf)-1] = '\0';
#if 0
			ret = sendto(sockfd,recvbuf,strlen(recvbuf),0,(struct sockaddr*)&sockaddr,addrlen);
			fprintf(stdout,"sendto :%d bytes \n",ret);
#endif		
			strcat(recvbuf,"  Broadcast");
			ret = sendto(sockfd,recvbuf,strlen(recvbuf),0,(struct sockaddr*)&mcastaddr,sizeof(mcastaddr));
			fprintf(stdout,"sendto  Broadcast:%d bytes \n",ret);
		}
		if(!strcmp(recvbuf,"quit"))
		{
			fprintf(stdout,"quit...\n");
			break;
		}
	}
#endif
	close(sockfd);
	return 0;
}
