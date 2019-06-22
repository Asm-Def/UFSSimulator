#ifndef PROCESSFDTABLE_H
#define PROCESSFDTABLE_H
class ProcessFDTable;
#include "UFSParams.h"
#include "INode.h"
#include <string>
#include <vector>
#include "FileDir.h"
#include "FileSystem.h"
#include "OpenedFile.h"

class ProcessFDTable
{
	FileSystem *FS;
	ufspid_t pid;
	uid_t uid;
	std::vector<OpenedFile> PFileList;
	public:
	int MaxFD() const { return PFileList.size(); }
	OpenedFile &getFile(int fd) { return PFileList[fd]; }

	ProcessFDTable(FileSystem *fs, ufspid_t p, uid_t u);
	int open(FileDir *fileDir);
	void close(int fd); // recycle fd
	
};

#endif
