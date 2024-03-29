#ifndef INODE_H
#define INODE_H

struct INode;
struct INodeMem;
#include "UFSParams.h"
#include <ctime>

struct INode // Stored in disk	size <= 128
{
	fmode_t mode; // (file type) (owner_rwx_flags) (other_rwx_flags)
	unsigned lcnt; // hard link count, default=1
	uid_t owner; // owner user id
	// full block count (default=-1)
	int blocks;
	// remain bytes (default = BLOCK_SIZE)
	bit_t rem_bytes; // remain bytes
	time_t atime; // last access time
	time_t mtime; // last modify time
	diskaddr_t size();

	INode();
	INode(fmode_t Type, uid_t uid, struct FileSystem *FS);
	bool checkR(uid_t t);
	bool checkW(uid_t t);
	bool checkX(uid_t t);
	bool isDir() { return (mode & FILE_TYPE_MASK) == FILE_TYPE_DIR; }
	bool isFile() { return (mode & FILE_TYPE_MASK) == FILE_TYPE_FILE; }
	bool isLink() { return (mode & FILE_TYPE_MASK) == FILE_TYPE_LINK; }
	bid_t direct_data[INODE_DIRECT_SIZE]; // direct datablocks
	bid_t indirect1, indirect2;
	// max block count = INODE_DATASIZE + sum of (BLOCKSIZE/sizeof(bid_t))^i
};

struct INodeMem // Stored in memory
{
	bid_t BlockID;
	bit_t Location; // location of INode in the block
	struct FileSystem *FS;
	
	diskaddr_t size();
	// get a INode pointer
	INode *getINode();
	INodeMem(bid_t blockID, bit_t loc, struct FileSystem *fs) : BlockID(blockID), Location(loc), FS(fs) {}
	~INodeMem()
	{
	}
};

#endif
