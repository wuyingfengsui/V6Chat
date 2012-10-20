#include "stdafx.h"
#include "LuaV6Chat3App.h"
#include "V6Util.h"
#include "V6ChatDef.h"
#include "User\UserList.h"
#include "Server\ServerList.h"
#include "User\P2PUserInfo.h"
#include "Sender\ClientMsgSender.h"
#include "Sender\ServerMsgSender.h"
#include "UnreadMsg\UnreadMsgList.h"
#include "FileList\SendFileList.h"
#include "FileList\RecvFileList.h"
#include "User\MyInfo.h"
#include "Handle.h"

#include <assert.h>
#include <string>
#include <sstream>
#include <Shlwapi.h>

using namespace std;

LuaV6Chat3App::LuaV6Chat3App(void)
{
}


LuaV6Chat3App::~LuaV6Chat3App(void)
{
}


V6Chat3App* __stdcall LuaV6Chat3App::Instance( void* )
{
	return V6Chat3App::Instance();
}

int LuaV6Chat3App::GetMyHead(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		XL_BITMAP_HANDLE myHead = V6Util::GetUserDataImageHandle(MYHEADIMAGE_PELATIVE_PATH);
		if(!XLUE_PushBitmap(luaState, myHead))
			lua_pushnil(luaState);
		return 1;                                          //返回值个数
	}

	lua_pushnil(luaState);
	return 1;
}

int LuaV6Chat3App::GetMyInfo(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		int sex=0, age=0;
		wstring name, introduction;
		
		if(App->GetMyInfo(name, sex, age, introduction))
		{
			string nameU = V6Util::UnicodeToUtf8(name.c_str());
			string introductionU = V6Util::UnicodeToUtf8(introduction.c_str());

			lua_pushstring(luaState, nameU.c_str());
			lua_pushnumber(luaState, sex);
			lua_pushnumber(luaState, age);
			lua_pushstring(luaState, introductionU.c_str());
			return 4;
		}
	}

	lua_pushnil(luaState);
	lua_pushnil(luaState);
	lua_pushnil(luaState);
	lua_pushnil(luaState);
	return 4;
}

int LuaV6Chat3App::GetMyIpAndPort(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		MyInfo *mi = MyInfo::Instance();
		lua_pushstring(luaState, mi->GetIpv4().c_str());
		lua_pushnumber(luaState, mi->GetIPv4TcpPort());
		lua_pushstring(luaState, mi->GetIpv6().c_str());
		lua_pushnumber(luaState, mi->GetIPv6TcpPort());

		return 4;
	}

	lua_pushnil(luaState);
	lua_pushnil(luaState);
	lua_pushnil(luaState);
	lua_pushnil(luaState);
	return 4;
}

int LuaV6Chat3App::SetMyIntroduction(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		size_t introduction_len=lua_objlen(luaState, 2);
		const char *introduction = lua_tolstring(luaState, 2, &introduction_len);
		MyInfo *mi = MyInfo::Instance();
		mi->SetIntroduction(V6Util::Utf8ToUnicode(introduction));
	}

	return 0;
}

int LuaV6Chat3App::SelectMyHead(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		SelectFileRetCode ret = App->SelectMyHead();
		lua_pushnumber(luaState, ret);

		if(ret == success)
		{
			XL_BITMAP_HANDLE myHead = V6Util::GetUserDataImageHandle(MYHEADIMAGE_PELATIVE_PATH);
		    if(!XLUE_PushBitmap(luaState, myHead))
			    lua_pushnil(luaState);
		}
		else
			lua_pushnil(luaState);

		return 2;
	}

	lua_pushnil(luaState);
	lua_pushnil(luaState);
	return 2;
}

int LuaV6Chat3App::AddTray(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		OS_HOSTWND_HANDLE wnd_handle = lua_touserdata(luaState, 2);
		App->AddTray(wnd_handle);
	}
	return 0;
}

int LuaV6Chat3App::Start(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		size_t name_len=lua_objlen(luaState, 2);
		const char *nick_name = lua_tolstring(luaState, 2, &name_len);
		int sex = static_cast<int>(lua_tonumber(luaState, 3));
		int age = static_cast<int>(lua_tonumber(luaState, 4));
		size_t introduction_len=lua_objlen(luaState, 5);
		const char *introduction = lua_tolstring(luaState, 5, &introduction_len);

		wstring wide_nick_name = V6Util::Utf8ToUnicode(nick_name);
		wstring wide_introduction = V6Util::Utf8ToUnicode(introduction);
		BOOL retCode = App->Start(wide_nick_name, sex, age, wide_introduction);

		lua_pushnumber(luaState, retCode);
		return 1;
	}

	lua_pushnil(luaState);
	return 1;
}

