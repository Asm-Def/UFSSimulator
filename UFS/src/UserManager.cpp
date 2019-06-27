#include "../include/UserManager.h"
#include "../include/FileSystem.h"
#include "../include/FileDir.h"
#include <vector>
using namespace std;

UserManager::UserManager(FileSystem *fs) : FS(fs)
{
}
UserManager::~UserManager()
{
	saveUsers();
	userlist.clear();
}
bool UserManager::Login(std::string name, std::string pass, uid_t &uid)
{
	loadUsers();
	for(auto &user : userlist) if(user.name == name)
	{
		uid = user.uid;
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
	saveUsers();
}
bool UserManager::saveUsers()
{
	FileDir *file = FS->FindDir(FS->getRoot(), "/etc/passwd", USER_ROOT_UID);
	FS->TruncFile(file, 0, USER_ROOT_UID);
	for(auto &user : userlist)
	{
		string tmp = user.name + " " + to_string(user.uid) + " " + user.pass + "\n";
		FS->WriteFile(file, tmp.c_str(), file->curINode.size(), tmp.length() + 1, USER_ROOT_UID);
	}
	return true;
}
bool UserManager::loadUsers()
{
	FileDir *file = FS->FindDir(FS->getRoot(), "/etc/passwd", USER_ROOT_UID);
	static char str[4096 * 32], tmpname[4096], pass[4096];
	uid_t uid;
	ssize_t sz = FS->ReadFile(file, str, 0, file->curINode.size(), USER_ROOT_UID);
	ssize_t it = 0;
	userlist.clear();
	while(it < sz)
	{
		int t = sscanf(str+it, "%s %u %s", tmpname, &uid, pass);
		if(t == 0) break;
		User user;
		user.uid = uid;
		user.name = tmpname;
		user.pass = user.pass;
		userlist.push_back(user);
		it += t;
	}
	return true;
}