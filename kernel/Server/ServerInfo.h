#pragma once
#include "../V6ChatDef.h"
#include "../V6Util.h"

#include <string>

#include "../../xmpp_interface/XmppClient.h"
#include "../../xmpp_interface/XmppFT.h"
#include "../Listener/ServerMsgListener.h"

struct ConnectThreadPkg
{
	std::string ip;
	int port;
	std::string uid;
	std::string upassword;
	XmppClient *c;
};

class ServerInfo
{
public:
	ServerInfo(const unsigned int id, const std::wstring &name, const std::string &ip, const int port, const std::wstring &description, const std::string &uid, const std::string &upassword)
		:id(id), name(name), ip(ip), port(port), description(description), uid(uid), upassword(upassword), state(server_disconnect),
		msgListener(id),
		haschange(false)
	{
		client.SetClientMessageListener(&msgListener);
		client.SetClientRosterListener(&msgListener);
	}

	~ServerInfo()
	{
		if(state == server_connected)
			client.Close();
	}

	// 异步操作
	void Connect();

	void DisConnect()
	{
		client.Close();
	}

	void SetState(ServerState state)
	{
		this->state = state;
	}

	unsigned int GetId() const
	{
		return id;
	}

	void SetId(unsigned int id)
	{
		if(this->id != id)
		{
			this->id = id;
			msgListener.SetId(id);
			this->haschange = true;
		}
	}

	const std::wstring &GetName() const
	{
		return name;
	}

	void SetName(const std::wstring &name)
	{
		if(this->name != name)
		{
			this->name = name;
			this->haschange = true;
		}
	}

	const std::string &GetIp() const
	{
		return ip;
	}

	void SetIp(const std::string &ip)
	{
		if(this->ip != ip)
		{
			this->ip = ip;
			this->haschange = true;
		}
	}

	int GetPort() const
	{
		return port;
	}

	void SetPort(int port)
	{
		if(this->port != port)
		{
			this->port = port;
			this->haschange = true;
		}
	}

	ServerState GetState() const
	{
		return state;
	}

	const std::wstring &GetDescription() const
	{
		return description;
	}

	void SetDescription(const std::wstring &description)
	{
		if(this->description != description)
		{
			this->description = description;
			this->haschange = true;
		}
	}

	const std::string &GetUid() const
	{
		return uid;
	}

	void SetUid(const std::string &uid)
	{
		if(this->uid != uid)
		{
			this->uid = uid;
			this->haschange = true;
		}
	}

	const std::string &GetUpassword()
	{
		return upassword;
	}

	void SetUpassword(const std::string &upassword)
	{
		if(this->upassword != upassword)
		{
			this->upassword = upassword;
			this->haschange = true;
		}
	}

	bool HasChange()
	{
		return haschange;
	}

	void SendText(const std::string &account, const std::string &text)         // text-utf8格式
	{
		if(state == server_connected)
			client.SendText(account.c_str(), text.c_str());
	}

	void AskUserInfo(const std::string &account)
	{
		if(state == server_connected)
			client.GetVCard(account);
	}

	void GetSearchUsersResult(std::list<VCardInfo>& vCardList)
	{
		if(state == server_connected)
			msgListener.GetSearchUsersResult(vCardList);
	}

	void ClearSearchUsersResult()
	{
		msgListener.ClearSearchUsersResult();
	}

	void AddUser(const std::string &account)
	{
		if(state == server_connected)
			client.AddRoster(account, account, "");
	}

	void DeleteUser(const std::string &account)
	{
		if(state == server_connected)
			client.DeleteRoster(account);
	}

	bool StartFileTransfer()
	{
		if(state == server_connected)
		{
			bool ret = xft.StartFileTransfer(client.GetClient());
			if(ret)
				xft.SetFileTransferListener(&msgListener);
			return ret;
		}

		return false;
	}

	bool SendFile(const std::string &account, const std::wstring &filePath, std::string &rsid);

private:
	static unsigned int __stdcall ConnectThread(void *parm);

	bool haschange;
	unsigned int id;
	std::wstring name;
	std::string ip;              // 服务器域名或IP，utf-8格式
	int port;

	std::wstring description;

	std::string uid;             // utf-8格式
	std::string upassword;

	ServerState state;

	XmppClient client;
	XmppFT xft;
	ServerMsgListener msgListener;
};