int LuaV6Chat3App::StartServer(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		ServerList *sList = ServerList::Instance();
		sList->Connect();
	}
	return 0;
}

int LuaV6Chat3App::Quit( lua_State *luaState )
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		App->Quit();
	}
	return 0;
}

int LuaV6Chat3App::DeleteTray(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		App->DeleteTray();
	}
	return 0;
}

int LuaV6Chat3App::GetCursorPos(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		tagPOINT point;
		BOOL success = App->GetCursorPos(&point);

		if(success == 1)
		{
			lua_pushnumber(luaState, point.x);
			lua_pushnumber(luaState, point.y);
		}
		else
		{
			lua_pushnil(luaState);
			lua_pushnil(luaState);
		}

		return 2;
	}

	lua_pushnil(luaState);
	lua_pushnil(luaState);
	return 2;
}

int LuaV6Chat3App::GetWorkareaPos(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		RECT point;
		BOOL success = SystemParametersInfo(SPI_GETWORKAREA,0,&point,0);


		if(success == 1)
		{
			lua_pushnumber(luaState, point.right - point.left);
			lua_pushnumber(luaState, point.bottom - point.top);
		}
		else
		{
			lua_pushnil(luaState);
			lua_pushnil(luaState);
		}

		return 2;
	}

	lua_pushnil(luaState);
	lua_pushnil(luaState);
	return 2;
}

int LuaV6Chat3App::GetImageHandle(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		size_t path_len=lua_objlen(luaState, 2);
		const char *path = lua_tolstring(luaState, 2, &path_len);

		XL_BITMAP_HANDLE image = V6Util::GetImageHandle(V6Util::Utf8ToUnicode(path).c_str());
		if(!XLUE_PushBitmap(luaState, image))
			lua_pushnil(luaState);
		return 1;
	}

	lua_pushnil(luaState);
	return 1;
}

int LuaV6Chat3App::OpenShellExecute(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		size_t path_len=lua_objlen(luaState, 2);
		const char *path = lua_tolstring(luaState, 2, &path_len);

		XLUE_HOSTWND_HANDLE handle = XLUE_GetHostWndByID(MAIN_WND);
		if(handle)
		{
			HWND wnd_handle = (HWND)XLUE_GetHostWndWindowHandle(handle);
			::ShellExecute(wnd_handle, L"open", V6Util::Utf8ToUnicode(path).c_str(), NULL, NULL , SW_SHOW);
		}
	}

	return 0;
}

int LuaV6Chat3App::PostWindowMessage(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		size_t wndId_len=lua_objlen(luaState, 2);
		const char *wndId = lua_tolstring(luaState, 2, &wndId_len);
		int msg = static_cast<int>(lua_tonumber(luaState, 3));
		int wp = static_cast<int>(lua_tonumber(luaState, 4));
		int lp = static_cast<int>(lua_tonumber(luaState, 5));

		XLUE_HOSTWND_HANDLE handle = XLUE_GetHostWndByID(wndId);
		if(handle)
		{
			HWND wnd_handle = (HWND)XLUE_GetHostWndWindowHandle(handle);
			::PostMessage(wnd_handle, msg, wp, lp);
		}
	}

	return 0;
}

int LuaV6Chat3App::FileExist(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		size_t pathname_len=lua_objlen(luaState, 2);
		const char *pathname = lua_tolstring(luaState, 2, &pathname_len);

		int exist = 0;
		HANDLE h = ::CreateFile(V6Util::Utf8ToUnicode(pathname).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(h != INVALID_HANDLE_VALUE)
		{
			exist = 1;
			CloseHandle(h);
		}

		lua_pushnumber(luaState, exist);
	}

	lua_pushnil(luaState);
	return 1;
}

int LuaV6Chat3App::CopyToClipboard(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		size_t wndId_len=lua_objlen(luaState, 2);
		const char *wndId = lua_tolstring(luaState, 2, &wndId_len);

		XLUE_HOSTWND_HANDLE handle = XLUE_GetHostWndByID(wndId);
		if(handle)
		{
			HWND wnd_handle = (HWND)XLUE_GetHostWndWindowHandle(handle);
			size_t content_len = lua_objlen(luaState, 3);
			const char *content = lua_tolstring(luaState, 3, &content_len);

			if(content && OpenClipboard(wnd_handle))  // 打开剪切板
			{
				EmptyClipboard();  // 清空剪切板,让当前窗口进程拥有剪切板
				string ansicontent(V6Util::Utf8ToAnsi(content));
				if(HANDLE hClip=GlobalAlloc(GMEM_MOVEABLE, ansicontent.length()+1)) // 分配内存对象返回地址(该函数是系统从全局堆中分配内存)
				{
					char* pBuf=(char*)GlobalLock(hClip); // 锁定全局内存中指定的内存块，并返回一个地址值，令其指向内存块的起始处
					strcpy_s(pBuf, ansicontent.length()+1, ansicontent.c_str()); // 将Str对象中的数据Copy到内存空间中
					GlobalUnlock(hClip); // 解锁全局内存块
					SetClipboardData(CF_TEXT,hClip); // 设置剪贴板数据
				}
				CloseClipboard(); // 关闭

				lua_pushboolean(luaState, true);
				return 1;
			}
		}
	}

	lua_pushboolean(luaState, false);
	return 1;
}

