#ifndef USERMANAGER_H
#define USERMANAGER_H
#include "UFSParams.h"
#include "User.h"
#include <fstream>
#include <vector>
class UserManager
{
	struct FileSystem *FS;
	bool loadUsers();
	bool saveUsers();
	std::vector<User> userlist;
	public:
	UserManager(struct FileSystem *fs);
	~UserManager();
	bool Login(std::string name, std::string pass, uid_t &uid);
	void CreateUser(std::string name, std::string pass);
};
#endif