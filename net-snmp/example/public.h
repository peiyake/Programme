#ifndef __PUBLIC_H__
#define __PUBLIC_H__

/*trap type*/
#define TRAP_TYPE_DEVICE		1
#define TRAP_TYPE_PROC			2
/*trap level*/
#define TRAP_LEVEL_WARNNING		1
#define TRAP_LEVEL_ERROR		2
#define TRAP_LEVEL_URGENCY		3

#define IFNAME_SIZE		32
typedef struct trapinfo
{
	int trapType;
	int trapLevel;
	time_t trapTime;
	char trapReason[128];
}trapinfo_t;

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
    char            ifName[IFNAME_SIZE];
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

#if 0	/*for old_api*/
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
#endif
struct bridgeTable_entry {
    /*
     * Index values 
     */
    long            brId;
    long            brIfID;

    long            old_brId;
    long            old_brIfID;
    
    char            brName[IFNAME_SIZE];
    size_t          brName_len;
    char            old_brName[IFNAME_SIZE];
    size_t          old_brName_len;
    char            brIfName[IFNAME_SIZE];
    size_t          brIfName_len;
    char            old_brIfName[IFNAME_SIZE];
    size_t          old_brIfName_len;

    int             valid;
    struct bridgeTable_entry *next;
};

SNMPAlarmCallback send_trap_alarm;

extern void init_ifInfoTable(void);
extern void init_devinfo(void);
extern void init_bridgeTable(void);
extern void sprint_mib_oid(char *buf, const oid *name, size_t len);
extern int send_trap_api(trapinfo_t *inf);

#endif
