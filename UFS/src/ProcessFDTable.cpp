#include "ProcessFDTable.h"

ProcessFDTable::ProcessFDTable(FileSystem *fs, ufspid_t p, uid_t u)
{
	FS = fs;
	pid = p;
	uid = u;
}
int ProcessFDTable::open(FileDir *fileDir)
{
	int fd = PFileList.size();
	INodeMem *inode = fileDir->curINode;
	fauth_t auth;
	if(uid == USER_ROOT_UID) auth = 7;
	else if(inode->owner == uid)
		auth = ((inode->mode & (7 << 3)) >> 3);
	else auth = (inode->mode & 7);
	PFileList.emplace_back(FS, fileDir, auth);
	return fd;
}
void ProcessFDTable::close(int fd) // recycle fd
{

}