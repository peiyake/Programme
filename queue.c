/*����洢ʵ�����ݽṹ*/
typedef struct 
{
	int data_a;
	char datab[32];
}data_t;
/*����ṹ*/
typedef struct list
{
	data_t value;
	struct list *next;
}list_t;
/*����ͷ����*/
typedef struct
{
	list_t *head;
	list_t *tail;
}list_queue_t;
/**
 * ��������:
 * ����˵��:
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
 * �������
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
 * ���ݳ���
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
 * �������Ƿ�Ϊ��
 * */
int queue_empty(list_queue_t *q)
{
	return (q->head==q->tail)?TRUE:FALSE;
}

