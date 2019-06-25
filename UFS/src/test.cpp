#include "../include/INode.h"
#include "../include/TestOS.h"
#include "../include/VHDController.h"
#include <bits/stdc++.h>
using namespace std;
class UnionTest
{
	public:
	virtual void test() = 0;
	bool doTest()
	{
		try { test(); }
		catch(string str)
		{
			cout << typeid(this).name() << ":" << str << endl;
			return false;
		}
		catch(const char *str)
		{
			cout << typeid(this).name() << ":" << str << endl;
			return false;
		}
		return true;
	}
};
class VHDControllerTest : public UnionTest
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
			//cout << x << endl;
			for(int i = 0;i < BLOCK_SIZE-1;++i) buff[i] = (rand() % 26) + 'a';
			buff[BLOCK_SIZE-1] = 0;
			lst.push_back(x);
			buf.push_back(string(buff));
			vhd.WriteBlock(buff, x);
		}
		for(int i = 1;i <= 20;++i)
		{
			vhd.ReadBlock(buff, lst[i-1]);
			assert(strcmp(buf[i-1].c_str(), buff) == 0);
			vhd.FreeBlock(lst[i-1]);
		}
		cout << typeid(this).name() << ": Succeed" << endl;
	}
};

int main()
{
	VHDControllerTest testerVHD;
	testerVHD.doTest();
	return 0;
}
