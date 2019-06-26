#ifndef UFSPARAMS_H
#define UFSPARAMS_H

#define		Throw(str)		throw std::string(__FILE__) + " " + std::to_string(__LINE__) + ": " + (str)
typedef		unsigned long long	disksize_t;
// Block
typedef		unsigned int	bid_t;
typedef		unsigned short	bit_t;
#ifdef DEBUG
const bit_t				BLOCK_SIZE		= (1 << 7);
#else
const bit_t				BLOCK_SIZE		= (1 << 12);
#endif
const unsigned short	BID_LEN			= sizeof(bid_t);
const unsigned short	BIT_LEN			= sizeof(bit_t);
const unsigned			GROUP_SIZE		= ((BLOCK_SIZE - BIT_LEN) / BID_LEN);
const bid_t				INFO_BLOCK_ID	= 0;
const bid_t				SUPER_BLOCK_ID	= 1;
const bid_t				ROOT_BLOCK_ID	= 2;
// INode
typedef		disksize_t		diskaddr_t;
typedef		long long		diskoff_t;
typedef		unsigned short	fmode_t;
#ifdef DEBUG
const bit_t				INODE_DIRECT_SIZE = (1u << 3);
#else
const bit_t				INODE_DIRECT_SIZE = 23u;
#endif
#define					INODE_SIZE		sizeof(INode)
#define					INODE_PER_BLOCK	(BLOCK_SIZE / INODE_SIZE)
#define					INODE_BCNT1		(BLOCK_SIZE/BID_LEN)
#define					INODE_BCNT2		((diskaddr_t)BLOCK_SIZE/BID_LEN * BLOCK_SIZE/BID_LEN)
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

// FileSystem
#define		INODE_CACHE_CNT	((1 << 18) / INODE_SIZE)

#endif
