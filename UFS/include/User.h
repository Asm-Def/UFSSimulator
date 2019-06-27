#ifndef USER_H
#define USER_H
#include "UFSParams.h"
#include <string>
struct User
{
	uid_t uid;
	std::string name;
	std::string pass;
};
#endif