int LuaV6Chat3App::GetUserList(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		UserList *list = UserList::Instance();

		lua_newtable(luaState);       //创建一个表格，放在栈顶
		int i=1;
		map<unsigned int, Handle<UserInfo>>& users = list->GetUsers();
		for(UserList::UserIter iter=users.begin(); iter!=users.end(); ++iter, ++i)
		{
			lua_pushnumber(luaState, i);
			lua_newtable(luaState);      //压入value,也是一个table（subtable）
			lua_pushstring(luaState, "groupname");
			lua_pushstring(luaState, V6Util::UnicodeToUtf8(iter->second->group_name.c_str()).c_str());
			lua_settable(luaState,-3);   //弹出key,value，并设置到subtable里面去
			lua_pushstring(luaState, "id");
			lua_pushnumber(luaState, iter->second->uid);
			lua_settable(luaState,-3);   //弹出key,value，并设置到subtable里面去
			lua_pushstring(luaState, "nickname");
			lua_pushstring(luaState, V6Util::UnicodeToUtf8(iter->second->nick_name.c_str()).c_str());
			lua_settable(luaState,-3);   //弹出key,value，并设置到subtable里面去
			lua_pushstring(luaState, "location");
			wstring location;
			iter->second->GetLocation(location);
			lua_pushstring(luaState, V6Util::UnicodeToUtf8(location.c_str()).c_str());
			lua_settable(luaState,-3);   //弹出key,value，并设置到subtable里面去
			wstring headImagePath(USERHEAD_RELATIVE_PATH);
			wstringstream sid;
			sid<<iter->second->uid;
			headImagePath = headImagePath + L"\\" + sid.str();
			XL_BITMAP_HANDLE bmp = V6Util::GetUserDataImageHandle(headImagePath.c_str());
			if(bmp)
			{
				lua_pushstring(luaState, "head");
				if(!XLUE_PushBitmap(luaState, bmp))
			        lua_pushnil(luaState);
			    lua_settable(luaState,-3);   //弹出key,value，并设置到subtable里面去
			}
			lua_settable(luaState,-3);//这时候父table的位置还是-3,弹出key,value(subtable),并设置到table里去
		}

		return 1;
	}

	lua_pushnil(luaState);
	return 1;
}

int LuaV6Chat3App::GetUserInfoByID(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		int id = lua_tointeger(luaState, 2);
		UserList *list = UserList::Instance();
		wstring nick_name, group_name, introduction, location;
		unsigned short sex = 0;
		unsigned int age = 0;

		if(list->GetUser(id, nick_name, group_name, sex, age, introduction, location))
		{
			lua_newtable(luaState);
			lua_pushstring(luaState, "name");
			lua_pushstring(luaState, V6Util::UnicodeToUtf8(nick_name.c_str()).c_str());
			lua_settable(luaState,-3);
			lua_pushstring(luaState, "groupname");
			lua_pushstring(luaState, V6Util::UnicodeToUtf8(group_name.c_str()).c_str());
			lua_settable(luaState,-3);
			lua_pushstring(luaState, "sex");
			lua_pushnumber(luaState, sex);
			lua_settable(luaState,-3);
			lua_pushstring(luaState, "age");
			lua_pushnumber(luaState, age);
			lua_settable(luaState,-3);
			lua_pushstring(luaState, "introduction");
			lua_pushstring(luaState, V6Util::UnicodeToUtf8(introduction.c_str()).c_str());
			lua_settable(luaState,-3);
			lua_pushstring(luaState, "location");
			lua_pushstring(luaState, V6Util::UnicodeToUtf8(location.c_str()).c_str());
			lua_settable(luaState,-3);
			wstring headImagePath(USERHEAD_RELATIVE_PATH);
			wstringstream sid;
			sid<<id;
			headImagePath = headImagePath + L"\\" + sid.str();
			XL_BITMAP_HANDLE bmp = V6Util::GetUserDataImageHandle(headImagePath.c_str());
			if(bmp)
			{
				lua_pushstring(luaState, "head");
				if(!XLUE_PushBitmap(luaState, bmp))
			        lua_pushnil(luaState);
			    lua_settable(luaState,-3);   //弹出key,value，并设置到subtable里面去
			}
			return 1;
		}
	}
	lua_pushnil(luaState);
	return 1;
}

