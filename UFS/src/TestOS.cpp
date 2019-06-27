#include "../include/TestOS.h"
#include <iostream>
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
	return file.read(buf, count,uid);
}
ssize_t Session::write(int fd, void *buf, size_t count)
{
	if(fd < 0 || fd >= processFDTable.MaxFD()) return -1;
	OpenedFile &file = processFDTable.getFile(fd);
	if(!(file.auth & FILE_OTHER_W)) return -1;
	return file.write(buf, count, uid);
}
void Session::Interact()
{
	int ch = cin.get();
}

void TestOS::Init(string vhdfilename, string rootpass)
{
	RootFS.CreateVHD(BLOCK_SIZE * 32, vhdfilename);
	RootFS.FormatVHD();
}
vector<string> TestOS::ListUsers()
{

}
Session *TestOS::Login(string name, string pass)
{
	uid_t uid;
	if(!userManager.Login(name, pass, uid)) return NULL;
	return new Session(RootFS.getRoot(), uid, 0, &RootFS);
}

TestOS::TestOS() : RootFS(), userManager(&RootFS)
{
	RootFS.LoadVHD("1.vhd");
}

int main()
{
	TestOS OS;
	Session *session;
	OS.Init("1.vhd", "123456");
	while(true)
	{
		cout << "0: Exit\n1:Login" << endl;
		int opt;
		cin >> opt;
		printf("opt = %d\n", opt);
		if(opt == 0) break;
		string name, pass;
		cout << "Please input Username:";
		cin >> name;
		cout << "Please input password";
		cin >> pass;
		if((session = OS.Login(name, pass)) != NULL)
		{
			session->Interact();
		}
	}
	return 0;
}