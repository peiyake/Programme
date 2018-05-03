#ifndef __MIB_H__
#define __MIB_H__

/*define oid prefix*/
#define ISO_ENTERPRISES		1,3,6,1,4,1
#define ROOT_OID			ISO_ENTERPRISES,170816	
#define EXAMPLE_OID			ROOT_OID,1
#define PCINFO_OID			EXAMPLE_OID,1


#define TRAP_ROOT_OID		PCINFO_OID,1
#define DEVINFO_OID			PCINFO_OID,2
#define IFINFO_OID			PCINFO_OID,3
#define BRIDGE_OID			PCINFO_OID,4

/*trap info oid */
#define TRAP_REPORT_NODE			TRAP_ROOT_OID,1
#define TRAP_INFO_TYPE_OID			TRAP_ROOT_OID,2,1
#define TRAP_INFO_LEVEL_OID			TRAP_ROOT_OID,2,2
#define TRAP_INFO_TIME_OID			TRAP_ROOT_OID,2,3
#define TRAP_INFO_REASON_OID		TRAP_ROOT_OID,2,4
	
/*devinfo scalar node*/
#define DEVINFO_SCALAR_HOSTNAME			DEVINFO_OID,1
#define DEVINFO_SCALAR_SYSTEMTYPE		DEVINFO_OID,2
#define DEVINFO_SCALAR_KERNELVERSION	DEVINFO_OID,3
#define DEVINFO_SCALAR_UPTIME			DEVINFO_OID,4
#define DEVINFO_SCALAR_IPFORWARD		DEVINFO_OID,5
#define DEVINFO_SCALAR_CPU				DEVINFO_OID,6
#define DEVINFO_SCALAR_MEM				DEVINFO_OID,7

/*table of ifinfo*/
#define IFINFO_NUM_OID		IFINFO_OID,1
#define IFINFO_TABLE_OID	IFINFO_OID,2

/*table of bridge */
#define BRIDGE_TABLE_OID	BRIDGE_OID,1



#if 0
/*ifinfo table column*/
#define IFINFO_TABLE_COL_IFID		IFINFO_TABLE_OID,1
#define IFINFO_TABLE_COL_IFNAME		IFINFO_TABLE_OID,2
#define IFINFO_TABLE_COL_IFIP		IFINFO_TABLE_OID,3
#define IFINFO_TABLE_COL_IFMAC		IFINFO_TABLE_OID,4

/*bridge table column*/
#define BRIDGE_TABLE_COL_BRID		BRIDGE_TABLE_OID,1
#define BRIDGE_TABLE_COL_BRIFID		BRIDGE_TABLE_OID,2
#define BRIDGE_TABLE_COL_BRNAME		BRIDGE_TABLE_OID,3
#define BRIDGE_TABLE_COL_BRIFNAME	BRIDGE_TABLE_OID,4

/*===============================================================*/
/*ifinfo table column oid*/
const oid ifinfo_table_col_ifid[] = {IFINFO_TABLE_COL_IFID};
const oid ifinfo_table_col_ifname[] = {IFINFO_TABLE_COL_IFNAME};
const oid ifinfo_table_col_ifip[] = {IFINFO_TABLE_COL_IFIP};
const oid ifinfo_table_col_ifmac[] = {IFINFO_TABLE_COL_IFMAC};

/*bridge table column oid*/
const oid bridge_table_col_brid = {BRIDGE_TABLE_COL_BRID};
const oid bridge_table_col_brifid = {BRIDGE_TABLE_COL_BRIFID};
const oid bridge_table_col_brname = {BRIDGE_TABLE_COL_BRNAME};
const oid bridge_table_col_brifname = {BRIDGE_TABLE_COL_BRIFNAME};
#endif
#endif
