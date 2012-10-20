#include "ClientMsgSender.h"
#include "../User/MyInfo.h"
#include "../V6Util.h"
#include "../V6ChatDef.h"
#include "../User/UserList.h"
#include "../User/TempUserList.h"
#include "WaitACKMsgSender.h"
#include "../FileList/SendFileList.h"
#include "../UnreadMsg/UnreadMsgList.h"

#include "../../socket_interface/socket_interface.h"

#include "../json/json.h"

#include <sstream>
#include <Shlwapi.h>
#include <XLUE.h>
#include <regex>                         // 正则表达式库
#include <process.h>

using namespace Json;
using namespace std;


void ClientMsgSender::AddUserAskMsg()
{
	Value msg, myInfo;
	FastWriter fast_writer;

	MyInfo *my = MyInfo::Instance();
	myInfo["nick_name"] = V6Util::UnicodeToAnsi(my->GetName().c_str());
	myInfo["sex"] = my->GetSex();
	myInfo["age"] = my->GetAge();
	myInfo["introduction"] = V6Util::UnicodeToAnsi(my->GetIntroduction().c_str());

	msg["type"] = ADD_P2PUSER_ASK;
	msg["source"] = source;
	msg["my_info"] = fast_writer.write(myInfo);

	WaitACKMsgSender *sender = WaitACKMsgSender::Instance();
	sender->AddMsg(ip, udpPort, fast_writer.write(msg), AfterAddP2PUserMsg);
}

void ClientMsgSender::ReplyAddUserMsg(bool agree)
{
	Value msg;
	FastWriter fast_writer;

	msg["type"] = REPLY_ADD_USER;
	msg["source"] = source;
	if(agree)
	{
		Value myInfo;
		MyInfo *my = MyInfo::Instance();
	    myInfo["nick_name"] = V6Util::UnicodeToAnsi(my->GetName().c_str());
	    myInfo["sex"] = my->GetSex();
	    myInfo["age"] = my->GetAge();
	    myInfo["introduction"] = V6Util::UnicodeToAnsi(my->GetIntroduction().c_str());
	    msg["my_info"] = fast_writer.write(myInfo);
	}

	WaitACKMsgSender *sender = WaitACKMsgSender::Instance();
	sender->AddMsg(ip, udpPort, fast_writer.write(msg), NULL);
}

void ClientMsgSender::TestOnlineMsg()
{
	Value msg, onlineInfo;
	FastWriter fast_writer;

	msg["type"] = USER_ONLINE;
	msg["source"] = source;

	WaitACKMsgSender *sender = WaitACKMsgSender::Instance();
	sender->AddMsg(ip, udpPort, fast_writer.write(msg), AfterOnlineMsg);
}

void ClientMsgSender::TestOnlineMsgAck(const std::string &mid)
{
	Value msg, box;
	FastWriter fast_writer;

	msg["type"] = USER_ONLINE_ACK;
	msg["source"] = source;

	string content = fast_writer.write(msg);
	box["content"] = content;
	box["mid"] = mid;
	stringstream sport;
	sport<<udpPort;
	send_udp_string(ip.c_str(), sport.str().c_str(), fast_writer.write(box).c_str());
}

unsigned long ClientMsgSender::KeepOnlineMsg()
{
	Value msg;
	FastWriter fast_writer;

	msg["type"] = USER_KEEPONLINE;
	msg["source"] = source;

	WaitACKMsgSender *sender = WaitACKMsgSender::Instance();
	return sender->AddMsg(ip, udpPort, fast_writer.write(msg), NULL, true);
}

void ClientMsgSender::KeepOnlineMsgAck(const std::string &mid)
{
	Value msg, box;
	FastWriter fast_writer;

	msg["type"] = USER_KEEPONLINE_ACK;
	msg["source"] = source;

	string content = fast_writer.write(msg);
	box["content"] = content;
	box["mid"] = mid;
	stringstream sport;
	sport<<udpPort;
	send_udp_string(ip.c_str(), sport.str().c_str(), fast_writer.write(box).c_str());
}

void ClientMsgSender::Offline()
{
	Value msg, box;
	msg["type"] = USER_OFFLINE;
	msg["source"] = source;

	FastWriter fast_writer;
	string content = fast_writer.write(msg);
	box["content"] = content;
	stringstream sport;
	sport<<udpPort;
	send_udp_string(ip.c_str(), sport.str().c_str(), fast_writer.write(box).c_str());
}

