#ifndef FILEDIR_H
#define FILEDIR_H
struct FileDir;
#include "UFSParams.h"
#include "INode.h"
#include <string>
#include <vector>

struct FileDir
{
	std::string name;
	INodeMem curINode;
	std::vector<FileDir*> subDirs;
	FileDir *parent; // =this when curINode representing '/'
	FileDir(std::string nm, INodeMem inode) : name(nm), curINode(inode), parent(this)
	{}
	~FileDir()
	{
		if(name != "." && name != "..") clearDirs();
	}
	void clearDirs()
	{
		INode *inode = curINode.getINode();
		if((inode->mode & FILE_TYPE_MASK)==FILE_TYPE_DIR)
		{
			for(FileDir* &dir : subDirs) delete dir;
		}
		subDirs.clear();
	}
	std::string fulldir()
	{
		if(parent == this) return "";
		return parent->fulldir() + "/" + name;
	}
};

#endif
