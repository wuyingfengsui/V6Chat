#include "../../socket_interface/socket_interface.h"

#include "WaitACKMsgSender.h"
#include "../V6ChatDef.h"
#include "../json/json.h"
#include "../Listener/ClientMsgListener.h"
#include "../IPSearcher/IPSearcher.h"

#include <sstream>

using namespace std;
using namespace Json;

WaitACKMsgSender *WaitACKMsgSender::sender = NULL;
CriticalSection WaitACKMsgSender::aCritSect;

WaitACKMsgSender *WaitACKMsgSender::Instance()
{
	SimpleCriticalSection m(aCritSect);

	if(NULL == sender)
		sender = new WaitACKMsgSender;

	return sender;
}

void WaitACKMsgSender::Release()
{
	SimpleCriticalSection m(aCritSect);

	if(sender)
	{
		delete sender;
		sender = NULL;
	}
}


unsigned long WaitACKMsgSender::AddMsg(const string &peerIp, const int peerPort, const string &message, const WAITACKMSGCALLBACKPROC callBack, bool forever)
{
	SimpleCriticalSection m(aCritSect);

	WaitACKMsg newmessage;
	newmessage.mid = midCount++;
	newmessage.peerIp = peerIp;
	stringstream sport, smid;
	sport<<peerPort; smid<<newmessage.mid;
	newmessage.peerPort = sport.str();
	Value json;
	json["mid"] = smid.str();
	json["content"] = message;
	FastWriter fast_writer;
	newmessage.message = fast_writer.write(json);
	newmessage.callBack = callBack;
	if(forever)
		newmessage.leftTryTimes = ETERNAL_TRY_TIMES;
	else
	    newmessage.leftTryTimes = MAX_TRY_TIMES - 1;
	newmessage.lastSendTime = ::GetTickCount();

	waitACKMsgList.push_back(newmessage);

	ClientMsgListener *listener = ClientMsgListener::Instance();
	SOCKET sendSock[2];
	sendSock[0] = listener->GetIPv4UDPSocket();
	sendSock[1] = listener->GetIPv6UDPSocket();
	send_udp_string_by_socket(sendSock[IsIpv6A(peerIp.c_str())], peerIp.c_str(), sport.str().c_str(), newmessage.message.c_str());
	return newmessage.mid;
}

void WaitACKMsgSender::DeleteMsg(bool hasSend, const unsigned long mid, const char *msg, const char *ip)
{
	SimpleCriticalSection m(aCritSect);

	for(MsgIter iter=waitACKMsgList.begin(); iter!=waitACKMsgList.end(); ++iter)
	{
		if(iter->mid == mid)
		{
			if(iter->callBack)
			{
				if(hasSend)
				{
					if(ip)
						iter->callBack(hasSend, ip, iter->peerPort, msg);
					else
						iter->callBack(hasSend, iter->peerIp, iter->peerPort, msg);
				}
				else
					iter->callBack(hasSend, iter->peerIp, iter->peerPort, iter->message);
			}
			waitACKMsgList.erase(iter);
			break;
		}
	}
}

bool WaitACKMsgSender::FindMsg(const unsigned long mid)
{
	SimpleCriticalSection m(aCritSect);

	for(MsgIter iter=waitACKMsgList.begin(); iter!=waitACKMsgList.end(); ++iter)
	{
		if(iter->mid == mid)
			return true;
	}

	return false;
}

void WaitACKMsgSender::TraverseAllMsg()
{
	SimpleCriticalSection m(aCritSect);

	for(MsgIter iter=waitACKMsgList.begin(); iter!=waitACKMsgList.end(); ++iter)
	{
		if(::GetTickCount() - iter->lastSendTime > (3*SEND_WAITACKMSG_LOOP_TIME))
		{
			ClientMsgListener *listener = ClientMsgListener::Instance();
			SOCKET sendSock[2];
			sendSock[0] = listener->GetIPv4UDPSocket();
			sendSock[1] = listener->GetIPv6UDPSocket();

			send_udp_string_by_socket(sendSock[IsIpv6A(iter->peerIp.c_str())], iter->peerIp.c_str(), iter->peerPort.c_str(), iter->message.c_str());

			if(0 == iter->leftTryTimes)
			{
				if(iter->callBack)
					iter->callBack(false, iter->peerIp, iter->peerPort, iter->message);
				iter = waitACKMsgList.erase(iter);

				if(iter == waitACKMsgList.end())
					break;
			}
			else if(ETERNAL_TRY_TIMES != iter->leftTryTimes)
			{
				--iter->leftTryTimes;
				iter->lastSendTime = ::GetTickCount();
			}
		}
	}
}

unsigned int __stdcall WaitACKMsgSender::Loop(LPVOID pParam)
{
	WaitACKMsgSender *sender = WaitACKMsgSender::Instance();
	while(sender)
	{
		sender->TraverseAllMsg();
		::Sleep(SEND_WAITACKMSG_LOOP_TIME);
		sender = WaitACKMsgSender::GetObj();
	}

	return 0;
}