#pragma once
#include <list>
#include <map>

#include "../Lock/SimpleCriticalSection.h"
#include "../Handle.h"
#include "UserInfo.h"

class UserList
{
public:
	typedef std::map<unsigned int, Handle<UserInfo>>::iterator UserIter;

	UserList():hasInit(false)
	{}
	~UserList()
	{}

	static UserList *Instance();
	static void Release();
	static UserList *GetObj()
	{ return list; }

	bool Init();

	int AddUser(const Handle<UserInfo> &user);                          // 返回uid，已存在则直接返回uid
	void AddUsers(const std::list<Handle<UserInfo>> &usersList);              // 添加多个用户
	unsigned int GetUserUidByIp(const std::string &ip);
	int GetUserUidByServerIdAndUid(unsigned int serverId, const std::string &serverUid);
	std::string GetUserIpByUid(const unsigned int uid);
	int GetUserTCPPortByUid(const unsigned int uid);
	int GetUserUDPPortByUid(const unsigned int uid);
	bool GetUserSender(const unsigned int uid, Handle<MsgSender> &sender);
	bool GetUser(const unsigned int uid, std::wstring &nick_name, std::wstring &group_name, unsigned short &sex, unsigned int &age, std::wstring &introduction, std::wstring &location);
	bool UpdateUser(const unsigned int uid, const std::wstring *nick_name, const std::wstring *group_name, const unsigned short sex, const unsigned int age, const std::wstring *introduction, const std::wstring *headImagePath, bool show = true);       // 指针形参可为NULL，表示不修改值
	bool UpdateUserState(const unsigned int uid, UserState state);
	bool UpdateUserPort(const unsigned int uid, int tcpPort, int udpPort);
	void DeleteUser(unsigned int uid);

	void RecordChatText(unsigned int uid, const std::wstring &text, std::string &sendTime);                               // 记录聊天内容并获得记录的时间，用于记录发送的内容
	void UpdateChatContent(unsigned int uid, const std::wstring &text, int contentType);								  // 记录并更新聊天内容，用于记录接收到的内容
	bool GetChatContent(unsigned int uid, std::wstring &text, std::string &time, int &contentType);                       // 每次调用仅获得一条未读聊天纪录

	void UpdateUserDisplay(unsigned int uid);
	void ShowChatContent(unsigned int uid);
	void UpdateChatFile(unsigned int uid, bool success);

	void Store();

	//非线程安全, 获得的用户数据不得修改
	std::map<unsigned int, Handle<UserInfo>> &GetUsers()
	{ return users; }

private:
	void TraverseAllUsers();
	int FindUser(const Handle<UserInfo> &user);                         // 返回uid，没找到返回0，非线程安全

	static UserList *list;

	std::map<unsigned int, Handle<UserInfo>> users;
	static CriticalSection aCritSect;
	bool hasInit;

	static unsigned int __stdcall Loop(LPVOID pParam);
};