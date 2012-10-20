#include "ServerList.h"
#include "../V6Util.h"

#include "../../db_interface/db_interface.h"

#include <sstream>
#include <XLUE.h>

using namespace std;

ServerList *ServerList::list = NULL;
CriticalSection ServerList::aCritSect;

ServerList *ServerList::Instance()
{
	SimpleCriticalSection m(aCritSect);

	if(NULL == list)
		list = new ServerList;

	return list;
}

void ServerList::Release()
{
	SimpleCriticalSection m(aCritSect);

	if(list)
	{
		delete list;
		list = NULL;
	}
}

bool ServerList::Init()
{
	char **serversTable = NULL;
	int rows = 0, cols = 0;
	if(!get_serversInformation(&serversTable, &rows, &cols))
		return false;

	for(int i=1; i<=rows; i++)
	{
		ServerInfo *hs = new ServerInfo(atoi(serversTable[SERVERSINFO_DB_ROWS*i]),
			V6Util::Utf8ToUnicode(serversTable[SERVERSINFO_DB_ROWS*i+1]),
			serversTable[SERVERSINFO_DB_ROWS*i+2],
			atoi(serversTable[SERVERSINFO_DB_ROWS*i+3]),
			V6Util::Utf8ToUnicode(serversTable[SERVERSINFO_DB_ROWS*i+4]),
			serversTable[SERVERSINFO_DB_ROWS*i+5],
			serversTable[SERVERSINFO_DB_ROWS*i+6]);

		servers.insert(make_pair(hs->GetId(),hs));
	}

	release_indb_table_data(serversTable);
	return true;
}

void ServerList::Connect()
{
	for(ServerIter iter=servers.begin(); iter!=servers.end(); iter++)
	{
		iter->second->Connect();
	}
}

unsigned int ServerList::AddServer(const Handle<ServerInfo> server)
{
	SimpleCriticalSection m(aCritSect);

	stringstream sport;
	sport << server->GetPort();

	int sid = insert_serverInformation(
		V6Util::UnicodeToUtf8(server->GetName().c_str()).c_str(),
		server->GetIp().c_str(),
		sport.str().c_str(),
		V6Util::UnicodeToUtf8(server->GetDescription().c_str()).c_str(),
		server->GetUid().c_str(),
		server->GetUpassword().c_str()
		);

	if(sid == 0)
		return 0;

	server->SetId(sid);
	server->Connect();

	servers.insert(std::make_pair(sid, server));

	UpdateServerDisplay(sid);
	return sid;
}


void ServerList::DeleteServer(unsigned int id)
{
	SimpleCriticalSection m(aCritSect);

	servers.erase(id);
}

void ServerList::SetServerState(unsigned int id, ServerState state)
{
	SimpleCriticalSection m(aCritSect);

	if(servers.count(id))
	{
		servers[id]->SetState(state);
		UpdateServerDisplay(id);
	}
}

bool ServerList::GetServer(unsigned int id, Handle<ServerInfo> &server)
{
	SimpleCriticalSection m(aCritSect);

	if(!servers.count(id))
		return false;

	server = servers[id];
	return true;
}

void ServerList::Store()
{
	for(ServerIter iter=servers.begin(); iter!=servers.end(); iter++)
	{
		if(iter->second->HasChange())
		{
			stringstream sid, sport;
			sid << iter->second->GetId();
			sport<<iter->second->GetPort();
			update_serverInformation(sid.str().c_str(), 
				V6Util::UnicodeToUtf8(iter->second->GetName().c_str()).c_str(),
				iter->second->GetIp().c_str(), 
				sport.str().c_str(),
				V6Util::UnicodeToUtf8(iter->second->GetDescription().c_str()).c_str(),
				iter->second->GetUid().c_str(),
				iter->second->GetUpassword().c_str());
		}
	}
}

void ServerList::UpdateServerDisplay(unsigned int sid)
{
	XLUE_HOSTWND_HANDLE handle = XLUE_GetHostWndByID(ADDFRIEND_WND);
	if(handle)
	{
		HWND wnd_handle = (HWND)XLUE_GetHostWndWindowHandle(handle);
		::PostMessage(wnd_handle, WM_UPDATESERVERLIST, 3, sid);
	}
}