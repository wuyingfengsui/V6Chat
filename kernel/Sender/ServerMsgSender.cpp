#include "ServerMsgSender.h"
#include "../V6ChatDef.h"
#include "../V6Util.h"

#include "../../socket_interface/socket_interface.h"
#include "../Server/ServerList.h"
#include "../FileList/SendFileList.h"

#include <Shlwapi.h>
#include <sstream>
#include <fstream>
#include <regex>

using namespace std;

void ServerMsgSender::AddUserAskMsg()
{
	ServerList *sList = ServerList::Instance();

	Handle<ServerInfo> server;
	if(sList->GetServer(serverid, server))
	{
		server->AddUser(serveruid);
	}
}

void ServerMsgSender::ReplyAddUserMsg(bool agree)
{
	if(agree)
	{
		ServerList *sList = ServerList::Instance();

		Handle<ServerInfo> server;
		if(sList->GetServer(serverid, server))
		{
			server->AddUser(serveruid);
		}
	}
}

void ServerMsgSender::UpdateProfileMsg()
{
	ServerList *sList = ServerList::Instance();

	Handle<ServerInfo> server;
	if(sList->GetServer(serverid, server))
	{
		server->AskUserInfo(serveruid);
	}
}

bool ServerMsgSender::GetPhotoByHttp(const std::string &photoURI, int photoType)
{
	WCHAR wszModulePath[MAX_PATH];
    GetModuleFileNameW(NULL,wszModulePath,MAX_PATH);
	PathAppend(wszModulePath, USERDATA_RELATIVE_PATH);
	PathAppend(wszModulePath, USERHEAD_RELATIVE_PATH);

	stringstream sid;
	if(photoType == 1)
		sid << V6Util::UnicodeToAnsi(BEFOER_TEMPUSERHEAD) << "_" << serverid << "_";
	sid << userid;
	if(string::npos != photoURI.find(".png"))
		sid << ".png";

	return get_file_by_http(photoURI.c_str(), V6Util::UnicodeToAnsi(wszModulePath).c_str(), sid.str().c_str());
}

bool ServerMsgSender::StorePhotoByStream(const std::string &stream, int photoType)
{
	WCHAR wszModulePath[MAX_PATH];
    GetModuleFileNameW(NULL,wszModulePath,MAX_PATH);
	PathAppend(wszModulePath, USERDATA_RELATIVE_PATH);
	PathAppend(wszModulePath, USERHEAD_RELATIVE_PATH);
	string path = V6Util::UnicodeToAnsi(wszModulePath);

	stringstream sid;
	if(photoType == 1)
		sid << V6Util::UnicodeToAnsi(BEFOER_TEMPUSERHEAD) << "_" << serverid << "_";
	sid << userid;

	ofstream of(path + "\\" + sid.str(), std::ios_base::out | std::ios_base::binary);
	if(of)
		of << stream;
	of.close();

	return true;
}

void ServerMsgSender::ChatWithTextMsg(const std::wstring &text, const std::string &sendTime)
{
	ServerList *sList = ServerList::Instance();

	Handle<ServerInfo> server;
	if(sList->GetServer(serverid, server))
	{
		string mtext = V6Util::UnicodeToAnsi(text.c_str());

		map<int, string> nis;

		tr1::regex rxFloat("<\\d+>");  
		tr1::sregex_token_iterator ite(mtext.begin(), mtext.end(), rxFloat), end; 

		SendFileList *list = SendFileList::Instance();
		// 检查内容中是否含有文件
		for ( ; ite != end; ++ite)  
		{  
			string w = ite->str();
			string wi = w.substr(1, w.length()-2);
			int ni = atoi(wi.c_str());
			nis.insert(make_pair(ni, wi));
		}

		for(map<int, string>::iterator it = nis.begin(); it != nis.end(); ++it)
		{
			wstring filepath;
			string rsid;
			if(list->GetFilePath(it->first, filepath))
			{
				if(server->SendFile(serveruid, filepath, rsid))
				{
					tr1::regex rxFloat(it->second);
					mtext = tr1::regex_replace(mtext, rxFloat, rsid);
				}
			}
		}

		server->SendText(serveruid, V6Util::AnsiToUtf8(mtext.c_str()));
	}
}