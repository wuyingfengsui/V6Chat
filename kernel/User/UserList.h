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

	int AddUser(const Handle<UserInfo> &user);                          // ����uid���Ѵ�����ֱ�ӷ���uid
	void AddUsers(const std::list<Handle<UserInfo>> &usersList);              // ��Ӷ���û�
	unsigned int GetUserUidByIp(const std::string &ip);
	int GetUserUidByServerIdAndUid(unsigned int serverId, const std::string &serverUid);
	std::string GetUserIpByUid(const unsigned int uid);
	int GetUserTCPPortByUid(const unsigned int uid);
	int GetUserUDPPortByUid(const unsigned int uid);
	bool GetUserSender(const unsigned int uid, Handle<MsgSender> &sender);
	bool GetUser(const unsigned int uid, std::wstring &nick_name, std::wstring &group_name, unsigned short &sex, unsigned int &age, std::wstring &introduction, std::wstring &location);
	bool UpdateUser(const unsigned int uid, const std::wstring *nick_name, const std::wstring *group_name, const unsigned short sex, const unsigned int age, const std::wstring *introduction, const std::wstring *headImagePath, bool show = true);       // ָ���βο�ΪNULL����ʾ���޸�ֵ
	bool UpdateUserState(const unsigned int uid, UserState state);
	bool UpdateUserPort(const unsigned int uid, int tcpPort, int udpPort);
	void DeleteUser(unsigned int uid);

	void RecordChatText(unsigned int uid, const std::wstring &text, std::string &sendTime);                               // ��¼�������ݲ���ü�¼��ʱ�䣬���ڼ�¼���͵�����
	void UpdateChatContent(unsigned int uid, const std::wstring &text, int contentType);								  // ��¼�������������ݣ����ڼ�¼���յ�������
	bool GetChatContent(unsigned int uid, std::wstring &text, std::string &time, int &contentType);                       // ÿ�ε��ý����һ��δ�������¼

	void UpdateUserDisplay(unsigned int uid);
	void ShowChatContent(unsigned int uid);
	void UpdateChatFile(unsigned int uid, bool success);

	void Store();

	//���̰߳�ȫ, ��õ��û����ݲ����޸�
	std::map<unsigned int, Handle<UserInfo>> &GetUsers()
	{ return users; }

private:
	void TraverseAllUsers();
	int FindUser(const Handle<UserInfo> &user);                         // ����uid��û�ҵ�����0�����̰߳�ȫ

	static UserList *list;

	std::map<unsigned int, Handle<UserInfo>> users;
	static CriticalSection aCritSect;
	bool hasInit;

	static unsigned int __stdcall Loop(LPVOID pParam);
};