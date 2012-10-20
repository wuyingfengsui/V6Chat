#include "XmppClientInner.h"

using namespace gloox;

XmppClientInner::XmppClientInner()
		:c(NULL), recv_session(NULL), m_vManager(NULL), hasLogin(false), loginType(0),
		clientMessageListener(NULL), clientRosterListener(NULL)
{}

XmppClientInner::~XmppClientInner()
{
	if(hasLogin)
	{
		c->disconnect();
	}

	if(c)
	{
		delete m_vManager;
		delete c;
	}
}

void XmppClientInner::Start(const char *servername, const char *account, const char *password)
{
	loginType = 1;

	c = new Client(servername);
	// �û���
	c->setUsername(account);
	// ����
	c->setPassword(password);
	// ������Դ
	c->setResource("gloox");

	/* ע����ϵ���б��������ʵ��handleItemAdded��handleItemSubscribed��handleItemRemoved��handleItemUpdated��
	* handleItemUnsubscribed��handleRoster��handleRosterPresence��handleSelfPresence��handleSubscriptionRequest��
	* handleUnsubscriptionRequest��handleNonrosterPresence��handleRosterError�ӿ�
	*/
	c->rosterManager()->registerRosterListener( this );
	// ע������״̬��������ʵ��onConnect��onDisconnect��onTLSConnect�ӿ�
	c->registerConnectionListener( this );
	// ע��Ựsession�������������»Ự����ʱ�����handleMessageSession�ӿ�
	c->registerMessageSessionHandler( this, 0 );
	// ע��log��������ʵ��onConnect��onDisconnect��onTLSConnect�ӿ�
	// c->logInstance().registerLogHandler( LogLevelDebug, LogAreaAll, this );

	m_vManager = new VCardManager( c );

	/*
	// ����֤����֤
	StringList ca;
	ca.push_back( "/path/to/cacert.crt" );
	c->setCACerts( ca );
	*/

	this->servername = servername;

	// ���� j->connect(false)ʱ����ʵ��������������ӣ����ӳɹ��᷵���档
	// Connect ��������Ϊ false ��ʾ��������ʽ���ӣ������Ϊ true ����Ϊ������ʽ���ӣ�����ȴ���������
	c->connect();
}

void XmppClientInner::Start(const char *account, const char *password, const char *serverip, int port)
{
	loginType = 2;
	c = new Client(account, password, serverip, "gloox", port);

	/* ע����ϵ���б��������ʵ��handleItemAdded��handleItemSubscribed��handleItemRemoved��handleItemUpdated��
	* handleItemUnsubscribed��handleRoster��handleRosterPresence��handleSelfPresence��handleSubscriptionRequest��
	* handleUnsubscriptionRequest��handleNonrosterPresence��handleRosterError�ӿ�
	*/
	c->rosterManager()->registerRosterListener( this );
	// ע������״̬��������ʵ��onConnect��onDisconnect��onTLSConnect�ӿ�
	c->registerConnectionListener( this );
	// ע��Ựsession�������������»Ự����ʱ�����handleMessageSession�ӿ�
	c->registerMessageSessionHandler( this, 0 );
	// ע��log��������ʵ��onConnect��onDisconnect��onTLSConnect�ӿ�
	// c->logInstance().registerLogHandler( LogLevelDebug, LogAreaAll, this );

	m_vManager = new VCardManager( c );

	// ���� j->connect(false)ʱ����ʵ��������������ӣ����ӳɹ��᷵���档
	// Connect ��������Ϊ false ��ʾ��������ʽ���ӣ������Ϊ true ����Ϊ������ʽ���ӣ�����ȴ���������
	c->connect();
}

void XmppClientInner::Close()
{
	if(hasLogin)
	{
		c->disconnect();
		hasLogin = false;
	}

	if(c)
	{
		delete m_vManager;
		delete c;
		c = NULL;
		m_vManager = NULL;
	}
}

void XmppClientInner::onConnect()
{
	if(clientMessageListener)
		clientMessageListener->Connect();
	hasLogin = true;
}

void XmppClientInner::onDisconnect( ConnectionError e )
{
	if(clientMessageListener)
		clientMessageListener->Disconnect();
	hasLogin = false;
}

bool XmppClientInner::onTLSConnect( const CertInfo& info )
{
	// �˴����¸�ֵ�ǿ���ֱ����ip:port���ӷ������ķ�ʽ(��������openfire)
	if(2 == loginType)
	{
		this->servername = info.server.substr(3);               // ������Ϊopenfire�������ĵĻ�info.server�ĸ�ʽΪCN-��������
		c->setServer(this->servername);
	}
	return true;                 // ����true��ʾ���ܴ�TLS���ӣ��緵��false��رմ�����
}