int LuaV6Chat3App::GetUnreadMessage(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		UnreadMsgList *list = UnreadMsgList::Instance();
		wstring unread;
		int type;
		unsigned int uid;
		if(list->GetUnreadMsg(type, uid, unread))
		{
			lua_pushstring(luaState, V6Util::UnicodeToUtf8(unread.c_str()).c_str());
			lua_pushnumber(luaState, type);
			lua_pushnumber(luaState, uid);
			return 3;
		}
	}

	lua_pushnil(luaState);
	lua_pushnil(luaState);
	lua_pushnil(luaState);
	return 3;
}

int LuaV6Chat3App::DeleteUnreadMessage(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		UnreadMsgList *list = UnreadMsgList::Instance();
		list->DeleteUnreadMsg();
	}

	return 0;
}


int LuaV6Chat3App::AddServer(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		size_t name_len=lua_objlen(luaState, 2);
		const char *name = lua_tolstring(luaState, 2, &name_len);
		size_t admain_len=lua_objlen(luaState, 3);
		const char *admain = lua_tolstring(luaState, 3, &admain_len);
		size_t account_len=lua_objlen(luaState, 4);
		const char *account = lua_tolstring(luaState, 4, &account_len);
		size_t password_len=lua_objlen(luaState, 5);
		const char *password = lua_tolstring(luaState, 5, &password_len);
		size_t introduction_len=lua_objlen(luaState, 6);
		const char *introduction = lua_tolstring(luaState, 6, &introduction_len);


		Handle<ServerInfo> server(new ServerInfo(
			0,
			V6Util::Utf8ToUnicode(name).c_str(),
			admain,
			DEFAULT_XMPP_PORT,
			V6Util::Utf8ToUnicode(introduction).c_str(),
			account,
			password
			));

		ServerList *list = ServerList::Instance();
		list->AddServer(server);
	}

	return 0;
}

int LuaV6Chat3App::GetServerList(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		ServerList *list = ServerList::Instance();

		lua_newtable(luaState);       // 创建一个表格，放在栈顶
		int i=1;
		std::map<unsigned int, Handle<ServerInfo>>& servers = list->GetServers();
		for(ServerList::ServerIter iter=servers.begin(); iter!=servers.end(); ++iter, ++i)
		{
			lua_pushnumber(luaState, i);
			lua_newtable(luaState);      // 压入value,也是一个table（subtable）
			lua_pushstring(luaState, "id");
			lua_pushnumber(luaState, iter->second->GetId());
			lua_settable(luaState,-3);   // 弹出key,value，并设置到subtable里面去
			lua_pushstring(luaState, "server_name");
			lua_pushstring(luaState, V6Util::UnicodeToUtf8(iter->second->GetName().c_str()).c_str());
			lua_settable(luaState,-3);   // 弹出key,value，并设置到subtable里面去
			lua_pushstring(luaState, "introduction");
			lua_pushstring(luaState, V6Util::UnicodeToUtf8(iter->second->GetDescription().c_str()).c_str());
			lua_settable(luaState,-3);   // 弹出key,value，并设置到subtable里面去
			lua_pushstring(luaState, "domain");
			lua_pushstring(luaState, iter->second->GetIp().c_str());
			lua_settable(luaState,-3);   // 弹出key,value，并设置到subtable里面去
			lua_pushstring(luaState, "state");
			lua_pushnumber(luaState, iter->second->GetState());
			lua_settable(luaState,-3);   // 弹出key,value，并设置到subtable里面去
			lua_settable(luaState,-3); // 这时候父table的位置还是-3,弹出key,value(subtable),并设置到table里去
		}

		return 1;
	}

	lua_pushnil(luaState);
	return 1;
}

