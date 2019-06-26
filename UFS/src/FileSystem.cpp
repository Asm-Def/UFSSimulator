#include "../include/FileSystem.h"
#include "../include/FileDir.h"
#include "../include/INode.h"
#include <string>
#include <iostream>
FileSystem::FileSystem() : rootDir(NULL)
{
	INodeCacheHead = new ItemList();
	CacheSize = 0;
}
FileSystem::~FileSystem()
{
	if(rootDir) delete rootDir;
	for(ItemList *it = INodeCacheHead->nxt, *tmp;it != INodeCacheHead;)
	{
		INodeCacheItem *cur = (INodeCacheItem *) it;
		if(cur->changed)
		{
			if(!WriteINode(&cur->value, cur->address.first, cur->address.second))
			{
				std::cout << (std::string(__FILE__) + " " + std::to_string(__LINE__)) << std::endl;
			}
		}
		it = it->nxt;
		delete cur;
	}
	delete INodeCacheHead;
}

// VHD

bool FileSystem::CreateVHD(disksize_t sz, std::string name)
{
	vhd.Create(sz, name);
	if(!FormatVHD())
	{
		return false;
	}
	return true;
}
bool FileSystem::LoadVHD(std::string vhdname)
{
	vhd.Load(vhdname);
	if(rootDir) delete rootDir;
	INodeMem inode(ROOT_BLOCK_ID, 0, this);
	rootDir = new FileDir("", inode);
	return true;
}
bool FileSystem::FormatVHD() // 重建文件系统(同时创建根目录)
{
	vhd.Format();
	INodeMem rootINode(ROOT_BLOCK_ID, 0, this);
	rootDir = new FileDir("", rootINode);
	return true;
}

// INodeCache

void FileSystem::PopCache(INodeCacheItem *cur)
{
	if(cur == NULL) Throw("Erase NULL");
	if(cur->lst == cur || cur->nxt == cur)
	{
		Throw("List Structure Error");
	}
	cur->lst->nxt = cur->nxt;
	cur->nxt->lst = cur->lst;
	--CacheSize;
}
void FileSystem::PushCache(INodeCacheItem *cur)
{
	(cur->nxt = INodeCacheHead->nxt) -> lst = cur;
	(cur->lst = INodeCacheHead) -> nxt = cur;
	++CacheSize;
}
void FileSystem::EraseCache(INodeCacheItem *cur)
{
	if(cur->changed)
	{
		WriteINode(&cur->value, cur->address.first, cur->address.second);
	}
	PopCache(cur);
	Set.erase(Set.find(cur));
}
void FileSystem::RefreshCache(INodeCacheItem *cur)
{
	PopCache(cur);
	PushCache(cur);
}

// INode

INode &FileSystem::AccessINode(bid_t BlockID, bit_t Location)
{
	static INodeCacheItem comp;
	comp.address.first = BlockID, comp.address.second = Location;
	std::set<INodeCacheItem*,Cmp>::iterator it;
	if((it = Set.find(&comp)) != Set.end())
	{
		RefreshCache(*it);
		return (*it)->value;
	}
	else
	{
		INodeCacheItem *ptr = *it;
		if(CacheSize == INODE_CACHE_CNT)
		{
			EraseCache(ptr);
		}
		else
		{
			ptr = new INodeCacheItem();
		}
		ptr->address = std::make_pair(BlockID, Location);
		Set.insert(ptr);
		ptr->changed = true;
		ReadINode(&ptr->value, BlockID, Location);
		ptr->value.atime = time(NULL);
		return ptr->value;
	}
}
INode &FileSystem::AccessINode(const INodeMem &inode)
{
	return AccessINode(inode.BlockID, inode.Location);
}
bool FileSystem::AllocINode(bid_t &BlockID, bit_t &Location)
{
	if(vhd.info.nxtINodeLocation == INODE_PER_BLOCK)
	{
		if(!vhd.AllocBlock(vhd.info.nxtINodeBlockID))
		{
			return false;
		}
		vhd.info.nxtINodeLocation = 0;
	}
	BlockID = vhd.info.nxtINodeBlockID;
	Location = vhd.info.nxtINodeLocation++;
	vhd.saveInfoBlock();
	return true;
}

