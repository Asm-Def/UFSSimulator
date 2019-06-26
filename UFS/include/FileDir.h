#ifndef FILEDIR_H
#define FILEDIR_H
struct FileDir;
#include "UFSParams.h"
#include "INode.h"
#include <string>

struct FileDir
{
	std::string name;
	INodeMem curINode;
	FileDir *parent; // =NULL when curINode representing '/'
	FileDir(std::string nm, INodeMem inode) : name(nm), curINode(inode), parent(NULL)
	{}
	~FileDir(){}
};

#endif
