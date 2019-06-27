#include "../include/FileSystem.h"
#include "../include/FileDir.h"
#include "../include/INode.h"
#include <string>
#include <iostream>
using namespace std;
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
	//cout << "sz" << sz << endl;
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
	ClearINodeCache();
	vhd.Format();
	INodeMem rootINode(ROOT_BLOCK_ID, 0, this);
	INode &inode = AccessINode(rootINode);
	inode = INode(FILE_TYPE_DIR, USER_ROOT_UID, this);
	rootDir = new FileDir("", rootINode);
	rootDir->subDirs.push_back(new FileDir(".", rootINode));
	rootDir->subDirs.push_back(new FileDir("..", rootINode));
	SaveDir(rootDir, USER_ROOT_UID);
	return true;
}

// INodeCache

void FileSystem::PopINodeCache(INodeCacheItem *cur)
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

// 不插入Set
void FileSystem::PushINodeCache(INodeCacheItem *cur)
{
	(cur->nxt = INodeCacheHead->nxt) -> lst = cur;
	(cur->lst = INodeCacheHead) -> nxt = cur;
	++CacheSize;
}
// 移出Set
void FileSystem::EraseINodeCache(INodeCacheItem *cur)
{
	if(cur->changed)
	{
		WriteINode(&cur->value, cur->address.first, cur->address.second);
	}
	PopINodeCache(cur);
	Set.erase(Set.find(cur));
}
void FileSystem::RefreshINodeCache(INodeCacheItem *cur)
{
	PopINodeCache(cur);
	PushINodeCache(cur);
}
void FileSystem::ClearINodeCache()
{
	CacheSize = 0;
	for(ItemList *it = INodeCacheHead->nxt, *tmp;it != INodeCacheHead;)
	{
		INodeCacheItem *cur = (INodeCacheItem *) it;
		if(cur->changed)
		{
			if(!WriteINode(&cur->value, cur->address.first, cur->address.second))
			{
				Throw(std::string(__FILE__) + " " + std::to_string(__LINE__));
			}
		}
		it = it->nxt;
		delete cur;
	}
	INodeCacheHead->nxt = INodeCacheHead->lst = INodeCacheHead;
	Set.clear();
}
// INode

