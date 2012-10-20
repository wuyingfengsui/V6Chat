/*
  Copyright (c) 2004-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef CLIENT_H__
#define CLIENT_H__

#include "clientbase.h"
#include "iqhandler.h"
#include "stanza.h"

#include <string>

namespace gloox
{

  class RosterManager;
  class NonSaslAuth;

  /**
   * @brief This class implements a basic Jabber Client.
   *
   * It supports @ref sasl_auth as well as TLS (Encryption), which can be
   * switched on/off separately. They are used automatically if the server supports them.
   *
   * To use, create a new Client instance and feed it connection credentials, either in the Constructor or
   * afterwards using the setters. You should then register packet handlers implementing the corresponding
   * Interfaces (ConnectionListener, PresenceHandler, MessageHandler, IqHandler, SubscriptionHandler),
   * and call @ref connect() to establish the connection to the server.<br>
   *
   * @note While the MessageHandler interface is still available (and will be in future versions)
   * it is now recommended to use the new @link gloox::MessageSession MessageSession @endlink for any
   * serious messaging.
   *
   * Simple usage example:
   * @code
   * using namespace gloox;
   *
   * void TestProg::doIt()
   * {
   *   Client* j = new Client( "user@server/resource", "password" );
   *   j->registerPresenceHandler( this );
   *   j->disco()->setVersion( "TestProg", "1.0" );
   *   j->disco()->setIdentity( "client", "bot" );
   *   j->connect();
   * }
   *
   * virtual void TestProg::presenceHandler( Stanza *stanza )
   * {
   *   // handle incoming presence packets here
   * }
   * @endcode
   *
   * However, you can skip the presence handling stuff if you make use of the RosterManager.
   *
   * By default, the library handles a few (incoming) IQ namespaces on the application's behalf. These
   * include:
   * @li jabber:iq:roster: by default the server-side roster is fetched and handled. Use
   * @ref rosterManager() and @ref RosterManager to interact with the Roster.
   * @li XEP-0092 (Software Version): If no version is specified, a name of "based on gloox" with
   * gloox's current version is announced.
   * @li XEP-0030 (Service Discovery): All supported/available services are announced. No items are
   * returned.
   * @note As of gloox 0.9, by default a priority of 0 is sent along with the initial presence.
   * @note As of gloox 0.9, initial presence is automatically sent. Presence: available, Priority: 0.
   * To disable sending of initial Presence use setPresence() with a value of PresenceUnavailable
   * prior to connecting.
   *
   * @section sasl_auth SASL Authentication
   *
   * Besides the simple, IQ-based authentication (XEP-0078), gloox supports several SASL (Simple
   * Authentication and Security Layer, RFC 2222) authentication mechanisms.
   * @li DIGEST-MD5: This mechanism is preferred over all other mechanisms if username and password are
   * provided to the Client instance. It is secure even without TLS encryption.
   * @li PLAIN: This mechanism is used if DIGEST-MD5 is not available. It is @b not secure without
   * encryption.
   * @li ANONYMOUS This mechanism is used if neither username nor password are set. The server generates
   * random, temporary username and resource and may restrict available services.
   * @li EXTERNAL This mechanism is currently only available if client certificate and private key
   * are provided. The server tries to figure out who the client is by external means -- for instance,
   * using the provided certificate or even the IP address. (The restriction to certificate/key
   * availability is likely to be lifted in the future.)
   *
   * Of course, all these mechanisms are not tried unless the server offers them.
   *
   * @author Jakob Schroeter <js@camaya.net>
   */
  class GLOOX_API Client : public ClientBase
  {
    public:

      friend class NonSaslAuth;
      friend class Parser;

      /**
       * Constructs a new Client which can be used for account registration only.
       * SASL and TLS are on by default. The port will be determined by looking up SRV records.
       * Alternatively, you can set the port explicitly by calling @ref setPort().
       * @param server The server to connect to.
       */
      Client( const std::string& server );

      /**
       * Constructs a new Client.
       * SASL and TLS are on by default. This should be the default constructor for most use cases.
       * The server address will be taken from the JID. The actual host will be resolved using SRV
       * records. The domain part of the JID is used as a fallback in case no SRV record is found, or
       * you can set the server address separately by calling @ref setServer().
       * @param jid A full Jabber ID used for connecting to the server.
       * @param password The password used for authentication.
       * @param port The port to connect to. The default of -1 means to look up the port via DNS SRV.
       */
      Client( const JID& jid, const std::string& password, int port = -1 );

      /**
       * Constructs a new Client.
       * SASL and TLS are on by default.
       * The actual host will be resolved using SRV records. The server value is used as a fallback
       * in case no SRV record is found.
       * @param username The username/local part of the JID.
       * @param resource The resource part of the JID.
       * @param password The password to use for authentication.
       * @param server The Jabber ID'S server part and the host name to connect to. If those are different
       * for your setup, use the second constructor instead.
       * @param port The port to connect to. The default of -1 means to look up the port via DNS SRV.
       */
      Client( const std::string& username, const std::string& password,
              const std::string& server, const std::string& resource, int port = -1 );

      /**
       * Virtual destructor.
       */
      virtual ~Client();

      /**
       * Use this function to @b re-try to bind a resource only in case you were notified about an
       * error by means of ConnectionListener::onResourceBindError().
       * You may (or should) use setResource() before.
       */
      void bindResource();

      /**
       * Returns the current prepped resource.
       * @return The resource used to connect.
       */
      const std::string& resource() const { return m_jid.resource(); }

      /**
       * Returns the current priority.
       * @return The priority of the current resource.
       */
      int priority() const { return m_priority; }

      /**
       * Sets the username to use to connect to the XMPP server.
       * @param username The username to authenticate with.
       */
      void setUsername( const std::string &username );

      /**
       * Sets the resource to use to connect to the XMPP server.
       * @param resource The resource to use to log into the server.
       */
      void setResource( const std::string &resource ) { m_jid.setResource( resource ); }

      /**
       * Use this function to set the entity's presence.
       * If used prior to establishing a connection, the set values will be sent with
       * the initial presence stanza.
       * If used while a connection already is established a repective presence stanza will be
       * sent out immediately.
       * @param presence The Presence value to set.
       * @param priority An optional priority value. Legal values: -128 <= priority <= 127
       * @param msg An optional message describing the presence state.
       * @since 0.9
       */
      void setPresence( Presence presence, int priority = 0, const std::string& msg = "" );

      /**
       * Returns the current presence.
       * @return The current presence.
       */
      Presence presence() const { return m_presence; }

      /**
       * Returns the current status message.
       * @return The current status message.
       */
      const std::string& status() const { return m_status; }

      /**
       * Use this function to add a StanzaExtension which will be sent with eacha nd every
       * Presence Stanza that is sent out. Use cases include
       * signed presence (@link gloox::GPGSigned GPGSigned @endlink, XEP-0027),
       * VCard avatar notifications (@link gloox::VCardUpdate VCardUpdate @endlink, XEP-0153),
       * and others (see @link gloox::StanzaExtension StanzaExtension @endlink for derived classes.
       * @param se The StanzaExtension to add. Client will become the owner of the given
       * StanzaExtension.
       * @note Currently there is no way to selectively remove an extension. Use
       * removePresenceExtensions() to remove all extensions.
       */
      void addPresenceExtension( StanzaExtension *se );

      /**
       * Use this function to remove all extensions added using addPresenceExtension().
       */
      void removePresenceExtensions();

      /**
       * This is a temporary hack to enforce Non-SASL login. You should not need to use it.
       * @param force Whether to force non-SASL auth. Default @b true.
       * @deprecated Please update the server to properly support SASL instead.
       */
      GLOOX_DEPRECATED void setForceNonSasl( bool force = true ) { m_forceNonSasl = force; }

      /**
       * Disables the automatic roster management.
       * You have to keep track of incoming presence yourself if
       * you want to have a roster.
       */
      void disableRoster();

      /**
       * This function gives access to the @c RosterManager object.
       * @return A pointer to the RosterManager.
       */
      RosterManager* rosterManager() { return m_rosterManager; }

      /**
       * Disconnects from the server.
       */
      void disconnect();

      /**
       * Initiates a login attempt (currently SASL External not supported).
       * This is useful after registering a new account. Simply use setUsername() and setPassword(),
       * and call login().
       * @return @b True if a login attempt could be started, @b false otherwise. A return
       * value of @b true does not indicate that login was successful.
       */
      bool login();

    protected:
      /**
       * Initiates non-SASL login.
       */
      void nonSaslLogin();

    private:
      virtual void handleStartNode() {}
      virtual bool handleNormalNode( Stanza *stanza );
      virtual void disconnect( ConnectionError reason );
      int getStreamFeatures( Stanza *stanza );
      int getSaslMechs( Tag *tag );
      int getCompressionMethods( Tag *tag );
      void processResourceBind( Stanza *stanza );
      void processCreateSession( Stanza *stanza );
      void sendPresence();
      void createSession();
      void negotiateCompression( StreamFeature method );
      void connected();
      virtual void rosterFilled();
      virtual void cleanup();

      void init();

      RosterManager *m_rosterManager;
      NonSaslAuth *m_auth;

      StanzaExtensionList m_presenceExtensions;

      Presence m_presence;
      std::string m_status;

      bool m_resourceBound;
      bool m_forceNonSasl;
      bool m_manageRoster;
      bool m_doAuth;

      int m_streamFeatures;
      int m_priority;

  };

}

#endif // CLIENT_H__
