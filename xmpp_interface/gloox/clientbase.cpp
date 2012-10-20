/*
  Copyright (c) 2005-2008 by Jakob Schroeter <js@camaya.net>
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

#include "clientbase.h"
#include "connectionbase.h"
#include "tlsbase.h"
#include "compressionbase.h"
#include "connectiontcpclient.h"
#include "disco.h"
#include "messagesessionhandler.h"
#include "parser.h"
#include "tag.h"
#include "stanza.h"
#include "connectionlistener.h"
#include "iqhandler.h"
#include "messagehandler.h"
#include "presencehandler.h"
#include "rosterlistener.h"
#include "subscriptionhandler.h"
#include "loghandler.h"
#include "taghandler.h"
#include "mucinvitationhandler.h"
#include "jid.h"
#include "base64.h"
#include "md5.h"
#include "tlsdefault.h"
#include "compressionzlib.h"

#include <cstdlib>
#include <string>
#include <map>
#include <list>
#include <algorithm>

#ifndef _WIN32_WCE
# include <sstream>
# include <iomanip>
#endif

namespace gloox
{

  ClientBase::ClientBase( const std::string& ns, const std::string& server, int port )
    : m_connection( 0 ), m_encryption( 0 ), m_compression( 0 ), m_disco( 0 ), m_namespace( ns ),
      m_xmllang( "en" ), m_server( server ), m_compressionActive( false ), m_encryptionActive( false ),
      m_compress( true ), m_authed( false ), m_sasl( true ), m_tls( TLSOptional ), m_port( port ),
      m_availableSaslMechs( SaslMechAll ),
      m_statisticsHandler( 0 ), m_mucInvitationHandler( 0 ),
      m_messageSessionHandlerChat( 0 ), m_messageSessionHandlerGroupchat( 0 ),
      m_messageSessionHandlerHeadline( 0 ), m_messageSessionHandlerNormal( 0 ),
      m_parser( 0 ), m_authError( AuthErrorUndefined ), m_streamError( StreamErrorUndefined ),
      m_streamErrorAppCondition( 0 ), m_selectedSaslMech( SaslMechNone ),
      m_idCount( 0 ), m_autoMessageSession( false )
  {
    init();
  }

  ClientBase::ClientBase( const std::string& ns, const std::string& password,
                          const std::string& server, int port )
    : m_connection( 0 ), m_encryption( 0 ), m_compression( 0 ), m_disco( 0 ), m_namespace( ns ),
      m_password( password ),
      m_xmllang( "en" ), m_server( server ), m_compressionActive( false ), m_encryptionActive( false ),
      m_compress( true ), m_authed( false ), m_block( false ), m_sasl( true ), m_tls( TLSOptional ),
      m_port( port ), m_availableSaslMechs( SaslMechAll ),
      m_statisticsHandler( 0 ), m_mucInvitationHandler( 0 ),
      m_messageSessionHandlerChat( 0 ), m_messageSessionHandlerGroupchat( 0 ),
      m_messageSessionHandlerHeadline( 0 ), m_messageSessionHandlerNormal( 0 ),
      m_parser( 0 ), m_authError( AuthErrorUndefined ), m_streamError( StreamErrorUndefined ),
      m_streamErrorAppCondition( 0 ), m_selectedSaslMech( SaslMechNone ),
      m_idCount( 0 ), m_autoMessageSession( false )
  {
    init();
  }

  void ClientBase::init()
  {
    if( !m_disco )
    {
      m_disco = new Disco( this );
      m_disco->setVersion( "based on gloox", GLOOX_VERSION );
    }

    m_streamError = StreamErrorUndefined;

    m_block = false;

    m_stats.totalBytesSent = 0;
    m_stats.totalBytesReceived = 0;
    m_stats.compressedBytesSent = 0;
    m_stats.compressedBytesReceived = 0;
    m_stats.uncompressedBytesSent = 0;
    m_stats.uncompressedBytesReceived = 0;
    m_stats.totalStanzasSent = 0;
    m_stats.totalStanzasReceived = 0;
    m_stats.iqStanzasSent = 0;
    m_stats.iqStanzasReceived = 0;
    m_stats.messageStanzasSent = 0;
    m_stats.messageStanzasReceived = 0;
    m_stats.s10nStanzasSent = 0;
    m_stats.s10nStanzasReceived = 0;
    m_stats.presenceStanzasSent = 0;
    m_stats.presenceStanzasReceived = 0;
    m_stats.encryption = false;
    m_stats.compression = false;

    cleanup();
  }

  ClientBase::~ClientBase()
  {
    delete m_connection;
    delete m_encryption;
    delete m_compression;
    delete m_parser;
    delete m_disco;

    MessageSessionList::const_iterator it = m_messageSessions.begin();
    for( ; it != m_messageSessions.end(); ++it )
      delete (*it);

    PresenceJidHandlerList::const_iterator it1 = m_presenceJidHandlers.begin();
    for( ; it1 != m_presenceJidHandlers.end(); ++it1 )
      delete (*it1).jid;
  }

  ConnectionError ClientBase::recv( int timeout )
  {
    if( !m_connection || m_connection->state() == StateDisconnected )
      return ConnNotConnected;

    return m_connection->recv( timeout );
  }

  bool ClientBase::connect( bool block )
  {
    if( m_server.empty() )
      return false;

    if( !m_parser )
      m_parser = new Parser( this );

    if( !m_connection )
      m_connection = new ConnectionTCPClient( this, m_logInstance, m_server, m_port );

    if( m_connection->state() >= StateConnecting )
      return true;

    if( !m_encryption )
      m_encryption = getDefaultEncryption();

    if( m_encryption )
    {
      m_encryption->setCACerts( m_cacerts );
      m_encryption->setClientCert( m_clientKey, m_clientCerts );
    }

    if( !m_compression )
      m_compression = getDefaultCompression();

    m_logInstance.log( LogLevelDebug, LogAreaClassClientbase, "This is gloox " + GLOOX_VERSION
                                                              + ", connecting..." );
    m_block = block;
    ConnectionError ret = m_connection->connect();
    return ret == ConnNoError;
  }

  void ClientBase::handleTag( Tag *tag )
  {
    if( !tag )
    {
      logInstance().log( LogLevelDebug, LogAreaClassClientbase, "stream closed" );
      disconnect( ConnStreamClosed );
      return;
    }

    Stanza *stanza = new Stanza( tag );

    logInstance().log( LogLevelDebug, LogAreaXmlIncoming, stanza->xml() );
    ++m_stats.totalStanzasReceived;

    if( tag->name() == "stream:stream" )
    {
      const std::string& version = stanza->findAttribute( "version" );
      if( !checkStreamVersion( version ) )
      {
        logInstance().log( LogLevelDebug, LogAreaClassClientbase, "This server is not XMPP-compliant"
            " (it does not send a 'version' attribute). Please fix it or try another one.\n" );
        disconnect( ConnStreamVersionError );
        return;
      }

      m_sid = stanza->findAttribute( "id" );
      handleStartNode();
    }
    else if( tag->name() == "stream:error" )
    {
      handleStreamError( stanza );
      disconnect( ConnStreamError );
    }
    else
    {
      if( !handleNormalNode( stanza ) )
      {
        switch( stanza->type() )
        {
          case StanzaIq:
            notifyIqHandlers( stanza );
            ++m_stats.iqStanzasReceived;
            break;
          case StanzaPresence:
            notifyPresenceHandlers( stanza );
            ++m_stats.presenceStanzasReceived;
            break;
          case StanzaS10n:
            notifySubscriptionHandlers( stanza );
            ++m_stats.s10nStanzasReceived;
            break;
          case StanzaMessage:
            notifyMessageHandlers( stanza );
            ++m_stats.messageStanzasReceived;
            break;
          default:
            notifyTagHandlers( tag );
            break;
        }
      }
    }

    if( m_statisticsHandler )
      m_statisticsHandler->handleStatistics( getStatistics() );

    delete stanza;
  }

  void ClientBase::handleCompressedData( const std::string& data )
  {
    if( m_encryption && m_encryptionActive )
      m_encryption->encrypt( data );
    else if( m_connection )
      m_connection->send( data );
    else
      m_logInstance.log( LogLevelError, LogAreaClassClientbase, "Compression finished, but chain broken" );
  }

  void ClientBase::handleDecompressedData( const std::string& data )
  {
    if( m_parser )
      parse( data );
    else
      m_logInstance.log( LogLevelError, LogAreaClassClientbase, "Decompression finished, but chain broken" );
  }

  void ClientBase::handleEncryptedData( const TLSBase* /*base*/, const std::string& data )
  {
    if( m_connection )
      m_connection->send( data );
    else
      m_logInstance.log( LogLevelError, LogAreaClassClientbase, "Encryption finished, but chain broken" );
  }

  void ClientBase::handleDecryptedData( const TLSBase* /*base*/, const std::string& data )
  {
    if( m_compression && m_compressionActive )
      m_compression->decompress( data );
    else if( m_parser )
      parse( data );
    else
      m_logInstance.log( LogLevelError, LogAreaClassClientbase, "Decryption finished, but chain broken" );
  }

  void ClientBase::handleHandshakeResult( const TLSBase* /*base*/, bool success, CertInfo &certinfo )
  {
    if( success )
    {
      if( !notifyOnTLSConnect( certinfo ) )
      {
        logInstance().log( LogLevelError, LogAreaClassClientbase, "Server's certificate rejected!" );
        disconnect( ConnTlsFailed );
      }
      else
      {
        logInstance().log( LogLevelDebug, LogAreaClassClientbase, "connection encryption active" );
        header();
      }
    }
    else
    {
      logInstance().log( LogLevelError, LogAreaClassClientbase, "TLS handshake failed!" );
      disconnect( ConnTlsFailed );
    }
  }

  void ClientBase::handleReceivedData( const ConnectionBase* /*connection*/, const std::string& data )
  {
    if( m_encryption && m_encryptionActive )
      m_encryption->decrypt( data );
    else if( m_compression && m_compressionActive )
      m_compression->decompress( data );
    else if( m_parser )
      parse( data );
    else
      m_logInstance.log( LogLevelError, LogAreaClassClientbase, "Received data, but chain broken" );
  }

  void ClientBase::handleConnect( const ConnectionBase* /*connection*/ )
  {
    header();
    if( m_block && m_connection )
    {
      m_connection->receive();
    }
  }

  void ClientBase::handleDisconnect( const ConnectionBase* /*connection*/, ConnectionError reason )
  {
    if( m_connection )
      m_connection->cleanup();
    notifyOnDisconnect( reason );
  }

  void ClientBase::disconnect( ConnectionError reason )
  {
    if( m_connection && m_connection->state() >= StateConnecting )
    {
      if( reason != ConnTlsFailed )
        send( "</stream:stream>" );

      m_connection->disconnect();
      m_connection->cleanup();
      m_parser->reset();

      if( m_encryption )
        m_encryption->cleanup();

      m_encryptionActive = false;
      m_compressionActive = false;

      notifyOnDisconnect( reason );
    }
  }

  void ClientBase::parse( const std::string& data )
  {
    if( m_parser && !m_parser->feed( data ) )
    {
      m_logInstance.log( LogLevelError, LogAreaClassClientbase, "parse error: " + data );
      Tag* e = new Tag( "stream:error" );
      new Tag( e, "restricted-xml", "xmlns", XMLNS_XMPP_STREAM );
      send( e );
      disconnect( ConnParseError );
    }
  }

  void ClientBase::header()
  {
    std::string head = "<?xml version='1.0' ?>";
    head += "<stream:stream to='" + m_jid.server() + "' xmlns='" + m_namespace + "' ";
    head += "xmlns:stream='http://etherx.jabber.org/streams'  xml:lang='" + m_xmllang + "' ";
    head += "version='" + XMPP_STREAM_VERSION_MAJOR + "." + XMPP_STREAM_VERSION_MINOR + "'>";
    send( head );
  }

  bool ClientBase::hasTls()
  {
#if defined( HAVE_GNUTLS ) || defined( HAVE_OPENSSL ) || defined( HAVE_WINTLS )
    return true;
#else
    return false;
#endif
  }

  void ClientBase::startTls()
  {
    Tag *start = new Tag( "starttls" );
    start->addAttribute( "xmlns", XMLNS_STREAM_TLS );
    send( start );
  }

  void ClientBase::setServer( const std::string &server )
  {
    m_server = server;
	m_jid.setServer(server);                      // 有修改，解决直接用ip登录时采用TLS验证时发生的错误
    if( m_connection )
      m_connection->setServer( server );
  }

  void ClientBase::setClientCert( const std::string& clientKey, const std::string& clientCerts )
  {
    m_clientKey = clientKey;
    m_clientCerts = clientCerts;
  }

  void ClientBase::startSASL( SaslMechanism type )
  {
    m_selectedSaslMech = type;

    Tag *a = new Tag( "auth" );
    a->addAttribute( "xmlns", XMLNS_STREAM_SASL );

    switch( type )
    {
      case SaslMechDigestMd5:
        a->addAttribute( "mechanism", "DIGEST-MD5" );
        break;
      case SaslMechPlain:
      {
        a->addAttribute( "mechanism", "PLAIN" );

        std::string tmp;
        if( m_authzid )
          tmp += m_authzid.bare();

        tmp += '\0';
        tmp += m_jid.username();
        tmp += '\0';
        tmp += m_password;
        a->setCData( Base64::encode64( tmp ) );
        break;
      }
      case SaslMechAnonymous:
        a->addAttribute( "mechanism", "ANONYMOUS" );
        a->setCData( getID() );
        break;
      case SaslMechExternal:
        a->addAttribute( "mechanism", "EXTERNAL" );
        if( m_authzid )
          a->setCData( Base64::encode64( m_authzid.bare() ) );
        else
          a->setCData( Base64::encode64( m_jid.bare() ) );
        break;
      case SaslMechGssapi:
      {
#ifdef _WIN32
        a->addAttribute( "mechanism", "GSSAPI" );
// The client calls GSS_Init_sec_context, passing in 0 for
// input_context_handle (initially) and a targ_name equal to output_name
// from GSS_Import_Name called with input_name_type of
// GSS_C_NT_HOSTBASED_SERVICE and input_name_string of
// "service@hostname" where "service" is the service name specified in
// the protocol's profile, and "hostname" is the fully qualified host
// name of the server.  The client then responds with the resulting
// output_token.
        std::string token;
        a->setCData( Base64::encode64( token ) );
//         etc... see gssapi-sasl-draft.txt
#else
        logInstance().log( LogLevelError, LogAreaClassClientbase,
                    "GSSAPI is not supported on this platform. You should never see this." );
#endif
        break;
      }
      default:
        break;
    }

    send( a );
  }

  void ClientBase::processSASLChallenge( const std::string& challenge )
  {
    Tag *t = new Tag( "response" );
    t->addAttribute( "xmlns", XMLNS_STREAM_SASL );

    const std::string& decoded = Base64::decode64( challenge );

    switch( m_selectedSaslMech )
    {
      case SaslMechDigestMd5:
      {
        if( decoded.substr( 0, 7 ) == "rspauth" )
        {
          break;
        }
        std::string realm;
        size_t r_pos = decoded.find( "realm=" );
        if( r_pos != std::string::npos )
        {
          size_t r_end = decoded.find( "\"", r_pos + 7 );
          realm = decoded.substr( r_pos + 7, r_end - (r_pos + 7 ) );
        }
        else
          realm = m_jid.server();

        size_t n_pos = decoded.find( "nonce=" );
        if( n_pos == std::string::npos )
        {
          return;
        }

        size_t n_end = decoded.find( "\"", n_pos + 7 );
        while( decoded.substr( n_end-1, 1 ) == "\\" )
          n_end = decoded.find( "\"", n_end + 1 );
        std::string nonce = decoded.substr( n_pos + 7, n_end - ( n_pos + 7 ) );

        std::string cnonce;
#ifdef _WIN32_WCE
        char cn[4*8+1];
        for( int i = 0; i < 4; ++i )
          sprintf( cn + i*8, "%08x", rand() );
        cnonce.assign( cn, 4*8 );
#else
        std::ostringstream cn;
        for( int i = 0; i < 4; ++i )
          cn << std::hex << std::setw( 8 ) << std::setfill( '0' ) << rand();
        cnonce = cn.str();
#endif

        MD5 md5;
        md5.feed( m_jid.username() );
        md5.feed( ":" );
        md5.feed( realm );
        md5.feed( ":" );
        md5.feed( m_password );
        md5.finalize();
        const std::string& a1_h = md5.binary();
        md5.reset();
        md5.feed( a1_h );
        md5.feed( ":" );
        md5.feed( nonce );
        md5.feed( ":" );
        md5.feed( cnonce );
        md5.finalize();
        const std::string& a1  = md5.hex();
        md5.reset();
        md5.feed( "AUTHENTICATE:xmpp/" );
        md5.feed( m_jid.server() );
        md5.finalize();
        const std::string& a2 = md5.hex();
        md5.reset();
        md5.feed( a1 );
        md5.feed( ":" );
        md5.feed( nonce );
        md5.feed( ":00000001:" );
        md5.feed( cnonce );
        md5.feed( ":auth:" );
        md5.feed( a2 );
        md5.finalize();
        const std::string& response_value = md5.hex();

        std::string response = "username=\"" + m_jid.username() + "\",realm=\"" + realm;
        response += "\",nonce=\""+ nonce + "\",cnonce=\"" + cnonce;
        response += "\",nc=00000001,qop=auth,digest-uri=\"xmpp/" + m_jid.server() + "\",response=";
        response += response_value;
        response += ",charset=utf-8";

        if( m_authzid )
          response += ",authzid=" + m_authzid.bare();

        t->setCData( Base64::encode64( response ) );

        break;
      }
      case SaslMechGssapi:
#ifdef _WIN32
        // see gssapi-sasl-draft.txt
#else
        m_logInstance.log( LogLevelError, LogAreaClassClientbase,
                           "Huh, received GSSAPI challenge?! This should have never happened!" );
#endif
        break;
      default:
        // should never happen.
        break;
    }

    send( t );
  }

  void ClientBase::processSASLError( Stanza *stanza )
  {
    if( stanza->hasChild( "aborted" ) )
      m_authError = SaslAborted;
    else if( stanza->hasChild( "incorrect-encoding" ) )
      m_authError = SaslIncorrectEncoding;
    else if( stanza->hasChild( "invalid-authzid" ) )
      m_authError = SaslInvalidAuthzid;
    else if( stanza->hasChild( "invalid-mechanism" ) )
      m_authError = SaslInvalidMechanism;
    else if( stanza->hasChild( "mechanism-too-weak" ) )
      m_authError = SaslMechanismTooWeak;
    else if( stanza->hasChild( "not-authorized" ) )
      m_authError = SaslNotAuthorized;
    else if( stanza->hasChild( "temporary-auth-failure" ) )
      m_authError = SaslTemporaryAuthFailure;
  }

  void ClientBase::send( Tag *tag )
  {
    if( !tag )
      return;

    send( tag->xml() );

    switch( tag->type() )
    {
      case StanzaIq:
        ++m_stats.iqStanzasSent;
        break;
      case StanzaMessage:
        ++m_stats.messageStanzasSent;
        break;
      case StanzaS10n:
        ++m_stats.s10nStanzasSent;
        break;
      case StanzaPresence:
        ++m_stats.presenceStanzasSent;
        break;
      default:
        break;
    }
    ++m_stats.totalStanzasSent;

    delete tag;

    if( m_statisticsHandler )
      m_statisticsHandler->handleStatistics( getStatistics() );
  }

  void ClientBase::send( const std::string& xml )
  {
    if( m_connection && m_connection->state() == StateConnected )
    {
      if( m_compression && m_compressionActive )
        m_compression->compress( xml );
      else if( m_encryption && m_encryptionActive )
        m_encryption->encrypt( xml );
      else
        m_connection->send( xml );

      logInstance().log( LogLevelDebug, LogAreaXmlOutgoing, xml );
    }
  }

  StatisticsStruct ClientBase::getStatistics()
  {
//     if( m_connection )
//       m_connection->getStatistics( m_stats.totalBytesReceived, m_stats.totalBytesSent,
//                                    m_stats.compressedBytesReceived, m_stats.compressedBytesSent,
//                                    m_stats.uncompressedBytesReceived, m_stats.uncompressedBytesSent,
//                                    m_stats.compression );
    return m_stats;
  }

  ConnectionState ClientBase::state() const
  {
    return m_connection ? m_connection->state() : StateDisconnected;
  }

  void ClientBase::whitespacePing()
  {
    send( " " );
  }

  void ClientBase::xmppPing( const JID& to )
  {
    const std::string& id = getID();

    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "to", to.full() );
    iq->addAttribute( "id", id );
    iq->addAttribute( "type", "get" );
    Tag *p = new Tag( iq, "ping" );
    p->addAttribute( "xmlns", XMLNS_XMPP_PING );

    send( iq );
  }

  const std::string ClientBase::getID()
  {
#ifdef _WIN32_WCE
    char r[8+1];
    sprintf( r, "%08x", rand() );
    std::string ret( r, 8 );
    return std::string( "uid" ) + ret;
#else
    std::ostringstream oss;
    oss << ++m_idCount;
    return std::string( "uid" ) + oss.str();
#endif
  }

  bool ClientBase::checkStreamVersion( const std::string& version )
  {
    if( version.empty() )
      return false;

    int major = 0;
    int minor = 0;
    int myMajor = atoi( XMPP_STREAM_VERSION_MAJOR.c_str() );

    size_t dot = version.find( "." );
    if( !version.empty() && dot && dot != std::string::npos )
    {
      major = atoi( version.substr( 0, dot ).c_str() );
      minor = atoi( version.substr( dot ).c_str() );
    }

    return myMajor >= major;
  }

  LogSink& ClientBase::logInstance()
  {
    return m_logInstance;
  }

  void ClientBase::setConnectionImpl( ConnectionBase *cb )
  {
    if( m_connection )
    {
      delete m_connection;
    }
    m_connection = cb;
  }

  void ClientBase::setEncryptionImpl( TLSBase *tb )
  {
    if( m_encryption )
    {
      delete m_encryption;
    }
    m_encryption = tb;
  }

  void ClientBase::setCompressionImpl( CompressionBase *cb )
  {
    if( m_compression )
    {
      delete m_compression;
    }
    m_compression = cb;
  }

  void ClientBase::handleStreamError( Stanza *stanza )
  {
    StreamError err = StreamErrorUndefined;
    const Tag::TagList& c = stanza->children();
    Tag::TagList::const_iterator it = c.begin();
    for( ; it != c.end(); ++it )
    {
      if( (*it)->name() == "bad-format" )
        err = StreamErrorBadFormat;
      else if( (*it)->name() == "bad-namespace-prefix" )
        err = StreamErrorBadNamespacePrefix;
      else if( (*it)->name() == "conflict" )
        err = StreamErrorConflict;
      else if( (*it)->name() == "connection-timeout" )
        err = StreamErrorConnectionTimeout;
      else if( (*it)->name() == "host-gone" )
        err = StreamErrorHostGone;
      else if( (*it)->name() == "host-unknown" )
        err = StreamErrorHostUnknown;
      else if( (*it)->name() == "improper-addressing" )
        err = StreamErrorImproperAddressing;
      else if( (*it)->name() == "internal-server-error" )
        err = StreamErrorInternalServerError;
      else if( (*it)->name() == "invalid-from" )
        err = StreamErrorInvalidFrom;
      else if( (*it)->name() == "invalid-id" )
        err = StreamErrorInvalidId;
      else if( (*it)->name() == "invalid-namespace" )
        err = StreamErrorInvalidNamespace;
      else if( (*it)->name() == "invalid-xml" )
        err = StreamErrorInvalidXml;
      else if( (*it)->name() == "not-authorized" )
        err = StreamErrorNotAuthorized;
      else if( (*it)->name() == "policy-violation" )
        err = StreamErrorPolicyViolation;
      else if( (*it)->name() == "remote-connection-failed" )
        err = StreamErrorRemoteConnectionFailed;
      else if( (*it)->name() == "resource-constraint" )
        err = StreamErrorResourceConstraint;
      else if( (*it)->name() == "restricted-xml" )
        err = StreamErrorRestrictedXml;
      else if( (*it)->name() == "see-other-host" )
      {
        err = StreamErrorSeeOtherHost;
        m_streamErrorCData = stanza->findChild( "see-other-host" )->cdata();
      }
      else if( (*it)->name() == "system-shutdown" )
        err = StreamErrorSystemShutdown;
      else if( (*it)->name() == "undefined-condition" )
        err = StreamErrorUndefinedCondition;
      else if( (*it)->name() == "unsupported-encoding" )
        err = StreamErrorUnsupportedEncoding;
      else if( (*it)->name() == "unsupported-stanza-type" )
        err = StreamErrorUnsupportedStanzaType;
      else if( (*it)->name() == "unsupported-version" )
        err = StreamErrorUnsupportedVersion;
      else if( (*it)->name() == "xml-not-well-formed" )
        err = StreamErrorXmlNotWellFormed;
      else if( (*it)->name() == "text" )
      {
        const std::string& lang = (*it)->findAttribute( "xml:lang" );
        if( !lang.empty() )
          m_streamErrorText[lang] = (*it)->cdata();
        else
          m_streamErrorText["default"] = (*it)->cdata();
      }
      else
        m_streamErrorAppCondition = (*it);

      if( err != StreamErrorUndefined && (*it)->hasAttribute( "xmlns", XMLNS_XMPP_STREAM ) )
        m_streamError = err;
    }
  }

  const std::string ClientBase::streamErrorText( const std::string& lang ) const
  {
    StringMap::const_iterator it = m_streamErrorText.find( lang );
    return ( it != m_streamErrorText.end() ) ? (*it).second : std::string();
  }

  void ClientBase::registerMessageSessionHandler( MessageSessionHandler *msh, int types )
  {
    if( types & StanzaMessageChat || types == 0 )
      m_messageSessionHandlerChat = msh;

    if( types & StanzaMessageNormal || types == 0 )
      m_messageSessionHandlerNormal = msh;

    if( types & StanzaMessageGroupchat || types == 0 )
      m_messageSessionHandlerGroupchat = msh;

    if( types & StanzaMessageHeadline || types == 0 )
      m_messageSessionHandlerHeadline = msh;
  }

  void ClientBase::registerPresenceHandler( PresenceHandler *ph )
  {
    if( ph )
      m_presenceHandlers.push_back( ph );
  }

  void ClientBase::removePresenceHandler( PresenceHandler *ph )
  {
    if( ph )
      m_presenceHandlers.remove( ph );
  }

  void ClientBase::registerPresenceHandler( const JID& jid, PresenceHandler *ph )
  {
    if( ph && jid )
    {
      JidPresHandlerStruct jph;
      jph.jid = new JID( jid.bare() );
      jph.ph = ph;
      m_presenceJidHandlers.push_back( jph );
    }
  }

  void ClientBase::removePresenceHandler( const JID& jid, PresenceHandler *ph )
  {
    PresenceJidHandlerList::iterator t;
    PresenceJidHandlerList::iterator it = m_presenceJidHandlers.begin();
    while( it != m_presenceJidHandlers.end() )
    {
      t = it;
      ++it;
      if( ( !ph || (*t).ph == ph ) && (*t).jid->bare() == jid.bare() )
      {
        delete (*t).jid;
        m_presenceJidHandlers.erase( t );
      }
    }
  }

  void ClientBase::trackID( IqHandler *ih, const std::string& id, int context )
  {
    if( ih && !id.empty() )
    {
      TrackStruct track;
      track.ih = ih;
      track.context = context;
      m_iqIDHandlers[id] = track;
    }
  }

  void ClientBase::removeIDHandler( IqHandler *ih )
  {
    IqTrackMap::iterator t;
    IqTrackMap::iterator it = m_iqIDHandlers.begin();
    while( it != m_iqIDHandlers.end() )
    {
      t = it;
      ++it;
      if( ih == (*t).second.ih )
        m_iqIDHandlers.erase( t );
    }
  }

  void ClientBase::registerIqHandler( IqHandler *ih, const std::string& xmlns )
  {
    if( ih && !xmlns.empty() )
      m_iqNSHandlers[xmlns] = ih;
  }

  void ClientBase::removeIqHandler( const std::string& xmlns )
  {
    if( !xmlns.empty() )
      m_iqNSHandlers.erase( xmlns );
  }

  void ClientBase::registerMessageSession( MessageSession *session )
  {
    if( session )
      m_messageSessions.push_back( session );
  }

  void ClientBase::disposeMessageSession( MessageSession *session )
  {
    if( !session )
      return;

    MessageSessionList::iterator it = std::find( m_messageSessions.begin(), m_messageSessions.end(),
                                                 session );
    if( it != m_messageSessions.end() )
    {
      delete (*it);
      m_messageSessions.erase( it );
    }
  }

  void ClientBase::registerMessageHandler( MessageHandler *mh )
  {
    if( mh )
      m_messageHandlers.push_back( mh );
  }

  void ClientBase::removeMessageHandler( MessageHandler *mh )
  {
    if( mh )
      m_messageHandlers.remove( mh );
  }

  void ClientBase::registerSubscriptionHandler( SubscriptionHandler *sh )
  {
    if( sh )
      m_subscriptionHandlers.push_back( sh );
  }

  void ClientBase::removeSubscriptionHandler( SubscriptionHandler *sh )
  {
    if( sh )
      m_subscriptionHandlers.remove( sh );
  }

  void ClientBase::registerTagHandler( TagHandler *th, const std::string& tag, const std::string& xmlns )
  {
    if( th && !tag.empty() )
    {
      TagHandlerStruct ths;
      ths.tag = tag;
      ths.xmlns = xmlns;
      ths.th = th;
      m_tagHandlers.push_back( ths );
    }
  }

  void ClientBase::removeTagHandler( TagHandler *th, const std::string& tag, const std::string& xmlns )
  {
    if( th )
    {
      TagHandlerList::iterator it = m_tagHandlers.begin();
      for( ; it != m_tagHandlers.end(); ++it )
      {
        if( (*it).th == th && (*it).tag == tag && (*it).xmlns == xmlns )
          m_tagHandlers.erase( it );
      }
    }
  }

  void ClientBase::registerStatisticsHandler( StatisticsHandler *sh )
  {
    if( sh )
      m_statisticsHandler = sh;
  }

  void ClientBase::removeStatisticsHandler()
  {
    m_statisticsHandler = 0;
  }

  void ClientBase::registerMUCInvitationHandler( MUCInvitationHandler *mih )
  {
    if( mih )
    {
      m_mucInvitationHandler = mih;
      m_disco->addFeature( XMLNS_MUC );
    }
  }

  void ClientBase::removeMUCInvitationHandler()
  {
    m_mucInvitationHandler = 0;
    m_disco->removeFeature( XMLNS_MUC );
  }

  void ClientBase::registerConnectionListener( ConnectionListener *cl )
  {
    if( cl )
      m_connectionListeners.push_back( cl );
  }

  void ClientBase::removeConnectionListener( ConnectionListener *cl )
  {
    if( cl )
      m_connectionListeners.remove( cl );
  }

  void ClientBase::notifyOnConnect()
  {
    ConnectionListenerList::const_iterator it = m_connectionListeners.begin();
    for( ; it != m_connectionListeners.end(); ++it )
    {
      (*it)->onConnect();
    }
  }

  void ClientBase::notifyOnDisconnect( ConnectionError e )
  {
    ConnectionListenerList::const_iterator it = m_connectionListeners.begin();
    for( ; it != m_connectionListeners.end(); ++it )
    {
      (*it)->onDisconnect( e );
    }
    init();
  }

  bool ClientBase::notifyOnTLSConnect( const CertInfo& info )
  {
    ConnectionListenerList::const_iterator it = m_connectionListeners.begin();
    for( ; it != m_connectionListeners.end() && (*it)->onTLSConnect( info ); ++it )
      ;
    return m_stats.encryption = ( it == m_connectionListeners.end() );
  }

  void ClientBase::notifyOnResourceBindError( ResourceBindError error )
  {
    ConnectionListenerList::const_iterator it = m_connectionListeners.begin();
    for( ; it != m_connectionListeners.end(); ++it )
    {
      (*it)->onResourceBindError( error );
    }
  }

  void ClientBase::notifyOnSessionCreateError( SessionCreateError error )
  {
    ConnectionListenerList::const_iterator it = m_connectionListeners.begin();
    for( ; it != m_connectionListeners.end(); ++it )
    {
      (*it)->onSessionCreateError( error );
    }
  }

  void ClientBase::notifyStreamEvent( StreamEvent event )
  {
    ConnectionListenerList::const_iterator it = m_connectionListeners.begin();
    for( ; it != m_connectionListeners.end(); ++it )
    {
      (*it)->onStreamEvent( event );
    }
  }

  void ClientBase::notifyPresenceHandlers( Stanza *stanza )
  {
    bool match = false;
    PresenceJidHandlerList::const_iterator t;
    PresenceJidHandlerList::const_iterator itj = m_presenceJidHandlers.begin();
    while( itj != m_presenceJidHandlers.end() )
    {
      t = itj++;
      if( (*t).jid->bare() == stanza->from().bare() && (*t).ph )
      {
        (*t).ph->handlePresence( stanza );
        match = true;
      }
    }
    if( match )
      return;

    PresenceHandlerList::const_iterator it = m_presenceHandlers.begin();
    for( ; it != m_presenceHandlers.end(); ++it )
    {
      (*it)->handlePresence( stanza );
    }
  }

  void ClientBase::notifySubscriptionHandlers( Stanza *stanza )
  {
    SubscriptionHandlerList::const_iterator it = m_subscriptionHandlers.begin();
    for( ; it != m_subscriptionHandlers.end(); ++it )
    {
      (*it)->handleSubscription( stanza );
    }
  }

  void ClientBase::notifyIqHandlers( Stanza *stanza )
  {
    bool res = false;

    IqHandlerMap::const_iterator it = m_iqNSHandlers.begin();
    for( ; it != m_iqNSHandlers.end(); ++it )
    {
      if( stanza->hasChildWithAttrib( "xmlns", (*it).first ) )
      {
        if( (*it).second->handleIq( stanza ) )
          res = true;
      }
    }

    IqTrackMap::iterator it_id = m_iqIDHandlers.find( stanza->id() );
    if( it_id != m_iqIDHandlers.end() )
    {
      if( (*it_id).second.ih->handleIqID( stanza, (*it_id).second.context ) )
        res = true;
      m_iqIDHandlers.erase( it_id );
    }

    if( !res && ( stanza->type() == StanzaIq ) &&
         ( ( stanza->subtype() == StanzaIqGet ) || ( stanza->subtype() == StanzaIqSet ) ) )
    {
      Tag *iq = new Tag( "iq" );
      iq->addAttribute( "type", "error" );
      iq->addAttribute( "id", stanza->id() );
      iq->addAttribute( "to", stanza->from().full() );
      Tag *e = new Tag( iq, "error", "type", "cancel", false );
      new Tag( e, "service-unavailable", "xmlns", XMLNS_XMPP_STANZAS );
      send( iq );
    }
  }

  void ClientBase::notifyMessageHandlers( Stanza *stanza )
  {
    if( m_mucInvitationHandler )
    {
      Tag *x = stanza->findChild( "x", "xmlns", XMLNS_MUC_USER );
      if( x && x->hasChild( "invite" ) )
      {
        Tag *i = x->findChild( "invite" );
        JID invitee( i->findAttribute( "from" ) );

        Tag * t = i->findChild( "reason" );
        std::string reason ( t ? t->cdata() : "" );

        t = x->findChild( "password" );
        std::string password ( t ? t->cdata() : "" );

        m_mucInvitationHandler->handleMUCInvitation( stanza->from(), invitee,
                                              reason, stanza->body(), password,
                                              i->hasChild( "continue" ) );
        return;
      }
    }

    MessageSessionList::const_iterator it1 = m_messageSessions.begin();
    for( ; it1 != m_messageSessions.end(); ++it1 )
    {
      if( (*it1)->target().full() == stanza->from().full() &&
            ( stanza->thread().empty() || (*it1)->threadID() == stanza->thread() ) &&
            ( (*it1)->types() & stanza->subtype() || (*it1)->types() == StanzaSubUndefined ) )
      {
        (*it1)->handleMessage( stanza );
        return;
      }
    }

    it1 = m_messageSessions.begin();
    for( ; it1 != m_messageSessions.end(); ++it1 )
    {
      if( (*it1)->target().bare() == stanza->from().bare() &&
            ( stanza->thread().empty() || (*it1)->threadID() == stanza->thread() ) &&
            ( (*it1)->types() & stanza->subtype() || (*it1)->types() == StanzaSubUndefined ) )
      {
        (*it1)->handleMessage( stanza );
        return;
      }
    }

    MessageSessionHandler *msHandler = 0;

    switch( stanza->subtype() )
    {
      case StanzaMessageChat:
        msHandler = m_messageSessionHandlerChat;
        break;
      case StanzaMessageNormal:
        msHandler = m_messageSessionHandlerNormal;
        break;
      case StanzaMessageGroupchat:
        msHandler = m_messageSessionHandlerGroupchat;
        break;
      case StanzaMessageHeadline:
        msHandler = m_messageSessionHandlerHeadline;
        break;
      default:
        break;
    }

    if( msHandler )
    {
      MessageSession *session = new MessageSession( this, stanza->from(), true, stanza->subtype() );
      msHandler->handleMessageSession( session );
      session->handleMessage( stanza );
    }
    else
    {
      MessageHandlerList::const_iterator it = m_messageHandlers.begin();
      for( ; it != m_messageHandlers.end(); ++it )
      {
        (*it)->handleMessage( stanza );
      }
    }
  }

  void ClientBase::notifyTagHandlers( Tag *tag )
  {
    TagHandlerList::const_iterator it = m_tagHandlers.begin();
    for( ; it != m_tagHandlers.end(); ++it )
    {
      if( (*it).tag == tag->name() && tag->hasAttribute( "xmlns", (*it).xmlns ) )
        (*it).th->handleTag( tag );
    }
  }

  CompressionBase* ClientBase::getDefaultCompression()
  {
    if( !m_compress )
      return 0;

#ifdef HAVE_ZLIB
    return new CompressionZlib( this );
#else
    return 0;
#endif
  }

  TLSBase* ClientBase::getDefaultEncryption()
  {
    if( m_tls == TLSDisabled || !hasTls() )
      return 0;

    return new TLSDefault( this, m_server );
  }

}
