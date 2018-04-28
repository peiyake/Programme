#ifndef __PUBLIC_H__
#define __PUBLIC_H__

typedef struct mib_node_info
{
	const char *nodename;
	const oid *nodeoid;
	int	oid_len;
	unsigned short permission;
}mib_node_info_t;

struct ifInfoTable_entry {
    /*
     * Index values 
     */
    long            ifID;

    /*
     * Column values 
     */
    char            ifName[32];
    size_t          ifName_len;
    in_addr_t       ifIP;
    in_addr_t       old_ifIP;
    char            ifMAC[6];
    size_t          ifMAC_len;

    /*
     * Illustrate using a simple linked list 
     */
    int             valid;
    struct ifInfoTable_entry *next;
};
struct bridgeTable_entry {
    /*
     * Index values 
     */
    long            brID;
    long			ifID;

    /*
     * Column values 
     */
    char            brName[32];
    size_t          brName_len;
    char            ifName[32];
    size_t          ifName_len;
    /*
     * Illustrate using a simple linked list 
     */
    int             valid;
    struct bridgeTable_entry *next;
};


extern void init_ifInfoTable(void);
extern void init_devinfo(void);
extern void init_bridgeTable(void);
extern void sprint_mib_oid(char *buf, const oid *name, size_t len);

#endif