void XmppClientInner::handleMessageSession( MessageSession *session )
{
	// ����ֻ����һ���Ự�����Ƚ�δ���ٵĻỰ���٣�����еĻ������Է��ڴ�й¶
	c->disposeMessageSession( recv_session );
	recv_session = session;
	// ע��˻Ự����Ϣ����������������Ϣ����ʱ�����handleMessage�ӿ�
	recv_session->registerMessageHandler( this );
}

void XmppClientInner::handleMessage( Stanza *stanza, MessageSession* /*session*/ )
{
	if(clientMessageListener)
	{
		std::string nick_name;
		RosterItem *rt = c->rosterManager()->getRosterItem(stanza->from());
		if(rt)
			nick_name = rt->name();
		clientMessageListener->RecvText(stanza->subtype(), stanza->from().server(), stanza->from().username(), stanza->body(), nick_name);
	}
}

void XmppClientInner::handleItemSubscribed( const JID& jid )
{
	if(clientRosterListener)
		clientRosterListener->RosterAdded(jid.server(), jid.username());
}

void XmppClientInner::handleItemRemoved( const JID& jid )
{
	if(clientRosterListener)
		clientRosterListener->RosterDeleted(jid.server(), jid.username());
}

void XmppClientInner::handleRoster( const Roster& roster )
{
	if(clientRosterListener)
		clientRosterListener->RecvAllRosters(roster);
}

void XmppClientInner::handleRosterPresence( const gloox::RosterItem& item, const std::string& resource,
	gloox::Presence presence, const std::string& msg,  const int photoType, const std::string& photo )
{
	if(clientRosterListener)
		clientRosterListener->RosterPresence(item, presence, msg, photoType, photo);
}

bool XmppClientInner::handleSubscriptionRequest( const JID& jid, const std::string& msg )
{
	if(c->rosterManager()->getRosterItem(jid))
	{
		c->rosterManager()->ackSubscriptionRequest(jid, true);
		return true;    // ���������ֵ������
	}
	else
	{
		std::string back_name;  // ��ע��
		if(clientRosterListener && clientRosterListener->RecvRosterSubscriptionRequest(jid.server(), jid.username(), msg, back_name))
		{
			StringList groups;
			JID id( jid );
			c->rosterManager()->subscribe( id, back_name, groups);
			return true;
		}
		else
			return false;
	}
}

void XmppClientInner::handleVCard( const JID& jid, VCard *vcard )
{
	if(clientRosterListener)
		clientRosterListener->RecvVCard(
		jid.server(), 
		jid.username(), 
		vcard->formattedname(), 
		vcard->nickname(), 
		vcard->desc(), 
		vcard->bday(),
		vcard->photo());
}

void XmppClientInner::handleVCardResult( VCardContext context, const JID& jid,
	StanzaError se)
{
	/*
	printf( "vcard result: context: %d, jid: %s, error: %d\n", context, jid.full().c_str(), se );
	*/
}

void XmppClientInner::handleLog( LogLevel level, LogArea area, const std::string& message )
{
	// printf("log: level: %d, area: %d, %s\n", level, area, message.c_str() );
}

void XmppClientInner::SendText(const char *peerAccount, const char *text)
{
	JID jid;
	std::string pa(peerAccount);
	pa = pa + "@" + servername;
	jid.setJID(pa);

	// ����һ���»Ự
	MessageSession *send_session = new MessageSession(c, jid);
	send_session->send(text); 

	c->disposeMessageSession(send_session);            // ���ô˷�����ʾ��������ɺ�j���Զ����ٴ˻Ự�������� delete send_session
}

void XmppClientInner::AddRoster(const std::string &account, const std::string &name, const std::string &group)
{
	StringList groups;
	if(group != "")
		groups.push_back(group);

	c->rosterManager()->add(JID(account + "@" + servername), name, groups);
	c->rosterManager()->subscribe(JID(account + "@" + servername), name, groups);
}

void XmppClientInner::DeleteRoster(const std::string &account)
{
	c->rosterManager()->remove(account + "@" + servername);
}

void XmppClientInner::GetVCard(const std::string &account)
{
	JID jid;
	std::string pa = account + "@" + servername;
	jid.setJID(pa);

	m_vManager->fetchVCard(jid, this);
}