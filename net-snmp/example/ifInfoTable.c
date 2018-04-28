#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <stdio.h>
#include <string.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>

#include "ifInfoTable.h"
#include "mib.h"
#include "public.h"
/*
 * column number definitions for table ifInfoTable 
 */
enum{
	COLUMN_IFID=1,
	COLUMN_IFNAME,
	COLUMN_IFIP,
	COLUMN_IFMAC
}ifinfotable_cols;

struct ifInfoTable_entry *ifInfoTable_head = NULL;

struct ifInfoTable_entry *ifInfoTable_createEntry(struct ifInfoTable_entry*pdata)
{
    struct ifInfoTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct ifInfoTable_entry);
    if (!entry)
        return NULL;

	memset(entry,0,sizeof(struct ifInfoTable_entry));
	
    entry->ifID = pdata->ifID;
    entry->ifIP = pdata->ifIP;
    memcpy(entry->ifMAC,pdata->ifMAC,6);
    entry->ifMAC_len = pdata->ifMAC_len;
   	strncpy(entry->ifName,pdata->ifName,strlen(pdata->ifName));
   	entry->ifName_len = pdata->ifName_len;

   	syslog(LOG_INFO,"Data instert:ifid[%d],ifname[%s]\n",entry->ifID,entry->ifName);
    entry->next = ifInfoTable_head;
    ifInfoTable_head = entry;
    return entry;
}

long ifInfoTable_getIdByname(char *ifName)
{
    struct ifInfoTable_entry *ptr, *prev;
    for (ptr = ifInfoTable_head, prev = NULL;
         ptr != NULL; prev = ptr, ptr = ptr->next) 
    {
    	if(!strncmp(ptr->ifName,ifName,strlen(ifName)))
    	return ptr->ifID;
    }
    return -1;
}
long ifInfoTable_getNum(void)
{
    struct ifInfoTable_entry *ptr, *prev;
    int i;
    for (i = 0,ptr = ifInfoTable_head, prev = NULL;
         ptr != NULL; prev = ptr, ptr = ptr->next,i++);

    return i;
}

void ifInfoTable_removeEntry(struct ifInfoTable_entry *entry)
{
    struct ifInfoTable_entry *ptr, *prev;
    if (!entry)
        return;                 /* Nothing to remove */

    for (ptr = ifInfoTable_head, prev = NULL;
         ptr != NULL; prev = ptr, ptr = ptr->next) {
        if (ptr == entry)
            break;
    }
    if (!ptr)
        return;                 /* Can't find it */

    if (prev == NULL)
        ifInfoTable_head = ptr->next;
    else
        prev->next = ptr->next;

    SNMP_FREE(entry);           /* XXX - release any other internal resources */
}
void set_ip(struct ifInfoTable_entry *pdata)
{
	char cmd[128];
	char ip[32];
	memset(cmd,0,128);
	struct in_addr addr;
	addr.s_addr = pdata->ifIP;

	sprintf(cmd,"ifconfig %s %s",pdata->ifName,(char *)inet_ntoa(addr));
	system(cmd);
}
int get_if_info(void)
{
    int fd;
    int interfaceNum = 0,i = 1;
    struct ifreq buf[MAX_IFNUM] = {0};
    struct ifconf ifc;
    struct ifreq ifrcopy;
	struct ifInfoTable_entry data;

	memset(&data,0,sizeof(data));
	
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
    	syslog(LOG_ERR,"syscket error:%s\n",strerror(errno));
        close(fd);
        return -1;
    }

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = (caddr_t)buf;
    if (!ioctl(fd, SIOCGIFCONF, (char *)&ifc))
    {
        interfaceNum = ifc.ifc_len / sizeof(struct ifreq);
        while (interfaceNum-- > 0)
        {
			data.ifID = i;
			strncpy(data.ifName,buf[interfaceNum].ifr_name,strlen(buf[interfaceNum].ifr_name));
			data.ifName_len = strlen(buf[interfaceNum].ifr_name);
            ifrcopy = buf[interfaceNum];
            if (ioctl(fd, SIOCGIFFLAGS, &ifrcopy))
            {
				syslog(LOG_ERR,"ioctl SIOCGIFFLAGS error:%s\n",strerror(errno));
                close(fd);
                return -1;
            }

            //get the mac of this interface  
            if (!ioctl(fd, SIOCGIFHWADDR, (char *)(&buf[interfaceNum])))
            {
            	memcpy(data.ifMAC,buf[interfaceNum].ifr_hwaddr.sa_data,6);
            	data.ifMAC_len = 6;
            }
            else
            {
				syslog(LOG_ERR,"ioctl SIOCGIFHWADDR error:%s\n",strerror(errno));
                close(fd);
                return -1;
            }

            //get the IP of this interface  

            if (!ioctl(fd, SIOCGIFADDR, (char *)&buf[interfaceNum]))
            {
            	data.ifIP = (in_addr_t)((struct sockaddr_in *)&(buf[interfaceNum].ifr_addr))->sin_addr.s_addr;
            }
            else
            {
				syslog(LOG_ERR,"ioctl SIOCGIFADDR error:%s\n",strerror(errno));
                close(fd);
                return -1;
            }
            
			ifInfoTable_createEntry(&data);
			i++;
        }
    }
    else
    {
		syslog(LOG_ERR,"ioctl SIOCGIFCONF error:%s\n",strerror(errno));
        close(fd);
        return -1;
    }
  
    close(fd);

    return 0;
}
void init_ifInfoTable_data(void)
{
	get_if_info();	
}
/** Initializes the ifInfoTable module */
void init_ifInfoTable(void)
{
	init_ifInfoTable_data();
    initialize_table_ifInfoTable();
}

