#ifndef USERMANAGER_H
#define USERMANAGER_H
#include "UFSParams.h"
#include "User.h"
#include <fstream>
#include <vector>
class UserManager
{
	struct FileSystem *FS;
	public:
	bool loadUsers();
	bool saveUsers();
	std::vector<User> userlist;
	UserManager(struct FileSystem *fs);
	~UserManager();
	bool Login(std::string name, std::string pass, uid_t &uid);
	bool Login(std::string name, std::string pass, uid_t &uid, User &us);
	void CreateUser(std::string name, std::string pass);
};
#endif