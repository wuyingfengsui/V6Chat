#include "TempUserList.h"

#include "../Lock/SimpleCriticalSection.h"

TempUserList *TempUserList::tempList = NULL;
CriticalSection TempUserList::aCritSect;

TempUserList *TempUserList::Instance()
{
	SimpleCriticalSection m(aCritSect);
	if(NULL == tempList)
		tempList = new TempUserList;

	return tempList;
}

void TempUserList::Release()
{
	SimpleCriticalSection m(aCritSect);
	if(tempList)
	{
		delete tempList;
		tempList = NULL;
	}
}


void TempUserList::AddUser(const TempUser &user)
{
	SimpleCriticalSection m(aCritSect);

	tempUsers.push_back(user);
}

bool TempUserList::FindUser(const std::string &ip)
{
	SimpleCriticalSection m(aCritSect);

	for(UserIter iter=tempUsers.begin(); iter!=tempUsers.end(); ++iter)
	{
		if(iter->ip == ip)
		{
			return true;
		}
	}
	return false;
}

void TempUserList::DeleteUser(const std::string &ip)
{
	SimpleCriticalSection m(aCritSect);

	for(UserIter iter=tempUsers.begin(); iter!=tempUsers.end(); ++iter)
	{
		if(iter->ip == ip)
		{
			iter = tempUsers.erase(iter);
			break;
		}
	}
}