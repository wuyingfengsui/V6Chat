#include "XmppClient.h"

#include <time.h>

void XmppClient::Start(const char *servername, const char *account, const char *password)
{
	ci.Start(servername, account, password);
}

void XmppClient::Start(const char *account, const char *password, const char *serverip, int port)
{
	ci.Start(account, password, serverip, port);
}

void XmppClient::Close()
{
	ci.Close();
}

void XmppClient::SendText(const char *peerAccount, const char *text)
{
	ci.SendText(peerAccount, text);
}

void XmppClient::AddRoster(const std::string &account, const std::string &name, const std::string &group)
{
	ci.AddRoster(account, name, group);
}

void XmppClient::DeleteRoster(const std::string &account)
{
	ci.DeleteRoster(account);
}

void XmppClient::GetVCard(const std::string &account)
{
	ci.GetVCard(account);
}

bool XmppClient::GetHasLogin() const
{
	return ci.GetHasLogin();
}

void XmppClient::SetClientMessageListener(IXmppClientMessageListener *clientMessageListener)
{
	ci.SetClientMessageListener(clientMessageListener);
}

void XmppClient::SetClientRosterListener(IXmppClientRosterListener *clientRosterListener)
{
	ci.SetClientRosterListener(clientRosterListener);
}