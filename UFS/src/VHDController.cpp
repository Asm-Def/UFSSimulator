#include"../include/UFSParams.h"
#include"../include/VHDController.h"
#include"../include/SuperBlock.h"
#include<iostream>
#include<cstring>

using namespace std;

bool VHDController::Format()
{
	if(!_file.is_open()) throw "no VHD mounted";
	bid_t number = info.blockNumber;
	#ifdef DEBUG
	cout << "number = " << number << endl;
	#endif
	superBlock.SetFullFlag();
	for(bid_t i = number-1;i > 2;--i)
	{
		if(!FreeBlock(i)) throw "Cannot Free Block " + i;
	}
	saveSuperBlock();
	return true;
}

// Create a new VHD without formatting
bool VHDController::Create(disksize_t sz,string name) 
{
	if(_file.is_open()) Save();
	setVHDname(name);
	this->info = InfoBlock(sz);
	_file.open(VHDname.c_str(), ios::out | ios::binary);
	_file.seekp(sz - 1, ios::beg);
	char ch = 0;
	_file.write(&ch, 1);
	_file.close();
	_file.open(this->VHDname.c_str(),ios::in|ios::out|ios::binary);    //open from disk
	saveInfoBlock();
	if(!_file) return false;
	return true;
}

bool VHDController::Exists()
{
	ifstream file(this->VHDname.c_str());  //open from disk 
	return file.good();
}

bool VHDController::Save()
{
	if(!_file.is_open()) return false;
	saveInfoBlock();
	saveSuperBlock();
	return true;
}

bool VHDController::Load(string vhdname)
{
	if(_file.is_open()) Save();
	VHDname = vhdname;
	_file.open(vhdname, ios::in | ios::out | ios::binary);
	if(!_file.is_open()) return false;
	loadInfoBlock();
	if(!ReadBlock((char *) &superBlock, SUPER_BLOCK_ID, 0, sizeof(SuperBlock)))
	{
		return false;
	}
	return true;
}

bool VHDController::ReadBlock(char *buff, bid_t blockID , bit_t begin ,  int len )
{
	if(!_file.is_open()) throw string(__FUNCTION__) + ":" + "no VHD mounted";
	if ( blockID < info.blockNumber && len+begin <= BLOCK_SIZE )
	{
		//printf("seekg %lld\n", (disksize_t) blockID * BLOCK_SIZE+begin);
		this->_file.seekg((disksize_t) blockID * BLOCK_SIZE+begin, ios::beg);      //the position of block = blockID *BLOCKSIZE+begin
		this->_file.read(buff,len);     //read the content of block
		if(!this->_file)
			return false;
		else
			return true;
	}
	else
	{
		printf("blockID=%d, len=%d, begin=%d, BLOCKSIZE=%d, number=%d\n", blockID, len, begin, BLOCK_SIZE, info.blockNumber);
		throw "ReadBlock parameters error";
	}
}
bool VHDController::WriteBlock(const char *buff, bid_t blockID , bit_t begin ,  int len )
{
	if(!_file.is_open()) throw string(__FUNCTION__) + ":" + "no VHD mounted";
	if( blockID < info.blockNumber && begin+len <= BLOCK_SIZE)
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
		saveSuperBlock();
	} 
	else         //not full
	{
		this->superBlock.cnt++;
		WriteBlock((char*) &superBlock.cnt, SUPER_BLOCK_ID, (bit_t) ((char*)&superBlock.cnt - (char*)&superBlock), sizeof(bit_t));
	}
	this->superBlock.freeStack[this->superBlock.cnt-1] = blockID;
	WriteBlock((char*) &superBlock.freeStack[this->superBlock.cnt-1], SUPER_BLOCK_ID, (bit_t) ((char*)&superBlock.freeStack[this->superBlock.cnt-1] - (char*)&superBlock), sizeof(bit_t));

	//saveSuperBlock();

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
		saveSuperBlock();
	}
	else
	{
		this->superBlock.cnt--;
		WriteBlock((char*) &superBlock.cnt, SUPER_BLOCK_ID, (bit_t) ((char*)&superBlock.cnt - (char*)&superBlock), sizeof(bit_t));
	}
	//saveSuperBlock();

	return true;
}
