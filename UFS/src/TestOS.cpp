#include "../include/TestOS.h"
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

diskoff_t Session::lseek(int fd, diskoff_t offset, int fromwhere)
{
	if(fd < 0 || fd >= processFDTable.MaxFD()) return -1;
	OpenedFile &file = processFDTable.getFile(fd);
	return file.lseek(offset, fromwhere);
}
ssize_t Session::read(int fd, void *buf, size_t count)
{
	if(fd < 0 || fd >= processFDTable.MaxFD()) return -1;
	OpenedFile &file = processFDTable.getFile(fd);
	if(!(file.auth & FILE_OTHER_R)) return -1;
	return file.read(buf, count,user.uid);
}
ssize_t Session::write(int fd, void *buf, size_t count)
{
	if(fd < 0 || fd >= processFDTable.MaxFD()) return -1;
	OpenedFile &file = processFDTable.getFile(fd);
	if(!(file.auth & FILE_OTHER_W)) return -1;
	return file.write(buf, count, user.uid);
}

void Session::ChangeDir(FileDir *&curDir, FileDir *newDir)
{
	INode &inode = FS->AccessINode(newDir->curINode);
	if(inode.isDir()) curDir = newDir;
	else if(inode.isFile()) throw newDir->name + " is not a direction";
	else
	{
		FileDir *tmp = FS->FollowLink(newDir, user.uid);
		if(tmp == newDir) throw newDir->name + " is a soft link";
		ChangeDir(curDir, newDir);
	}
}
void Session::Interact()
{
	std::cout << "Welcome, " << user.name << endl;
	std::cin.get();
	while(true)
	{
		string fulldir = curDir->fulldir();
		printf("%s@UFSSimulator %s\n$ ", user.name.c_str(), fulldir == "" ? "/" : fulldir.c_str());
		string line; getline(cin, line);
		string cmd;
		std::cout << "command = \"" << line << "\"" << endl;
		istringstream ss(line);
		ss >> cmd;
		if(cmd == "") continue;
		else if(cmd == "exit") break;
		else if(cmd == "cd")
		{
			string dir; ss >> dir;
			FileDir *tmp;
			try
			{
				tmp = FS->FindDir(this->curDir, dir, user.uid);
				if(tmp == NULL) continue;
				ChangeDir(curDir, tmp);
			}
			catch(string str) { printf("Failed to goto %s: %s\n", dir.c_str(), str.c_str()); continue; }
		}
		else if(cmd == "ls")
		{
			string opt; ss >> opt;
			try { FS->ListDir(curDir, user.uid); } catch(std::string str) { std::cout << str << endl; continue; }
			for(auto &subdir : curDir->subDirs)
			{
				if(opt == "-l")
				{
					try {
						std::cout << FS->FileDirDetail(subdir, user.uid);
					}
					catch (string str) { printf("%s : %s\n", subdir->name.c_str(), str.c_str()); }
				}
				else std::cout << subdir->name << " ";
			}
			puts("");
		}
		else if(cmd == "mkdir")
		{
			string dir; ss >> dir;
			try{ FS->MakeDir(curDir, dir, user.uid); }
			catch(string str) { printf("Failed to make dir %s: %s\n", dir.c_str(), str.c_str()); continue; }
		}
		else if(cmd == "touch")
		{
			string dir; ss >> dir;
			try{ FS->Touch(curDir, dir, user.uid); }
			catch(string str) { printf("Failed to touch %s: %s\n", dir.c_str(), str.c_str()); continue; }
		}
		else if(cmd == "rm")
		{
			string dir; ss >> dir;
			FileDir *tmp;
			try
			{
				FS->Remove(curDir, dir, user.uid);
			}
			catch(string str)
			{
				printf("Failed to remove %s: %s\n", dir.c_str(), str.c_str());
				continue;
			}
			catch(const char *str)
			{
				printf("Failed to remove %s: %s\n", dir.c_str(), str);
				continue;
			}
		}
		else if(cmd == "mkhl")
		{
			string dest, src; ss >> src >> dest;
			try{ FS->MakeHardLink(curDir, dest, src, user.uid); }
			catch(string str) { printf("cannot make hardlink %s: %s\n", dest.c_str(), str.c_str()); continue; }
		}
		else if(cmd == "mksl")
		{
			string dest, src; ss >> src >> dest;
			try{ FS->MakeSoftLink(curDir, dest, src, user.uid); }
			catch(string str) { printf("cannot make softlink %s: %s\n", dest.c_str(), str.c_str()); continue; }
		}
		else if(cmd == "cat")
		{
			static char buff[BLOCK_SIZE];
			string fname; ss >> fname;
			FileDir *file;
			try { file = FS->FindDir(curDir, fname, user.uid); }
			catch(string str) { printf("cannot find file %s:%s\n", fname.c_str(), str.c_str()); continue; }
			INode &inode = FS->AccessINode(file->curINode);
			if(!inode.checkR(user.uid))
			{
				printf("permission denied on %s\n", fname.c_str());
				continue;
			}
			diskoff_t it = 0, t;
			while(it < inode.size())
			{
				t = FS->ReadFile(inode, buff, it, BLOCK_SIZE, user.uid);
				for(int i = 0;i < t;++i) putchar(buff[i]);
				//printf("t = %lld, it = %lld, size = %lld\n", t, it, inode.size());
				it += t;
			}
			putchar('\n');
		}
		else if(cmd == "write")
		{
			string fname, buff; ss >> fname >> buff;
			FileDir *file;
			try { file = FS->FindDir(curDir, fname, user.uid); }
			catch(string str) { printf("cannot find file %s:%s\n", fname.c_str(), str.c_str()); continue; }
			INode &inode = FS->AccessINode(file->curINode);
			if(!inode.checkW(user.uid))
			{
				printf("permission denied on %s\n", fname.c_str());
				continue;
			}
			FS->WriteFile(inode, buff.c_str(), 0, buff.length() + 1, user.uid);
		}
		else if(cmd == "adduser")
		{
			if(user.uid != USER_ROOT_UID)
			{
				printf("permission denied, please use root\n");
				continue;
			}
			string uname, upass;
			ss >> uname >> upass;
			if(uname == "")
			{
				printf("username should be non-empty\n");
				continue;
			}
			os->userManager.CreateUser(uname, upass);
			printf("%s successfully created\n", uname.c_str());
		}
	}
}

