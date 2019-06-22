#ifndef VHDCONTROLLER_H
#define VHDCONTROLLER_H
class VHDController;
#include "UFSParams.h"
#include <string>
class VHDController
{
	disksize_t size;
	std::string VHDname;
	FILE *_file;
	// TODO: 柱面数、扇区数等具体参数
	// TODO: 页面缓存机制
	public:
	VHDController();
	~VHDController();
	int getSize() { return size; }
	bool Create(disksize_t sz, std::string name);
	bool Load(std::string vhdname);
	bool ReadBlock(void *buff, bid_t blockID, bit_t begin = 0, int len = BLOCK_SIZE);
	bool WriteBlock(void *buff, bid_t blockID, bit_t begin = 0, int len = BLOCK_SIZE);
	//以上两个函数一次只读写一个Block
};
#endif
