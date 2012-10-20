#include "ServerMsgListener.h"
#include "../V6ChatDef.h"
#include "../V6Util.h"
#include "../Server/ServerList.h"
#include "../User/UserList.h"
#include "../User/ServerUserInfo.h"
#include "../UnreadMsg/UnreadMsgList.h"
#include "../FileList/RecvFileList.h"

#include "../../xmpp_interface/gloox/gloox.h"

#include <Shlwapi.h>
#include <XLUE.h>

#include <sstream>

using namespace std;

void ServerMsgListener::Connect()
{
	if(ServerList *servers = ServerList::Instance())
	{
		servers->SetServerState(id, server_connected);

		Handle<ServerInfo> my_server;
		if(servers->GetServer(id, my_server))
		{
			my_server->StartFileTransfer();
		}
	}
}

void ServerMsgListener::Disconnect()
{
	if(ServerList *server = ServerList::Instance())
		server->SetServerState(id, server_disconnect);
}

bool ServerMsgListener::RecvRosterSubscriptionRequest(     // 收到添加联系人请求事件
		const std::string &serverName, 
		const std::string &peerAccount,
		const std::string &msg,
		std::string &backName
		)
{
	std::wstring servername = V6Util::Utf8ToUnicode(serverName.c_str());
	std::wstring wpeerAcnt = V6Util::Utf8ToUnicode(peerAccount.c_str());
	std::wstring message = wpeerAcnt + L"("+ servername + L")";
	UnreadMsg umsg(ADD_SERVERUSER_ASK, id, wpeerAcnt, message);

	UnreadMsgList *list = UnreadMsgList::Instance();
	list->AddUnreadMsg(umsg);                                                      //将添加好友请求放入未读消息栈
	list->DisplayMsg();                                                            //尝试向用户显示该消息

	return false;
}


void ServerMsgListener::RosterAdded(        // 联系人成功添加事件
		const std::string &serverName, 
		const std::string &peerAccount
		)
{
	UserList *uList = UserList::Instance();
	ServerList *server = ServerList::Instance();
	Handle<ServerInfo> my_server;
	wstring sername;
	if(server->GetServer(id, my_server))
	{
		sername = my_server->GetName();
	}

	string account = V6Util::Utf8ToAnsi(peerAccount.c_str());
	std::wstring group = sername + DEFAULT_GROUP_NAME;

	std::size_t loc = account.find("@");
	if(std::string::npos != loc)
		account = account.substr(0, loc);

	account = V6Util::AnsiToUtf8(account.c_str());

	uList->AddUser(new ServerUserInfo(
			0,
			V6Util::Utf8ToUnicode(account.c_str()),
			group,
			0,
			0,
			L"",
			L"",
			id,
			account
			));
}


void ServerMsgListener::RosterDeleted(        // 联系人删除事件
		const std::string &serverName, 
		const std::string &peerAccount
		)
{

}

void ServerMsgListener::RecvAllRosters(      // 接收到服务器上发来的联系人列表(在刚登录时)
		const gloox::Roster& roster
		)
{
	UserList *uList = UserList::Instance();
	ServerList *server = ServerList::Instance();
	Handle<ServerInfo> my_server;
	wstring sername;
	if(server->GetServer(id, my_server))
	{
		sername = my_server->GetName();
	}

	std::list<Handle<UserInfo>> usersList;
	for(gloox::Roster::const_iterator it = roster.begin() ; it != roster.end(); ++it )
	{
		string account = V6Util::Utf8ToAnsi((*it).first.c_str());
		std::size_t loc = account.find("@");
		if(std::string::npos != loc)
			account = account.substr(0, loc);
		account = V6Util::AnsiToUtf8(account.c_str());

		if(int uid = uList->GetUserUidByServerIdAndUid(id, account))
		{
			// 用户已添加过
		}
		else
		{
			std::wstring group;
			if(it->second->groups().size() == 0)
				group = sername + DEFAULT_GROUP_NAME;
			else
				group = V6Util::Utf8ToUnicode(it->second->groups().front().c_str());

			usersList.push_back(new ServerUserInfo(
				0,
				V6Util::Utf8ToUnicode(it->second->name().c_str()),
				group,
				0,
				0,
				L"",
				L"",
				id,
				account
				));
		}
	}

	uList->AddUsers(usersList);
}

void ServerMsgListener::RosterPresence(      // 联系人出席事件（在线状态改变）
		const gloox::RosterItem& item,
		gloox::Presence presence,
		const std::string& status,
		const int photoType,
		const std::string& photo
		)
{
	string account = V6Util::Utf8ToAnsi(item.jid().c_str());
	std::size_t loc = account.find("@");
	if(std::string::npos != loc)
		account = account.substr(0, loc);

	UserList *uList = UserList::Instance();
	int uid = uList->GetUserUidByServerIdAndUid(id, V6Util::AnsiToUtf8(account.c_str()));
	if(uid)
	{
		uList->UpdateUser(uid, 
			&V6Util::Utf8ToUnicode(item.name().c_str()),
			NULL,
			NULL,
			NULL,
			&V6Util::Utf8ToUnicode(status.c_str()),
			NULL,
			false);                      // 对于状态改变，之后还会更新显示，故这儿不更新

		ServerMsgSender ser(id, item.name(), uid);
		if(photoType == 1)
			ser.GetPhotoByHttp(V6Util::Utf8ToAnsi(photo.c_str()));
		else if(photoType == 2)
			ser.StorePhotoByStream(photo);
		
		if(presence == 6)  // 离线
			uList->UpdateUserState(uid, user_offline);
		else
			uList->UpdateUserState(uid, user_online);
	}
}

