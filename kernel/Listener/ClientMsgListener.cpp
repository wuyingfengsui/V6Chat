#include "ClientMsgListener.h"
#include "../V6ChatDef.h"
#include "../V6Util.h"
#include "../User/UserList.h"
#include "../User/TempUserList.h"
#include "../User/P2PUserInfo.h"
#include "../Sender/ClientMsgSender.h"
#include "../Sender/WaitACKMsgSender.h"
#include "../UnreadMsg/UnreadMsgList.h"
#include "../FileList/SendFileList.h"
#include "../FileList/RecvFileList.h"
#include "../IPSearcher/IPSearcher.h"

#include "../../socket_interface/socket_interface.h"

#include <sstream>
#include <regex>                         // ������ʽ��
#include <Shlwapi.h>
#include <time.h>
#include <XLUE.h>

ClientMsgListener *ClientMsgListener::listener = NULL;
CriticalSection ClientMsgListener::aCritSect;

static const RecvUDPMsgItem _ClientRecvUDPMsgCallBackFuncs[]=
{
	{USER_ONLINE, ClientMsgListener::OnOnlineMsg},
	{USER_ONLINE_ACK, ClientMsgListener::OnOnlineAckMsg},
	{USER_KEEPONLINE, ClientMsgListener::OnKeepOnlineMsg},
	{USER_KEEPONLINE_ACK, ClientMsgListener::OnKeepOnlineAckMsg},
	{USER_OFFLINE, ClientMsgListener::OnOfflineMsg},
	{ADD_P2PUSER_ASK, ClientMsgListener::OnAddUserAskMsg},
	{REPLY_ADD_USER, ClientMsgListener::OnReplyAddUserMsg},
	{TEXT_CHAT, ClientMsgListener::OnChatWithTextMsg},
	{NORMAL_ACK, ClientMsgListener::OnNormalAckMsg},
	{0, NULL}
};

static const RecvTCPMsgItem _ClientRecvTCPMsgCallBackFuncs[]=
{
	{USER_INFO_UPDATE, ClientMsgListener::OnUserInfoUpdateMsg},
	{GET_FILE, ClientMsgListener::OnGetFileMsg},
	{SEND_FILE, ClientMsgListener::OnSendFileMsg},
	{0, NULL}
};

ClientMsgListener *ClientMsgListener::Instance()
{
	SimpleCriticalSection m(aCritSect);

	if(NULL == listener)
		listener = new ClientMsgListener;

	return listener;
}

void ClientMsgListener::Release()
{
	SimpleCriticalSection m(aCritSect);

	if(listener)
	{
		if(SOCKET_ERROR != listener->v4UdpSock)
		{
			if((release_socket(listener->v4UdpSock)) && (release_socket(listener->v4TcpSock)))
			    V6Util::V6Log("%s", "�ͻ���IPv4�ر�ʧ��");
		}

		if(SOCKET_ERROR != listener->v6UdpSock)
		{
			if((release_socket(listener->v6UdpSock)) && (release_socket(listener->v6TcpSock)))
			    V6Util::V6Log("%s", "�ͻ���IPv6�ر�ʧ��");
		}

		delete listener;
		listener = NULL;
	}
}