int LuaV6Chat3App::GetServerInfoByID(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		int id = lua_tointeger(luaState, 2);
		ServerList *list = ServerList::Instance();
		Handle<ServerInfo> server;

		if(list->GetServer(id, server))
		{
			lua_newtable(luaState);
			lua_pushstring(luaState, "id");
			lua_pushnumber(luaState, server->GetId());
			lua_settable(luaState,-3);   // 弹出key,value，并设置到subtable里面去
			lua_pushstring(luaState, "server_name");
			lua_pushstring(luaState, V6Util::UnicodeToUtf8(server->GetName().c_str()).c_str());
			lua_settable(luaState,-3);   // 弹出key,value，并设置到subtable里面去
			lua_pushstring(luaState, "account");
			lua_pushstring(luaState, server->GetUid().c_str());
			lua_settable(luaState,-3);   // 弹出key,value，并设置到subtable里面去
			lua_pushstring(luaState, "password");
			lua_pushstring(luaState, server->GetUpassword().c_str());
			lua_settable(luaState,-3);   // 弹出key,value，并设置到subtable里面去
			lua_pushstring(luaState, "introduction");
			lua_pushstring(luaState, V6Util::UnicodeToUtf8(server->GetDescription().c_str()).c_str());
			lua_settable(luaState,-3);   // 弹出key,value，并设置到subtable里面去
			lua_pushstring(luaState, "domain");
			lua_pushstring(luaState, server->GetIp().c_str());
			lua_settable(luaState,-3);   // 弹出key,value，并设置到subtable里面去
			lua_pushstring(luaState, "state");
			lua_pushnumber(luaState, server->GetState());
			lua_settable(luaState,-3);   // 弹出key,value，并设置到subtable里面去
			return 1;
		}
	}
	lua_pushnil(luaState);
	return 1;
}

int LuaV6Chat3App::ChangeServerInfo(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		int id = lua_tointeger(luaState, 2);
		ServerList *list = ServerList::Instance();
		Handle<ServerInfo> server;

		if(list->GetServer(id, server))
		{
			size_t name_len=lua_objlen(luaState, 3);
			const char *name = lua_tolstring(luaState, 3, &name_len);
			size_t admain_len=lua_objlen(luaState, 4);
			const char *admain = lua_tolstring(luaState, 4, &admain_len);
			size_t account_len=lua_objlen(luaState, 5);
			const char *account = lua_tolstring(luaState, 5, &account_len);
			size_t password_len=lua_objlen(luaState, 6);
			const char *password = lua_tolstring(luaState, 6, &password_len);
			size_t introduction_len=lua_objlen(luaState, 7);
			const char *introduction = lua_tolstring(luaState, 7, &introduction_len);

			if(name)
				server->SetName(V6Util::Utf8ToUnicode(name));
			if(admain)
				server->SetIp(admain);
			if(account)
				server->SetUid(account);
			if(password)
				server->SetUpassword(password);
			if(introduction)
				server->SetDescription(V6Util::Utf8ToUnicode(introduction));

			if(server->HasChange())
			{
				server->DisConnect();
				server->Connect();
			}
		}
	}
	return 0;
}

int LuaV6Chat3App::SearchFriendOnServer(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		int id = lua_tointeger(luaState, 2);

		ServerList *list = ServerList::Instance();
		Handle<ServerInfo> server;

		if(list->GetServer(id, server))
		{
			server->ClearSearchUsersResult();

			size_t search_content_len=lua_objlen(luaState, 3);
			const char *search_content = lua_tolstring(luaState, 3, &search_content_len);
			server->AskUserInfo(search_content);
		}
	}

	return 0;
}

int LuaV6Chat3App::GetSearchFriendResult(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		int id = lua_tointeger(luaState, 2);

		ServerList *list = ServerList::Instance();
		Handle<ServerInfo> server;

		if(list->GetServer(id, server))
		{
			std::list<VCardInfo> vCardList;
			server->GetSearchUsersResult(vCardList);

			lua_newtable(luaState);       // 创建一个表格，放在栈顶
			int i=1;
			for(std::list<VCardInfo>::const_iterator iter=vCardList.begin(); iter!=vCardList.end(); ++iter, ++i)
			{
				lua_pushnumber(luaState, i);
				lua_newtable(luaState);      // 压入value,也是一个table（subtable）
				lua_pushstring(luaState, "id");
				lua_pushnumber(luaState, iter->uid);
				lua_settable(luaState,-3);   // 弹出key,value，并设置到subtable里面去
				lua_pushstring(luaState, "account");
				lua_pushstring(luaState, iter->account.c_str());
				lua_settable(luaState,-3);   // 弹出key,value，并设置到subtable里面去
				lua_pushstring(luaState, "formatted_name");
				lua_pushstring(luaState, iter->formattedName.c_str());
				lua_settable(luaState,-3);   // 弹出key,value，并设置到subtable里面去
				wstring headImagePath(USERHEAD_RELATIVE_PATH);
				headImagePath = headImagePath + L"\\" + iter->photoRelativePath;
				XL_BITMAP_HANDLE bmp = V6Util::GetUserDataImageHandle(headImagePath.c_str());
				if(bmp)
				{
					lua_pushstring(luaState, "head");
					if(!XLUE_PushBitmap(luaState, bmp))
						lua_pushnil(luaState);
					lua_settable(luaState,-3);   //弹出key,value，并设置到subtable里面去
				}
				lua_settable(luaState,-3); // 这时候父table的位置还是-3,弹出key,value(subtable),并设置到table里去
			}

			return 1;
		}
	}
    lua_pushnil(luaState);
	return 1;
}


