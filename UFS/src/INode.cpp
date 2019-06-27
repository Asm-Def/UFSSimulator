#include "../include/INode.h"
#include "../include/FileSystem.h"

INode::INode()
{

}
INode::INode(fmode_t Type, uid_t uid, FileSystem *FS)
{
	owner = uid;
	this->blocks = -1;
	this->rem_bytes = BLOCK_SIZE;
	this->lcnt = 1;
	this->atime = this->mtime = time(NULL);
	this->indirect1 = this->indirect2 = 0;
	if(Type == FILE_TYPE_DIR)
	{
		unsigned cnt = 0;
		INode &inode = *this;
		FS->WriteFile(inode, (char*) &cnt, 0, sizeof(cnt), uid);
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
}
diskaddr_t INode::size()
{
	if(blocks == -1) return 0;
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
bool INode::checkR(uid_t t)
{
	return t == owner ? (mode & FILE_OWNER_R) : (mode & FILE_OTHER_R);
}
bool INode::checkW(uid_t t)
{
	return t == owner ? (mode & FILE_OWNER_W) : (mode & FILE_OTHER_W);
}
bool INode::checkX(uid_t t)
{
	return t == owner ? (mode & FILE_OWNER_X) : (mode & FILE_OTHER_X);
}