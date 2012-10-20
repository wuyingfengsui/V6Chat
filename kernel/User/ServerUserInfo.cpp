#include "ServerUserInfo.h"
#include "../V6Util.h"
#include "../../db_interface/db_interface.h"
#include "../Server/ServerList.h"

#include <sstream>

bool ServerUserInfo::StoreInDB()
{
	std::stringstream sid, ssex, sage, sserverId;
	sid<<uid;ssex<<sex;sage<<age;sserverId<<serverid;
	return (update_peerProfile(sid.str().c_str(), V6Util::UnicodeToUtf8(nick_name.c_str()).c_str(), V6Util::UnicodeToUtf8(group_name.c_str()).c_str(),
		ssex.str().c_str(), sage.str().c_str(), V6Util::UnicodeToUtf8(introduction.c_str()).c_str(), V6Util::UnicodeToUtf8(headImagePath.c_str()).c_str(),
		sserverId.str().c_str(), serveruid.c_str(), NULL, NULL, NULL) != 0);
}

bool ServerUserInfo::Delete()
{
	std::stringstream sid;
	sid << uid;

	ServerList *sList = ServerList::Instance();

	Handle<ServerInfo> server;
	if(sList->GetServer(serverid, server))
	{
		server->DeleteUser(serveruid);
		return delete_peerProfile(sid.str().c_str());
	}

	return false;
}

void ServerUserInfo::GetLocation(std::wstring &location)
{
	if(user_offline == state)
	{
		location = L"离线";
		return;
	}
	else
	{
		ServerList *sList = ServerList::Instance();

		Handle<ServerInfo> server;
		if(sList->GetServer(serverid, server))
		{
			location = server->GetName();
		}
		else
			location = L"未知来源";
	}
}


void ServerUserInfo::UpdateProfile()
{
	unsigned long now = ::GetTickCount();
	if(now - lastUpdateTime > LIVE_TIME)
	{
		messageSender->UpdateProfileMsg();
		lastUpdateTime = now;
	}
}