bool ClientMsgListener::Listen(int IPv4Port, int IPv6Port)
{
	if(hasListen)
		return true;

	int port = IPv4Port;
	srand((unsigned int)time(NULL));

	//����IPv4����
	for(int i=0; i<2*MAX_TRY_TIMES; i++)
	{
		if(-1 == port || 0 == port)
			port = rand()%64512 + 1024;

		std::stringstream sport;
		sport<<port;

		v4TcpSock = build_tcp_listener(AF_INET, sport.str().c_str(), ListenerWithSocketCallBack);
		v4UdpSock = build_udp_listener(AF_INET, sport.str().c_str(), ListenerCallBack);
		
		if((v4TcpSock == SOCKET_ERROR) || (v4UdpSock == SOCKET_ERROR))
		{
			if(v4TcpSock != SOCKET_ERROR)
				release_socket(v4TcpSock);
			if(v4UdpSock != SOCKET_ERROR)
				release_socket(v4UdpSock);
		}
		else
		{
			v4Port = port;
			hasListen = true;
			break;
		}

		port = rand()%64512 + 1024;
	}

	port = IPv6Port;
	//����IPv6����
	for(int i=0; i<2*MAX_TRY_TIMES; i++)
	{
		if(-1 == port|| 0 == port)
			port = rand()%64512 + 1024;

		std::stringstream sport;
		sport<<port;

		v6TcpSock = build_tcp_listener(AF_INET6, sport.str().c_str(), ListenerWithSocketCallBack);
		v6UdpSock = build_udp_listener(AF_INET6, sport.str().c_str(), ListenerCallBack);
		
		if((v6TcpSock == SOCKET_ERROR) || (v6UdpSock == SOCKET_ERROR))
		{
			if(v6TcpSock != SOCKET_ERROR)
				release_socket(v6TcpSock);
			if(v6UdpSock != SOCKET_ERROR)
				release_socket(v6UdpSock);
		}
		else
		{
			v6Port = port;
			hasListen = true;
			break;
		}
		port = rand()%64512 + 1024;
	}

	if(!hasListen)
		V6Util::V6Log("%s", "�ͻ��˼���ʧ��");

	return hasListen;
}

void __stdcall ClientMsgListener::ListenerCallBack(char const*rec, char const*ip, int port)
{
	Json::Reader reader;
	Json::Value msg, content, source;
	if(rec && reader.parse(rec, msg) 
		&& !msg["content"].isNull() && reader.parse(msg["content"].asCString(), content))
	{
		//AnalyzeSource(source);

		for (int i = 0; ; i++)
		{
			if(_ClientRecvUDPMsgCallBackFuncs[i].msgType == 0)
				break;
			else if(!content["type"].isNull() && content["type"].asInt() == _ClientRecvUDPMsgCallBackFuncs[i].msgType && _ClientRecvUDPMsgCallBackFuncs[i].callBack)
			{
				if(msg["mid"].isNull())
					_ClientRecvUDPMsgCallBackFuncs[i].callBack(ip, port, content, NULL);
				else
				    _ClientRecvUDPMsgCallBackFuncs[i].callBack(ip, port, content, msg["mid"].asCString());
				break;
			}
		}
	}
}

void __stdcall ClientMsgListener::ListenerWithSocketCallBack(SOCKET sock, char const*rec, char const*ip, int port)
{
	Json::Reader reader;
	Json::Value msg, source;
	if(rec && reader.parse(rec, msg))
	{
		//AnalyzeSource(source);

		for (int i = 0; ; i++)
		{
			if(_ClientRecvTCPMsgCallBackFuncs[i].msgType == 0)
				break;
			else if(!msg["type"].isNull() && msg["type"].asInt() == _ClientRecvTCPMsgCallBackFuncs[i].msgType && _ClientRecvTCPMsgCallBackFuncs[i].callBack)
			{
				_ClientRecvTCPMsgCallBackFuncs[i].callBack(sock, ip, port, msg);
			}
		}
	}
}


void ClientMsgListener::OnOnlineMsg(const char *ip, int port, const Json::Value &content, const char *mid)
{
	UserList *list = UserList::Instance();

	if(unsigned int uid = list->GetUserUidByIp(ip))
	{
		list->UpdateUserState(uid, user_online);                                  //�����û�״̬�����ߣ�
		list->UpdateUserPort(uid, port, port);                                    //�����û�ʹ�ö˿ڣ�ż����仯��

		ClientMsgSender sender(ip, -1, port);    
		sender.TestOnlineMsgAck(mid);                                       //ACK
		if(int peerTcpPort = list->GetUserTCPPortByUid(uid))
		{
			ClientMsgSender tsender(ip, peerTcpPort, -1);
			tsender.UpdateProfileMsg();                            //��Է������ҵ���������
		}
	}
}

