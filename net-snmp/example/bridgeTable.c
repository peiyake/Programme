#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <errno.h>
#include "public.h"
#include "mib.h"
#include "bridgeTable.h"

struct bridgeTable_entry * bridgeTable_createEntry(struct bridgeTable_entry *data);
void bridgeTable_removeEntry(struct bridgeTable_entry *entry);

/*
 * column number definitions for table bridgeInfoTable 
 */
enum{
	COLUMN_BRID=1,
	COLUMN_BRIFID,
	COLUMN_BRNAME,
	COLUMN_BRIFNAME
}ifinfotable_cols;

struct bridgeTable_entry *bridgeTable_head = NULL;

void init_bridge_data(void)
{
	FILE *f = NULL;
	FILE *f2 = NULL;
	char buf[128],*p;
	struct bridgeTable_entry data;
	char cmd[128];
	int index = 1,ret;
	f = popen("brctl show| grep -v bridge |awk 'NF>2 {print $1}'","r");
	if(NULL == f)
	{
		syslog(LOG_ERR,"popen err:$s\n",strerror(errno));
		return;
	}
	memset(buf,0,128);
	
	while(NULL != fgets(buf,sizeof(buf),f))
	{
		memset(&data,0,sizeof(struct bridgeTable_entry));
		buf[strlen(buf) -1] = '\0';
		
		if(strlen(buf) == 0) continue;
			
		data.brId = index;
		strncpy(data.brName,buf,strlen(buf));
		data.brName_len = strlen(data.brName);
		
		/*get interface on bridge*/
		memset(cmd,0,128);
		sprintf(cmd,"ls -l /sys/class/net/%s/brif/ | awk '{print $9}'",data.brName);
		
		f2 = popen(cmd,"r");
		if(NULL == f2)
		{
			syslog(LOG_ERR,"popen err:$s\n",strerror(errno));
			continue;
		}
		memset(buf,0,sizeof(buf));
		while(NULL != fgets(buf,sizeof(buf),f2))
		{
			buf[strlen(buf) -1] = '\0';
			
			if(strlen(buf) == 0) continue;

			strncpy(data.brIfName,buf,strlen(buf));
			data.brIfName_len = strlen(data.brIfName);

			ret = ifInfoTable_getIdByname(data.brIfName);
			if(-1 == ret) continue;

			data.brIfID = ret;

			/*add list*/
			bridgeTable_createEntry(&data);
			memset(buf,0,sizeof(buf));
		}
		pclose(f2);
		index++;
		memset(buf,0,sizeof(buf));
	}
	pclose(f);

	return;
}
/** Initializes the bridgeTable module */

void initialize_table_bridgeTable(void)
{
    const oid       bridgeTable_oid[] = {BRIDGE_TABLE_OID};
    const size_t    bridgeTable_oid_len = OID_LENGTH(bridgeTable_oid);
    
    netsnmp_handler_registration *reg;
    netsnmp_iterator_info *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration("bridgeTable",
                                            bridgeTable_handler,
                                            bridgeTable_oid,
                                            bridgeTable_oid_len,
                                            HANDLER_CAN_RWRITE);

    table_info = SNMP_MALLOC_TYPEDEF(netsnmp_table_registration_info);
    netsnmp_table_helper_add_indexes(table_info, ASN_INTEGER,   /* index: brId */
                                     ASN_INTEGER,       /* index: brIfID */
                                     0);
    table_info->min_column = COLUMN_BRID;
    table_info->max_column = COLUMN_BRIFNAME;

    iinfo = SNMP_MALLOC_TYPEDEF(netsnmp_iterator_info);
    iinfo->get_first_data_point = bridgeTable_get_first_data_point;
    iinfo->get_next_data_point = bridgeTable_get_next_data_point;
    iinfo->table_reginfo = table_info;

    netsnmp_register_table_iterator(reg, iinfo);
}


struct bridgeTable_entry * bridgeTable_createEntry(struct bridgeTable_entry *data)
{
    struct bridgeTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct bridgeTable_entry);
    if (!entry)
        return NULL;
	memset(entry,0,sizeof(struct bridgeTable_entry));
	
    entry->brId = data->brId;
    entry->brIfID = data->brIfID;
    entry->brName_len = strlen(data->brName);
    entry->brIfName_len = strlen(data->brIfName);
    strcpy(entry->brName,data->brName);
    strcpy(entry->brIfName,data->brIfName);
    
   	syslog(LOG_INFO,"Bridge[%d][%d]:%s-%s\n",entry->brId,entry->brIfID,entry->brName,entry->brIfName);
    entry->next = bridgeTable_head;
    bridgeTable_head = entry;
    return entry;
}

