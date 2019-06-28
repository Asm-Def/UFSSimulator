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
	TestOS *os;
	User user;
	ufspid_t pid;
	ProcessFDTable processFDTable;
	public:
	Session(FileDir *dir, User u, ufspid_t p, FileSystem *fs, TestOS *o)
		: curDir(dir), user(u), pid(p), FS(fs), processFDTable(fs, p, u.uid), os(o) {}
		
	bool AddUser(string name, string pass = ""); // only while uid=0
	bool CreateFile(string *dir);
	int OpenFile(string *dir); // return an FD in processFDTable
	void ChangeDir(FileDir *&curDir, FileDir *newDir);
	diskoff_t lseek(int fd, diskoff_t offset, int fromwhere);
	ssize_t read(int fd, void *buf, size_t count);
	ssize_t write(int fd, void *buf, size_t count);
	void Interact();
};

class TestOS
{
	FileSystem RootFS;

	public:
	UserManager userManager;
	TestOS();
	void Init(string vhdfilename, string rootpass = "root");
	vector<string> ListUsers();
	Session *Login(string name, string pass);
};

#endif
