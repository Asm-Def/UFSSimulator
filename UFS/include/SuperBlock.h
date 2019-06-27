#ifndef SUPERBLOCK_H_INCLUDE
#define SUPERBLOCK_H_INCLUDE

#include"UFSParams.h"
#include<cstring>
using namespace std;
struct InfoBlock
{
	disksize_t size;
	bid_t blockNumber;
	bid_t nxtINodeBlockID;
	bit_t nxtINodeLocation;
	uid_t user_cnt;
	InfoBlock(disksize_t sz = 2, bid_t nxtblock = ROOT_BLOCK_ID, bit_t nxtinode = 1, uid_t uc = 1)
		: size(sz), nxtINodeBlockID(nxtblock), nxtINodeLocation(nxtinode), user_cnt(uc)
	{
		blockNumber = (size+BLOCK_SIZE-1) / BLOCK_SIZE;
	}
};
struct SuperBlock
{
	bit_t cnt;       //the number of free blocks
	bid_t freeStack[GROUP_SIZE];          //free block stack
	SuperBlock() : cnt(0) {}   //the initialization of superblock

	//初始化第一个组长块
	void SetFullFlag()
	{
		cnt = 1;
		freeStack[0] = 0;
	}
};
#endif 