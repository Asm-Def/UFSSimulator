#ifndef OPENEDFILE_H
#define OPENEDFILE_H

struct OpenedFile;
#include "FileSystem.h"
#include "UFSParams.h"

struct OpenedFile
{
	FileSystem *FS;
	FileDir *curDir;
	diskaddr_t cur; // current pointer in the file
	fauth_t auth; // 0-7
	OpenedFile(FileSystem *fs, FileDir *dir, fauth_t au)
		: FS(fs), curDir(dir), cur(0), auth(au) { }

	diskoff_t lseek(diskoff_t offset, int fromwhere);
	ssize_t read(void *buf, size_t count);
	ssize_t write(void *buf, size_t count);
};

#endif