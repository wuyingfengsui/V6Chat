#pragma once
#include "../V6ChatDef.h"

#include <string>
#include <map>
#include <utility>

#include "ServerInfo.h"
#include "../Lock/SimpleCriticalSection.h"
#include "../Handle.h"

class ServerList
{
public:
	typedef std::map<unsigned int, Handle<ServerInfo>>::iterator ServerIter;

	ServerList()
	{}
	~ServerList()
	{}

	static ServerList *Instance();
	static void Release();

	bool Init();
	void Connect();

	unsigned int AddServer(const Handle<ServerInfo> server);
	void DeleteServer(unsigned int id);

	void SetServerState(unsigned int id, ServerState state);
	bool GetServer(unsigned int id, Handle<ServerInfo> &server);

	void Store();

	//非线程安全
	std::map<unsigned int, Handle<ServerInfo>> &GetServers()
	{ return servers; }

	void UpdateServerDisplay(unsigned int sid);

private:
	static ServerList *list;
	static CriticalSection aCritSect;

	std::map<unsigned int, Handle<ServerInfo>> servers;
	CRITICAL_SECTION mutex;
};