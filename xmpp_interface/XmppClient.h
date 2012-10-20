#pragma once

#include "XmppClientRosterListener.h"
#include "XmppClientMessageListener.h"
#include "XmppFileTransfer.h"
#include "XmppClientInner.h"

#include <string>

#ifdef XMPP_INTERFACE_EXPORTS
class _declspec(dllexport) XmppClient
#else
class __declspec(dllimport) XmppClient
#endif
{
public:
	void Start(const char *servername, const char *account, const char *password);

	void Start(const char *account, const char *password, const char *serverip, int port);

	void Close();

	gloox::Client *GetClient()
	{
		return ci.GetClient();
	}

	void SendText(const char *peerAccount, const char *text);

	void AddRoster(const std::string &account, const std::string &name, const std::string &group);

	void DeleteRoster(const std::string &account);

	void GetVCard(const std::string &account);

	bool GetHasLogin() const;

	void SetClientMessageListener(IXmppClientMessageListener *clientMessageListener);

	void SetClientRosterListener(IXmppClientRosterListener *clientRosterListener);

private:
	XmppClientInner ci;
};