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

	// 该方法即为实现 ConnectionListener 监听器接口中的连接成功的 方法实现。     
	virtual void onConnect();

	// 该方法即为实现 ConnectionListener 监听器接口中的连接失败或者断开网络的方法实现。
	virtual void onDisconnect( gloox::ConnectionError e );

	// 该方法即为实现 ConnectionListener 监听器接口中的安全连接成功的方法实现。
	virtual bool onTLSConnect( const gloox::CertInfo& info );


	// 该方法即为实现 MessageSessionHandler 监听器接口中的有新会话建立的方法实现。
	virtual void handleMessageSession( gloox::MessageSession *session );

	// 该方法即为实现 MessageHandler 监听器接口中的有新消息到来的方法实现。
	virtual void handleMessage( gloox::Stanza *stanza, gloox::MessageSession* /*session*/ );

	/**
	* 服务端收到新联系人添加事件
	*/
	virtual void handleItemAdded( const gloox::JID& jid )
	{}

	/**
	* 对方成功添加事件
	*/
	virtual void handleItemSubscribed( const gloox::JID& jid );

	/**
	* 联系人移出事件
	*/
	virtual void handleItemRemoved( const gloox::JID& jid );

	/**
	* 联系人在服务器上的资料被更新事件
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
	* 接收服务器上所有的联系人
	*/
	virtual void handleRoster( const gloox::Roster& roster );

	/**
	* 联系人状态改变事件
	*/
	virtual void handleRosterPresence( const gloox::RosterItem& item, const std::string& resource,
		gloox::Presence presence, const std::string& msg, const int photoType, const std::string& photo );

	/**
	* 自己状态改变事件
	*/
	virtual void handleSelfPresence( const gloox::RosterItem& item, const std::string& resource,
		gloox::Presence presence, const std::string& msg, const std::string& photo )
	{}

	/**
	* 处理添加联系人请求事件，如果对方已被添加则返回值被忽略且应该调用RosterManager::ackSubscriptionRequest()回复对方
	*/
	virtual bool handleSubscriptionRequest( const gloox::JID& jid, const std::string& msg );

	/**
	* 其他联系人将自己删除事件，返回true则自己也在服务器上将对方删除
	*/
	virtual bool handleUnsubscriptionRequest( const gloox::JID& jid, const std::string& msg )
	{
		return true;
	}

	/**
	* 陌生人发来出席消息事件
	*/
	virtual void handleNonrosterPresence( gloox::Stanza* stanza )
	{}

	/**
	* 服务器返回错误
	*/
	virtual void handleRosterError( gloox::Stanza* stanza )
	{}

	/**
	* 接收到个人资料事件
	* vcard不需释放
	*/
	virtual void handleVCard( const gloox::JID& jid, gloox::VCard *vcard );

	/**
	* 个人资料存储事件
	* @param context The operation which yielded the result.
	* @param jid The JID involved.
	* @param se The error, if any. If equal to @c StanzaErrorUndefined no error occured.
	*/
	virtual void handleVCardResult( VCardContext context, const gloox::JID& jid,
		gloox::StanzaError se);

	// 该方法即为实现 LogHandler 监听器接口中的日志记录方法实现。
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
	std::string servername;  // 服务器域名或IP
	gloox::Client *c; // 客户端实例对象指针
	gloox::MessageSession *recv_session; // 接收会话实例对象指针
	gloox::VCardManager *m_vManager;  // 用户名片资料管理者

	int loginType; // 1-通过域名连接，2-通过ip:port方式连接
	bool hasLogin; // 是否已登录

	IXmppClientMessageListener *clientMessageListener; // 客户端消息监听器
	IXmppClientRosterListener *clientRosterListener; // 客户端联系人信息监听器
};