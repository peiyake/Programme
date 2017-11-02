/*链表存储实际数据结构*/
typedef struct 
{
	int data_a;
	char datab[32];
}data_t;
/*链表结构*/
typedef struct list
{
	data_t value;
	struct list *next;
}list_t;
/*队列头定义*/
typedef struct
{
	list_t *head;
	list_t *tail;
}list_queue_t;
/**
 * 函数功能:
 * 参数说明:
 *
 * */
int queue_init(list_queue_t *q)
{
	q->head = q->tail = (list_t*)malloc(sizeof(list_t));
	if(!q->head)
	{
		log_printf(LOG_ERROR,"queue init fail!\n");
		return FALSE;
	}
	q->tail->next = NULL;
	return TRUE;
}
/**
 * 数据入队
 * */
int queue_add(list_queue_t *q,data_t *data)
{
	list_t *new;
	new = (list_t *)malloc(sizeof(list_t));
	if(NULL == new)
	{
		log_printf(LOG_ERROR,"queue_add malloc fail!\n");
		return FALSE;
	}
	memcpy(&(new->value),data,sizeof(data_t));
	
	new->next = NULL;
	q->tail->next = new;
	q->tail = new;
	return TRUE;
}
/*
 * 数据出队
 * */
int queue_del(list_queue_t *q,data_t *data)
{
	list_t *tmp;
	if(q->head == q->tail)
	{
		log_printf(LOG_ERROR,"queue is empty!\n");
		return FALSE;
	}
	tmp = q->head->next;
	memcpy(data,&(tmp->value),sizeof(data_t));
	q->head->next = tmp->next;
	if(tmp == q->tail)
	{
		q->tail = q->head;
	}
	free(tmp);
	return TRUE;
}
/**
 * 检测队列是否为空
 * */
int queue_empty(list_queue_t *q)
{
	return (q->head==q->tail)?TRUE:FALSE;
}