bool FileSystem::ReadINode(INode *inode, bid_t BlockID, bit_t Location)
{
	if(inode == NULL) return false;
	vhd.ReadBlock((char *) inode, BlockID, Location * INODE_SIZE, INODE_SIZE);
}
bool FileSystem::WriteINode(const INode *inode, bid_t BlockID, bit_t Location)
{
	if(inode == NULL) return false;
	vhd.WriteBlock((char *) inode, BlockID, Location * INODE_SIZE, INODE_SIZE);
}
bid_t FileSystem::getKthBlock(const INode &inode, bid_t K)
{
	if(K > inode.blocks) Throw("K is too large");
	if(K < INODE_DIRECT_SIZE) return inode.direct_data[K];
	K -= INODE_DIRECT_SIZE;
	bid_t ans;
	if(K < INODE_BCNT1)
	{
		vhd.ReadBlock((char*)&ans, inode.indirect1, K*BID_LEN, BID_LEN);
		return ans;
	}
	K -= INODE_BCNT1;
	if(K < INODE_BCNT2)
	{
		bid_t tmp, ind1;
		tmp = K / (BLOCK_SIZE / BID_LEN); // K所在的一阶间址表在二阶间址表中的序号
		vhd.ReadBlock((char*) &ind1, inode.indirect2, tmp * BID_LEN, BID_LEN); // K所在的一阶间址表地址
		K -= tmp * (BLOCK_SIZE / BID_LEN);
		vhd.ReadBlock((char*)&ans, ind1, K*BID_LEN, BID_LEN);
		return ans;
	}
	else Throw("K is too large");
}
void FileSystem::setKthBlock(INode &inode, bid_t K, bid_t blockID)
{
	if(K != inode.blocks + 1 || inode.rem_bytes != BLOCK_SIZE)
	{
		Throw("Not time for getting new block");
	}
	if(K < INODE_DIRECT_SIZE)
	{
		inode.direct_data[K] = blockID;
		return;
	}
	K -= INODE_DIRECT_SIZE;
	if(K < INODE_BCNT1)
	{
		if(K == 0) vhd.AllocBlock(inode.indirect1);
		if(!vhd.WriteBlock((char*)&blockID, inode.indirect1, K*BID_LEN, BID_LEN))
		{
			Throw("Failed to write in indirect1 : " + std::to_string(K * BID_LEN));
		}
		return;
	}
	K -= INODE_BCNT1;
	if(K < INODE_BCNT2)
	{
		if(K == 0) vhd.AllocBlock(inode.indirect2);
		bid_t tmp, ind1;
		tmp = K / (BLOCK_SIZE / BID_LEN); // K所在的一阶间址表在二阶间址表中的序号
		K -= tmp * (BLOCK_SIZE / BID_LEN);
		if(K == 0) // 需要在indirect2块中新增一个一阶间址表的blockID
		{
			vhd.AllocBlock(ind1);
			if(!vhd.WriteBlock((char*) &ind1, inode.indirect2, tmp * BID_LEN, BID_LEN))
			{
				Throw("Failed to write to indirect2 : " + std::to_string(tmp * BID_LEN));
			}
		}
		else
		{
			if(!vhd.ReadBlock((char*) &ind1, inode.indirect2, tmp * BID_LEN, BID_LEN)) // K所在的一阶间址表地址
			{
				Throw("Failed to Read from indirect2 : " + std::to_string(tmp * BID_LEN));
			}
		}
		vhd.WriteBlock((char*)&blockID, ind1, K*BID_LEN, BID_LEN);
	}
	else Throw("K is too large");
}

bid_t FileSystem::AppendBlock(INode &inode)
{
	if(inode.rem_bytes != BLOCK_SIZE)
		Throw("Not time for Appending");
	inode.atime = inode.mtime = time(NULL);
	inode.rem_bytes = 0;
	bid_t newBlock;
	vhd.AllocBlock(newBlock);
	setKthBlock(inode, ++inode.blocks, newBlock);
	return newBlock;
}

// Dir

void FileSystem::ListDir(FileDir *curDir)
{
	curDir->subDirs.clear();
	//TODO
}
void FileSystem::SaveDir(FileDir *curDir)
{

}
FileDir *FileSystem::FindDir(FileDir *cur, std::string dir)
{

}
void FileSystem::AddFileDir(FileDir *curDir, FileDir *newDir)
{

}

bool FileSystem::MakeDir(FileDir *curDir, std::string fname, uid_t uid)
{

}

// File

//由完整路径名得到倒数第二层目录
std::string getDir(const std::string &fullDir)
{
	std::string ans = fullDir;
	while(!ans.empty() && ans.back() != '/') ans.pop_back();
	if(!ans.empty()) ans.pop_back();
	else Throw("Wrong Format");
	return ans;
}

//由完整目录名得到最后一层
std::string getName(const std::string &fullDir)
{
	int it = fullDir.length() - 1;
	while(it && fullDir[it-1] != '/') --it;
	std::string ans = "";
	while(it != fullDir.length()) ans += fullDir[it++];
	return ans;
}
bool FileSystem::MakeHardLink(FileDir *curDir, std::string Dest, std::string Src, uid_t uid)
{
	FileDir *destDir, *srcDir;
	std::string filename = getName(Dest);

	srcDir = FindDir(curDir, Src);
	if(srcDir == NULL) Throw("Wrong director of Src");
	INode &src = AccessINode(srcDir->curINode);

	if((src.mode & FILE_TYPE_MASK) == FILE_TYPE_DIR)
		Throw("Can't make hard link for directors");

	destDir = FindDir(curDir, getDir(Dest));
	if(destDir == NULL) Throw("Wrong director of Dest");

	if(FindDir(destDir, filename) != NULL)
		Throw(filename + " already exists");

	FileDir *newDir = new FileDir(filename, srcDir->curINode);
	AddFileDir(destDir, newDir);
	return true;
}
bool FileSystem::MakeSoftLink(FileDir *curDir, std::string Dest, std::string Src, uid_t uid)
{
	// 处理目录、文件名

	FileDir *destDir;
	std::string filename = getName(Dest);
	destDir = FindDir(curDir, getDir(Dest));
	if(destDir == NULL) Throw("Wrong director of Dest");

	// 创建INode和目录
	bid_t BlockID;
	bit_t Location;
	AllocINode(BlockID, Location);
	INodeMem inodemem(BlockID, Location, this);
	FileDir *newDir = new FileDir(filename, inodemem);
	
	// 设置INode信息
	INode &inode = AccessINode(inodemem);
	inode.mode = FILE_TYPE_LINK | FILE_OWNER_R | FILE_OWNER_W | FILE_OTHER_R | FILE_OWNER_W;
	inode.owner = uid;
	WriteFile(newDir, Src.c_str(), 0, Src.length() + 1, uid);

	AddFileDir(destDir, newDir);

	return true;
}
bool FileSystem::ReadFile(FileDir *file, char *buff, diskaddr_t begin, size_t size, uid_t uid)
{
	
}
bool FileSystem::WriteFile(FileDir *file, const char *buff, diskaddr_t begin, size_t size, uid_t uid)
{

}