void ClientMsgSender::UpdateProfileMsg()
{
	Value msg, myInfo;
	FastWriter fast_writer;
	msg["type"] = USER_INFO_UPDATE;
	msg["source"] = source;

	MyInfo *my = MyInfo::Instance();
	myInfo["nick_name"] = V6Util::UnicodeToAnsi(my->GetName().c_str());
	myInfo["sex"] = my->GetSex();
	myInfo["age"] = my->GetAge();
	myInfo["introduction"] = V6Util::UnicodeToAnsi(my->GetIntroduction().c_str());
	msg["my_info"] = fast_writer.write(myInfo);

	stringstream sport;
	sport<<tcpPort;

	async_send_tcp_string(ip.c_str(), sport.str().c_str(), fast_writer.write(msg).c_str(), AfterUpdateProfileMsg);
}

bool ClientMsgSender::GetFileMsg(const int fileId, const std::string &filePath)
{
	Value msg;
	FastWriter fast_writer;
	msg["type"] = GET_FILE;
	msg["source"] = source;
	msg["number"] = fileId;

	stringstream sport;
	sport<<tcpPort;

	SOCKET sock;

	for(int i = 0; i < MAX_TRY_TIMES; ++i)
	{
		if(send_tcp_string_without_closesocket(ip.c_str(), sport.str().c_str(), fast_writer.write(msg).c_str(), &sock))
		{
			if(receive_small_file_by_tcp_socket(sock, filePath.c_str()))
			{
				release_socket(sock);
				return true;
			}

			release_socket(sock);
		}
	}
	return false;
}

bool ClientMsgSender::SendFileMsg(const string& ip, const int tcpPort, const int fileId, const string& source)
{
	wstring wfpath;
	SendFileList* flist = SendFileList::Instance();
	if(!flist->GetFilePath(fileId, wfpath))
		return false;

	string fpath = V6Util::UnicodeToAnsi(wfpath.c_str());
	Value msg;
	FastWriter fast_writer;
	msg["type"] = SEND_FILE;
	msg["source"] = source;
	msg["number"] = fileId;
	msg["filename"] = fpath.substr(fpath.find_last_of("\\")+1);

	stringstream sport;
	sport<<tcpPort;

	SOCKET sock;

	for(int i = 0; i < MAX_TRY_TIMES; ++i)
	{
		if(send_tcp_string_without_closesocket(ip.c_str(), sport.str().c_str(), fast_writer.write(msg).c_str(), &sock))
		{
			if(send_small_file_by_tcp_socket(sock, fpath.c_str()))
			{
				release_socket(sock);
				return true;
			}

			release_socket(sock);
		}
	}
	return false;
}

struct SendTextThreadPkg
{
	char* ip;
	int udpPort;
	int tcpPort;
	char* text;
	char* sendTime;
	char* source;
};

void ClientMsgSender::ChatWithTextMsg(const std::wstring &text, const std::string &sendTime)
{
	string mtext = V6Util::UnicodeToAnsi(text.c_str());
	int iplen = ip.length() + 1;
	int textlen = mtext.length() + 1;
	int timelen = sendTime.length() + 1;
	int sourcelen = source.length() + 1;

	SendTextThreadPkg *thpkg = new SendTextThreadPkg;
	thpkg->ip = new char[iplen];
	memcpy(thpkg->ip, ip.c_str(), iplen);
	thpkg->text = new char[textlen];
	memcpy(thpkg->text, mtext.c_str(), textlen);
	thpkg->sendTime = new char[timelen];
	memcpy(thpkg->sendTime, sendTime.c_str(), timelen);
	thpkg->source = new char[sourcelen];
	memcpy(thpkg->source, source.c_str(), sourcelen);
	thpkg->tcpPort = tcpPort;
	thpkg->udpPort = udpPort;

	::CloseHandle((HANDLE)_beginthreadex(NULL, 0, SendTextThread, thpkg, 0, NULL));
}

void ClientMsgSender::NormalMsgAck(const std::string &mid)
{
	Value msg, box;
	FastWriter fast_writer;

	msg["type"] = NORMAL_ACK;
	msg["source"] = source;

	string content = fast_writer.write(msg);
	box["content"] = content;
	box["mid"] = mid;
	stringstream sport;
	sport<<udpPort;
	send_udp_string(ip.c_str(), sport.str().c_str(), fast_writer.write(box).c_str());
}


void __stdcall ClientMsgSender::AfterOnlineMsg(bool success, const std::string &ip, const std::string &port, const std::string &msg)
{
	if(success)
	{
		UserList *list = UserList::Instance();
		if(unsigned int uid = list->GetUserUidByIp(ip))
		{
			list->UpdateUserState(uid, user_online);

			if(int peerTcpPort = list->GetUserTCPPortByUid(uid))
			{
				ClientMsgSender sender(ip, peerTcpPort, -1);
		        sender.UpdateProfileMsg();
			}
		}
	}
}

