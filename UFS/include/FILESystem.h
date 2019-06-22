#ifndef FILESYSTEM_H
#define FILESYSTEM_H
class FileSytem;
#include "FileDir.h"
#include "VHDController.h"
#include "INode.h"
#include "UFSParams.h"
#include <vector>

class FileSystem
{
	VHDController vhd;
	
	// find director from cur by dir[]. return NULL if no exist
	public:
	FileSystem();
	~FileSystem();
	bool CreateVHD(disksize_t sz, std::string name);
	bool LoadVHD(std::string vhdname);

	FileDir *FindDir(FileDir *cur, const char dir[]);
	bool MakeDir(FileDir *curDir, std::string fname, uid_t uid); // create director {fname} if not exist
	bool Touch(FileDir *curDir, std::string fname, uid_t uid); // create fild {fname} if not exist
	bool Remove(FileDir *curDir, std::string fname, uid_t uid); // remove an INode and its data
	std::vector<FileDir*> ListDir(FileDir *curDir);
};

#endif