void ClientMsgListener::OnOnlineAckMsg(const char *ip, int port, const Json::Value &content, const char *mid)
{
	UserList *list = UserList::Instance();

    if(unsigned int uid = list->GetUserUidByIp(ip))
	{
		list->UpdateUserState(uid, user_online);                                      //�����û�״̬�����ߣ�

		std::stringstream smid(mid), sport;
		unsigned long lmid = 0;

		WaitACKMsgSender *sender = WaitACKMsgSender::Instance();
		smid>>lmid;

		Json::FastWriter jsonW;
		sender->DeleteMsg(true, lmid, jsonW.write(content).c_str(), ip);

		if(int peerTcpPort = list->GetUserTCPPortByUid(uid))
		{
			ClientMsgSender msgSender(ip, peerTcpPort, -1);
			msgSender.UpdateProfileMsg();                            //��Է������ҵ���������
		}
	}
}

void ClientMsgListener::OnKeepOnlineMsg(const char *ip, int port, const Json::Value &content, const char *mid)
{
	UserList *list = UserList::Instance();

	if(unsigned int uid = list->GetUserUidByIp(ip))
	{
		list->UpdateUserState(uid, user_online);                                      //ˢ���û�״̬�����ߣ��������ʱ�䣩
		ClientMsgSender sender(ip, -1, port);
		sender.KeepOnlineMsgAck(mid);
	}
}

void ClientMsgListener::OnKeepOnlineAckMsg(const char *ip, int port, const Json::Value &content, const char *mid)
{
	UserList *list = UserList::Instance();
	if(unsigned int uid = list->GetUserUidByIp(ip))
		list->UpdateUserState(uid, user_online);                                      //ˢ���û�״̬�����ߣ�
}

void ClientMsgListener::OnOfflineMsg(const char *ip, int port, const Json::Value &content, const char *mid)
{
	UserList *list = UserList::Instance();
	if(unsigned int uid = list->GetUserUidByIp(ip))
		list->UpdateUserState(uid, user_offline);                                     //�����û�״̬�����ߣ�
}

void ClientMsgListener::OnAddUserAskMsg(const char *ip, int port, const Json::Value &content, const char *mid)
{
	Json::Reader reader;
	ClientMsgSender sender(ip, -1, port);
	sender.NormalMsgAck(mid);

	Json::Value peerInfo;
	if(reader.parse(content["my_info"].asCString(), peerInfo))
	{
		std::wstring pname = V6Util::AnsiToUnicode(peerInfo["nick_name"].asString().c_str());
		pAddr myAddr = _GetAddress(ip);
		std::wstring message = pname + L"("+ V6Util::AnsiToUnicode(myAddr->Country) + V6Util::AnsiToUnicode(myAddr->Local) + L")";
		UnreadMsg msg(ADD_P2PUSER_ASK, pname, ip, port, message);

		UnreadMsgList *list = UnreadMsgList::Instance();
		list->AddUnreadMsg(msg);                                                      //����Ӻ����������δ����Ϣջ
		list->DisplayMsg();                                                           //�������û���ʾ����Ϣ
	}
}

void ClientMsgListener::OnReplyAddUserMsg(const char *ip, int port, const Json::Value &content, const char *mid)
{
	Json::Reader reader;
	TempUserList *tmpList = TempUserList::Instance();
	
	if(tmpList->FindUser(ip))                                                         //����ʱ�û��б��в��ң�ȷ��������Է����͹���Ӻ�������
	{
		ClientMsgSender sender(ip, port, port);

		if(!content["my_info"].isNull())
		{
			Json::Value peerInfo;
			if(reader.parse(content["my_info"].asCString(), peerInfo))
			{
				Handle<UserInfo> newuser(new P2PUserInfo(
						0, 
						V6Util::AnsiToUnicode(peerInfo["nick_name"].asString().c_str()), 
						DEFAULT_GROUP_NAME, 
						peerInfo["sex"].asInt(), 
						peerInfo["age"].asInt(), 
						V6Util::AnsiToUnicode(peerInfo["introduction"].asCString()), 
						L"", 
						ip, 
						port,                                                         // �ֽ׶�tcp�˿���udp�˿�һ��
						port));

				 UserList *userlist = UserList::Instance();
			     userlist->AddUser(newuser);                                          //��Ӵ˺���
				 sender.TestOnlineMsg();
			}
		}

		sender.NormalMsgAck(mid);
		sender.UpdateProfileMsg();

		tmpList->DeleteUser(ip);                                                  //ɾ����ʱ�б��еĶ�Ӧ��Ϣ
	}
}

