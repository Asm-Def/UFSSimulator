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
	disksize_t getSize() { return size; }
	bool Format(); // 格式化当前的VHD
	bool Create(disksize_t sz, std::string name);
	bool Load(std::string vhdname);
	bool ReadBlock(void *buff, bid_t blockID, bit_t begin = 0, int len = BLOCK_SIZE);
	bool WriteBlock(void *buff, bid_t blockID, bit_t begin = 0, int len = BLOCK_SIZE);
	bool FreeBlock(bid_t blockID); // 释放blockID
	bool AllocBlock(bid_t lastBlock, bid_t &newBlock); // 分配空闲块存入newBlock，尽可能接近lastBlock(局部性)
	//以上两个函数一次只读写一个Block
};
#endif
