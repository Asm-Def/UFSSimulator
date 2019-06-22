#ifndef VHDCONTROLLER_H
#define VHDCONTROLLER_H

#include "UFSParams.h"
#include <string>
class VHDController
{
	disksize_t size;
	std::string VHDname;
	FILE *_file;
	// TODO: 柱面数、扇区数等具体参数
	public:
	VHDController();
	~vHDController();
	int getSize() { return size; }
	bool Create(disksize_t sz, std::string name);
	bool Load(std::string vhdname);
	bool Save();
	bool ReadBlock(bid_t blockID, char buff[], int len = BLOCKSIZE);
	bool WriteBlock(bid_t blockID, char buff[], int len = BLOCK_SIZE);
};
#endif
