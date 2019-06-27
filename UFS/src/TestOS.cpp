#include "TestOS.h"
diskoff_t Session::lseek(int fd, diskoff_t offset, int fromwhere)
{
	if(fd < 0 || fd >= processFDTable.MaxFD()) return -1;
	OpenedFile &file = processFDTable.getFile(fd);
	return file.lseek(offset, fromwhere);
}
ssize_t Session::read(int fd, void *buf, size_t count)
{
	if(fd < 0 || fd >= processFDTable.MaxFD()) return -1;
	OpenedFile &file = processFDTable.getFile(fd);
	if(!(file.auth & FILE_OTHER_R)) return -1;
	return file.read(buf, count,uid);
}
ssize_t Session::write(int fd, void *buf, size_t count)
{
	if(fd < 0 || fd >= processFDTable.MaxFD()) return -1;
	OpenedFile &file = processFDTable.getFile(fd);
	if(!(file.auth & FILE_OTHER_W)) return -1;
	return file.write(buf, count, uid);
}
void Session::Interact()
{
	// TODO
}