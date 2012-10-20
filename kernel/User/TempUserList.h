#pragma once

#include <vector>
#include <string>

#include "../Lock/SimpleCriticalSection.h"

struct TempUser
{
	TempUser(const std::string &ip, int port):ip(ip), port(port){}
    std::string ip;
    int port;
};

class TempUserList
{
public:
	typedef std::vector<TempUser>::iterator UserIter;

	TempUserList()
	{}
	~TempUserList()
	{}

	static TempUserList *Instance();
	static void Release();

	void AddUser(const TempUser &user);
    bool FindUser(const std::string &ip);
	void DeleteUser(const std::string &ip);

private:
	static TempUserList *tempList;
	static CriticalSection aCritSect;

	std::vector<TempUser> tempUsers;
};