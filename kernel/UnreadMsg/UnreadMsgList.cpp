#include "UnreadMsgList.h"

#include "../V6ChatDef.h"
#include "../Lock/SimpleCriticalSection.h"

#include <XLUE.h>

UnreadMsgList *UnreadMsgList::msgList = NULL;
CriticalSection UnreadMsgList::aCritSect;

UnreadMsgList *UnreadMsgList::Instance()
{
	SimpleCriticalSection m(aCritSect);

	if(NULL == msgList)
		msgList = new UnreadMsgList;

	return msgList;
}

void UnreadMsgList::Release()
{
	SimpleCriticalSection m(aCritSect);

	if(msgList)
	{
		delete msgList;
		msgList = NULL;
	}
}


void UnreadMsgList::AddUnreadMsg(const UnreadMsg &msg)
{
	SimpleCriticalSection m(aCritSect);
	messages.push(msg);
}

bool UnreadMsgList::GetUnreadMsg(int &msgType, unsigned int &replyId, std::wstring &replyName, std::string &replyIP, int &replyPort, std::wstring &msg)
{
	SimpleCriticalSection m(aCritSect);

	if(messages.empty())
		return false;

	std::stack<UnreadMsg>::const_reference top = messages.top();
	msgType = top.messageType;
	replyId = top.replyId;
	replyName = top.replyName;
	replyIP = top.replyIP;
	replyPort = top.replyPort;
	msg = top.message;
	return true;
}

bool UnreadMsgList::GetUnreadMsg(int &msgType, unsigned int &replyId, std::wstring &msg)
{
	SimpleCriticalSection m(aCritSect);

	if(messages.empty())
		return false;

	std::stack<UnreadMsg>::const_reference top = messages.top();
	msgType = top.messageType;
	replyId = top.replyId;
	msg = top.message;
	return true;
}

bool UnreadMsgList::GetUnreadMsg(int &msgType, std::wstring &msg)
{
	SimpleCriticalSection m(aCritSect);

	if(messages.empty())
		return false;

	std::stack<UnreadMsg>::const_reference top = messages.top();
	msgType = top.messageType;
	msg = top.message;
	return true;
}

bool UnreadMsgList::GetUnreadMsg(std::wstring &msg)
{
	SimpleCriticalSection m(aCritSect);

	if(messages.empty())
		return false;

	std::stack<UnreadMsg>::const_reference top = messages.top();
	msg = top.message;
	return true;
}

void UnreadMsgList::DeleteUnreadMsg()
{
	SimpleCriticalSection m(aCritSect);

	messages.pop();
}

bool UnreadMsgList::DisplayMsg()
{
	SimpleCriticalSection m(aCritSect);

	if(messages.empty())
		return false;

	std::stack<UnreadMsg>::const_reference top = messages.top();

	XLUE_HOSTWND_HANDLE handle = XLUE_GetHostWndByID(MAIN_WND);
	if(handle)
	{
		HWND wnd_handle = (HWND)XLUE_GetHostWndWindowHandle(handle);
		::PostMessage(wnd_handle, WM_SHOWUNREADMSG, top.messageType, 0);
	}

	return true;
}