INode &FileSystem::AccessINode(bid_t BlockID, bit_t Location)
{
	static INodeCacheItem comp;
	comp.address.first = BlockID, comp.address.second = Location;
	std::set<INodeCacheItem*,Cmp>::iterator it;
	if((it = Set.find(&comp)) != Set.end())
	{
		RefreshINodeCache(*it);
		(*it)->value.atime = time(NULL);
		return (*it)->value;
	}
	else // 新曾一个缓存项
	{
		INodeCacheItem *ptr;
		if(CacheSize == INODE_CACHE_CNT)
		{
			ptr = (INodeCacheItem *) INodeCacheHead->lst;
			EraseINodeCache(ptr);
			Set.erase(ptr);
		}
		else
		{
			ptr = new INodeCacheItem();
		}
		ptr->address = std::make_pair(BlockID, Location);
		Set.insert(ptr);
		PushINodeCache(ptr);
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
	return true;
}
bool FileSystem::WriteINode(const INode *inode, bid_t BlockID, bit_t Location)
{
	if(inode == NULL) return false;
	vhd.WriteBlock((char *) inode, BlockID, Location * INODE_SIZE, INODE_SIZE);
	return true;
}
bid_t FileSystem::getKthBlock(const INode &inode, bid_t K)
{
	//cout << K << " " << inode.blocks << endl;
	if((long long) K > inode.blocks) Throw("K is too large");
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
// 仅在K == inode.blocks && inode.rembytes == BLOCK_SIZE时使用. 不改变inode.blocks和rembytes
void FileSystem::setKthBlock(INode &inode, bid_t K, bid_t blockID)
{
	if(K != inode.blocks + 1 || inode.rem_bytes != BLOCK_SIZE)
	{
		//cout << "K = " << K << " blocks=" << inode.blocks << " rembytes=" << inode.rem_bytes << endl;
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
	bid_t newBlock;
	vhd.AllocBlock(newBlock);
	setKthBlock(inode, inode.blocks+1, newBlock);
	++inode.blocks;
	inode.rem_bytes = 0;
	return newBlock;
}

bool FileSystem::PopBlock(INode &inode)
{
	if(inode.blocks < 0) return false;
	inode.rem_bytes = BLOCK_SIZE;
	if(inode.blocks < INODE_DIRECT_SIZE)
	{
		vhd.FreeBlock(inode.direct_data[inode.blocks--]);
		return true;
	}
	bid_t K = inode.blocks - INODE_DIRECT_SIZE, blockID;
	--inode.blocks;
	if(K < INODE_BCNT1)
	{
		if(!vhd.ReadBlock((char*)&blockID, inode.indirect1, K*BID_LEN, BID_LEN))
		{
			Throw("Failed to read from indirect1 : " + std::to_string(K * BID_LEN));
		}
		if(!vhd.FreeBlock(blockID)) Throw("Failed to FreeBlock");
		if(K == 0)
		{
			if(!vhd.FreeBlock(inode.indirect1)) Throw("Failed to FreeBlock");
			inode.indirect1 = 0;
		}
		return true;
	}
	K -= INODE_BCNT1;
	if(K < INODE_BCNT2)
	{
		bid_t tmp, ind1;
		tmp = K / (BLOCK_SIZE / BID_LEN); // K所在的一阶间址表在二阶间址表中的序号
		if(!vhd.ReadBlock((char*) &ind1, inode.indirect2, tmp * BID_LEN, BID_LEN))
		{
			Throw("Failed to Read from indirect2 : " + std::to_string(tmp * BID_LEN));
		}
		if(!vhd.ReadBlock((char*) &blockID, ind1, (K-tmp*(BLOCK_SIZE/BID_LEN))*BID_LEN, BID_LEN))
		{
			Throw("Failed to Read from indirect2 : " + std::to_string(tmp * BID_LEN));
		}
		if(!vhd.FreeBlock(blockID)) Throw("Failed to FreeBlock");

		if(K - tmp * (BLOCK_SIZE / BID_LEN) == 0) // 需要在indirect2块中删除一个一阶间址表的blockID
		{
			if(!vhd.FreeBlock(ind1)) Throw("Failed to Free indirect2[" + std::to_string(tmp) + "]");
		}
		if(K == 0)
		{
			if(!vhd.FreeBlock(inode.indirect2)) Throw("Failed to Free indirect2");
		}
		vhd.WriteBlock((char*)&blockID, ind1, K*BID_LEN, BID_LEN);
		return true;
	}
	else Throw("K is too large");
}

// Dir

// 将FileDir表从磁盘中读出
void FileSystem::ListDir(FileDir *curDir, uid_t uid)
{
	static char namebuff[BLOCK_SIZE];
	//curDir->subDirs.clear();
	if(!curDir->subDirs.empty()) return;
	unsigned cnt;
	int sz = 0;
	INode &inode = AccessINode(curDir->curINode);
	if(!inode.isDir())
	{
		Throw(curDir->name + " is not a director");
	}

	{
		//printf("curDir = \"%s\", cur->INode = (%d,%d)\n", curDir->name.c_str(), curDir->curINode.BlockID, curDir->curINode.Location);
	}

	if(!inode.checkR(uid)) Throw("Permission Denied by " + curDir->name);

	sz += ReadFile(curDir, (char*) &cnt, 0, sizeof(cnt), uid);
	bid_t BlockID;
	bit_t Location;
	for(int i = 0;i < cnt;++i)
	{
		bit_t len;
		sz += ReadFile(curDir, (char*) &len, sz, sizeof(len), uid);
		sz += ReadFile(curDir, namebuff, sz, len, uid);
		sz += ReadFile(curDir, (char*) &BlockID, sz, sizeof(bid_t), uid);
		sz += ReadFile(curDir, (char*) &Location, sz, sizeof(bit_t), uid);
		FileDir *tmp = new FileDir(string(namebuff), INodeMem(BlockID, Location, this));
		curDir->subDirs.push_back(tmp);
	}
}
void FileSystem::SaveDir(FileDir *curDir, uid_t uid)
{
	unsigned cnt = curDir->subDirs.size();
	int sz = 0;
	sz += WriteFile(curDir, (char*) &cnt, 0, sizeof(cnt), uid);
	for(int i = 0;i < cnt;++i)
	{
		bit_t len = curDir->subDirs[i]->name.length() + 1;
		sz += WriteFile(curDir, (char*) &len, sz, sizeof(len), uid);
		sz += WriteFile(curDir, curDir->subDirs[i]->name.c_str(), sz, len, uid);
		sz += WriteFile(curDir, (char*) &curDir->subDirs[i]->curINode.BlockID, sz, sizeof(bid_t), uid);
		sz += WriteFile(curDir, (char*) &curDir->subDirs[i]->curINode.Location, sz, sizeof(bit_t), uid);
	}
}
FileDir *FileSystem::FindLastDir(FileDir *cur, std::string dir, std::string &last, uid_t uid)
{
	std::string from = getDir(dir);
	last = getName(dir);
	if(last == "")
	{
		dir.pop_back();
		from = getDir(dir), last = getName(dir);
	}
	//cout << from << " " << last << endl;
	return FindDir(cur, from, uid);
}
FileDir *FileSystem::FindDir(FileDir *cur, std::string dir, uid_t uid)
{
	if(dir == "") return cur;
	if(dir == "/") return rootDir;
	if(dir == "." || dir == "./") return cur;
	if(dir == ".." || dir == "../") return cur->parent;

	std::string from, last;
	//printf("cur=%x, last=%s\n", cur, last.c_str());

	cur = FindLastDir(cur, dir, last, uid);
	//printf("cur=%x, last=%s\n", cur, last.c_str());

	// TODO: 文件名结尾出现/的情况
	if(last == "") return cur;


	INode &inode = AccessINode(cur->curINode);
	fmode_t tmp = (uid == inode.owner) ? ((inode.mode & (7<<3))>>3) : (inode.mode & 7);

	{
		//printf("curDir = \"%s\", cur->INode = (%d,%d)\n", cur->name.c_str(), cur->curINode.BlockID, cur->curINode.Location);
	}

	ListDir(cur, uid);
	for(auto &it : cur->subDirs) if(it->name == last)
	{
		if((tmp & FILE_OTHER_X) == 0) // cannot move into cur
		{
			Throw("Failed to visit " + it->name + " in " + cur->name + " : Perssion Denied");
		}
		return it;
	}
	return cur;
}
void FileSystem::AddFileDir(FileDir *curDir, FileDir *newDir, uid_t uid)
{
	curDir->subDirs.push_back(newDir);
	newDir->parent = curDir;
	SaveDir(curDir, uid);
}

bool FileSystem::MakeDir(FileDir *curDir, std::string fname, uid_t uid)
{
	std::string from, last;
	from = getDir(fname), last = getName(fname);
	curDir = FindDir(curDir, fname, uid);

	bid_t BlockID;
	bit_t Location;
	AllocINode(BlockID, Location);
	FileDir *newDir = new FileDir(last, INodeMem(BlockID, Location, this));
	INode &inode = AccessINode(newDir->curINode);
	inode = INode(FILE_TYPE_DIR, uid, this);

	FileDir *thisDir = new FileDir(".", newDir->curINode);
	FileDir *parentDir = new FileDir("..", curDir->curINode);
	AddFileDir(newDir, thisDir, uid);
	AddFileDir(newDir, parentDir, uid);

	AddFileDir(curDir, newDir, uid);
	return true;
}

// File


bool FileSystem::Touch(struct FileDir *curDir, std::string fname, uid_t uid)
{
	std::string from, last;
	from = getDir(fname), last = getName(fname);
	curDir = FindDir(curDir, from, uid);
	
	bid_t BlockID;
	bit_t Location;
	AllocINode(BlockID, Location);
	FileDir *newDir = new FileDir(last, INodeMem(BlockID, Location, this));
	INode &inode = AccessINode(newDir->curINode);
	inode = INode(FILE_TYPE_FILE, uid, this);

	AddFileDir(curDir, newDir, uid);
	return true;
}
// TODO: INode及Blocks回收
bool FileSystem::Remove(struct FileDir *curDir, std::string fname, uid_t uid)
{
	std::string from, last;
	from = getDir(fname), last = getName(fname);
	curDir = FindDir(curDir, from, uid);

	// TODO: INode回收
	for(vector<FileDir*>::iterator it = curDir->subDirs.begin();it != curDir->subDirs.end();++it) if((*it)->name == last)
	{
		curDir->subDirs.erase(it);
		return true;
	}
	Throw(fname + " not found");
	return false;
}

//由完整路径名得到倒数第二层目录
std::string getDir(const std::string &fullDir)
{
	std::string ans = fullDir;
	while(!ans.empty() && ans.back() != '/') ans.pop_back();
	//if(!ans.empty()) ans.pop_back();
	//else Throw("Wrong Format");
	return ans;
}
//由完整目录名得到最后一层
std::string getName(const std::string &fullDir)
{
	int it = fullDir.length();
	while(it && fullDir[it-1] != '/') --it;
	std::string ans = "";
	while(it != fullDir.length()) ans += fullDir[it++];
	return ans;
}
bool FileSystem::MakeHardLink(FileDir *curDir, std::string Dest, std::string Src, uid_t uid)
{
	FileDir *destDir, *srcDir;
	std::string filename = getName(Dest);

	srcDir = FindDir(curDir, Src, uid);
	if(srcDir == NULL) Throw("Wrong director of Src");
	INode &src = AccessINode(srcDir->curINode);

	if((src.mode & FILE_TYPE_MASK) == FILE_TYPE_DIR)
		Throw("Can't make hard link for directors");

	destDir = FindDir(curDir, getDir(Dest), uid);
	if(destDir == NULL) Throw("Wrong director of Dest");

	if(FindDir(destDir, filename, uid) != NULL)
		Throw(filename + " already exists");

	FileDir *newDir = new FileDir(filename, srcDir->curINode);
	AddFileDir(destDir, newDir, uid);
	return true;
}
bool FileSystem::MakeSoftLink(FileDir *curDir, std::string Dest, std::string Src, uid_t uid)
{
	// 处理目录、文件名

	FileDir *destDir;
	std::string filename = getName(Dest);
	destDir = FindDir(curDir, getDir(Dest), uid);
	if(destDir == NULL) Throw("Wrong director of Dest");

	// 创建INode和目录
	bid_t BlockID;
	bit_t Location;
	AllocINode(BlockID, Location);
	INodeMem inodemem(BlockID, Location, this);
	FileDir *newDir = new FileDir(filename, inodemem);
	
	// 设置INode信息
	INode &inode = AccessINode(inodemem);
	inode = INode(FILE_TYPE_LINK, uid, this);
	WriteFile(newDir, Src.c_str(), 0, Src.length() + 1, uid);
	AddFileDir(destDir, newDir, uid);

	return true;
}
ssize_t FileSystem::ReadFile(FileDir *file, char *buff, diskaddr_t begin, size_t size, uid_t uid)
{
	return ReadFile(AccessINode(file->curINode), buff, begin, size, uid);
}
ssize_t FileSystem::ReadFile(INode &inode, char *buff, diskaddr_t begin, size_t size, uid_t uid)
{
	if(begin >= inode.size())
	{
		return EOF;
	}
	diskaddr_t end = begin + size, it;
	bid_t K, blockID;
	bit_t L, cnt;
	if(end > inode.size()) // 到达文件末尾
	{
		end = inode.size();
		size = end - begin;
		buff[size] = EOF;
	}
	for(K = begin / BLOCK_SIZE, it = K * BLOCK_SIZE, L = begin-it;it < end;++K, it += BLOCK_SIZE, L = 0)
	{
		blockID = getKthBlock(inode, K);
		if(end - it > BLOCK_SIZE) cnt = BLOCK_SIZE;
		else cnt = end - it;
		vhd.ReadBlock(buff + it + L - begin, blockID, L, cnt);
	}
	return end - begin;
}
ssize_t FileSystem::WriteFile(FileDir *file, const char *buff, diskaddr_t begin, size_t size, uid_t uid)
{
	return WriteFile(AccessINode(file->curINode), buff, begin, size, uid);
}
ssize_t FileSystem::WriteFile(INode &inode, const char *buff, diskaddr_t begin, size_t size, uid_t uid)
{
	if(!size) return 0;
	diskaddr_t end = begin + size, it;
	bid_t K = begin / BLOCK_SIZE, blockID;
	it = K * BLOCK_SIZE;
	bit_t L = begin - it, cnt;
	static char Buff0[BLOCK_SIZE] = {0};
	
	// 当写入位置超过文件大小时，扩张文件
	if(inode.blocks == -1)
	{
		blockID = AppendBlock(inode);
	}

	if(inode.blocks < K)
	{
		blockID = getKthBlock(inode, inode.blocks);
		while(inode.blocks < K)
		{
			if(inode.rem_bytes != BLOCK_SIZE)
			{
				vhd.WriteBlock(Buff0, blockID, inode.rem_bytes, BLOCK_SIZE - inode.rem_bytes);
				inode.rem_bytes = BLOCK_SIZE;
			}
			blockID = AppendBlock(inode);
		}
	}
	else blockID = getKthBlock(inode, K);
	if(inode.blocks == K && inode.rem_bytes < L)
	{
		vhd.WriteBlock(Buff0, blockID, inode.rem_bytes, L - inode.rem_bytes);
		inode.rem_bytes = L;
	}
	//printf("K=%d, blocks=%d, begin=%lld, it=%lld, L=%d, size=%d\n", K, inode.blocks, begin, it, L, size);
	for(;it < end;it += BLOCK_SIZE, L = 0)
	{
		if(end - it - L > BLOCK_SIZE) cnt = BLOCK_SIZE;
		else cnt = end - it - L;
		vhd.WriteBlock(buff + it + L - begin, blockID, L, cnt);
		//printf("K=%d, blocks=%d, L=%d, cnt=%d\n", K, inode.blocks, L, cnt);
		if(K == inode.blocks)
		{
			if(L + cnt > inode.rem_bytes) // 写入后文件扩张
			{
				if((inode.rem_bytes = L + cnt) == BLOCK_SIZE && it + BLOCK_SIZE < end) //写满一块，还要继续写
				{
					blockID = AppendBlock(inode);
				}
				++K;
			}
			else // 不会继续循环
			{
				break;
			}
		}
		else
		{
			blockID = getKthBlock(inode, ++K);
		}
	}
	return end - begin;
}

diskaddr_t FileSystem::TruncFile(struct FileDir *file, diskaddr_t length, uid_t uid)
{
	//TODO
	bid_t K = length / BLOCK_SIZE; // blocks -> K
	INode &inode = AccessINode(file->curINode);
	inode.atime = inode.mtime = time(NULL);
	while(inode.blocks > K)
	{
		PopBlock(inode);
	}
	inode.rem_bytes = length % BLOCK_SIZE; //rem_bytes -> K
	return length;
}
