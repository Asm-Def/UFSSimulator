#include "../include/INode.h"
#include "../include/FileSystem.h"

diskaddr_t INodeMem::size()
{
	INode &inode = FS->AccessINode(BlockID, Location);
	return (diskaddr_t) inode.blocks * BLOCK_SIZE + inode.rem_bytes;
}
INode *INodeMem::getINode() // get a INode pointer
{
	//if(inode == NULL) inode = new INode();
	INode *inode = &FS->AccessINode(BlockID, Location);
	return inode;
}