#include "../include/INode.h"
#include "../include/FileSystem.h"

INode::INode()
{

}
INode::INode(fmode_t Type, uid_t uid)
{
	if(Type == FILE_TYPE_DIR)
	{
		this->mode = Type | FILE_OWNER_R | FILE_OWNER_W | FILE_OWNER_X | FILE_OTHER_R | FILE_OTHER_X;
	}
	else if(Type == FILE_TYPE_FILE)
	{
		this->mode = Type | FILE_OWNER_R | FILE_OWNER_W | FILE_OTHER_R;
	}
	else
	{
		this->mode = Type | FILE_OWNER_R | FILE_OWNER_W | FILE_OWNER_X | FILE_OTHER_R | FILE_OTHER_X;
	}
	owner = uid;
	this->blocks = -1;
	this->rem_bytes = BLOCK_SIZE;
	this->lcnt = 1;
	this->atime = this->mtime = time(NULL);
	this->indirect1 = this->indirect2 = 0;
}
diskaddr_t INode::size()
{
	return (diskaddr_t) blocks * BLOCK_SIZE + rem_bytes;
}
diskaddr_t INodeMem::size()
{
	INode &inode = FS->AccessINode(BlockID, Location);
	return inode.size();
}
INode *INodeMem::getINode() // get a INode pointer
{
	//if(inode == NULL) inode = new INode();
	INode *inode = &FS->AccessINode(BlockID, Location);
	return inode;
}