void ClientMsgListener::OnChatWithTextMsg(const char *ip, int port, const Json::Value &content, const char *mid)
{
	UserList *list = UserList::Instance();

	if(unsigned int uid = list->GetUserUidByIp(ip))
	{
		if(!content["text"].isNull() && !content["time"].isNull())
		{
			UserList *userlist = UserList::Instance();
			ClientMsgSender sender(ip, userlist->GetUserTCPPortByUid(uid), port);    
			sender.NormalMsgAck(mid);                                       // ACK

			std::string text(content["text"].asCString());

			if(!content["file_list"].isNull())                                    // �����ı��е��ļ����Ϊ�Լ������б��е��ļ���ţ�����еĻ���
			{
				Json::Value file_list;
				Json::Reader reader;
				if(reader.parse(content["file_list"].asCString(), file_list))
				{
					if(file_list.size() != 0)
					{
						std::stringstream suid;
						int uid = userlist->GetUserUidByIp(ip);
						suid << uid;

						WCHAR wszModulePath[MAX_PATH];
						GetModuleFileName(NULL,wszModulePath,MAX_PATH);
						PathAppend(wszModulePath, USERDATA_RELATIVE_PATH);
						PathAppend(wszModulePath, USERHEAD_RECVFILE_PATH);
						std::string filePath = V6Util::UnicodeToAnsi(wszModulePath);
						filePath = filePath + "\\";
						std::string fileName = suid.str() + "_";
						SYSTEMTIME  sysTime;
						::GetLocalTime(&sysTime);
						char st[15];
						sprintf_s(st, "%4d%02d%02d%02d%02d%02d", sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
						fileName = fileName + st + "_";

						for(Json::Value::iterator i = file_list.begin(); i != file_list.end(); ++i)
						{
							int fileid = atoi(i.memberName());
							RecvFileList *rfl = RecvFileList::Instance();
							std::stringstream sfileid, snfileid;

							if((*i).asString().length() == 0)                    // �ļ��Ѵ��������������������tcp�ٶȽ�������δ���ݵ��ϲ㣬���ʵ��ӳٵȴ�
							{
								bool suget = false;
								for(int iter = 0; iter < MAX_TRY_TIMES; ++iter)
								{
									int recvfilenumber;
									if(rfl->GetDownloadedFileNumber(uid, i.memberName(), recvfilenumber))
									{
										sfileid<<"<"<<fileid<<">";
										snfileid<<"<"<<recvfilenumber<<">";
										suget = true;
										break;
									}

									::Sleep(100);
								}

								// ʧ��
								if(!suget)
								{
									sfileid<<"<"<<fileid<<">";
									snfileid<<"<0>";
								}
							}
							else
							{
								std::string newfileName = fileName + i.memberName() + "_" + (*i).asString();
								std::string filePathName = filePath + newfileName;
								if(sender.GetFileMsg(fileid, filePathName))
								{
									int nfileid = rfl->AddFileItem(V6Util::AnsiToUnicode(newfileName.c_str()));

									sfileid<<"<"<<fileid<<">";
									snfileid<<"<"<<nfileid<<">";
								}
								else
								{
									sfileid<<"<"<<fileid<<">";
									snfileid<<"<0>";
								}
							}
							std::tr1::regex rxFloat(sfileid.str());
							text = std::tr1::regex_replace(text, rxFloat, snfileid.str());
						}
					}
				}
			}

			std::wstring wtext = V6Util::AnsiToUnicode(text.c_str());
			userlist = UserList::GetObj();
			userlist->UpdateChatContent(uid, wtext, user_content);               // ��¼��������������
			userlist->ShowChatContent(uid);
		}
	}
}

void ClientMsgListener::OnNormalAckMsg(const char *ip, int port, const Json::Value &content, const char *mid)
{
	std::stringstream smid(mid);
	int lmid = 0;
	smid>>lmid;
	WaitACKMsgSender *sender = WaitACKMsgSender::Instance();

	if(sender->FindMsg(lmid))
	{
		Json::FastWriter jsonW;
		sender->DeleteMsg(true, lmid, jsonW.write(content).c_str(), ip);
	}
}

void ClientMsgListener::OnUserInfoUpdateMsg(SOCKET sock, const char *ip, int port, const Json::Value &content)
{
	UserList *list = UserList::Instance();
	if(unsigned int uid = list->GetUserUidByIp(ip))
	{
		ReceiveHeadImage(sock, uid);

		Json::Value peerInfo;
		Json::Reader reader;
		if(reader.parse(content["my_info"].asCString(), peerInfo))
			list->UpdateUser(uid, 
			&V6Util::AnsiToUnicode(peerInfo["nick_name"].asCString()), 
			NULL, 
			peerInfo["sex"].asInt(), 
			peerInfo["age"].asInt(), 
			&V6Util::AnsiToUnicode(peerInfo["introduction"].asCString()), 
			NULL);
	}
}

void ClientMsgListener::OnGetFileMsg(SOCKET sock, const char *ip, int port, const Json::Value &content)
{
	UserList *list = UserList::Instance();

	if(unsigned int uid = list->GetUserUidByIp(ip))
	{
		SendFileList *list = SendFileList::Instance();
		std::wstring filepath;
		bool has = list->GetFilePath(content["number"].asInt(), filepath);
		if(has)
			send_small_file_by_tcp_socket(sock, V6Util::UnicodeToAnsi(filepath.c_str()).c_str());
	}
}

void ClientMsgListener::OnSendFileMsg(SOCKET sock, const char *ip, int port, const Json::Value &content)
{
	UserList *list = UserList::Instance();

	if(unsigned int uid = list->GetUserUidByIp(ip))
	{
		int fid = content["number"].asInt();
		std::stringstream suid, sfid;
		suid << uid;
		sfid << fid;

		WCHAR wszModulePath[MAX_PATH];
		GetModuleFileName(NULL,wszModulePath,MAX_PATH);
		PathAppend(wszModulePath, USERDATA_RELATIVE_PATH);
		PathAppend(wszModulePath, USERHEAD_RECVFILE_PATH);
		std::string filePath = V6Util::UnicodeToAnsi(wszModulePath);
		filePath = filePath + "\\";
		std::string fileName = suid.str() + "_";
		SYSTEMTIME  sysTime;
		::GetLocalTime(&sysTime);
		char st[15];
		sprintf_s(st, "%4d%02d%02d%02d%02d%02d", sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
		fileName = fileName + st + "_";
		std::string newfileName = fileName + sfid.str() + "_" + content["filename"].asCString();
		std::string filePathName = filePath + newfileName;

		if(receive_small_file_by_tcp_socket(sock, filePathName.c_str()))
		{
			RecvFileList *rfl = RecvFileList::Instance();
			rfl->AddFileItem(uid, sfid.str(), V6Util::AnsiToUnicode(newfileName.c_str()), true);
		}
	}
}

bool ClientMsgListener::ReceiveHeadImage(SOCKET sock, unsigned int uid)
{
	WCHAR wszModulePath[MAX_PATH];
	GetModuleFileName(NULL,wszModulePath,MAX_PATH);
    PathAppend(wszModulePath, USERDATA_RELATIVE_PATH);
	PathAppend(wszModulePath, USERHEAD_RELATIVE_PATH);
	std::wstring filepath(wszModulePath);
	filepath +=  L"\\";
	std::wstringstream sid;
	sid<<uid;
	filepath += sid.str();
	filepath += USERHEAD_EXT;
	return receive_small_file_by_tcp_socket(sock, V6Util::UnicodeToAnsi(filepath.c_str()).c_str());
}

void ClientMsgListener::AnalyzeSource(const Json::Value &source)
{
	/*
	user->nick_name = V6Util::AnsiToUnicode(peer["nick_name"].asCString());
	user->sex = peer["sex"].asInt();
	user->age = peer["age"].asInt();
	user->introduction = V6Util::AnsiToUnicode(peer["introduction"].asCString());
	*/
}