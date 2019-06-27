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
	void loadInfoBlock()
	{
		ReadBlock((char *) &info, INFO_BLOCK_ID, 0, sizeof(InfoBlock));
	}
	public:
	void saveInfoBlock()
	{
		WriteBlock((char *) &info, INFO_BLOCK_ID, 0, sizeof(InfoBlock));
	}
	InfoBlock info;
	VHDController() : _file(), VHDname(""), info() {}
	~VHDController()
	{
		if(_file.is_open()) Save();
		_file.close();
	}
	disksize_t getSize() { return info.size; }
	string getVHDname(){  return VHDname;}
	bool Format();     //格式化当前VHD 
	void setVHDname(string name){ this->VHDname=name; }
	
	bool Create(disksize_t sz, std::string name);         //Init a Virtual HD 
	bool Exists();    // Check the exitence of VHD
	bool Load(std::string vhdname);    //Load VHD  
	bool Save(); // Save current VHD;
	bool ReadBlock(char *buff, bid_t blockID , bit_t begin = 0 , int len = BLOCK_SIZE);
	bool WriteBlock(const char *buff, bid_t blockID , bit_t begin = 0 ,  int len = BLOCK_SIZE);
	//以上两个函数一次只读写一个Block
	bool FreeBlock(bid_t blockID);      //回收空闲块 
	bool AllocBlock(bid_t &newblock);    //分配空闲块
};


#endif