#include "../include/FileSystem.h"
#include "../include/FileDir.h"
#include "../include/INode.h"

FileSystem::FileSystem() : rootDir(NULL)
{
}
FileSystem::~FileSystem()
{
	if(rootDir == NULL) return;
	delete rootDir;
}
bool FileSystem::AllocINode(bid_t &BlockID, bit_t &Location)
{
	if(vhd.info.nxtINodeLocation == INODE_PER_BLOCK)
	{
		vhd.info.nxtINodeLocation = 0;
		if(!vhd.AllocBlock(vhd.info.nxtINodeBlockID))
		{
			return false;
		}
	}
	BlockID = vhd.info.nxtINodeBlockID;
	Location = vhd.info.nxtINodeLocation++;
	vhd.saveInfoBlock();
	return true;
}

bool FileSystem::CreateVHD(disksize_t sz, std::string name)
{
	vhd.Create(sz, name);
	if(!FormatVHD())
	{
		return false;
	}
	return true;
}
bool FileSystem::LoadVHD(std::string vhdname)
{
	vhd.Load(vhdname);
	if(rootDir) delete rootDir;
	INodeMem inode(ROOT_BLOCK_ID, 0, this);
	ReadINode(inode.getINode(), ROOT_BLOCK_ID, 0);
	rootDir = new FileDir("", inode);
}
bool FileSystem::ReadINode(INode *inode, bid_t BlockID, bit_t Location)
{
	if(inode == NULL) return false;
	vhd.ReadBlock((char *) inode, BlockID, Location * INODE_SIZE, INODE_SIZE);
}
bool FileSystem::WriteINode(INode *inode, bid_t BlockID, bit_t Location)
{
	if(inode == NULL) return false;
	vhd.WriteBlock((char *) inode, BlockID, Location * INODE_SIZE, INODE_SIZE);
}
bool FileSystem::FormatVHD() // 重建文件系统(同时创建根目录)
{
	vhd.Format();
	INodeMem rootINode(ROOT_BLOCK_ID, 0, this);
	ReadINode(rootINode.getINode(), ROOT_BLOCK_ID, 0);

	rootDir = new FileDir("", rootINode);
	return true;
}