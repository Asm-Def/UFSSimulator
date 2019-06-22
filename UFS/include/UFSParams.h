#ifndef UFSPARAMS_H
#define UFSPARAMS_H

typedef		unsigned long long	disksize_t;
// Block
#define		BLOCK_SIZE		(1 << 12)
typedef		unsigned int	bid_t;
typedef		unsigned short	bit_t;
#define		BID_LEN			sizeof(bit_t)
// INode
typedef		disksize_t		diskaddr_t;
typedef		long long		diskoff_t;
#define		INODE_DIRECT_SIZE	(1 << 5)
#define		INODE_TOT_BCNT1	(INODE_DIRECT_SIZE+BLOCK_SIZE/sizeof(bid_t))
#define		INODE_TOT_BCNT2 (INODE_TOT_BCNT1+(diskaddr_t)BLOCK_SIZE/sizeof(bit_t) * BLOCK_SIZE/sizeof(bit_t))
typedef		disksize_t		diskaddr_t;
typedef		unsigned short	fmode_t;
#define		FILE_OWNER_R	(1 << 6)
#define		FILE_OWNER_W	(1 << 5)
#define		FILE_OWNER_X	(1 << 4)
#define		FILE_OTHER_R	(1 << 3)
#define		FILE_OTHER_W	(1 << 2)
#define		FILE_OTHER_X	(1 << 1)

#define		FILE_TYPE_MASK	(3 << 7)
#define		FILE_TYPE_FILE	(0 << 7)
#define		FILE_TYPE_DIR	(1 << 7)
#define		FILE_TYPE_LINK	(2 << 7)

// User
typedef 	unsigned int	uid_t;
#define		USER_ROOT_UID	0
#define		USER_PASSWD		"/etc/shadow"
#define		USER_HOME_DIR	"/home"

// ProcessFDTable
typedef		unsigned int	ufspid_t;
typedef		unsigned char	fauth_t;

#endif
