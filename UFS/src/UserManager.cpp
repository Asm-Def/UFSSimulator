#include "../include/UserManager.h"
#include "../include/FileSystem.h"
#include "../include/FileDir.h"
#include <vector>
#include <sstream>
using namespace std;

UserManager::UserManager(FileSystem *fs) : FS(fs)
{
}
UserManager::~UserManager()
{
	userlist.clear();
}
#include <iostream>
bool UserManager::Login(std::string name, std::string pass, uid_t &uid)
{
	User us;
	return Login(name, pass, uid, us);
}
bool UserManager::Login(std::string name, std::string pass, uid_t &uid, User &us)
{
	loadUsers();
	for(auto &user : userlist) std::cout << " userlist: " << user.name << " " << user.pass << endl;
	for(auto &user : userlist) if(user.name == name)
	{
		uid = user.uid;
		us = user;
		if(user.pass != pass) return false;
		else return true;
	}
	return false;
}
void UserManager::CreateUser(std::string name, std::string pass)
{
	loadUsers();
	User user;
	user.name = name;
	user.pass = pass;
	user.uid = userlist.size();
	userlist.push_back(user);
	FileDir *curDir = FS->FindDir(FS->getRoot(), "/home", USER_ROOT_UID);
	bid_t bid; bit_t bit;
	FS->AccessINode(bid, bit);
	FS->MakeDir(curDir, name, USER_ROOT_UID);
	INode &inode = FS->AccessINode((FS->FindDir(curDir, name, user.uid))->curINode);
	inode.mode = (FILE_TYPE_DIR | FILE_OWNER_R | FILE_OWNER_W | FILE_OWNER_X);
	inode.owner = user.uid;
	saveUsers();
}
bool UserManager::saveUsers()
{
	FileDir *file = FS->FindDir(FS->getRoot(), "/etc/passwd", USER_ROOT_UID);
	FS->TruncFile(file, 0, USER_ROOT_UID);
	disksize_t sz = 0;
	for(auto &user : userlist)
	{
		string tmp = user.name + " " + to_string(user.uid) + " " + user.pass + "\n";
		sz += FS->WriteFile(file, tmp.c_str(), sz, tmp.length(), USER_ROOT_UID);
	}
	FS->WriteFile(file, "", sz, 1, USER_ROOT_UID);
	return true;
}
bool UserManager::loadUsers()
{
	FileDir *file = NULL;
	try{file = FS->FindDir(FS->getRoot(), "/etc/passwd", USER_ROOT_UID);}
	catch(string str) { printf("cannot find userlist:%s\n", str.c_str());}
	if(file == NULL) throw (std::string) "no userlist";
	static char str[4096 * 32];
	string tmpname, pass;
	uid_t uid;
	ssize_t sz = FS->ReadFile(file, str, 0, file->curINode.size(), USER_ROOT_UID);
	userlist.clear();
	istringstream ss(str);
	while(true)
	{
		if(!(ss >> tmpname >> uid >> pass)) break;
		//printf("UserManager::loadUsers: %s %u %s\n", tmpname.c_str(), uid, pass.c_str());
		User user;
		user.uid = uid;
		user.name = tmpname;
		user.pass = pass;
		userlist.push_back(user);
	}
	return true;
}