#include "stdafx.h"

#include "V6ChatDef.h"
#include "V6Chat3App.h"
#include "resource.h"
#include "IPSearcher\IPSearcher.h"
#include "V6Util.h"
#include "User\MyInfo.h"
#include "Listener\ClientMsgListener.h"
#include "User\UserList.h"
#include "Server\ServerList.h"
#include "Sender\WaitACKMsgSender.h"
#include "User\TempUserList.h"
#include "UnreadMsg\UnreadMsgList.h"
#include "FileList\RecvFileList.h"
#include "FileList\SendFileList.h"

#include "../socket_interface/socket_interface.h"
#include "../db_interface/db_interface.h"

#include <Shlwapi.h>
#include <Commdlg.h>
#include <string>
#include <sstream>
#include <stack>

using namespace std;

V6Chat3App::V6Chat3App(void)
{
	hasStart = false;
}


V6Chat3App::~V6Chat3App(void)
{
}

BOOL V6Chat3App::GetMyInfo(std::wstring &name, int sex, int age, wstring &introduction)
{
	MyInfo *myInfo = MyInfo::Instance();
	if(!hasStart)
		myInfo->Init();
	name = myInfo->GetName();
	sex = myInfo->GetSex();
	age = myInfo->GetAge();
	introduction = myInfo->GetIntroduction();
	return true;
}

SelectFileRetCode V6Chat3App::SelectMyHead()
{
	wstring szFile;
    SelectFileRetCode ret;

	if (V6Util::SelectFile(L"png文件(*.png)\0*.png\0", L"请选择一张头像图片", szFile))
    {
		HANDLE hFile = CreateFile(szFile.c_str(),GENERIC_READ,
                                   FILE_SHARE_READ|FILE_SHARE_WRITE,NULL, 
                                    OPEN_EXISTING ,FILE_ATTRIBUTE_NORMAL,NULL);
		if(INVALID_HANDLE_VALUE == hFile)
			ret = invalid_file;

		DWORD LowPart = GetFileSize(hFile, NULL);
		if((INVALID_FILE_SIZE == LowPart) || (LowPart > HEAD_IMAGE_SIZE))
		{
			if(INVALID_HANDLE_VALUE != hFile)
				CloseHandle(hFile);
			return size_too_large;
		}

		WCHAR wszModulePath[MAX_PATH];
        GetModuleFileNameW(NULL,wszModulePath,MAX_PATH);
	    PathAppend(wszModulePath, USERDATA_RELATIVE_PATH);
	    PathAppend(wszModulePath, MYHEADIMAGE_PELATIVE_PATH);

		CopyFile(szFile.c_str(), wszModulePath, 0);
		ret = success;

		if(INVALID_HANDLE_VALUE != hFile)
			CloseHandle(hFile);
	}
	else
		ret = user_cancel;

	return ret;
}

void V6Chat3App::AddTray(OS_HOSTWND_HANDLE wnd_handle)
{
	m_trayNID.cbSize = sizeof(NOTIFYICONDATA);
	m_trayNID.hWnd = (HWND)wnd_handle;
	m_trayNID.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	m_trayNID.uCallbackMessage = WM_TRAYMSG;
	wcscpy_s(m_trayNID.szTip, L"V6Chat");
	m_trayNID.hIcon = LoadIcon(::GetModuleHandle(L"kernel.dll"), MAKEINTRESOURCE(IDI_V6ICON));
	Shell_NotifyIcon(NIM_ADD, &m_trayNID);
}

