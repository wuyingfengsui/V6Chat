#pragma once
#include <vector>
#include <string>
#include <process.h>

#include "../Lock/SimpleCriticalSection.h"

//收到或未收到确认信息的回调函数，第一参数表示是否收到，第二参数为对方IP，第三参数为对方端口，第四参数收到时为对方发来的确认信息，未收到时为未收到的信息
typedef void (__stdcall* WAITACKMSGCALLBACKPROC)(bool, const std::string &, const std::string &, const std::string &);

struct WaitACKMsg
{
	unsigned long mid;
	int leftTryTimes;
	unsigned long lastSendTime;
	std::string peerIp;
	std::string peerPort;
	std::string message;
	WAITACKMSGCALLBACKPROC callBack;
};

class WaitACKMsgSender
{
public:
	typedef std::vector<WaitACKMsg>::iterator MsgIter;

	WaitACKMsgSender(): midCount(0)
	{ 
		CloseHandle((HANDLE)_beginthreadex(NULL,0,Loop,NULL,0,NULL));
	}
	~WaitACKMsgSender()
	{}

	static WaitACKMsgSender *Instance();
	static void Release();
	static WaitACKMsgSender *GetObj()
	{ return sender; }

	unsigned long AddMsg(const std::string &peerIp, const int peerPort, const std::string &message, const WAITACKMSGCALLBACKPROC callBack, bool forever=false);
	void DeleteMsg(bool hasSend, const unsigned long mid, const char *msg=NULL, const char *ip=NULL);            // 添加IP字段以防IPv6字段不一致
	bool FindMsg(const unsigned long mid);

private:
	void TraverseAllMsg();

	static WaitACKMsgSender *sender;
	static CriticalSection aCritSect;

	std::vector<WaitACKMsg> waitACKMsgList;
	unsigned long midCount;

	static unsigned int __stdcall Loop(LPVOID pParam);
};