int LuaV6Chat3App::AddP2PFriend(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		size_t ip_len=lua_objlen(luaState, 2);
		const char *ip = lua_tolstring(luaState, 2, &ip_len);
		size_t port_len=lua_objlen(luaState, 3);
		const char *port = lua_tolstring(luaState, 3, &port_len);
		string sip(ip);

		ClientMsgSender sender(ip, -1, atoi(port));
		sender.AddUserAskMsg();
	}

	return 0;
}

int LuaV6Chat3App::AddServerFriend(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		int id = lua_tointeger(luaState, 2);
		size_t account_len=lua_objlen(luaState, 3);
		const char *account = lua_tolstring(luaState, 3, &account_len);

		ServerMsgSender sender(id, account, 0);
		sender.AddUserAskMsg();
	}
	return 0;
}

int LuaV6Chat3App::AddFriendAck(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		int choice = lua_tointeger(luaState, 2);

		UnreadMsgList *msglist = UnreadMsgList::Instance();
		int type, port;
		unsigned int repid;
		wstring name, msg;
		string ip;
		msglist->GetUnreadMsg(type, repid, name, ip, port, msg);
		if(type == ADD_P2PUSER_ASK)
		{
			ClientMsgSender sender(ip, -1, port);
			sender.ReplyAddUserMsg(choice == 1);

			if(choice == 1)
			{
				Handle<UserInfo> newuser(new P2PUserInfo(0, name, DEFAULT_GROUP_NAME, 0, 0, L"", L"", ip, port, port));

				UserList *userlist = UserList::Instance();
				userlist->AddUser(newuser);
				sender.TestOnlineMsg();
			}
		}
		else
		{
			ServerMsgSender sender(repid, V6Util::UnicodeToUtf8(name.c_str()), 0);
			sender.ReplyAddUserMsg(choice == 1);
		}
	}

	return 0;
}

int LuaV6Chat3App::ChangeFriendGroup(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		lua_Integer uid = lua_tointeger(luaState, 2);

		size_t group_name_len=lua_objlen(luaState, 3);
		const char *group_name = lua_tolstring(luaState, 3, &group_name_len);

		UserList *userlist = UserList::Instance();
		userlist->UpdateUser(uid, NULL, &V6Util::Utf8ToUnicode(group_name), 3, 0, NULL, NULL);
	}

	return 0;
}

int LuaV6Chat3App::DeleteFriend(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		lua_Integer uid = lua_tointeger(luaState, 2);
		
		UserList *userlist = UserList::Instance();
		userlist->DeleteUser(uid);
	}

	return 0;
}


int LuaV6Chat3App::ChatWithText(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		lua_Integer uid = lua_tointeger(luaState, 2);

		size_t text_len=lua_objlen(luaState, 3);
		const char *text = lua_tolstring(luaState, 3, &text_len);

		wstring wtext = V6Util::Utf8ToUnicode(text);
		string st;

		UserList *list = UserList::Instance();
		list->RecordChatText(uid, wtext, st);
		Handle<MsgSender> sender;
		list->GetUserSender(uid, sender);
		sender->ChatWithTextMsg(wtext, st);

		lua_pushstring(luaState, st.c_str());
		return 1;
	}

	lua_pushnil(luaState);
	return 1;
}

int LuaV6Chat3App::SelectChatFile(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		wstring szFile;

		if (V6Util::SelectFile(L"文件(*.*)\0*.*\0", L"请选择要发送的文件", szFile))
		{
			SelectFileRetCode ret;

			HANDLE hFile = CreateFile(szFile.c_str(),GENERIC_READ,
                                   FILE_SHARE_READ|FILE_SHARE_WRITE,NULL, 
                                    OPEN_EXISTING ,FILE_ATTRIBUTE_NORMAL,NULL);
			if(INVALID_HANDLE_VALUE == hFile)
				ret = invalid_file;

			DWORD LowPart = GetFileSize(hFile, NULL);
			if((INVALID_FILE_SIZE == LowPart) || (LowPart > SMALL_FILE_MAXSIZE))
				ret = size_too_large;
			else
				ret = success;

			if(INVALID_HANDLE_VALUE != hFile)
				CloseHandle(hFile);

			lua_pushnumber(luaState, ret);

			if(success == ret)
			{
				lua_pushstring(luaState, V6Util::UnicodeToUtf8(szFile.c_str()).c_str());

				SendFileList *fl = SendFileList::Instance();
				lua_pushnumber(luaState, fl->AddFileItem(szFile));
			}
			else
			{
				lua_pushnil(luaState);
				lua_pushnil(luaState);
			}
		}
		else
		{
			lua_pushnumber(luaState, user_cancel);
			lua_pushnil(luaState);
			lua_pushnil(luaState);
		}

		return 3;
	}

	lua_pushnil(luaState);
	lua_pushnil(luaState);
	lua_pushnil(luaState);
	return 3;
}

