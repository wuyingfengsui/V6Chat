#include "P2PUserInfo.h"
#include "../V6Util.h"
#include "../../db_interface/db_interface.h"
#include "../IPSearcher/IPSearcher.h"
#include "../Sender/WaitACKMsgSender.h"

#include <sstream>

bool P2PUserInfo::StoreInDB()
{
	std::stringstream sid, ssex, sage, stcp, sudp;
		sid<<uid;ssex<<sex;sage<<age;stcp<<tcpPort;sudp<<udpPort;
		return (update_peerProfile(sid.str().c_str(), V6Util::UnicodeToUtf8(nick_name.c_str()).c_str(), V6Util::UnicodeToUtf8(group_name.c_str()).c_str(),
			ssex.str().c_str(), sage.str().c_str(), V6Util::UnicodeToUtf8(introduction.c_str()).c_str(), V6Util::UnicodeToUtf8(headImagePath.c_str()).c_str(),
			NULL, NULL, ip.c_str(), stcp.str().c_str(), sudp.str().c_str()) != 0);
}

bool P2PUserInfo::Delete()
{
	SetState(user_offline);

	std::stringstream sid;
	sid << uid;
	return delete_peerProfile(sid.str().c_str());
}

bool P2PUserInfo::CheckOnline()
{
	bool isOnline = true;
	if(user_online == state)
	{
		if((::GetTickCount()-lastActiveTime) > LIVE_TIME)
		{
			SetState(user_offline);
			isOnline = false;
		}
	}
	else if(user_offline == state)
		isOnline = false;

	return isOnline;
}

void P2PUserInfo::SetState(const UserState &state)
{
	this->state = state;

	if(user_online == state)
	{
		if(!ackMsgid)
		{
		    ClientMsgSender sender(ip, -1, udpPort);                                   // 可能有问题
		    ackMsgid = sender.KeepOnlineMsg();
		}
		lastActiveTime = ::GetTickCount();
	}
	else if(user_offline == state)
	{
		if(ackMsgid)
			if(WaitACKMsgSender *sender = WaitACKMsgSender::Instance())                     // 可能有问题
				sender->DeleteMsg(true, ackMsgid);

		ackMsgid = 0;
		lastActiveTime = 0;
	}
}

void P2PUserInfo::GetLocation(std::wstring &location)
{
	if(user_offline == state)
	{
		location = L"离线";
		return;
	}
	else
	{
	    pAddr myAddr = _GetAddress(ip.c_str());
	    if(myAddr)
		{
			location = V6Util::AnsiToUnicode(myAddr->Country) + V6Util::AnsiToUnicode(myAddr->Local);
		}
		else
			location = L"离线";
	}
}