/*
 * remove a row from the table 
 */
void bridgeTable_removeEntry(struct bridgeTable_entry *entry)
{
    struct bridgeTable_entry *ptr, *prev;

    if (!entry)
        return;                 /* Nothing to remove */

    for (ptr = bridgeTable_head, prev = NULL;
         ptr != NULL; prev = ptr, ptr = ptr->next) {
        if (ptr == entry)
            break;
    }
    if (!ptr)
        return;                 /* Can't find it */

    if (prev == NULL)
        bridgeTable_head = ptr->next;
    else
        prev->next = ptr->next;

    SNMP_FREE(entry);           /* XXX - release any other internal resources */
}


/*
 * Example iterator hook routines - using 'get_next' to do most of the work 
 */
netsnmp_variable_list * bridgeTable_get_first_data_point(void **my_loop_context,
                                 void **my_data_context,
                                 netsnmp_variable_list * put_index_data,
                                 netsnmp_iterator_info *mydata)
{
    *my_loop_context = bridgeTable_head;
    return bridgeTable_get_next_data_point(my_loop_context,
                                           my_data_context, put_index_data,
                                           mydata);
}

netsnmp_variable_list *bridgeTable_get_next_data_point(void **my_loop_context,
                                void **my_data_context,
                                netsnmp_variable_list * put_index_data,
                                netsnmp_iterator_info *mydata)
{
    struct bridgeTable_entry *entry =
        (struct bridgeTable_entry *) *my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if (entry) {
        snmp_set_var_typed_integer(idx, ASN_INTEGER, entry->brId);
        idx = idx->next_variable;
        snmp_set_var_typed_integer(idx, ASN_INTEGER, entry->brIfID);
        idx = idx->next_variable;
        *my_data_context = (void *) entry;
        *my_loop_context = (void *) entry->next;
        return put_index_data;
    } else {
        return NULL;
    }
}


