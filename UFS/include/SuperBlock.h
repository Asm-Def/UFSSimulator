#ifndef SUPERBLOCK_H_INCLUDE
#define SUPERBLOCK_H_INCLUDE

#include"UFSParams.h"
#include<cstring>
using namespace std;
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