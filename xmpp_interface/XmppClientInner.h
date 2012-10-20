#pragma once

#include <string>

#include "gloox\client.h"
#include "gloox\rostermanager.h"
#include "gloox\connectionlistener.h"
#include "gloox\rosterlistener.h"
#include "gloox\disco.h"
#include "gloox\messagesessionhandler.h"
#include "gloox\messageeventfilter.h"
#include "gloox\messageeventhandler.h"
#include "gloox\messagehandler.h"
#include "gloox\chatstatefilter.h"
#include "gloox\chatstatehandler.h"
#include "gloox\vcardhandler.h"
#include "gloox\vcardmanager.h"

#include "XmppClientRosterListener.h"
#include "XmppClientMessageListener.h"

class XmppClientInner : public gloox::MessageSessionHandler, public gloox::ConnectionListener, public gloox::MessageHandler,
	public gloox::RosterListener,
	public gloox::VCardHandler,
	public gloox::LogHandler
{
public:
	XmppClientInner();
	~XmppClientInner();

	void Start(const char *servername, const char *account, const char *password);

	void Start(const char *account, const char *password, const char *serverip, int port);

	void Close();

	// �÷�����Ϊʵ�� ConnectionListener �������ӿ��е����ӳɹ��� ����ʵ�֡�     
	virtual void onConnect();

	// �÷�����Ϊʵ�� ConnectionListener �������ӿ��е�����ʧ�ܻ��߶Ͽ�����ķ���ʵ�֡�
	virtual void onDisconnect( gloox::ConnectionError e );

	// �÷�����Ϊʵ�� ConnectionListener �������ӿ��еİ�ȫ���ӳɹ��ķ���ʵ�֡�
	virtual bool onTLSConnect( const gloox::CertInfo& info );


	// �÷�����Ϊʵ�� MessageSessionHandler �������ӿ��е����»Ự�����ķ���ʵ�֡�
	virtual void handleMessageSession( gloox::MessageSession *session );

	// �÷�����Ϊʵ�� MessageHandler �������ӿ��е�������Ϣ�����ķ���ʵ�֡�
	virtual void handleMessage( gloox::Stanza *stanza, gloox::MessageSession* /*session*/ );

	/**
	* ������յ�����ϵ������¼�
	*/
	virtual void handleItemAdded( const gloox::JID& jid )
	{}

	/**
	* �Է��ɹ�����¼�
	*/
	virtual void handleItemSubscribed( const gloox::JID& jid );

	/**
	* ��ϵ���Ƴ��¼�
	*/
	virtual void handleItemRemoved( const gloox::JID& jid );

	/**
	* ��ϵ���ڷ������ϵ����ϱ������¼�
	*/
	virtual void handleItemUpdated( const gloox::JID& jid )
	{}

	/**
	* Reimplement this function if you want to be notified about items which
	* removed subscription authorization.
	* @param jid The item's full address.
	*/
	virtual void handleItemUnsubscribed( const gloox::JID& jid )
	{}

	/**
	* ���շ����������е���ϵ��
	*/
	virtual void handleRoster( const gloox::Roster& roster );

	/**
	* ��ϵ��״̬�ı��¼�
	*/
	virtual void handleRosterPresence( const gloox::RosterItem& item, const std::string& resource,
		gloox::Presence presence, const std::string& msg, const int photoType, const std::string& photo );

	/**
	* �Լ�״̬�ı��¼�
	*/
	virtual void handleSelfPresence( const gloox::RosterItem& item, const std::string& resource,
		gloox::Presence presence, const std::string& msg, const std::string& photo )
	{}

	/**
	* ���������ϵ�������¼�������Է��ѱ�����򷵻�ֵ��������Ӧ�õ���RosterManager::ackSubscriptionRequest()�ظ��Է�
	*/
	virtual bool handleSubscriptionRequest( const gloox::JID& jid, const std::string& msg );

	/**
	* ������ϵ�˽��Լ�ɾ���¼�������true���Լ�Ҳ�ڷ������Ͻ��Է�ɾ��
	*/
	virtual bool handleUnsubscriptionRequest( const gloox::JID& jid, const std::string& msg )
	{
		return true;
	}

	/**
	* İ���˷�����ϯ��Ϣ�¼�
	*/
	virtual void handleNonrosterPresence( gloox::Stanza* stanza )
	{}

	/**
	* ���������ش���
	*/
	virtual void handleRosterError( gloox::Stanza* stanza )
	{}

	/**
	* ���յ����������¼�
	* vcard�����ͷ�
	*/
	virtual void handleVCard( const gloox::JID& jid, gloox::VCard *vcard );

	/**
	* �������ϴ洢�¼�
	* @param context The operation which yielded the result.
	* @param jid The JID involved.
	* @param se The error, if any. If equal to @c StanzaErrorUndefined no error occured.
	*/
	virtual void handleVCardResult( VCardContext context, const gloox::JID& jid,
		gloox::StanzaError se);

	// �÷�����Ϊʵ�� LogHandler �������ӿ��е���־��¼����ʵ�֡�
	virtual void handleLog( gloox::LogLevel level, gloox::LogArea area, const std::string& message );

	void SendText(const char *peerAccount, const char *text);

	void AddRoster(const std::string &account, const std::string &name, const std::string &group);

	void DeleteRoster(const std::string &account);

	void GetVCard(const std::string &account);

	bool GetHasLogin() const
	{
		return hasLogin;
	}

	gloox::Client *GetClient()
	{
		return c;
	}

	gloox::Roster *GetRosters()
	{
		return c->rosterManager()->roster();
	}

	void SetClientMessageListener(IXmppClientMessageListener *clientMessageListener)
	{
		this->clientMessageListener = clientMessageListener;
	}

	void SetClientRosterListener(IXmppClientRosterListener *clientRosterListener)
	{
		this->clientRosterListener = clientRosterListener;
	}

private:
	std::string servername;  // ������������IP
	gloox::Client *c; // �ͻ���ʵ������ָ��
	gloox::MessageSession *recv_session; // ���ջỰʵ������ָ��
	gloox::VCardManager *m_vManager;  // �û���Ƭ���Ϲ�����

	int loginType; // 1-ͨ���������ӣ�2-ͨ��ip:port��ʽ����
	bool hasLogin; // �Ƿ��ѵ�¼

	IXmppClientMessageListener *clientMessageListener; // �ͻ�����Ϣ������
	IXmppClientRosterListener *clientRosterListener; // �ͻ�����ϵ����Ϣ������
};