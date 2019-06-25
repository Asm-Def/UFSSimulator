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
	VHDController vhd; // 模拟一块磁盘
	FileDir *rootDir; // 指向根目录的FileDir
	public:
	FileSystem();
	~FileSystem();
	bool CreateVHD(disksize_t sz, std::string name);
	bool LoadVHD(std::string vhdname);
	bool FormatVHD(); // 重建文件系统(同时创建根目录)

	/**
	 * 列出curDir表示的目录下的所有FileDir
	 * 过程：FileDir(当前目录) => INodeMem(目录对应的INode) => 根据寻址表将该目录下的所有FileDir及它们对应的INodeMem读入内存 => 返回结果
	 */
	std::vector<FileDir*> ListDir(FileDir *curDir);
	FileDir *FindDir(FileDir *cur, std::string dir);  // find director from cur by dir[]. return NULL if no exist
	bool MakeDir(FileDir *curDir, std::string fname, uid_t uid); // create director {fname} if not exist (新创建的FileDir内容中应包含"."和".."两个FileDir，它们的INode地址分别为当前目录本身的INode和上级目录的INode)
	bool Touch(FileDir *curDir, std::string fname, uid_t uid); // create fild {fname} if not exist
	bool Remove(FileDir *curDir, std::string fname, uid_t uid); // remove an INode and its data
	bool MakeHardLink(FileDir *curDir, std::string Dest, std::string Src, uid_t uid);
	bool MakeSoftLink(FileDir *curDir, std::string Dest, std::string Src, uid_t uid);
};

#endif
