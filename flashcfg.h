/*  define the IO_ZONE to operating */
#define  RAID_CONFIG        0
#define  RAID_USERLIST     1
#define  RAID_ETH_ZERO    2
#define  RAID_ETH_ONE      3
#define  RAID_OPERA_LOG  4
#define  RAID_MON_LOG     5
#define  RAID_BLACKLIST    6
#define  RAID_CONFIG_BACKUP    7

/* define the flash offset and size */
#define  RAID_CONFIG_OFFSET        0x1D00000
#define  RAID_USERLIST_OFFSET     0x1D20000
#define  RAID_ETH_ZERO_OFFSET    0x1D40000
#define  RAID_ETH_ONE_OFFSET      0x1D60000
#define  RAID_OPERA_LOG_OFFSET  0x1D80000
#define  RAID_MON_LOG_OFFSET     0x1DA0000
#define  RAID_BLACKLIST_OFFSET    0x1DC0000
#define  RAID_CONFIG_BACKUP_OFFSET    0x1DE0000

#define  RAID_RESERVED_OFFSET    0x1E00000


#define FLASH_CONFIG_ZONE_SIZE 64*1024     //flash config zone must be a full sector

/*define the directory written to*/
#define BLACKLIST_DIR_READ 		 "/usr/local/apache/htdocs/RaidManager/Monitor/blacklist.txt"
#define RAIDCONFIG_DIR_READ 	 "/usr/local/apache/htdocs/RaidManager/Monitor/raidConfig.xml"
#define RAIDCONFIG_DIR_READ_BACKUP 	 "/usr/local/apache/htdocs/RaidManager/Monitor/raidConfig_backup.xml"
#define IFCFG_ETH0_DIR_READ 	 "/usr/local/apache/htdocs/RaidManager/Monitor/ifcfg-eth0"
#define IFCFG_ETH1_DIR_READ	  "/usr/local/apache/htdocs/RaidManager/Monitor/ifcfg-eth1"
#define LOG_DIR_READ			  "/usr/local/apache/htdocs/RaidManager/Monitor/log"
#define OPERATION_LOG_DIR_READ 	 "/usr/local/apache/htdocs/RaidManager/Monitor/operation_log"
#define USERS_DIR_READ 			 "/usr/local/apache/htdocs/RaidManager/Monitor/users.txt"

/*define the directory read from*/
#define BLACKLIST_DIR_WRITE 	 "/usr/local/apache/htdocs/RaidManager/logon/blacklist.txt"
#define RAIDCONFIG_DIR_WRITE 	 "/usr/local/apache/htdocs/RaidManager/Monitor/raidConfig.xml"
#define RAIDCONFIG_DIR_WRITE_BACKUP 	 "/usr/local/apache/htdocs/RaidManager/Monitor/raidConfig_backup.xml"
#define IFCFG_ETH0_DIR_WRITE	  "/etc/sysconfig/network-scripts/ifcfg-eth0"
#define IFCFG_ETH1_DIR_WRITE	  "/etc/sysconfig/network-scripts/ifcfg-eth1"
#define LOG_DIR_WRITE				  "/usr/local/apache/htdocs/RaidManager/Monitor/log"
#define OPERATION_LOG_DIR_WRITE  	"/usr/local/apache/htdocs/RaidManager/log/log"
#define USERS_DIR_WRITE			  "/usr/local/apache/htdocs/RaidManager/users.txt"

