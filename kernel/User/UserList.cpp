#include "P2PUserInfo.h"
#include "ServerUserInfo.h"
#include "../../db_interface/db_interface.h"
#include "../V6Util.h"
#include "UserList.h"
#include "../UnreadMsg/UnreadMsgList.h"

#include <sstream>
#include <process.h>
#include <XLUE.h>

using namespace std;

UserList *UserList::list = NULL;
CriticalSection UserList::aCritSect;

UserList *UserList::Instance()
{
	SimpleCriticalSection m(aCritSect);
	if(NULL == list)
		list = new UserList;

	return list;
}

void UserList::Release()
{
	SimpleCriticalSection m(aCritSect);
	if(list)
	{
		delete list;
		list = NULL;
	}
}


bool UserList::Init()
{
	if(hasInit)
		return true;
	users.clear();

	char **peersTable = NULL;
	int rows = 0, cols = 0;
	if(!get_peersProfile(&peersTable, &rows, &cols))
		return false;

	for(int i=1; i<=rows; i++)
	{
		int serverId=0, id=0, sex=0, age=0;
		wstring nick_name, group_name, introduction, headImagePath;
		if(peersTable[PEERSINFO_DB_ROWS*i])
			id = atoi(peersTable[PEERSINFO_DB_ROWS*i]);
		if(peersTable[PEERSINFO_DB_ROWS*i+1])
			nick_name = V6Util::Utf8ToUnicode(peersTable[PEERSINFO_DB_ROWS*i+1]);
		if(peersTable[PEERSINFO_DB_ROWS*i+2])
			group_name = V6Util::Utf8ToUnicode(peersTable[PEERSINFO_DB_ROWS*i+2]);
		if(peersTable[PEERSINFO_DB_ROWS*i+3])
			sex = atoi(peersTable[PEERSINFO_DB_ROWS*i+3]);
		if(peersTable[PEERSINFO_DB_ROWS*i+4])
			age = atoi(peersTable[PEERSINFO_DB_ROWS*i+4]);
		if(peersTable[PEERSINFO_DB_ROWS*i+5])
			introduction = V6Util::Utf8ToUnicode(peersTable[PEERSINFO_DB_ROWS*i+5]);
		if(peersTable[PEERSINFO_DB_ROWS*i+6])
			headImagePath = V6Util::Utf8ToUnicode(peersTable[PEERSINFO_DB_ROWS*i+6]);
		if(peersTable[PEERSINFO_DB_ROWS*i+7])
		    serverId = atoi(peersTable[PEERSINFO_DB_ROWS*i+7]);

		if(serverId)
		{
			string serveruid;
			if(peersTable[PEERSINFO_DB_ROWS*i+8])
				serveruid = peersTable[PEERSINFO_DB_ROWS*i+8];

			UserInfo *new_user = new ServerUserInfo(id, nick_name, group_name, sex, age, introduction, headImagePath, serverId, serveruid);

			users.insert(make_pair(new_user->uid, new_user));
		}
		else
		{
			string ip;
			int tcpPort=0, udpPort=0;
			if(peersTable[PEERSINFO_DB_ROWS*i+9])
				ip = peersTable[PEERSINFO_DB_ROWS*i+9];
			if(peersTable[PEERSINFO_DB_ROWS*i+10])
				tcpPort = atoi(peersTable[PEERSINFO_DB_ROWS*i+10]);
			if(peersTable[PEERSINFO_DB_ROWS*i+11])
				udpPort = atoi(peersTable[PEERSINFO_DB_ROWS*i+11]);

			UserInfo *new_user = new P2PUserInfo(id, nick_name, group_name, sex, age, introduction, headImagePath, ip, tcpPort, udpPort);
			
			ClientMsgSender sender(ip, tcpPort, udpPort);
			sender.TestOnlineMsg();

			users.insert(make_pair(new_user->uid, new_user));
		}
	}

	release_indb_table_data(peersTable);

	CloseHandle((HANDLE)_beginthreadex(NULL,0,Loop,NULL,0,NULL));

	hasInit = true;
	return true;
}