void __stdcall ClientMsgSender::AfterAddP2PUserMsg(bool success, const std::string &ip, const std::string &port, const std::string &msg)
{
	if(success)
	{
		TempUserList *tempList = TempUserList::Instance();
		TempUser user(ip, atoi(port.c_str()));
		tempList->AddUser(user);

		XLUE_HOSTWND_HANDLE handle = XLUE_GetHostWndByID(MAIN_WND);
	    if(handle)
	    {
		    HWND wnd_handle = (HWND)XLUE_GetHostWndWindowHandle(handle);
		    ::PostMessage(wnd_handle, WM_SHOWSENDEDMSG, 0, 0);
	    }
	}
}

void __stdcall ClientMsgSender::AfterUpdateProfileMsg(SOCKET sock, char const *str)
{
	SendHeadImage(sock);
}

void __stdcall ClientMsgSender::AfterChatWithTextMsg(bool success, const std::string &ip, const std::string &port, const std::string &msg)
{
	if(!success)
	{
		Reader reader;
		Value content, text;
		if(reader.parse(msg, text) && !text["content"].isNull() && reader.parse(text["content"].asCString(), content) 
			&& !content["text"].isNull() && !content["time"].isNull())
		{
			string error("您发送的 “");
			error += content["text"].asCString();
			error += "” 接收失败！";
			UserList *userlist = UserList::Instance();
			if(unsigned int uid = userlist->GetUserUidByIp(ip))
			{
				wstring wtext = V6Util::AnsiToUnicode(error.c_str());
				userlist->UpdateChatContent(uid, wtext, system_content);

				// 在界面上显示
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
					UnreadMsg msg(TEXT_CHAT, uid, wtext);
					UnreadMsgList *list = UnreadMsgList::Instance();
					list->AddUnreadMsg(msg);
					list->DisplayMsg();                                                           //尝试向用户显示该消息
				}
			}
		}
	}
	else
	{
		UserList *list = UserList::Instance();
		if(unsigned int uid = list->GetUserUidByIp(ip))
		{
			list->UpdateUserState(uid, user_online);

			if(int peerTcpPort = list->GetUserTCPPortByUid(uid))
			{
				ClientMsgSender sender(ip, peerTcpPort, -1);
		        sender.UpdateProfileMsg();
			}
		}
	}
}


unsigned int __stdcall ClientMsgSender::SendTextThread(LPVOID pParam)
{
	SendTextThreadPkg* thpkg = (SendTextThreadPkg*)pParam;
	string ip = thpkg->ip;
	string source = thpkg->source;
	string mtext = thpkg->text;
	string sendTime = thpkg->sendTime;
	int udpPort = thpkg->udpPort;
	int tcpPort = thpkg->tcpPort;
	delete[] thpkg->ip; delete[] thpkg->sendTime; delete[] thpkg->source; delete[] thpkg->text; delete[] thpkg; thpkg = NULL;

	Value msg, file_list;
	FastWriter fast_writer;
	
	msg["type"] = TEXT_CHAT;
	msg["source"] = source;
	msg["text"] = mtext;
	msg["time"] = sendTime;

	tr1::regex rxFloat("<\\d+>");  
    tr1::sregex_token_iterator ite(mtext.begin(), mtext.end(), rxFloat), end; 
	
	SendFileList *list = SendFileList::Instance();

	// 检查内容中是否含有文件
	for ( ; ite != end; ++ite)  
	{  
		string w = ite->str();
		string wi = w.substr(1, w.length()-2);
		int ni = atoi(wi.c_str());

		if(SendFileMsg(ip, tcpPort, ni, source))                         // 尝试直接使用tcp向对方发送，同步，延时较大（对方不在内网）
		{
			file_list[wi.c_str()] = "";
		}
		else
		{                                            // 如果失败则先将文件基本信息传给对方，对方反过来连接获取之（对方在内网）
			wstring filepath;
			if(list->GetFilePath(ni, filepath))
			{
				filepath = filepath.substr(filepath.find_last_of(L"\\")+1);
				file_list[wi.c_str()] = V6Util::UnicodeToAnsi(filepath.c_str());
			}
		}
	}
	msg["file_list"] = fast_writer.write(file_list);

	WaitACKMsgSender *sender = WaitACKMsgSender::Instance();
	sender->AddMsg(ip, udpPort, fast_writer.write(msg), AfterChatWithTextMsg);

	return 0;
}


bool ClientMsgSender::SendHeadImage(SOCKET sock)
{
	WCHAR wszModulePath[MAX_PATH];
    GetModuleFileNameW(NULL,wszModulePath,MAX_PATH);
	PathAppend(wszModulePath, USERDATA_RELATIVE_PATH);
	PathAppend(wszModulePath, MYHEADIMAGE_PELATIVE_PATH);
	return send_small_file_by_tcp_socket(sock, V6Util::UnicodeToAnsi(wszModulePath).c_str());
}