int LuaV6Chat3App::GetSendFile(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		lua_Integer sid = lua_tointeger(luaState, 2);

		wstring path;

		SendFileList *fl = SendFileList::Instance();
		if(fl->GetFilePath(sid, path))
			lua_pushstring(luaState, V6Util::UnicodeToUtf8(path.c_str()).c_str());
		else
			lua_pushnil(luaState);

		return 1;
	}

	lua_pushnil(luaState);
	return 1;
}

int LuaV6Chat3App::GetRecvFile(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		WCHAR wszModulePath[MAX_PATH];
		GetModuleFileName(NULL,wszModulePath,MAX_PATH);
		PathAppend(wszModulePath, USERDATA_RELATIVE_PATH);
		PathAppend(wszModulePath, USERHEAD_RECVFILE_PATH);
		wstring path = wszModulePath;
		path = path + L"\\";

		RecvFileList *fl = RecvFileList::Instance();
		wstring fileName;

		size_t sid_len=lua_objlen(luaState, 2);
		const char *sid = lua_tolstring(luaState, 2, &sid_len);

		if(int nid = atoi(sid))
		{
			if(fl->GetFilePath(nid, fileName))
			{
				path += fileName;
				lua_pushstring(luaState, V6Util::UnicodeToUtf8(path.c_str()).c_str());
			}
			else
				lua_pushnil(luaState);
		}
		else if(sid)                  // 使用Server的情况
		{
			size_t uid_len=lua_objlen(luaState, 3);
			const char *uid = lua_tolstring(luaState, 3, &uid_len);

			if(uid && fl->GetDownloadedFilePath(atoi(uid), sid, fileName))
			{
				path = fileName;
				lua_pushstring(luaState, V6Util::UnicodeToUtf8(path.c_str()).c_str());
			}
			else
				lua_pushnil(luaState);
		}
		else
			lua_pushnil(luaState);

		return 1;
	}

	lua_pushnil(luaState);
	return 1;
}

int LuaV6Chat3App::GetChatContent(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		lua_Integer uid = lua_tointeger(luaState, 2);

		UserList *list = UserList::Instance();

		lua_newtable(luaState);       //创建一个表格，放在栈顶
		int i=1;
		wstring content;
		string time;
		int type = 0;
		while(list->GetChatContent(uid, content, time, type))
		{
			lua_pushnumber(luaState, i);
			lua_newtable(luaState);
			lua_pushstring(luaState, "time");
			lua_pushstring(luaState, time.c_str());
			lua_settable(luaState,-3);
			lua_pushstring(luaState, "message");
			lua_pushstring(luaState, V6Util::UnicodeToUtf8(content.c_str()).c_str());
			lua_settable(luaState,-3);
			lua_pushstring(luaState, "message_type");
			lua_pushinteger(luaState, type);
			lua_settable(luaState,-3);
			lua_settable(luaState,-3);//这时候父table的位置还是-3,弹出key,value(subtable),并设置到table里去
			++i;
		}
		return 1;
	}

	lua_pushnil(luaState);
	return 1;
}

int LuaV6Chat3App::GetChatHistory(lua_State *luaState)
{
	V6Chat3App *App = GetV6Chat3App(luaState);
	if (App)
	{
		lua_Integer uid = lua_tointeger(luaState, 2);
		bool dest = (lua_toboolean(luaState, 3) == 1);
		lua_Integer mid = lua_tointeger(luaState, 4);
		int num = EACH_CHATHISTORYITEM_NUM;
		vector<Handle<ChatHistoryItem>> his;
		int beginMid = 0, endMid = 0;

		if(App->GetChatHistory(uid, dest, mid, num, his, beginMid, endMid))
		{
			lua_newtable(luaState);       //创建一个表格，放在栈顶
			int i=1;
			for(vector<Handle<ChatHistoryItem>>::iterator iter = his.begin(); iter != his.end(); ++iter, ++i)
			{
				lua_pushnumber(luaState, i);
				lua_newtable(luaState);
				lua_pushstring(luaState, "name");
				lua_pushstring(luaState, V6Util::UnicodeToUtf8((*iter)->name.c_str()).c_str());
				lua_settable(luaState,-3);
				lua_pushstring(luaState, "time");
				lua_pushstring(luaState, (*iter)->time.c_str());
				lua_settable(luaState,-3);
				lua_pushstring(luaState, "message");
				lua_pushstring(luaState, V6Util::UnicodeToUtf8((*iter)->content.c_str()).c_str());
				lua_settable(luaState,-3);
				lua_settable(luaState,-3);//这时候父table的位置还是-3,弹出key,value(subtable),并设置到table里去
			}

			lua_pushinteger(luaState, beginMid);
			lua_pushinteger(luaState, endMid);
			return 3;
		}
	}

	lua_pushnil(luaState);
	lua_pushnil(luaState);
	lua_pushnil(luaState);
	return 3;
}


