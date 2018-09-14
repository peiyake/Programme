#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <time.h>
#include <errno.h>
#include "libtools.h"

time_t get_bj_time()
{
    return time(NULL) + 8*3600;
}
int queue_create(queue_t *queue)
{

    queue->head = queue->tail = (data_t *)malloc(sizeof(data_t));
    if(NULL == queue->head) return Ret_ERR;

    queue->head->next = NULL;
    return Ret_OK;
}

int queue_push(queue_t *queue,void *data,int datalen)
{
    if ((NULL == data) || (NULL == queue))
        return Ret_ERR;

    data_t *newnode = NULL;
    newnode = (data_t*)malloc(sizeof(data_t));
    if (NULL == newnode)
        return Ret_ERR;

    newnode->len = datalen;
    newnode->data = malloc(newnode->len);
    if(NULL == newnode->data)
    {
        free(newnode);
        return Ret_ERR;
    }
    memcpy(newnode->data,data,newnode->len);
    newnode->next = NULL;

    queue->tail->next = newnode;
    queue->tail = newnode;
    return Ret_OK;
}

int queue_pull(queue_t *queue,void *data,int datalen)
{
    data_t *tmp;
    if ((NULL == data) || (NULL == queue))
        return Ret_ERR;

    if (queue->head == queue->tail)
        return Ret_ERR;

    tmp = queue->head->next;
    if(tmp->len != datalen)
        return Ret_ERR;
    memcpy(data,tmp->data,tmp->len);
	queue->head->next = tmp->next;
	if(tmp == queue->tail)
		queue->tail = queue->head;
    free(tmp->data);
	free(tmp);
	return Ret_OK;
}

int queue_destory(queue_t *queue)
{
    data_t *tmp = NULL;
    if(NULL == queue)
        return Ret_ERR;
    for(;;)
    {
        if (queue->head == queue->tail)
        {
            free(queue->head);
            break;
        }
        tmp = queue->head->next;
        queue->head->next = tmp->next;
        if(tmp == queue->tail)
            queue->tail = queue->head;
        free(tmp->data);
        free(tmp);
    }
    queue->head = queue->tail = NULL;
    return Ret_OK;
}

int queue_empty(queue_t *queue)
{
    return (queue->head == queue->tail)?Ret_OK:Ret_ERR;
}

int queue_count(queue_t *queue)
{
    int count = 0;
    data_t *tmp = NULL;
    tmp = queue->head->next;
    for(tmp ; tmp != NULL ; tmp = tmp->next)
        count++;
    return count;
}

int create_semaphore(sem_t *sem,unsigned int sem_num)
{
    if((NULL == sem) || (sem_num < 1))
        return Ret_ERR;
    if (0 != sem_init(sem, 0, sem_num))
        return Ret_ERR;
    return Ret_OK;
}

int semaphore_p(sem_t *sem,int type,time_t time)
{
    if(NULL == sem)
        return Ret_ERR;

    int ret;
    struct timespec time_s;

    if(time <= 0)
        time = 0;
    time_s.tv_sec = time;
    time_s.tv_nsec = 0;
	switch(type)
	{
		case NO_WAIT:
			ret = sem_trywait(sem);
			break;
		case WAIT_FOREVER:
			ret = sem_wait(sem);
			break;
		case WAIT_TIME:
			ret = sem_timedwait(sem,&time_s);
		break;
	}
	return (ret == 0) ? Ret_OK : Ret_ERR;
}
int semaphore_v(sem_t *sem)
{
    if(NULL == sem)
        return Ret_ERR;

    int ret;
	ret = sem_post(sem);
	if(0 != ret)
        return Ret_ERR;
    return Ret_OK;
}

int get_mac(const char *name, char *dst)
{
	int socket_fd,ret;
	struct ifreq ifr_mac;
	char mac_buf[32];

    memset(mac_buf, 0, 32);
    memset(&ifr_mac, 0, sizeof(struct ifreq));
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(-1 == socket_fd)
        return Ret_ERR;
    strcpy(ifr_mac.ifr_name,name);
    ret = ioctl(socket_fd, SIOCGIFHWADDR, &ifr_mac);
    if(-1 == ret)
        return Ret_ERR;
    sprintf(mac_buf, "%02X-%02X-%02X-%02X-%02X-%02X",
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[0],
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[1],
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[2],
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[3],
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[4],
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[5]);
    close(socket_fd);
    strcpy(dst, mac_buf);
    return Ret_OK;
}