int UserList::AddUser(const Handle<UserInfo> &user)
{
	SimpleCriticalSection m(aCritSect);

	if(!hasInit)
		return 0;

	int userid = 0;
	if(userid = FindUser(user))
		return userid;

	stringstream ssex, sage, sudpPort, stcpPort;
	ssex<<user->sex; sage<<user->age; sudpPort<<user->udpPort; stcpPort<<user->tcpPort;

	P2PUserInfo *p2pUser = dynamic_cast<P2PUserInfo *>(&(*user));
	if(p2pUser)
	{
	    userid = insert_peerProfile(
		    V6Util::UnicodeToUtf8(user->nick_name.c_str()).c_str(),
		    V6Util::UnicodeToUtf8(user->group_name.c_str()).c_str(),
		    ssex.str().c_str(),
		    sage.str().c_str(),
		    V6Util::UnicodeToUtf8(user->introduction.c_str()).c_str(),
		    V6Util::UnicodeToUtf8(user->headImagePath.c_str()).c_str(),
		    NULL,
		    NULL,
		    user->ip.c_str(),
			stcpPort.str().c_str(),
			sudpPort.str().c_str()
		    );
	}
	ServerUserInfo *serverUser = dynamic_cast<ServerUserInfo *>(&(*user));
	if(serverUser)
	{
		stringstream sserverid;
		sserverid<<serverUser->serverid;
		userid = insert_peerProfile(
		    V6Util::UnicodeToUtf8(user->nick_name.c_str()).c_str(),
		    V6Util::UnicodeToUtf8(user->group_name.c_str()).c_str(),
		    ssex.str().c_str(),
		    sage.str().c_str(),
		    V6Util::UnicodeToUtf8(user->introduction.c_str()).c_str(),
		    V6Util::UnicodeToUtf8(user->headImagePath.c_str()).c_str(),
			sserverid.str().c_str(),
			serverUser->serveruid.c_str(),
		    user->ip.c_str(),
			stcpPort.str().c_str(),
			sudpPort.str().c_str()
		    );
	}

	if(!userid)
		return 0;

	stringstream suid;
	suid << userid;
	user->uid = userid;
	users[userid] = user;

	UpdateUserDisplay(userid);
	return userid;
}

// 需自行调用delete[]释放*dest内存
inline void convertString2CharArray(char **dest, const string& source)
{
	int sourcelen = source.size() + 1;
	*dest = new char[sourcelen];
	strcpy(*dest, source.c_str());
}

// 需自行调用delete[]释放*dest内存
inline void convertWString2CharArray(char **dest, const wstring& source)
{
	string msource = V6Util::UnicodeToUtf8(source.c_str());
	int msourcelen = msource.size() + 1;
	*dest = new char[msourcelen];
	strcpy(*dest, msource.c_str());
}

void freeString(char **s)
{
	if(*s)
	{
		delete[] (*s);
		*s = NULL;
	}
}

