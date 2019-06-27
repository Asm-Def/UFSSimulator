#include "OpenedFile.h"
diskoff_t OpenedFile::lseek(diskoff_t offset, int fromwhere)
{
	if(fromwhere == SEEK_CUR){
		if(cur + offset < 0) return -1;
		cur += offset;
	}
	else if(fromwhere == SEEK_SET){
		if(offset < 0) return -1;
		cur = offset;
	}
	else
	{
		diskaddr_t sz = curDir->curINode->size();
		if(sz + offset < 0) return -1;
		cur = sz + offset;
	}
	return cur;
}
ssize_t OpenedFile::read(void *buf, size_t count, uid_t uid)
{
	ssize_t sz = FS->ReadFile(this->curDir, (char *) buf, cur, count, uid);
	if(sz < 0) return sz;
	cur += sz;
	return sz;
}
ssize_t OpenedFile::write(void *buf, size_t count, uid_t uid)
{
	ssize_t sz = FS->WriteFile(this->curDir, (char *) buf, cur, count, uid);
	if(sz < 0) return sz;
	cur += sz;
	return sz;
}