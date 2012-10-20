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
	// 用户名
	c->setUsername(account);
	// 密码
	c->setPassword(password);
	// 设置资源
	c->setResource("gloox");

	/* 注册联系人列表监听器，实现handleItemAdded，handleItemSubscribed，handleItemRemoved，handleItemUpdated，
	* handleItemUnsubscribed，handleRoster，handleRosterPresence，handleSelfPresence，handleSubscriptionRequest，
	* handleUnsubscriptionRequest，handleNonrosterPresence和handleRosterError接口
	*/
	c->rosterManager()->registerRosterListener( this );
	// 注册连接状态监听器，实现onConnect，onDisconnect和onTLSConnect接口
	c->registerConnectionListener( this );
	// 注册会话session监听器，当有新会话建立时会调用handleMessageSession接口
	c->registerMessageSessionHandler( this, 0 );
	// 注册log监听器，实现onConnect，onDisconnect和onTLSConnect接口
	// c->logInstance().registerLogHandler( LogLevelDebug, LogAreaAll, this );

	m_vManager = new VCardManager( c );

	/*
	// 数字证书认证
	StringList ca;
	ca.push_back( "/path/to/cacert.crt" );
	c->setCACerts( ca );
	*/

	this->servername = servername;

	// 调用 j->connect(false)时，即实现与服务器的连接，连接成功会返回真。
	// Connect 函数参数为 false 表示不阻塞方式连接，而如果为 true ，则为阻塞方式连接，即会等待接收数据
	c->connect();
}

void XmppClientInner::Start(const char *account, const char *password, const char *serverip, int port)
{
	loginType = 2;
	c = new Client(account, password, serverip, "gloox", port);

	/* 注册联系人列表监听器，实现handleItemAdded，handleItemSubscribed，handleItemRemoved，handleItemUpdated，
	* handleItemUnsubscribed，handleRoster，handleRosterPresence，handleSelfPresence，handleSubscriptionRequest，
	* handleUnsubscriptionRequest，handleNonrosterPresence和handleRosterError接口
	*/
	c->rosterManager()->registerRosterListener( this );
	// 注册连接状态监听器，实现onConnect，onDisconnect和onTLSConnect接口
	c->registerConnectionListener( this );
	// 注册会话session监听器，当有新会话建立时会调用handleMessageSession接口
	c->registerMessageSessionHandler( this, 0 );
	// 注册log监听器，实现onConnect，onDisconnect和onTLSConnect接口
	// c->logInstance().registerLogHandler( LogLevelDebug, LogAreaAll, this );

	m_vManager = new VCardManager( c );

	// 调用 j->connect(false)时，即实现与服务器的连接，连接成功会返回真。
	// Connect 函数参数为 false 表示不阻塞方式连接，而如果为 true ，则为阻塞方式连接，即会等待接收数据
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
	// 此处重新赋值是考虑直接用ip:port连接服务器的方式(仅考虑了openfire)
	if(2 == loginType)
	{
		this->servername = info.server.substr(3);               // 服务器为openfire且是中文的话info.server的格式为CN-服务器名
		c->setServer(this->servername);
	}
	return true;                 // 返回true表示接受此TLS连接，如返回false则关闭此连接
}

void XmppClientInner::handleMessageSession( MessageSession *session )
{
	// 此例只考虑一个会话，故先将未销毁的会话销毁（如果有的话），以防内存泄露
	c->disposeMessageSession( recv_session );
	recv_session = session;
	// 注册此会话的消息监听器，当有新消息到来时会调用handleMessage接口
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
		return true;    // 此情况返回值被忽略
	}
	else
	{
		std::string back_name;  // 备注名
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

	// 建立一个新会话
	MessageSession *send_session = new MessageSession(c, jid);
	send_session->send(text); 

	c->disposeMessageSession(send_session);            // 调用此方法表示当发送完成后j会自动销毁此会话，故无需 delete send_session
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