void UserList::AddUsers(const std::list<Handle<UserInfo>> &usersList)
{
	SimpleCriticalSection m(aCritSect);

	if(!hasInit)
		return;

	insert_peer_profile_list db_list = new new_peer_profile;
	memset(db_list, 0, sizeof(new_peer_profile));

	int last_uid = last_insert_peerProfileUid();

	new_peer_profile* rear = db_list;
	for(std::list<Handle<UserInfo>>::const_iterator iter = usersList.begin(); iter != usersList.end(); ++iter)
	{
		if(FindUser(*iter))
			continue;

		stringstream suid;
		suid << ++last_uid;
		(*iter)->uid = last_uid;
		users[last_uid] = *iter;
		UpdateUserDisplay(last_uid);

		new_peer_profile* new_peer = new new_peer_profile;

		stringstream ssex, sage, sudpPort, stcpPort;
		ssex<<(*iter)->sex; sage<<(*iter)->age; sudpPort<<(*iter)->udpPort; stcpPort<<(*iter)->tcpPort;

		convertString2CharArray(&(new_peer->sex), ssex.str());
		convertString2CharArray(&(new_peer->age), sage.str());
		convertString2CharArray(&(new_peer->udp_port), sudpPort.str());
		convertString2CharArray(&(new_peer->tcp_port), stcpPort.str());
		convertString2CharArray(&(new_peer->last_ip), (*iter)->ip);

		convertWString2CharArray(&(new_peer->group_name), (*iter)->group_name);
		convertWString2CharArray(&(new_peer->head_image_path), (*iter)->headImagePath);
		convertWString2CharArray(&(new_peer->introduction), (*iter)->introduction);
		convertWString2CharArray(&(new_peer->nick_name), (*iter)->nick_name);

		P2PUserInfo *p2pUser = dynamic_cast<P2PUserInfo *>(&(**iter));
		if(p2pUser)
		{
			convertString2CharArray(&(new_peer->server_id), "");
			convertString2CharArray(&(new_peer->server_uid), "");
		}
		ServerUserInfo *serverUser = dynamic_cast<ServerUserInfo *>(&(**iter));
		if(serverUser)
		{
			stringstream sserverid;
			sserverid<<serverUser->serverid;
			convertString2CharArray(&(new_peer->server_id), sserverid.str());
			convertString2CharArray(&(new_peer->server_uid), serverUser->serveruid);
		}

		new_peer->next = NULL;
		rear->next = new_peer;
		rear = new_peer;
	}

	insert_peersProfile(db_list);

	while(db_list)
	{
		new_peer_profile* n = db_list->next;

		freeString(&(db_list->age));
		freeString(&(db_list->group_name));
		freeString(&(db_list->head_image_path));
		freeString(&(db_list->introduction));
		freeString(&(db_list->last_ip));
		freeString(&(db_list->nick_name));
		freeString(&(db_list->server_id));
		freeString(&(db_list->server_uid));
		freeString(&(db_list->sex));
		freeString(&(db_list->tcp_port));
		freeString(&(db_list->udp_port));
		delete db_list;

		db_list = n;
	}
}

class serveruser_finder
{
public:
	serveruser_finder(const unsigned int id, const std::string &uid): sid(id), suid(uid)
	{}

	bool operator ()(const std::map<unsigned int, Handle<UserInfo>>::value_type &value)
	{
		ServerUserInfo *serverUser = dynamic_cast<ServerUserInfo *>(&(*value.second));
		if(serverUser)
		{
			return (serverUser->serverid == sid) && (serverUser->serveruid == suid);           // server用户以server id和server端的uid区分
		}

		return false;
	}

private:
	unsigned int sid;
	std::string suid;
};

class p2puser_finder
{
public:
	p2puser_finder(const std::string &ip): ip(ip)
	{}

	bool operator ()(const std::map<unsigned int, Handle<UserInfo>>::value_type &value)
	{
		P2PUserInfo *p2pUser = dynamic_cast<P2PUserInfo *>(&(*value.second));
		if(p2pUser)
		{
			return p2pUser->ip == ip;                       // p2p以ip区别
		}

		return false;
	}

private:
	std::string ip;
};

int UserList::FindUser(const Handle<UserInfo> &user)
{
	ServerUserInfo *serverUser = dynamic_cast<ServerUserInfo *>(&(*user));    // 如果是Server用户
	if(serverUser)
	{
		UserIter iter = find_if(users.begin(), users.end(), serveruser_finder(serverUser->serverid, serverUser->serveruid));
		if(iter == users.end())
			return 0;
		else
			return iter->first;
	}

	P2PUserInfo *p2pUser = dynamic_cast<P2PUserInfo *>(&(*user));    // 如果是P2P用户
	if(p2pUser)
	{
		UserIter iter = find_if(users.begin(), users.end(), p2puser_finder(p2pUser->ip));
		if(iter == users.end())
			return 0;
		else
			return iter->first;
	}
	return 0;
}

unsigned int UserList::GetUserUidByIp(const std::string &ip)
{
	SimpleCriticalSection m(aCritSect);

	for(UserIter iter=users.begin(); iter!=users.end(); ++iter)
	{
		if(iter->second->ip == ip)
		{
			return iter->second->uid;
		}
	}
	return 0;
}

int UserList::GetUserUidByServerIdAndUid(unsigned int serverId, const std::string &serverUid)
{
	SimpleCriticalSection m(aCritSect);

	UserIter iter = find_if(users.begin(), users.end(), serveruser_finder(serverId, serverUid));
	if(iter == users.end())
		return 0;
	else
		return iter->first;
}

std::string UserList::GetUserIpByUid(const unsigned int uid)
{
	SimpleCriticalSection m(aCritSect);

	UserIter iter = users.find(uid);
	if(iter != users.end())
		return iter->second->ip;
	return "";
}

