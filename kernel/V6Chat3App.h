#pragma once
#include "V6ChatDef.h"
#include "V6Util.h"

#include "Bean\ChatHistoryItem.h"

#include "Handle.h"

#include <string>
#include <vector>

class V6Chat3App
{
public:
	V6Chat3App(void);
	~V6Chat3App(void);

	void AddTray(OS_HOSTWND_HANDLE wnd_handle);
	BOOL GetMyInfo(std::wstring &name, int sex, int age, std::wstring &introduction);
	SelectFileRetCode SelectMyHead();
	BOOL Start(const std::wstring &nick_name, int sex, int age, const std::wstring &introduction);
	void Quit();
	void DeleteTray();

	BOOL GetChatHistory(int uid, bool dest, int mid, int num, std::vector<Handle<ChatHistoryItem>> &his, int &beginMid, int &endMid);

	BOOL GetCursorPos(LPPOINT point);
	
	static V6Chat3App *Instance();

private:
	bool hasStart;
	NOTIFYICONDATA m_trayNID;		//ÍÐÅÌÊý¾Ý
};