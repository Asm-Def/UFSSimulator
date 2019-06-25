#ifndef VHDCONTROLLER_H
#define VHDCONTROLLER_H

class VHDController;

#include "UFSParams.h"
#include "SuperBlock.h" 
#include <string>
#include <fstream>
#ifdef DEBUG
#include <iostream>
#endif
class VHDController
{
	disksize_t size;
	std::string VHDname;           //VHD name 
	fstream  _file;
	SuperBlock superBlock;
	// TODO: 柱面数、扇区数等具体参数
	// TODO: 页面缓存机制
	void saveSuperBlock()
	{
		#ifdef DEBUG
		//cout << "cnt:" << superBlock.cnt << " ";
		//for(int i = 0;i < GROUP_SIZE;++i) std::cout << superBlock.freeStack[i] << " "; std::cout << endl;
		#endif
		WriteBlock((char *) &superBlock, SUPER_BLOCK_ID);
	}
	void loadSuperBlock()
	{
		ReadBlock((char *) &superBlock, SUPER_BLOCK_ID);
	}
	public:
	VHDController() : _file(), size(0), VHDname("") {}
	~VHDController() {};
	disksize_t getSize() { return size; }
	string getVHDname(){  return VHDname;}
	bool Format();     //格式化当前VHD 
	void setVHDname(string name){ this->VHDname=name; }
	
	bool Create(disksize_t sz, std::string name);         //Init a Virtual HD 
	bool Exists();    // Check the exitence of VHD
	bool Load(std::string vhdname);    //Load VHD  
	bool ReadBlock(char *buff, bid_t blockID , bit_t begin = 0 , int len = BLOCK_SIZE);
	bool WriteBlock(char *buff, bid_t blockID , bit_t begin = 0 ,  int len = BLOCK_SIZE);
	//以上两个函数一次只读写一个Block
	bool FreeBlock(bid_t blockID);      //回收空闲块 
	bool AllocBlock(bid_t &newblock);    //分配空闲块
};


#endif