int UserList::GetUserTCPPortByUid(const unsigned int uid)
{
	SimpleCriticalSection m(aCritSect);

	int port = 0;
	UserIter iter = users.find(uid);
	if(iter != users.end())
		port = iter->second->tcpPort;
	return port;
}

int UserList::GetUserUDPPortByUid(const unsigned int uid)
{
	SimpleCriticalSection m(aCritSect);

	int port = 0;
	UserIter iter = users.find(uid);
	if(iter != users.end())
		port = iter->second->udpPort;
	return port;
}

bool UserList::GetUserSender(const unsigned int uid, Handle<MsgSender> &sender)
{
	SimpleCriticalSection m(aCritSect);

	if(!users.count(uid))
		return false;

	sender = users[uid]->messageSender;
	return true;
}

bool UserList::GetUser(const unsigned int uid, std::wstring &nick_name, std::wstring &group_name, unsigned short &sex, unsigned int &age, std::wstring &introduction, std::wstring &location)
{
	Handle<UserInfo> user;
	{
		SimpleCriticalSection m(aCritSect);

		if(!users.count(uid))
			return false;

		user = users[uid];
	}

	nick_name = user->nick_name;
	group_name = user->group_name;
	sex = user->sex;
	age = user->age;
	introduction = user->introduction;
	user->GetLocation(location);

	ServerUserInfo *serverUser = dynamic_cast<ServerUserInfo *>(&(*user));    // 如果是Server用户
	if(serverUser)
	{
		serverUser->UpdateProfile();
	}

	return true;
}

bool UserList::UpdateUser(const unsigned int uid, const std::wstring *nick_name, const std::wstring *group_name, const unsigned short sex, const unsigned int age, const std::wstring *introduction, const std::wstring *headImagePath, bool show)
{
	SimpleCriticalSection m(aCritSect);

	if(!users.count(uid))
		return false;

	if(nick_name && (users[uid]->nick_name != *nick_name))
	{
		users[uid]->nick_name = *nick_name;
		users[uid]->hasChange = true;
	}
	if(group_name && (users[uid]->group_name != *group_name))
	{
		users[uid]->group_name = *group_name;
		users[uid]->hasChange = true;
	}
	if((sex < 3) && (users[uid]->sex != sex))
	{
		users[uid]->sex = sex;
		users[uid]->hasChange = true;
	}
	if(age && (users[uid]->age != age))
	{
		users[uid]->age = age;
		users[uid]->hasChange = true;
	}
	if(introduction && (users[uid]->introduction != *introduction))
	{
		users[uid]->introduction = *introduction;
		users[uid]->hasChange = true;
	}
	if(headImagePath && (users[uid]->introduction != *headImagePath))
	{
		users[uid]->introduction = *headImagePath;
		users[uid]->hasChange = true;
	}

	if(show)
		UpdateUserDisplay(uid);

	return true;
}

bool UserList::UpdateUserState(const unsigned int uid, UserState state)
{
	UserState oldState;
	{
		SimpleCriticalSection m(aCritSect);

		if(!(list && users.count(uid)))
			return false;

		oldState = users[uid]->GetState();

		users[uid]->SetState(state);
	}
	if(state != oldState)
		UpdateUserDisplay(uid);

	return true;
}

bool UserList::UpdateUserPort(const unsigned int uid, int tcpPort, int udpPort)
{
	SimpleCriticalSection m(aCritSect);

	if(!users.count(uid))
		return false;

	if(users[uid]->tcpPort != tcpPort)
	{
		users[uid]->tcpPort = tcpPort;
		users[uid]->hasChange = true;
	}
	if(users[uid]->udpPort != udpPort)
	{
		users[uid]->udpPort = udpPort;
		users[uid]->hasChange = true;
	}

	return true;
}

void UserList::DeleteUser(unsigned int uid)
{
	SimpleCriticalSection m(aCritSect);

	if(hasInit && users.count(uid))
	{
		if(users[uid]->Delete())         // 在数据库(及服务器)上执行删除
			users.erase(uid);
	}
}

void UserList::RecordChatText(unsigned int uid, const std::wstring &text, std::string &sendTime)
{
	SimpleCriticalSection m(aCritSect);

	if(hasInit && users.count(uid))
		users[uid]->RecordText(true, text, sendTime);
}