void TestOS::Init(string vhdfilename, string rootpass)
{
	RootFS.CreateVHD(BLOCK_SIZE * 32, vhdfilename);
	RootFS.FormatVHD();
}
vector<string> TestOS::ListUsers()
{
	if(!userManager.loadUsers())
	{
		Throw("Cannot list user");
	}
	vector<string> ans;
	for(auto user : userManager.userlist)
	{
		ans.push_back(user.name);
	}
	return ans;
}
Session *TestOS::Login(string name, string pass)
{
	uid_t uid;
	User user;
	if(!userManager.Login(name, pass, uid, user)) return NULL;
	return new Session(RootFS.getRoot(), user, 0, &RootFS, this);
}

TestOS::TestOS() : RootFS(), userManager(&RootFS)
{
	if(!RootFS.LoadVHD("1.vhd"))
	{
		RootFS.CreateVHD(BLOCK_SIZE * 32, "1.vhd");
	}
}

int main()
{
	TestOS OS;
	Session *session;
	while(true)
	{
		std::cout << "0: Exit\n1: Login\n2: Show Users\n3: Create new vhd\n" << endl;
		int opt;
		std::cin >> opt;
		printf("opt = %d\n", opt);
		if(opt == 0) break;
		if(opt == 2)
		{
			auto lst = OS.ListUsers();
			for(auto &user : lst) std::cout << user << endl;
			continue;
		}
		else if(opt == 3)
		{
			string vhdname;
			std::cin >> vhdname;
			OS.Init(vhdname, "123456");
			continue;
		}
		std::cin.sync();
		string name, pass;
		std::cout << "Please input Username:";
		std::cin >> name;
		std::cout << "Please input password:";
		std::cin >> pass;
		if((session = OS.Login(name, pass)) != NULL)
		{
			session->Interact();
			delete session;
		}
		else std::cout << "wrong username or password" << endl;
	}
	return 0;
}