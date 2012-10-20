/*
  Copyright (c) 2004-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#ifdef _WIN32
# include "config.h.win"
#elif defined( _WIN32_WCE )
# include "../config.h.win"
#else
# include "config.h"
#endif

#include "client.h"
#include "rostermanager.h"
#include "disco.h"
#include "logsink.h"
#include "nonsaslauth.h"
#include "tag.h"
#include "stanzaextensionfactory.h"
#include "stanzaextension.h"
#include "tlsbase.h"

#if !defined( _WIN32 ) && !defined( _WIN32_WCE )
# include <unistd.h>
#endif

#ifndef _WIN32_WCE
# include <iostream>
# include <sstream>
#else
# include <stdio.h>
#endif

namespace gloox
{

  Client::Client( const std::string& server )
    : ClientBase( XMLNS_CLIENT, server ),
      m_rosterManager( 0 ), m_auth( 0 ),
      m_presence( PresenceAvailable ), m_resourceBound( false ), m_forceNonSasl( false ),
      m_manageRoster( true ), m_doAuth( false ),
      m_streamFeatures( 0 ), m_priority( 0 )
  {
    m_jid.setServer( server );
    init();
  }

  Client::Client( const JID& jid, const std::string& password, int port )
    : ClientBase( XMLNS_CLIENT, password, "", port ),
      m_rosterManager( 0 ), m_auth( 0 ),
      m_presence( PresenceAvailable ), m_resourceBound( false ), m_forceNonSasl( false ),
      m_manageRoster( true ), m_doAuth( true ),
      m_streamFeatures( 0 ), m_priority( 0 )
  {
    m_jid = jid;
    m_server = m_jid.serverRaw();
    init();
  }

  Client::Client( const std::string& username, const std::string& password,
                  const std::string& server, const std::string& resource, int port )
    : ClientBase( XMLNS_CLIENT, password, server, port ),
      m_rosterManager( 0 ), m_auth( 0 ),
      m_presence( PresenceAvailable ), m_resourceBound( false ), m_forceNonSasl( false ),
      m_manageRoster( true ), m_doAuth( true ),
      m_streamFeatures( 0 ), m_priority( 0 )
  {
    m_jid.setUsername( username );
    m_jid.setServer( server );
    m_jid.setResource( resource );

    init();
  }

  Client::~Client()
  {
    removePresenceExtensions();
    delete m_rosterManager;
    delete m_auth;
  }

  void Client::init()
  {
    m_rosterManager = new RosterManager( this );
    m_disco->setIdentity( "client", "bot" );
  }

  void Client::setUsername( const std::string &username )
  {
    m_jid.setUsername( username );
    m_doAuth = true;
  }

  bool Client::handleNormalNode( Stanza *stanza )
  {
    if( stanza->name() == "stream:features" )
    {
      m_streamFeatures = getStreamFeatures( stanza );

      if( m_tls == TLSRequired && !m_encryptionActive
          && ( !m_encryption || !( m_streamFeatures & StreamFeatureStartTls ) ) )
      {
        logInstance().log( LogLevelError, LogAreaClassClient,
                    "Client is configured to require TLS but either the server didn't offer TLS or "
                    "TLS support is not compiled in." );
        disconnect( ConnTlsNotAvailable );
      }
      else if( m_tls > TLSDisabled && m_encryption && !m_encryptionActive
          && ( m_streamFeatures & StreamFeatureStartTls ) )
      {
        notifyStreamEvent( StreamEventEncryption );
        startTls();
      }
      else if( m_sasl )
      {
        if( m_authed )
        {
          if( m_streamFeatures & StreamFeatureBind )
          {
            notifyStreamEvent( StreamEventResourceBinding );
            bindResource();
          }
        }
        else if( m_doAuth && !username().empty() && !password().empty() )
        {
          if( !login() )
          {
            logInstance().log( LogLevelError, LogAreaClassClient,
                                     "the server doesn't support any auth mechanisms we know about" );
            disconnect( ConnNoSupportedAuth );
          }
        }
        else if( m_doAuth && !m_clientCerts.empty() && !m_clientKey.empty()
                 && m_streamFeatures & SaslMechExternal && m_availableSaslMechs & SaslMechExternal )
        {
          notifyStreamEvent( StreamEventAuthentication );
          startSASL( SaslMechExternal );
        }
#ifdef _WIN32
        else if( m_doAuth && m_streamFeatures & SaslMechGssapi && m_availableSaslMechs & SaslMechGssapi )
        {
          notifyStreamEvent( StreamEventAuthentication );
          startSASL( SaslMechGssapi );
        }
#endif
        else if( m_doAuth && m_streamFeatures & SaslMechAnonymous
                 && m_availableSaslMechs & SaslMechAnonymous )
        {
          notifyStreamEvent( StreamEventAuthentication );
          startSASL( SaslMechAnonymous );
        }
        else
        {
          notifyStreamEvent( StreamEventFinished );
          connected();
        }
      }
      else if( m_compress && m_compression && !m_compressionActive
          && ( m_streamFeatures & StreamFeatureCompressZlib ) )
      {
        notifyStreamEvent( StreamEventCompression );
        negotiateCompression( StreamFeatureCompressZlib );
      }
//       else if( ( m_streamFeatures & StreamFeatureCompressDclz )
//               && m_connection->initCompression( StreamFeatureCompressDclz ) )
//       {
//         negotiateCompression( StreamFeatureCompressDclz );
//       }
      else if( m_streamFeatures & StreamFeatureIqAuth )
      {
        notifyStreamEvent( StreamEventAuthentication );
        nonSaslLogin();
      }
      else
      {
        logInstance().log( LogLevelError, LogAreaClassClient,
                           "fallback: the server doesn't support any auth mechanisms we know about" );
        disconnect( ConnNoSupportedAuth );
      }
    }
    else if( ( stanza->name() == "proceed" ) && stanza->hasAttribute( "xmlns", XMLNS_STREAM_TLS ) )
    {
      logInstance().log( LogLevelDebug, LogAreaClassClient, "starting TLS handshake..." );

      if( m_encryption )
      {
        m_encryptionActive = true;
        m_encryption->handshake();
      }
    }
    else if( ( stanza->name() == "failure" ) && stanza->hasAttribute( "xmlns", XMLNS_STREAM_TLS ) )
    {
      logInstance().log( LogLevelError, LogAreaClassClient, "TLS handshake failed (server-side)!" );
      disconnect( ConnTlsFailed );
    }
    else if( ( stanza->name() == "failure" ) && stanza->hasAttribute( "xmlns", XMLNS_COMPRESSION ) )
    {
      logInstance().log( LogLevelError, LogAreaClassClient, "stream compression init failed!" );
      disconnect( ConnCompressionFailed );
    }
    else if( ( stanza->name() == "compressed" ) && stanza->hasAttribute( "xmlns", XMLNS_COMPRESSION ) )
    {
      logInstance().log( LogLevelDebug, LogAreaClassClient, "stream compression inited" );
      m_compressionActive = true;
      header();
    }
    else if( ( stanza->name() == "challenge" ) && stanza->hasAttribute( "xmlns", XMLNS_STREAM_SASL ) )
    {
      logInstance().log( LogLevelDebug, LogAreaClassClient, "processing SASL challenge" );
      processSASLChallenge( stanza->cdata() );
    }
    else if( ( stanza->name() == "failure" ) && stanza->hasAttribute( "xmlns", XMLNS_STREAM_SASL ) )
    {
      logInstance().log( LogLevelError, LogAreaClassClient, "SASL authentication failed!" );
      processSASLError( stanza );
      disconnect( ConnAuthenticationFailed );
    }
    else if( ( stanza->name() == "success" ) && stanza->hasAttribute( "xmlns", XMLNS_STREAM_SASL ) )
    {
      logInstance().log( LogLevelDebug, LogAreaClassClient, "SASL authentication successful" );
      setAuthed( true );
      header();
    }
    else
    {
      if( ( stanza->name() == "iq" ) && stanza->hasAttribute( "id", "bind" ) )
      {
        processResourceBind( stanza );
      }
      else if( ( stanza->name() == "iq" ) && stanza->hasAttribute( "id", "session" ) )
      {
        processCreateSession( stanza );
      }
      else
        return false;
    }

    return true;
  }

  int Client::getStreamFeatures( Stanza *stanza )
  {
    if( stanza->name() != "stream:features" )
      return 0;

    int features = 0;

    if( stanza->hasChild( "starttls", "xmlns", XMLNS_STREAM_TLS ) )
      features |= StreamFeatureStartTls;

    if( stanza->hasChild( "mechanisms", "xmlns", XMLNS_STREAM_SASL ) )
      features |= getSaslMechs( stanza->findChild( "mechanisms" ) );

    if( stanza->hasChild( "bind", "xmlns", XMLNS_STREAM_BIND ) )
      features |= StreamFeatureBind;

    if( stanza->hasChild( "session", "xmlns", XMLNS_STREAM_SESSION ) )
      features |= StreamFeatureSession;

    if( stanza->hasChild( "auth", "xmlns", XMLNS_STREAM_IQAUTH ) )
      features |= StreamFeatureIqAuth;

    if( stanza->hasChild( "register", "xmlns", XMLNS_STREAM_IQREGISTER ) )
      features |= StreamFeatureIqRegister;

    if( stanza->hasChild( "compression", "xmlns", XMLNS_STREAM_COMPRESS ) )
      features |= getCompressionMethods( stanza->findChild( "compression" ) );

    if( features == 0 )
      features = StreamFeatureIqAuth;

    return features;
  }

  int Client::getSaslMechs( Tag *tag )
  {
    int mechs = SaslMechNone;

    if( tag->hasChildWithCData( "mechanism", "DIGEST-MD5" ) )
      mechs |= SaslMechDigestMd5;

    if( tag->hasChildWithCData( "mechanism", "PLAIN" ) )
      mechs |= SaslMechPlain;

    if( tag->hasChildWithCData( "mechanism", "ANONYMOUS" ) )
      mechs |= SaslMechAnonymous;

    if( tag->hasChildWithCData( "mechanism", "EXTERNAL" ) )
      mechs |= SaslMechExternal;

    if( tag->hasChildWithCData( "mechanism", "GSSAPI" ) )
      mechs |= SaslMechGssapi;

    return mechs;
  }

  int Client::getCompressionMethods( Tag *tag )
  {
    int meths = 0;

    if( tag->hasChildWithCData( "method", "zlib" ) )
      meths |= StreamFeatureCompressZlib;

    if( tag->hasChildWithCData( "method", "lzw" ) )
      meths |= StreamFeatureCompressDclz;

    return meths;
  }

  bool Client::login()
  {
    bool retval = true;

    if( m_streamFeatures & SaslMechDigestMd5 && m_availableSaslMechs & SaslMechDigestMd5
        && !m_forceNonSasl )
    {
      notifyStreamEvent( StreamEventAuthentication );
      startSASL( SaslMechDigestMd5 );
    }
    else if( m_streamFeatures & SaslMechPlain && m_availableSaslMechs & SaslMechPlain
             && !m_forceNonSasl )
    {
      notifyStreamEvent( StreamEventAuthentication );
      startSASL( SaslMechPlain );
    }
    else if( m_streamFeatures & StreamFeatureIqAuth || m_forceNonSasl )
    {
      notifyStreamEvent( StreamEventAuthentication );
      nonSaslLogin();
    }
    else
      retval = false;

    return retval;
  }

  void Client::bindResource()
  {
    if( !m_resourceBound )
    {
      Tag *iq = new Tag( "iq" );
      iq->addAttribute( "type", "set" );
      iq->addAttribute( "id", "bind" );
      Tag *b = new Tag( iq, "bind" );
      b->addAttribute( "xmlns", XMLNS_STREAM_BIND );
      if( !resource().empty() )
        new Tag( b, "resource", resource() );

      send( iq );
    }
  }

  void Client::processResourceBind( Stanza *stanza )
  {
    switch( stanza->subtype() )
    {
      case StanzaIqResult:
      {
        Tag *bind = stanza->findChild( "bind" );
        Tag *jid = bind->findChild( "jid" );
        m_jid.setJID( jid->cdata() );
        m_resourceBound = true;

        if( m_streamFeatures & StreamFeatureSession )
          createSession();
        else
          connected();
        break;
      }
      case StanzaIqError:
      {
        Tag *error = stanza->findChild( "error" );
        if( stanza->hasChild( "error", "type", "modify" )
            && error->hasChild( "bad-request", "xmlns", XMLNS_XMPP_STANZAS ) )
        {
          notifyOnResourceBindError( RbErrorBadRequest );
        }
        else if( stanza->hasChild( "error", "type", "cancel" ) )
        {
          if( error->hasChild( "not-allowed", "xmlns", XMLNS_XMPP_STANZAS ) )
            notifyOnResourceBindError( RbErrorNotAllowed );
          else if( error->hasChild( "conflict", "xmlns", XMLNS_XMPP_STANZAS ) )
            notifyOnResourceBindError( RbErrorConflict );
          else
            notifyOnResourceBindError( RbErrorUnknownError );
        }
        else
          notifyOnResourceBindError( RbErrorUnknownError );
        break;
      }
      default:
        break;
    }
  }

  void Client::createSession()
  {
    notifyStreamEvent( StreamEventSessionCreation );
    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "type", "set" );
    iq->addAttribute( "id", "session" );
    Tag *s = new Tag( iq, "session" );
    s->addAttribute( "xmlns", XMLNS_STREAM_SESSION );

    send( iq );
  }

  void Client::processCreateSession( Stanza *stanza )
  {
    switch( stanza->subtype() )
    {
      case StanzaIqResult:
      {
        connected();
        break;
      }
      case StanzaIqError:
      {
        Tag *error = stanza->findChild( "error" );
        if( stanza->hasChild( "error", "type", "wait" )
            && error->hasChild( "internal-server-error", "xmlns", XMLNS_XMPP_STANZAS ) )
        {
          notifyOnSessionCreateError( ScErrorInternalServerError );
        }
        else if( stanza->hasChild( "error", "type", "auth" )
                 && error->hasChild( "forbidden", "xmlns", XMLNS_XMPP_STANZAS ) )
        {
          notifyOnSessionCreateError( ScErrorForbidden );
        }
        else if( stanza->hasChild( "error", "type", "cancel" )
                 && error->hasChild( "conflict", "xmlns", XMLNS_XMPP_STANZAS ) )
        {
          notifyOnSessionCreateError( ScErrorConflict );
        }
        else
          notifyOnSessionCreateError( ScErrorUnknownError );
        break;
      }
      default:
        break;
    }
  }

  void Client::negotiateCompression( StreamFeature method )
  {
    Tag *t = new Tag( "compress" );
    t->addAttribute( "xmlns", XMLNS_COMPRESSION );

    if( method == StreamFeatureCompressZlib )
      new Tag( t, "method", "zlib" );

    if( method == StreamFeatureCompressDclz )
      new Tag( t, "method", "lzw" );

    send( t );
  }

  void Client::addPresenceExtension( StanzaExtension *se )
  {
    m_presenceExtensions.push_back( se );
  }

  void Client::removePresenceExtensions()
  {
    StanzaExtensionList::iterator it = m_presenceExtensions.begin();
    for( ; it != m_presenceExtensions.end(); ++it )
    {
      delete (*it);
    }
    m_presenceExtensions.clear();
  }

  void Client::setPresence( Presence presence, int priority, const std::string& status )
  {
    m_presence = presence;
    m_status = status;

    if( priority < -128 )
      m_priority = -128;
    if( priority > 127 )
      m_priority = 127;
    else
      m_priority = priority;

    sendPresence();
  }

  void Client::disableRoster()
  {
    m_manageRoster = false;
    delete m_rosterManager;
    m_rosterManager = 0;
  }

  void Client::nonSaslLogin()
  {
    if( !m_auth )
      m_auth = new NonSaslAuth( this );
    m_auth->doAuth( m_sid );
  }

  void Client::sendPresence()
  {
    if( m_presence != PresenceUnknown &&
        state() >= StateConnected )
    {
      JID jid;
      Stanza *p = Stanza::createPresenceStanza( jid, m_status, m_presence );
#ifdef _WIN32_WCE
      char tmp[5];
      tmp[4] = '\0';
      sprintf( tmp, "%s", m_priority );
      new Tag( p, "priority", tmp );
#else
      std::ostringstream oss;
      oss << m_priority;
      new Tag( p, "priority", oss.str() );
#endif
      StanzaExtensionList::const_iterator it = m_presenceExtensions.begin();
      for( ; it != m_presenceExtensions.end(); ++it )
      {
        p->addChild( (*it)->tag() );
      }

      send( p );
    }
  }

  void Client::connected()
  {
    if( m_authed )
    {
      if( m_manageRoster )
      {
        notifyStreamEvent( StreamEventRoster );
        m_rosterManager->fill();
      }
      else
        rosterFilled();
    }
    else
    {
      notifyStreamEvent( StreamEventFinished );
      notifyOnConnect();
    }
  }

  void Client::rosterFilled()
  {
    sendPresence();
    notifyStreamEvent( StreamEventFinished );
    notifyOnConnect();
  }

  void Client::disconnect()
  {
    disconnect( ConnUserDisconnected );
  }

  void Client::disconnect( ConnectionError reason )
  {
    m_resourceBound = false;
    m_authed = false;
    m_streamFeatures = 0;
    ClientBase::disconnect( reason );
  }

  void Client::cleanup()
  {
    m_authed = false;
    m_resourceBound = false;
    m_streamFeatures = 0;
  }

}