void UserList::UpdateChatContent(unsigned int uid, const std::wstring &text, int contentType)
{
	SimpleCriticalSection m(aCritSect);
	std::string time;

	users[uid]->RecordText(false, text, time);
	users[uid]->RecvText(contentType, text, time);
}

bool UserList::GetChatContent(unsigned int uid, std::wstring &text, std::string &time, int &contentType)
{
	SimpleCriticalSection m(aCritSect);

	if(hasInit && users.count(uid) && !(users[uid]->unreadChatContent.empty()))
	{
		ChatMsg msg = users[uid]->unreadChatContent.front();
		text = msg.msg;
		time = msg.time;
		contentType = msg.msg_type;
		users[uid]->unreadChatContent.pop();
		return true;
	}

	return false;
}

void UserList::Store()
{
	if(hasInit)
	{
		SimpleCriticalSection m(aCritSect);
		for(UserIter iter=users.begin(); iter!=users.end(); ++iter)
		{
			if(iter->second->hasChange)
				iter->second->StoreInDB();
		}
	}
}


void UserList::TraverseAllUsers()
{
	if(hasInit)
	{
		SimpleCriticalSection m(aCritSect);

		if(list)
			for(UserIter iter=users.begin(); iter!=users.end(); ++iter)
			{
				if(user_online == iter->second->GetState())
				{
					bool isOnline = iter->second->CheckOnline();
					if(!isOnline)
						UpdateUserDisplay(iter->second->uid);
				}
			}
	}
}

void UserList::UpdateUserDisplay(unsigned int uid)
{
	XLUE_HOSTWND_HANDLE handle = XLUE_GetHostWndByID(FRIENDLIST_WND);
	if(handle)
	{
		HWND wnd_handle = (HWND)XLUE_GetHostWndWindowHandle(handle);
		::PostMessage(wnd_handle, WM_UPDATEFRIENDLIST, 3, uid);
	}

	stringstream suid;
	suid << BEFORE_CHAT_WND << uid;

	XLUE_HOSTWND_HANDLE chat_handle = XLUE_GetHostWndByID(suid.str().c_str());
	if(chat_handle)
	{
		HWND wnd_handle = (HWND)XLUE_GetHostWndWindowHandle(chat_handle);
		::PostMessage(wnd_handle, WM_UPDATEFRIENDLIST, 3, 0);
	}
}

void UserList::ShowChatContent(unsigned int uid)
{
	std::stringstream suid;
	suid << BEFORE_CHAT_WND << uid;

	XLUE_HOSTWND_HANDLE handle = XLUE_GetHostWndByID(suid.str().c_str());
	if(handle)
	{
		HWND wnd_handle = (HWND)XLUE_GetHostWndWindowHandle(handle);
		::PostMessage(wnd_handle, WM_UPDATECHATPANEL, 0, 0);
	}
	else
	{
		wstring wtext;
		{
			SimpleCriticalSection m(aCritSect);

			if(hasInit && users.count(uid) && !(users[uid]->unreadChatContent.empty()))
			{
				wtext = users[uid]->unreadChatContent.front().msg;
			}
		}

		if(0 != wtext.length())
		{
			UnreadMsg msg(TEXT_CHAT, uid, wtext);
			UnreadMsgList *list = UnreadMsgList::Instance();
			list->AddUnreadMsg(msg);
			list->DisplayMsg();                                                           //尝试向用户显示该消息
		}
	}
}

void UserList::UpdateChatFile(unsigned int uid, bool success)
{
	std::stringstream suid;
	suid << BEFORE_CHAT_WND << uid;

	XLUE_HOSTWND_HANDLE handle = XLUE_GetHostWndByID(suid.str().c_str());
	if(handle)
	{
		HWND wnd_handle = (HWND)XLUE_GetHostWndWindowHandle(handle);
		::PostMessage(wnd_handle, WM_UPDATECHATDOWNLOADFILE, 0, success);
	}
}

unsigned int __stdcall UserList::Loop(LPVOID pParam)
{
	UserList *list = UserList::Instance();
	while(list)
	{
		list->TraverseAllUsers();
		::Sleep(LIVE_TIME);
		list = UserList::GetObj();
	}

	return 0;
}