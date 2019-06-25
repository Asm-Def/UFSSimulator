#include "../include/INode.h"
#include "../include/TestOS.h"
#include "../include/VHDController.h"
#include <bits/stdc++.h>
using namespace std;
class VHDControllerTest
{
	VHDController vhd;
	vector<bid_t> lst;
	vector<string> buf;
	char buff[BLOCK_SIZE];
	public:
	VHDControllerTest()
	{
		vhd.Create((disksize_t) 1024, "1.vhd");
		vhd.Format();
	}
	void test()
	{
		for(int i = 1;i <= 20;++i)
		{
			bid_t x;
			vhd.AllocBlock(x);
			cout << x << endl;
			for(int i = 0;i < BLOCK_SIZE-1;++i) buff[i] = (rand() % 26) + 'a';
			buff[BLOCK_SIZE-1] = 0;
			lst.push_back(x);
			buf.push_back(string(buff));
			vhd.WriteBlock(buff, x);
		}
		for(int i = 1;i <= 20;++i)
		{
			cout << buf[i-1] << endl;
			vhd.ReadBlock(buff, lst[i-1]);
			cout << buff << endl;
			vhd.FreeBlock(lst[i-1]);
		}
	}
};

int main()
{
	VHDControllerTest testerVHD;
	try
	{
		testerVHD.test();
	}
	catch(string str)
	{
		cout << str << endl;
	}
	catch(const char *str)
	{
		cout << str << endl;
	}
	return 0;
}