void initialize_table_ifInfoTable(void)
{
    const oid       ifInfoTable_oid[] ={IFINFO_TABLE_OID};
    const size_t    ifInfoTable_oid_len = OID_LENGTH(ifInfoTable_oid);
    netsnmp_handler_registration *reg;
    netsnmp_iterator_info *iinfo;
    netsnmp_table_registration_info *table_info;

    DEBUGMSGTL(("ifInfoTable:init", "initializing table ifInfoTable\n"));

    reg = netsnmp_create_handler_registration("ifInfoTable",
                                            ifInfoTable_handler,
                                            ifInfoTable_oid,
                                            ifInfoTable_oid_len,
                                            HANDLER_CAN_RWRITE);

    table_info = SNMP_MALLOC_TYPEDEF(netsnmp_table_registration_info);
    netsnmp_table_helper_add_indexes(table_info, ASN_INTEGER,   /* index: ifID */
                                     0);
    table_info->min_column = COLUMN_IFID;
    table_info->max_column = COLUMN_IFMAC;

    iinfo = SNMP_MALLOC_TYPEDEF(netsnmp_iterator_info);
    iinfo->get_first_data_point = ifInfoTable_get_first_data_point;
    iinfo->get_next_data_point = ifInfoTable_get_next_data_point;
    iinfo->table_reginfo = table_info;

    netsnmp_register_table_iterator(reg, iinfo);
}


netsnmp_variable_list *ifInfoTable_get_first_data_point(void **my_loop_context,
                                 void **my_data_context,
                                 netsnmp_variable_list * put_index_data,
                                 netsnmp_iterator_info *mydata)
{
    *my_loop_context = ifInfoTable_head;
    return ifInfoTable_get_next_data_point(my_loop_context,
                                           my_data_context, put_index_data,
                                           mydata);
}

netsnmp_variable_list *ifInfoTable_get_next_data_point(void **my_loop_context,
                                void **my_data_context,
                                netsnmp_variable_list * put_index_data,
                                netsnmp_iterator_info *mydata)
{
    struct ifInfoTable_entry *entry =
        (struct ifInfoTable_entry *) *my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if (entry) {
        snmp_set_var_typed_integer(idx, ASN_INTEGER, entry->ifID);
        idx = idx->next_variable;
        *my_data_context = (void *) entry;
        *my_loop_context = (void *) entry->next;
        return put_index_data;
    } else {
        return NULL;
    }
}


/** handles requests for the ifInfoTable table */
int ifInfoTable_handler(netsnmp_mib_handler *handler,
                    netsnmp_handler_registration *reginfo,
                    netsnmp_agent_request_info *reqinfo,
                    netsnmp_request_info *requests)
{

    netsnmp_request_info *request;
    netsnmp_table_request_info *table_info;
    struct ifInfoTable_entry *table_entry;
	int ret;
	
    DEBUGMSGTL(("ifInfoTable:handler", "Processing request (%d)\n",
                reqinfo->mode));

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request = requests; request; request = request->next) {
            table_entry = (struct ifInfoTable_entry *)
                netsnmp_extract_iterator_context(request);
            table_info = netsnmp_extract_table_info(request);

            switch (table_info->colnum) {
            case COLUMN_IFID:
                if (!table_entry) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer(request->requestvb, ASN_INTEGER,
                                           table_entry->ifID);
                break;
            case COLUMN_IFNAME:
                if (!table_entry) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_value(request->requestvb, ASN_OCTET_STR,
                                         table_entry->ifName,
                                         table_entry->ifName_len);
                break;
            case COLUMN_IFIP:
                if (!table_entry) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer(request->requestvb,
                                           ASN_IPADDRESS,
                                           table_entry->ifIP);
                break;
            case COLUMN_IFMAC:
                if (!table_entry) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_value(request->requestvb, ASN_OCTET_STR,
                                         table_entry->ifMAC,
                                         table_entry->ifMAC_len);
                break;
            default:
                netsnmp_set_request_error(reqinfo, request,
                                          SNMP_NOSUCHOBJECT);
                break;
            }
        }
        break;

        /*
         * Write-support
         */
    case MODE_SET_RESERVE1:
        for (request = requests; request; request = request->next) {
            table_entry = (struct ifInfoTable_entry *)
                netsnmp_extract_iterator_context(request);
            table_info = netsnmp_extract_table_info(request);

            switch (table_info->colnum) {
            case COLUMN_IFIP:
                ret = netsnmp_check_vb_int(request->requestvb);
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
            table_entry = (struct ifInfoTable_entry *)
                netsnmp_extract_iterator_context(request);
            table_info = netsnmp_extract_table_info(request);

            switch (table_info->colnum) {
            case COLUMN_IFIP:
                table_entry->old_ifIP = table_entry->ifIP;
                table_entry->ifIP = *request->requestvb->val.integer;
                set_ip(table_entry);
                break;
            }
        }
        break;

    case MODE_SET_UNDO:
        for (request = requests; request; request = request->next) {
            table_entry = (struct ifInfoTable_entry *)
                netsnmp_extract_iterator_context(request);
            table_info = netsnmp_extract_table_info(request);

            switch (table_info->colnum) {
            case COLUMN_IFIP:
                table_entry->ifIP = table_entry->old_ifIP;
                table_entry->old_ifIP = 0;                
                break;
            }
        }
        break;

    case MODE_SET_COMMIT: 
        break;
    }
    return SNMP_ERR_NOERROR;
}
