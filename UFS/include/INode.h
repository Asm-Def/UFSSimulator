#ifndef INODE_H
#define INODE_H

#include "UFSParams.h"

struct INode // Stored in disk
{
	fmode_t mode; // (file type) (owner_rwx_flags) (other_rwx_flags)
	unsigned lcnt; // hard link count, default=1
	uid_t owner; // owner user id
	unsigned blocks; // full block count
	bit_t rem_bytes; // remain bytes
	time_t atime; // last access time
	time_t mtime; // last modify time

	bid_t direct_data[INODE_DATASIZE]; // direct datablocks
	bid_t indirect1, indirect2;
	// max block count = INODE_DATASIZE + sum of (BLOCKSIZE/sizeof(bid_t))^i

	INode()
	{
		
	}
};

#endif