BOOL V6Chat3App::Start(const std::wstring &nick_name, int sex, int age, const std::wstring &introduction)
{
	/**
	   获取个人信息
	**/
	MyInfo *myInfo = MyInfo::Instance();
	myInfo->Init(nick_name, sex, age, introduction);

	/**
	   开始网络部分
	**/
	/**
	   获取服务器信息
	**/
	ServerList *sList = ServerList::Instance();
	sList->Init();
	/*
	//建立组播监听
	SOCKET multicast_sock = build_multicast_listener("ff15::1", "65000", MulticastListenerCallBack);
	//建立UDP监听
	SOCKET udp_sock = build_udp_listener("10000", UDPListenerCallBack);
	//组播个人信息
	bool success = multicast_string(
		"ff15::1", //224.0.0.1,ff15::1
		"65000", 
		255, 
		my_profile.GenerateJsonString().c_str()
		);
		*/
	WaitACKMsgSender *ackMsgSender = WaitACKMsgSender::Instance();
	ClientMsgListener *cleListener = ClientMsgListener::Instance();
	bool cleSuccess = cleListener->Listen(myInfo->GetIPv4TcpPort(), myInfo->GetIPv6TcpPort());

	//监听建立失败则退出程序
	if(!cleSuccess)
	{
		::PostQuitMessage(0);
		return 0;
	}


	myInfo->SetIPv4TcpPort(cleListener->GetIPv4Port());
	myInfo->SetIPv6TcpPort(cleListener->GetIPv6Port());
	
	char **cipv4, **cipv6;
	int ipv4Num = 0, ipv6Num = 0;
	if(get_local_ip(&cipv4, &cipv6, &ipv4Num, &ipv6Num))
	{
		if(ipv4Num > 0)
		{
			myInfo->SetIpv4(cipv4[0]);
			release_socket_array_data((void ***)&cipv4);
		}

		if(ipv6Num > 0)
		{
			myInfo->SetIpv6(cipv6[0]);
			release_socket_array_data((void ***)&cipv6);
		}
	}

	/**
	   获取用户信息
	**/
	UserList *list = UserList::Instance();
	list->Init();

	hasStart = true;
	return true;
}

void V6Chat3App::Quit()
{
	if(hasStart)
	{
		hasStart = false;

		// ClientMsgListener::Release();
		// ServerMsgListener::Release();
		// WaitACKMsgSender::Release();

		UserList *list = UserList::Instance();
		list->Store();
		UserList::Release();
		ServerList *sList = ServerList::Instance();
		sList->Store();
		// ServerList::Release();
		MyInfo *myInfo = MyInfo::Instance();
		myInfo->Store();
		// MyInfo::Release();

		// TempUserList::Release();
		// UnreadMsgList::Release();
		// RecvFileList::Release();
		// SendFileList::Release();
		::PostQuitMessage(0);
	}
	else
		::PostQuitMessage(0);
}

void V6Chat3App::DeleteTray()
{
	Shell_NotifyIcon(NIM_DELETE, &m_trayNID);
}


BOOL V6Chat3App::GetCursorPos(LPPOINT point)
{
	return ::GetCursorPos(point);
}


BOOL V6Chat3App::GetChatHistory(int uid, bool dest, int mid, int num, std::vector<Handle<ChatHistoryItem>> &his, int &beginMid, int &endMid)
{
	int bMid = 0;
	if(dest)
		bMid = mid;
	else
	{
		if(mid == 0)
			bMid = -1;
		else if((bMid = mid - num) < 0)
		{
			bMid = 0;
			num = mid;
		}
	}
	stringstream suid, smid, snum;
	suid << uid;
	smid << bMid;
	snum << num;

	char **hisTable = NULL;
	int rows = 0, cols = 0;
	if(!get_chatItems(suid.str().c_str(), smid.str().c_str(), snum.str().c_str(), &hisTable, &rows, &cols))
		return false;

	if(mid == 0)
	{
		if(rows)
			endMid = atoi(hisTable[CHATHISTORY_DB_ROWS]);

		stack<Handle<ChatHistoryItem>> hisStack;
		int i;
		for(i=1; i<=rows; i++)
		{
			Handle<ChatHistoryItem> item(new ChatHistoryItem);
			if(hisTable[CHATHISTORY_DB_ROWS*i+1])
				item->name = V6Util::Utf8ToUnicode(hisTable[CHATHISTORY_DB_ROWS*i+1]);
			if(hisTable[CHATHISTORY_DB_ROWS*i+2])
				item->content = V6Util::Utf8ToUnicode(hisTable[CHATHISTORY_DB_ROWS*i+2]);
			if(hisTable[CHATHISTORY_DB_ROWS*i+3])
				item->time = hisTable[CHATHISTORY_DB_ROWS*i+3];

			hisStack.push(item);
		}

		if(rows)
			beginMid = atoi(hisTable[CHATHISTORY_DB_ROWS*(i-1)]);

		while(!hisStack.empty())
		{
			his.push_back(hisStack.top());
			hisStack.pop();
		}
	}
	else
	{
		if(rows)
			beginMid = atoi(hisTable[CHATHISTORY_DB_ROWS]);

		int i;
		for(i=1; i<=rows; i++)
		{
			Handle<ChatHistoryItem> item(new ChatHistoryItem);
			if(hisTable[CHATHISTORY_DB_ROWS*i+1])
				item->name = V6Util::Utf8ToUnicode(hisTable[CHATHISTORY_DB_ROWS*i+1]);
			if(hisTable[CHATHISTORY_DB_ROWS*i+2])
				item->content = V6Util::Utf8ToUnicode(hisTable[CHATHISTORY_DB_ROWS*i+2]);
			if(hisTable[CHATHISTORY_DB_ROWS*i+3])
				item->time = hisTable[CHATHISTORY_DB_ROWS*i+3];

			his.push_back(item);
		}

		if(rows)
			endMid = atoi(hisTable[CHATHISTORY_DB_ROWS*(i-1)]);
	}

	release_indb_table_data(hisTable);
	return true;
}


