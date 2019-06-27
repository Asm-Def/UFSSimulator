#ifndef TESTOS_H
#define TESTOS_H
class Session;
class TestOS;

#include "UFSParams.h"
#include "FileSystem.h"
#include "UserManager.h"
#include "ProcessFDTable.h"
#include <vector>
#include <string>
using namespace std;

class Session
{
	FileSystem *FS;
	FileDir *curDir;
	uid_t uid;
	ufspid_t pid;
	ProcessFDTable processFDTable;
	public:
	Session(FileDir *dir, uid_t u, ufspid_t p, FileSystem *fs)
		: curDir(dir), uid(u), pid(p), processFDTable(fs, p, u) {}
		
	bool AddUser(string name, string pass = ""); // only while uid=0
	bool CreateFile(string *dir);
	int OpenFile(string *dir); // return an FD in processFDTable

	diskoff_t lseek(int fd, diskoff_t offset, int fromwhere);
	ssize_t read(int fd, void *buf, size_t count);
	ssize_t write(int fd, void *buf, size_t count);
	void Interact();
};

class TestOS
{
	FileSystem RootFS;
	UserManager userManager;

	public:
	TestOS();
	void Init(string vhdfilename, string rootpass = "root");
	vector<string> ListUsers();
	Session *Login(string name, string pass);
};

#endif