void ServerMsgListener::RecvVCard(           // 接收到服务器上发来的名片信息
		const std::string &serverName, 
		const std::string &peerAccount,
		const std::string &formattedName,
		const std::string &nickName,
		const std::string &description,
		const std::string &birthDay,
		const gloox::VCard::Photo &photo
		)
{
	UserList *uList = UserList::Instance();
	int uid = uList->GetUserUidByServerIdAndUid(id, peerAccount);
	if(uid)
	{
		ServerMsgSender ser(id, peerAccount, uid);
		if(photo.extval.size() != 0)
			ser.GetPhotoByHttp(V6Util::Utf8ToAnsi(photo.extval.c_str()));
		else if(photo.binval.size() != 0)
			ser.StorePhotoByStream(photo.binval);
		
		uList->UpdateUserDisplay(uid);
		return;
	}

	if(formattedName.length() || nickName.length())
	{
		VCardInfo info;
		info.uid = vCardList.size();
		info.account = peerAccount;
		if(formattedName.length())
			info.formattedName = formattedName;
		else
			info.formattedName = nickName;

		ServerMsgSender ser(id, peerAccount, info.uid);
		wstringstream spath;
		spath << BEFOER_TEMPUSERHEAD << L"_" << id << "_" << info.uid;

		if(photo.extval.size() != 0)
		{
			if(ser.GetPhotoByHttp(V6Util::Utf8ToAnsi(photo.extval.c_str()), 1))
				info.photoRelativePath = spath.str();
		}
		else if(photo.binval.size() != 0)
		{
			if(ser.StorePhotoByStream(photo.binval, 1))
				info.photoRelativePath = spath.str();
		}

		vCardList.push_back(info);
	}

	XLUE_HOSTWND_HANDLE handle = XLUE_GetHostWndByID(ADDFRIEND_WND);
	if(handle)
	{
		HWND wnd_handle = (HWND)XLUE_GetHostWndWindowHandle(handle);
		::PostMessage(wnd_handle, WM_UPDATESEARCHUSERRESULT, NULL, id);
	}
}

void ServerMsgListener::RecvText(            // 收到聊天消息事件
		int type,
		const std::string &serverName, 
		const std::string &peerAccount, 
		const std::string &body, 
		const std::string &subject
		)
{
	UserList *uList = UserList::Instance();
	int uid = uList->GetUserUidByServerIdAndUid(id, peerAccount);
	uList->UpdateChatContent(uid, V6Util::Utf8ToUnicode(body.c_str()).c_str(), user_content);               // 记录并更新聊天内容
	uList->ShowChatContent(uid);
}


bool ServerMsgListener::RecvFileTransferRequest(
		const std::string &serverName, 
		const std::string &peerAccount, 
		const std::string& sid,
		const std::string& fileName, 
		long size, 
		const std::string& hash,
		const std::string& date, 
		const std::string& mimeType,
		const std::string& description,
		std::string &filePath
		)
{
	UserList *uList = UserList::Instance();
	if(int uid = uList->GetUserUidByServerIdAndUid(id, peerAccount))
	{
		WCHAR wszModulePath[MAX_PATH];
		std::stringstream suid;
		suid << uid;

		GetModuleFileName(NULL,wszModulePath,MAX_PATH);
		PathAppend(wszModulePath, USERDATA_RELATIVE_PATH);
		PathAppend(wszModulePath, USERHEAD_RECVFILE_PATH);
		string filepath = V6Util::UnicodeToUtf8(wszModulePath);
		filePath = filepath + "\\";

		wstring filepathname = wszModulePath;
		filepathname += L"\\";
		filepathname += V6Util::Utf8ToUnicode(fileName.c_str());

		RecvFileList* rlist = RecvFileList::Instance();
		rlist->AddFileItem(uid, sid, filepathname);
		return true;
	}
	else
		return false;
}

void ServerMsgListener::FileTransferEnd(
		bool success,
		const std::string &serverName,
		const std::string &peerAccount, 
		const std::string& sid
		)
{
	UserList *uList = UserList::Instance();
	if(int uid = uList->GetUserUidByServerIdAndUid(id, peerAccount))
	{
		RecvFileList* rlist = RecvFileList::Instance();
		if(rlist->ChangeFileState(uid, sid, true))
			uList->UpdateChatFile(uid, success);
	}
}