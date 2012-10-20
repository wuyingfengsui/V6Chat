#include "ServerInfo.h"

#include <process.h>
#include <Windows.h>
#include <regex>

using namespace std;

void ServerInfo::Connect()
{
	if(state == server_disconnect)
	{
		ConnectThreadPkg *pkg = new ConnectThreadPkg;
		pkg->c = &client;
		pkg->ip = ip;
		pkg->port = port;
		pkg->uid = uid;
		pkg->upassword = upassword;
		::CloseHandle((HANDLE)_beginthreadex(NULL, 0, ConnectThread, pkg, 0, NULL));
	}
}

unsigned int __stdcall ServerInfo::ConnectThread(void *parm)
{
	ConnectThreadPkg *pkg = (ConnectThreadPkg *)parm;

	tr1::regex rxFloat("\\d+\\.\\d+\\.\\d+\\.\\d+");
	if(tr1::regex_match(pkg->ip, rxFloat))                                             // ipv4类型，暂时只考虑ipv4
		pkg->c->Start(pkg->uid.c_str(), pkg->upassword.c_str(), pkg->ip.c_str(), pkg->port);
	else
		pkg->c->Start(pkg->ip.c_str(), pkg->uid.c_str(), pkg->upassword.c_str());

	delete pkg;
	return 0;
}

bool ServerInfo::SendFile(const std::string &account, const std::wstring &filePath, std::string &rsid)
{
	return xft.SendFile(account, V6Util::UnicodeToUtf8(filePath.c_str()), rsid);
}