/** handles requests for the bridgeTable table */
int bridgeTable_handler(netsnmp_mib_handler *handler,
                    netsnmp_handler_registration *reginfo,
                    netsnmp_agent_request_info *reqinfo,
                    netsnmp_request_info *requests)
{
	int ret;
    netsnmp_request_info *request;
    netsnmp_table_request_info *table_info;
    struct bridgeTable_entry *table_entry;

    switch (reqinfo->mode) {
    case MODE_GET:
        for (request = requests; request; request = request->next) {
            table_entry = (struct bridgeTable_entry *)
                netsnmp_extract_iterator_context(request);
            table_info = netsnmp_extract_table_info(request);

            switch (table_info->colnum) {
            case COLUMN_BRID:
                if (!table_entry) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer(request->requestvb, ASN_INTEGER,
                                           table_entry->brId);
                break;
            case COLUMN_BRIFID:
                if (!table_entry) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer(request->requestvb, ASN_INTEGER,
                                           table_entry->brIfID);
                break;
            case COLUMN_BRNAME:
                if (!table_entry) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_value(request->requestvb, ASN_OCTET_STR,
                                         table_entry->brName,
                                         table_entry->brName_len);
                break;
            case COLUMN_BRIFNAME:
                if (!table_entry) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_value(request->requestvb, ASN_OCTET_STR,
                                         table_entry->brIfName,
                                         table_entry->brIfName_len);
                break;
            default:
                netsnmp_set_request_error(reqinfo, request,
                                          SNMP_NOSUCHOBJECT);
                break;
            }
        }
        break;

    case MODE_SET_RESERVE1:
        for (request = requests; request; request = request->next) {
            table_entry = (struct bridgeTable_entry *)
                netsnmp_extract_iterator_context(request);
            table_info = netsnmp_extract_table_info(request);

            switch (table_info->colnum) {
            case COLUMN_BRID:
                ret = netsnmp_check_vb_int(request->requestvb);
                if (ret != SNMP_ERR_NOERROR) {
                    netsnmp_set_request_error(reqinfo, request, ret);
                    return SNMP_ERR_NOERROR;
                }
                break;
            case COLUMN_BRIFID:
                ret = netsnmp_check_vb_int(request->requestvb);
                if (ret != SNMP_ERR_NOERROR) {
                    netsnmp_set_request_error(reqinfo, request, ret);
                    return SNMP_ERR_NOERROR;
                }
                break;
            case COLUMN_BRNAME:
                ret = netsnmp_check_vb_type_and_max_size(request->requestvb,
                                                       ASN_OCTET_STR,
                                                       sizeof(table_entry->brName));
                if (ret != SNMP_ERR_NOERROR) {
                    netsnmp_set_request_error(reqinfo, request, ret);
                    return SNMP_ERR_NOERROR;
                }
                break;
            case COLUMN_BRIFNAME:
                ret = netsnmp_check_vb_type_and_max_size(request->requestvb,
                                                       ASN_OCTET_STR,
                                                       sizeof(table_entry->brIfName));
                if (ret != SNMP_ERR_NOERROR) {
                    netsnmp_set_request_error(reqinfo, request, ret);
                    return SNMP_ERR_NOERROR;
                }
                break;
            default:
                netsnmp_set_request_error(reqinfo, request,
                                          SNMP_ERR_NOTWRITABLE);
                return SNMP_ERR_NOERROR;
            }
        }
        break;

    case MODE_SET_RESERVE2:
        break;

    case MODE_SET_FREE:
        break;

    case MODE_SET_ACTION:
        for (request = requests; request; request = request->next) {
            table_entry = (struct bridgeTable_entry *)
                netsnmp_extract_iterator_context(request);
            table_info = netsnmp_extract_table_info(request);

            switch (table_info->colnum) {
            case COLUMN_BRID:
                table_entry->old_brId = table_entry->brId;
                table_entry->brId = *request->requestvb->val.integer;
                break;
            case COLUMN_BRIFID:
                table_entry->old_brIfID = table_entry->brIfID;
                table_entry->brIfID = *request->requestvb->val.integer;
                break;
            case COLUMN_BRNAME:
                memcpy(table_entry->old_brName,
                       table_entry->brName, sizeof(table_entry->brName));
                table_entry->old_brName_len = table_entry->brName_len;
                memset(table_entry->brName, 0,
                       sizeof(table_entry->brName));
                memcpy(table_entry->brName,
                       request->requestvb->val.string,
                       request->requestvb->val_len);
                table_entry->brName_len = request->requestvb->val_len;
                break;
            case COLUMN_BRIFNAME:
                memcpy(table_entry->old_brIfName,
                       table_entry->brIfName,
                       sizeof(table_entry->brIfName));
                table_entry->old_brIfName_len = table_entry->brIfName_len;
                memset(table_entry->brIfName, 0,
                       sizeof(table_entry->brIfName));
                memcpy(table_entry->brIfName,
                       request->requestvb->val.string,
                       request->requestvb->val_len);
                table_entry->brIfName_len = request->requestvb->val_len;
                break;
            }
        }
        break;

    case MODE_SET_UNDO:
        for (request = requests; request; request = request->next) {
            table_entry = (struct bridgeTable_entry *)
                netsnmp_extract_iterator_context(request);
            table_info = netsnmp_extract_table_info(request);

            switch (table_info->colnum) {
            case COLUMN_BRID:
                table_entry->brId = table_entry->old_brId;
                table_entry->old_brId = 0;
                break;
            case COLUMN_BRIFID:
                table_entry->brIfID = table_entry->old_brIfID;
                table_entry->old_brIfID = 0;
                break;
            case COLUMN_BRNAME:
                memcpy(table_entry->brName,
                       table_entry->old_brName,
                       sizeof(table_entry->brName));
                memset(table_entry->old_brName, 0,
                       sizeof(table_entry->brName));
                table_entry->brName_len = table_entry->old_brName_len;
                break;
            case COLUMN_BRIFNAME:
                memcpy(table_entry->brIfName,
                       table_entry->old_brIfName,
                       sizeof(table_entry->brIfName));
                memset(table_entry->old_brIfName, 0,
                       sizeof(table_entry->brIfName));
                table_entry->brIfName_len = table_entry->old_brIfName_len;
                break;
            }
        }
        break;

    case MODE_SET_COMMIT:
        break;
    }
    return SNMP_ERR_NOERROR;
}
/** Initializes the bridgeTable module */
void init_bridgeTable(void)
{
	init_bridge_data();
	initialize_table_bridgeTable();
}