V6Chat3App* LuaV6Chat3App::GetV6Chat3App( lua_State* luaState )
{
	V6Chat3App** lplpLuaObj = reinterpret_cast<V6Chat3App**>(luaL_checkudata(luaState,1, XUNLEI_IDEAPP_CLASS));   
	if(lplpLuaObj != NULL)
	{
		return *lplpLuaObj;
	}

	return NULL;   
}

static XLLRTGlobalAPI LuaIDEAppMemberFunctions[] = 
{
	{"GetCursorPos", LuaV6Chat3App::GetCursorPos },
	{"GetWorkareaPos", LuaV6Chat3App::GetWorkareaPos },
	{"GetImageHandle", LuaV6Chat3App::GetImageHandle },
	{"OpenShellExecute", LuaV6Chat3App::OpenShellExecute },
	{"PostWindowMessage", LuaV6Chat3App::PostWindowMessage },
	{"FileExist", LuaV6Chat3App::FileExist},
	{"CopyToClipboard", LuaV6Chat3App::CopyToClipboard},

	{"GetUserList", LuaV6Chat3App::GetUserList },
	{"GetUserInfoByID", LuaV6Chat3App::GetUserInfoByID},
	{"GetUnreadMessage", LuaV6Chat3App::GetUnreadMessage},
	{"DeleteUnreadMessage", LuaV6Chat3App::DeleteUnreadMessage},

	{"AddServer", LuaV6Chat3App::AddServer },
	{"GetServerList", LuaV6Chat3App::GetServerList },
	{"GetServerInfoByID", LuaV6Chat3App::GetServerInfoByID },
	{"ChangeServerInfo", LuaV6Chat3App::ChangeServerInfo },
	{"SearchFriendOnServer", LuaV6Chat3App::SearchFriendOnServer },
	{"GetSearchFriendResult", LuaV6Chat3App::GetSearchFriendResult },

	{"AddP2PFriend", LuaV6Chat3App::AddP2PFriend },
	{"AddServerFriend", LuaV6Chat3App::AddServerFriend },
	{"AddFriendAck", LuaV6Chat3App::AddFriendAck },
	{"ChangeFriendGroup", LuaV6Chat3App::ChangeFriendGroup },
	{"DeleteFriend", LuaV6Chat3App::DeleteFriend },

	{"ChatWithText", LuaV6Chat3App::ChatWithText },
	{"GetChatContent", LuaV6Chat3App::GetChatContent },
	{"SelectChatFile", LuaV6Chat3App::SelectChatFile },
	{"GetSendFile", LuaV6Chat3App::GetSendFile },
	{"GetRecvFile", LuaV6Chat3App::GetRecvFile },
	{"GetChatHistory", LuaV6Chat3App::GetChatHistory },

	{"DeleteTray", LuaV6Chat3App::DeleteTray},
	{"AddTray", LuaV6Chat3App::AddTray },
	{"Quit", LuaV6Chat3App::Quit },
	{"Start", LuaV6Chat3App::Start },
	{"StartServer", LuaV6Chat3App::StartServer},
	{"GetMyHead", LuaV6Chat3App::GetMyHead},
	{"GetMyInfo", LuaV6Chat3App::GetMyInfo},
	{"GetMyIpAndPort", LuaV6Chat3App::GetMyIpAndPort},
	{"SetMyIntroduction", LuaV6Chat3App::SetMyIntroduction},
	{"SelectMyHead", LuaV6Chat3App::SelectMyHead},
	{NULL, NULL}
};

long LuaV6Chat3App::RegisterObj( XL_LRT_ENV_HANDLE hEnv )
{
	assert(hEnv);
	if(hEnv == NULL)
	{
		return XLLRT_RESULT_ENV_INVALID;
	}

	XLLRTObject theObject;
	theObject.ClassName = XUNLEI_IDEAPP_CLASS;
	theObject.MemberFunctions = LuaIDEAppMemberFunctions;
	theObject.ObjName = XUNLEI_IDEAPP_OBJ;
	theObject.userData = NULL;
	theObject.pfnGetObject = (fnGetObject)LuaV6Chat3App::Instance;

	long lRet = XLLRT_RegisterGlobalObj(hEnv,theObject); 
	assert(lRet == XLLRT_RESULT_SUCCESS);
	return lRet;
}