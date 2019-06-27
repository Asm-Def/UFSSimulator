#ifndef FILESYSTEM_H
#define FILESYSTEM_H
class FileSytem;
#include "VHDController.h"
#include "INode.h"
#include "UFSParams.h"
#include <vector>
#include <set>
#include <map>

struct ItemList
{
	ItemList *nxt, *lst;
	ItemList() : nxt(this), lst(this) {}
};
struct INodeCacheItem : ItemList
{
	pair<bid_t, bit_t> address;
	INode value;
	bool changed;
	INodeCacheItem(pair<bid_t, bit_t> addr, const INode &val)
	: address(addr), value(val), changed(0), ItemList() {}
	INodeCacheItem() : address(), ItemList() {}
};

struct Cmp
{
	bool operator () (const INodeCacheItem *a, const INodeCacheItem *b) { return a->address < b->address; }
};
std::string getDir(const std::string &fullDir);
std::string getName(const std::string &fullDir);
class FileSystem
{
	VHDController vhd; // 模拟一块磁盘
	struct FileDir *rootDir; // 指向根目录的FileDir

	// INode

	bool ReadINode(INode *inode, bid_t BlockID, bit_t Location);
	bool WriteINode(const INode *inode, bid_t BlockID, bit_t Location);
	bool AllocINode(bid_t &BlockID, bit_t &Location);
	bid_t getKthBlock(const INode &inode, bid_t K);
	void setKthBlock(INode &inode, bid_t K, bid_t blockID);
	// 确保inode.rem_bytes == BLOCK_SIZE
	bid_t AppendBlock(INode &inode);

	// INodeCache

	ItemList *INodeCacheHead;
	int CacheSize;
	void RefreshINodeCache(INodeCacheItem *);
	void EraseINodeCache(INodeCacheItem *);
	void PopINodeCache(INodeCacheItem *);
	void PushINodeCache(INodeCacheItem *);
	std::set<INodeCacheItem*, Cmp> Set;

	public:
	FileSystem();
	~FileSystem();

	// VHD

	// 创建并格式化
	bool CreateVHD(disksize_t sz, std::string name);
	bool LoadVHD(std::string vhdname);
	bool FormatVHD(); // 重建文件系统(同时创建根目录)
	
	// INodeCache

	INode &AccessINode(bid_t BlockID, bit_t Location);
	INode &AccessINode(const INodeMem &inode);

	// Dir
	/**
	 * 列出curDir表示的目录下的所有FileDir
	 * 过程：FileDir(当前目录) => INodeMem(目录对应的INode) => 根据寻址表将该目录下的所有FileDir及它们对应的INodeMem读入内存 => 返回结果
	 */
	void ListDir(FileDir *curDir, uid_t uid);

	struct FileDir *FindLastDir(FileDir *cur, std::string dir, std::string &last, uid_t uid);
	// find director from cur by dir[]. return NULL if no exist
	struct FileDir *FindDir(FileDir *cur, std::string dir, uid_t uid);
	// Save director
	void SaveDir(FileDir *curDir, uid_t uid);
	// 在curDir目录下新增一个FileDir
	void AddFileDir(FileDir *curDir, FileDir *newDir, uid_t uid);
	// create director {fname} if not exist (新创建的FileDir内容中应包含"."和".."两个FileDir，它们的INode地址分别为当前目录本身的INode和上级目录的INode)
	bool MakeDir(struct FileDir *curDir, std::string fname, uid_t uid);

	// File

	// create fild {fname} if not exist
	bool Touch(struct FileDir *curDir, std::string fname, uid_t uid);
	// remove an INode and its data
	bool Remove(struct FileDir *curDir, std::string fname, uid_t uid); 
	bool MakeHardLink(struct FileDir *curDir, std::string Dest, std::string Src, uid_t uid);
	bool MakeSoftLink(struct FileDir *curDir, std::string Dest, std::string Src, uid_t uid);
	ssize_t ReadFile(struct FileDir *file, char *buff, diskaddr_t begin, size_t size, uid_t uid);
	ssize_t WriteFile(struct FileDir *file, const char *buff, diskaddr_t begin, size_t size, uid_t uid);

	 // 将文件截短至{length}，返回得到的实际长度。回收多余的磁盘块
	diskaddr_t TruncFile(struct FileDir *file, diskaddr_t length, uid_t uid);
};

#endif