/*

BOOL V6Chat3App::AddFriend(const friend_info **pinfo, int *id, std::wstring &name, std::wstring &location, std::wstring &group_name)
{
	if(!((*pinfo)|| (*pinfo)->Uid == my_profile.Uid))
		return false;

	if(((*pinfo)->state == FRIEND_ONLINE)&& (group_name == L"身边的人"))
	{
	    //组播个人信息
	    multicast_string(
		"ff15::1", 
		"65000", 
		255, 
		my_profile.GenerateJsonString().c_str()
		);
	}

	*id = (*pinfo)->Uid;
	name = (*pinfo)->nick_name;
	location = (*pinfo)->location;
	group_name = (*pinfo)->group_name;

	list<friend_info>::iterator old_friend = FindOutFriendByUid(*id);
	if(old_friend != friends_list.end())
	{
		old_friend->nick_name = (*pinfo)->nick_name;
		old_friend->sex = (*pinfo)->sex;
		old_friend->age = (*pinfo)->age;
		old_friend->introduction = (*pinfo)->introduction;
		old_friend->location = (*pinfo)->location;
		old_friend->state = (*pinfo)->state;
		old_friend->ip = (*pinfo)->ip;
		group_name = old_friend->group_name;             //分组应以本地分组为准
	}
	else
	    friends_list.push_back(**pinfo);

	V6Util::ReleaseData((void *)*pinfo);
	return true;
}

BOOL V6Chat3App::OffFriend(const friend_info **pinfo, int *id, std::wstring &group_name)
{
	if(!(*pinfo))
		return false;

	*id = (*pinfo)->Uid;
	V6Util::ReleaseData((void *)*pinfo);

	list<friend_info>::iterator off_friend = FindOutFriendByUid(*id);
	if(off_friend == friends_list.end())
	    return false;

	group_name = off_friend->group_name;

	off_friend->state = FRIEND_OFFLINE;
	return true;
}

*/


V6Chat3App * V6Chat3App::Instance()
{
	static V6Chat3App ideApp;
	return &ideApp;
}

/*

void V6Chat3App::MulticastListenerCallBack(char const*rec, char const*ip, int ipId)
{
	XLUE_HOSTWND_HANDLE handle = XLUE_GetHostWndByID("MainTab1");

	if(handle)
	{
	    HWND wnd_handle = (HWND)XLUE_GetHostWndWindowHandle(handle);
		if(wnd_handle)
		{
			Json::Reader reader;
			Json::Value json_object;
			if (!reader.parse(rec, json_object))
                return;

			if(json_object["state"].asInt() == FRIEND_ONLINE)
			{
			    friend_info *new_friend = new friend_info;
			    new_friend->Uid = ipId;
			    new_friend->nick_name = V6Util::AnsiToUnicode(json_object["nick_name"].asString().c_str());
			    new_friend->sex = json_object["sex"].asInt();
			    new_friend->age = json_object["age"].asInt();
			    new_friend->introduction = V6Util::AnsiToUnicode(json_object["introduction"].asString().c_str());
			    pAddr myAddr = _GetAddress(ip);
			    if(myAddr)
			    {
			        new_friend->location = V6Util::AnsiToUnicode(myAddr->Country);
			        new_friend->location = new_friend->location + L" " + V6Util::AnsiToUnicode(myAddr->Local);
			    }
			    new_friend->group_name = L"身边的人";

				new_friend->state = FRIEND_ONLINE;
			    new_friend->ip = ip;
			    ::PostMessage(wnd_handle, WM_UPDATEFRIENDLIST, 1, (WPARAM)new_friend);
			}
			else
			{
				friend_info *leave_friend = new friend_info;
			    leave_friend->Uid = ipId;
				leave_friend->ip = ip;

				::PostMessage(wnd_handle, WM_UPDATEFRIENDLIST, 4, (WPARAM)leave_friend);
			}
		}
	}
}

void V6Chat3App::UDPListenerCallBack(char const*rec, char const*ip, int ipId)
{
}

*/