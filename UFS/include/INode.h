#ifndef INODE_H
#define INODE_H

struct INode;
struct INodeMem;
#include "UFSParams.h"
#include "FileSystem.h"
#include <ctime>

struct INode // Stored in disk	size <= 128
{
	fmode_t mode; // (file type) (owner_rwx_flags) (other_rwx_flags)
	unsigned lcnt; // hard link count, default=1
	uid_t owner; // owner user id
	unsigned blocks; // full block count
	bit_t rem_bytes; // remain bytes
	time_t atime; // last access time
	time_t mtime; // last modify time

	bid_t direct_data[INODE_DIRECT_SIZE]; // direct datablocks
	bid_t indirect1, indirect2;
	// max block count = INODE_DATASIZE + sum of (BLOCKSIZE/sizeof(bid_t))^i

	INode();
	diskaddr_t size() const;
	bid_t GetBlock(bid_t number); //TODO
	bid_t AppendBlock();
	bool PopBlock();
};

struct INodeMem : INode // Stored in memory
{
	bid_t BlockID;
	bit_t Location; // location of INode in the block
	FileSystem *FS;

	INodeMem();//TODO
};

#endif
