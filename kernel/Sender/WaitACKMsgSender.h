#pragma once
#include <vector>
#include <string>
#include <process.h>

#include "../Lock/SimpleCriticalSection.h"

//�յ���δ�յ�ȷ����Ϣ�Ļص���������һ������ʾ�Ƿ��յ����ڶ�����Ϊ�Է�IP����������Ϊ�Է��˿ڣ����Ĳ����յ�ʱΪ�Է�������ȷ����Ϣ��δ�յ�ʱΪδ�յ�����Ϣ
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
	void DeleteMsg(bool hasSend, const unsigned long mid, const char *msg=NULL, const char *ip=NULL);            // ���IP�ֶ��Է�IPv6�ֶβ�һ��
	bool FindMsg(const unsigned long mid);

private:
	void TraverseAllMsg();

	static WaitACKMsgSender *sender;
	static CriticalSection aCritSect;

	std::vector<WaitACKMsg> waitACKMsgList;
	unsigned long midCount;

	static unsigned int __stdcall Loop(LPVOID pParam);
};