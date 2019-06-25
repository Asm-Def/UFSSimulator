#ifndef UFSPARAMS_H
#define UFSPARAMS_H

typedef		unsigned long long	disksize_t;
// Block
typedef		unsigned int	bid_t;
typedef		unsigned short	bit_t;
#ifdef DEBUG
const bit_t				BLOCK_SIZE		= (1 << 5);
#else
const bit_t				BLOCK_SIZE		= (1 << 12);
#endif
const unsigned short	BID_LEN			= sizeof(bit_t);
const unsigned			GROUP_SIZE		= ((BLOCK_SIZE - sizeof(bit_t)) / sizeof(bid_t));
const bid_t				SUPER_BLOCK_ID	= 0;
const bid_t				ROOT_BLOCK_ID	= 1;
// INode
typedef		disksize_t		diskaddr_t;
typedef		long long		diskoff_t;
typedef		unsigned short	fmode_t;
#ifdef DEBUG
const bit_t				INODE_DIRECT_SIZE = (1u << 3);
#else
const bit_t				INODE_DIRECT_SIZE = (1u << 5);
#endif
const disksize_t		INODE_TOT_BCNT1	= (INODE_DIRECT_SIZE+BLOCK_SIZE/sizeof(bid_t));
const disksize_t		INODE_TOT_BCNT2 = (INODE_TOT_BCNT1+(diskaddr_t)BLOCK_SIZE/sizeof(bit_t) * BLOCK_SIZE/sizeof(bit_t));
#define		FILE_OWNER_R	(1 << 5)
#define		FILE_OWNER_X	(1 << 4)
#define		FILE_OWNER_W	(1 << 3)
#define		FILE_OTHER_R	(1 << 2)
#define		FILE_OTHER_W	(1 << 1)
#define		FILE_OTHER_X	(1 << 0)
#define		FILE_TYPE_MASK	(3 << 6)
#define		FILE_TYPE_FILE	(0 << 6)
#define		FILE_TYPE_DIR	(1 << 6)
#define		FILE_TYPE_LINK	(2 << 6)

// User
typedef 	unsigned int	uid_t;
const uid_t				USER_ROOT_UID = 0;
#define		USER_PASSWD		"/etc/shadow"
#define		USER_HOME_DIR	"/home"

// ProcessFDTable
typedef		unsigned int	ufspid_t;
typedef		unsigned char	fauth_t;

#endif
