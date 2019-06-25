#include"../include/UFSParams.h"
#include"../include/VHDController.h"
#include"../include/SuperBlock.h"
#include<iostream>
#include<cstring>

using namespace std;

bool VHDController::Format()
{
	if(!_file.is_open()) throw "no VHD mounted";
	bid_t number = (this->size+BLOCK_SIZE-1) / BLOCK_SIZE;
	#ifdef DEBUG
	//cout << "number = " << number << endl;
	#endif
	superBlock.SetFullFlag();
	for(bid_t i = number-1;i > 0;--i)
	{
		if(!FreeBlock(i)) throw "Cannot Free Block " + i;
	}
	saveSuperBlock();
	return true;
}

// Create a new VHD without opening or formatting
bool VHDController::Create(disksize_t sz,string name) 
{
	setVHDname(name);
	this->size = sz;
	_file.open(this->VHDname.c_str(),ios::in|ios::out|ios::binary);    //open from disk
	if(!_file) return false;
	return true;
}

bool VHDController::Exists()
{
	ifstream file(this->VHDname.c_str());  //open from disk 
	return file.good();
}

bool VHDController::Load(string vhdname)
{
	VHDname = vhdname;
	_file.open(vhdname, ios::in | ios::out | ios::binary);
	if(!ReadBlock((char *) &superBlock, SUPER_BLOCK_ID, 0, sizeof(SuperBlock)))
	{
		return false;
	}
	return true;
}

bool VHDController::ReadBlock(char *buff, bid_t blockID , bit_t begin ,  int len )
{
	if(!_file.is_open()) throw string(__FUNCTION__) + ":" + "no VHD mounted";
	disksize_t number = this->size / BLOCK_SIZE;
	if ( blockID < number && len+begin <= BLOCK_SIZE )
	{
		this->_file.seekg((disksize_t) blockID * BLOCK_SIZE+begin, ios::beg);      //the position of block = blockID *BLOCKSIZE+begin
		this->_file.read(buff,len);     //read the content of block
		if(!this->_file)
			return false;
		else
			return true;
	}
	else throw "ReadBlock parameters error";
}
bool VHDController::WriteBlock(char *buff, bid_t blockID , bit_t begin ,  int len )
{
	if(!_file.is_open()) throw string(__FUNCTION__) + ":" + "no VHD mounted";
	disksize_t number = this->size / BLOCK_SIZE;
	if( blockID < number && begin+len <= BLOCK_SIZE)
	{
		this->_file.seekp((disksize_t) blockID * BLOCK_SIZE+begin, ios::beg);
		this->_file.write(buff,len);    //write the content of buff into disk 
		if(!this->_file)
		    return false;
		else
		    return true;
	}
	else throw "WriteBlock parameters error";
}

bool VHDController::FreeBlock(bid_t blockID)
{
	if(!_file.is_open()) throw "no VHD mounted";
	if(this->superBlock.cnt == GROUP_SIZE)      //   the memeory of superblock is full
	{
		if(!this->WriteBlock((char *)&this->superBlock , blockID , 0 , sizeof(SuperBlock))){
			throw "Write error";
			return false;
		}
		//for(int i = 0;i < GROUP_SIZE;++i) printf("%lld ", superBlock.freeStack[i]);puts("");
		memset((char *)&this->superBlock,0,sizeof(SuperBlock));
		this->superBlock.cnt = 1;
	} 
	else         //not full
		this->superBlock.cnt++;
	this->superBlock.freeStack[this->superBlock.cnt-1] = blockID;
	saveSuperBlock();
	return true;
}

bool VHDController::AllocBlock(bid_t &newblock)
{
	if(!_file.is_open()) throw "no VHD mounted";
	if(this->superBlock.freeStack[this->superBlock.cnt-1] == 0) return false;      //the memeory is empty
	newblock = this->superBlock.freeStack[this->superBlock.cnt-1]; 
	if(this->superBlock.cnt == 1)
	{
		if(!this->ReadBlock((char *)&this->superBlock , newblock , 0 , sizeof(SuperBlock)))
		{
			//for(int i = 0;i < GROUP_SIZE;++i) printf("%lld ", superBlock.freeStack[i]);puts("");
			throw string("Fail to ReadBlock at ") + __FILE__ + ":" + to_string(__LINE__);
			return false;
		}
	}
	else this->superBlock.cnt--;
	saveSuperBlock();
	return true;
}
