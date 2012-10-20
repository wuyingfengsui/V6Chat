/*
  Copyright (c) 2006-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "socks5bytestreammanager.h"
#include "socks5bytestreamhandler.h"
#include "socks5bytestreamserver.h"
#include "socks5bytestream.h"
#include "clientbase.h"
#include "disco.h"
#include "connectionbase.h"
#include "sha.h"

#include <cstdlib>

namespace gloox
{

  SOCKS5BytestreamManager::SOCKS5BytestreamManager( ClientBase *parent, SOCKS5BytestreamHandler* s5bh )
    : m_parent( parent ), m_socks5BytestreamHandler( s5bh ), m_server( 0 )
  {
    if( m_parent )
      m_parent->registerIqHandler( this, XMLNS_BYTESTREAMS );
  }

  SOCKS5BytestreamManager::~SOCKS5BytestreamManager()
  {
    if( m_parent )
    {
      m_parent->removeIqHandler( XMLNS_BYTESTREAMS );
      m_parent->removeIDHandler( this );
    }

    S5BMap::iterator it = m_s5bMap.begin();
    for( ; it != m_s5bMap.end(); ++it )
    {
      delete (*it).second;
      (*it).second = 0;
    }
  }

  void SOCKS5BytestreamManager::addStreamHost( const JID& jid, const std::string& host, int port )
  {
    StreamHost sh;
    sh.jid = jid;
    sh.host = host;
    sh.port = port;
    m_hosts.push_back( sh );
  }

  bool SOCKS5BytestreamManager::requestSOCKS5Bytestream( const JID& to, S5BMode /*mode*/,
                                                         const std::string& sid )
  {
    if( !m_parent )
      return false;

    if( m_hosts.empty() )
    {
      m_parent->logInstance().log( LogLevelWarning, LogAreaClassS5BManager,
                                   "No stream hosts set, cannot request bytestream." );
      return false;
    }

    const std::string& msid = sid.empty() ? m_parent->getID() : sid;
    const std::string& id = m_parent->getID();
    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "type", "set" );
    iq->addAttribute( "to", to.full() );
    iq->addAttribute( "id", id );
    Tag *q = new Tag( iq, "query", "xmlns", XMLNS_BYTESTREAMS );
    q->addAttribute( "sid", msid );
    q->addAttribute( "mode", /*( mode == S5BTCP ) ?*/ "tcp" /*: "udp"*/ );

    StreamHostList::const_iterator it = m_hosts.begin();
    for( ; it != m_hosts.end(); ++it )
    {
      Tag* s = new Tag( q, "streamhost", "jid", (*it).jid.full() );
      s->addAttribute( "host", (*it).host );
      s->addAttribute( "port", (*it).port );
    }

    if( m_server )
    {
      SHA sha;
      sha.feed( msid );
      sha.feed( m_parent->jid().full() );
      sha.feed( to.full() );
      m_server->registerHash( sha.hex() );
    }

    AsyncS5BItem asi;
    asi.sHosts = m_hosts;
    asi.id = id;
    asi.from = to;
    asi.incoming = false;
    m_asyncTrackMap[msid] = asi;

    m_trackMap[id] = msid;
    m_parent->trackID( this, id, S5BOpenStream );
    m_parent->send( iq );

    return true;
  }

  void SOCKS5BytestreamManager::acknowledgeStreamHost( bool success, const JID& jid,
                                                       const std::string& sid )
  {
    AsyncTrackMap::const_iterator it = m_asyncTrackMap.find( sid );
    if( it == m_asyncTrackMap.end() || !m_parent )
      return;

    Tag *iq = new Tag( "iq" );

    if( (*it).second.incoming )
    {
      iq->addAttribute( "to", (*it).second.from.full() );
      iq->addAttribute( "id", (*it).second.id );

      if( success )
      {
        iq->addAttribute( "type", "result" );
        Tag* q = new Tag( iq, "query", "xmlns", XMLNS_BYTESTREAMS );
        new Tag( q, "streamhost-used", "jid", jid.full() );
      }
      else
      {
        iq->addAttribute( "type", "error" );
        Tag* e = new Tag( iq, "error" );
        e->addAttribute( "code", "404" );
        e->addAttribute( "type", "cancel" );
        new Tag( e, "item-not-found", "xmlns", XMLNS_XMPP_STANZAS );
      }
    }
    else
    {
      if( success )
      {
        const std::string& id = m_parent->getID();
        iq->addAttribute( "to", jid.full() );
        iq->addAttribute( "id", id );
        iq->addAttribute( "type", "set" );
        Tag* q = new Tag( iq, "query", "xmlns", XMLNS_BYTESTREAMS );
        q->addAttribute( "sid", sid );
        new Tag( q, "activate", (*it).second.from.full() );

        m_trackMap[id] = sid;
        m_parent->trackID( this, id, S5BActivateStream );
      }
    }

    m_parent->send( iq );
  }

  bool SOCKS5BytestreamManager::handleIq( Stanza *stanza )
  {
    Tag* q = stanza->findChild( "query", "xmlns", XMLNS_BYTESTREAMS );
    if( !q || !m_socks5BytestreamHandler )
      return false;

    if( m_trackMap.find( stanza->id() ) != m_trackMap.end() )
      return false;

    switch( stanza->subtype() )
    {
      case StanzaIqSet:
      {
        const std::string& sid = q->findAttribute( "sid" );
        const std::string& mode = q->findAttribute( "mode" );
        if( haveStream( stanza->from() ) || sid.empty() || mode == "udp" )
        {
          rejectSOCKS5Bytestream( stanza->from(), stanza->id(), StanzaErrorNotAcceptable );
          return true;
        }
        AsyncS5BItem asi;
        Tag::TagList& l = q->children();
        Tag::TagList::const_iterator it = l.begin();
        for( ; it != l.end(); ++it )
        {
          if( (*it)->name() == "streamhost" && (*it)->hasAttribute( "jid" )
                && (*it)->hasAttribute( "host" ) && (*it)->hasAttribute( "port" ) )
          {
            StreamHost sh;
            sh.jid = (*it)->findAttribute( "jid" );
            sh.host = (*it)->findAttribute( "host" );
            sh.port = atoi( (*it)->findAttribute( "port" ).c_str() );
            asi.sHosts.push_back( sh );
          }
        }
        asi.id = stanza->id();
        asi.from = stanza->from();
        asi.incoming = true;
        m_asyncTrackMap[sid] = asi;
        m_socks5BytestreamHandler->handleIncomingSOCKS5BytestreamRequest( sid, stanza->from() );
        break;
      }
      case StanzaIqError:
        m_socks5BytestreamHandler->handleSOCKS5BytestreamError( stanza, std::string() );
        break;
      default:
        break;
    }

    return true;
  }

  const StreamHost* SOCKS5BytestreamManager::findProxy( const JID& from, const std::string& hostjid,
                                                        const std::string& sid )
  {
    AsyncTrackMap::const_iterator it = m_asyncTrackMap.find( sid );
    if( it == m_asyncTrackMap.end() )
      return 0;

    if( (*it).second.from == from )
    {
      StreamHostList::const_iterator it2 = (*it).second.sHosts.begin();
      for( ; it2 != (*it).second.sHosts.end(); ++it2 )
      {
        if( (*it2).jid == hostjid )
        {
          return &(*it2);
        }
      }
    }

    return 0;
  }

  bool SOCKS5BytestreamManager::haveStream( const JID& from )
  {
    S5BMap::const_iterator it = m_s5bMap.begin();
    for( ; it != m_s5bMap.end(); ++it )
    {
      if( (*it).second && (*it).second->target() == from )
        return true;
    }
    return false;
  }

  void SOCKS5BytestreamManager::acceptSOCKS5Bytestream( const std::string& sid )
  {
    AsyncTrackMap::iterator it = m_asyncTrackMap.find( sid );
    if( it == m_asyncTrackMap.end() || !m_socks5BytestreamHandler )
      return;

    SOCKS5Bytestream* s5b = new SOCKS5Bytestream( this, m_parent->connectionImpl()->newInstance(),
                                                  m_parent->logInstance(),
                                                  (*it).second.from, m_parent->jid(), sid );
    s5b->setStreamHosts( (*it).second.sHosts );
    m_s5bMap[sid] = s5b;
    m_socks5BytestreamHandler->handleIncomingSOCKS5Bytestream( s5b );
  }

  void SOCKS5BytestreamManager::rejectSOCKS5Bytestream( const std::string& sid )
  {
    AsyncTrackMap::iterator it = m_asyncTrackMap.find( sid );
    if( it != m_asyncTrackMap.end() )
    {
      rejectSOCKS5Bytestream( (*it).second.from, (*it).second.id, StanzaErrorNotAcceptable );
      m_asyncTrackMap.erase( it );
    }
  }

  void SOCKS5BytestreamManager::rejectSOCKS5Bytestream( const JID& from, const std::string& id,
                                                        StanzaError reason )
  {
    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "type", "error" );
    iq->addAttribute( "to", from.full() );
    iq->addAttribute( "id", id );
    Tag *e = new Tag( iq, "error" );
    switch( reason )
    {
      case StanzaErrorForbidden:
      {
        new Tag( iq, "query", "xmlns", XMLNS_BYTESTREAMS );
        e->addAttribute( "code", "403" );
        e->addAttribute( "type", "auth" );
        Tag *f = new Tag( e, "forbidden" );
        f->addAttribute( "xmlns", XMLNS_XMPP_STANZAS );
        break;
      }
      case StanzaErrorFeatureNotImplemented:
      {
        e->addAttribute( "code", "404" );
        e->addAttribute( "type", "cancel" );
        Tag *f = new Tag( e, "item-not-found" );
        f->addAttribute( "xmlns", XMLNS_XMPP_STANZAS );
        break;
      }
      case StanzaErrorNotAllowed:
      {
        new Tag( iq, "query", "xmlns", XMLNS_BYTESTREAMS );
        e->addAttribute( "code", "405" );
        e->addAttribute( "type", "cancel" );
        Tag *f = new Tag( e, "not-allowed" );
        f->addAttribute( "xmlns", XMLNS_XMPP_STANZAS );
        break;
      }
      case StanzaErrorNotAcceptable:
      default:
      {
        e->addAttribute( "code", "406" );
        e->addAttribute( "type", "auth" );
        Tag *f = new Tag( e, "not-acceptable" );
        f->addAttribute( "xmlns", XMLNS_XMPP_STANZAS );
        break;
      }
    }
    m_parent->send( iq );
  }

  bool SOCKS5BytestreamManager::handleIqID( Stanza *stanza, int context )
  {
    StringMap::iterator it = m_trackMap.find( stanza->id() );
    if( it == m_trackMap.end() )
      return false;

    switch( context )
    {
      case S5BOpenStream:
      {
        switch( stanza->subtype() )
        {
          case StanzaIqResult:
          {
            Tag* q = stanza->findChild( "query", "xmlns", XMLNS_BYTESTREAMS );
            if( !q || !m_socks5BytestreamHandler )
              return false;

            Tag* s = q->findChild( "streamhost-used" );
            if( !s || !s->hasAttribute( "jid" ) )
              return false;

            const std::string & proxy = s->findAttribute( "jid" );
            const StreamHost* sh = findProxy( stanza->from(), proxy, (*it).second );
            if( sh )
            {
              SOCKS5Bytestream* s5b = 0;
              bool selfProxy = ( proxy == m_parent->jid().full() && m_server );
              if( selfProxy )
              {
                SHA sha;
                sha.feed( (*it).second );
                sha.feed( m_parent->jid().full() );
                sha.feed( stanza->from().full() );
                s5b = new SOCKS5Bytestream( this, m_server->getConnection( sha.hex() ),
                                            m_parent->logInstance(),
                                            m_parent->jid(), stanza->from(),
                                            (*it).second );
              }
              else
              {
                s5b = new SOCKS5Bytestream( this, m_parent->connectionImpl()->newInstance(),
                                            m_parent->logInstance(),
                                            m_parent->jid(), stanza->from(),
                                            (*it).second );
                StreamHostList shl;
                shl.push_back( *sh );
                s5b->setStreamHosts( shl );
              }
              m_s5bMap[(*it).second] = s5b;
              m_socks5BytestreamHandler->handleOutgoingSOCKS5Bytestream( s5b );
              if( selfProxy )
                s5b->activate();
            }
            break;
          }
          case StanzaIqError:
            m_socks5BytestreamHandler->handleSOCKS5BytestreamError( stanza, (*it).second );
            break;
          default:
            break;
        }
        break;
      }
      case S5BActivateStream:
      {
        switch( stanza->subtype() )
        {
          case StanzaIqResult:
          {
            S5BMap::const_iterator it5 = m_s5bMap.find( (*it).second );
            if( it5 != m_s5bMap.end() )
              (*it5).second->activate();
            break;
          }
          case StanzaIqError:
            m_socks5BytestreamHandler->handleSOCKS5BytestreamError( stanza, (*it).second );
            break;
          default:
            break;
        }
        break;
      }
      default:
        break;
    }
    m_trackMap.erase( it );

    return false;
  }

  bool SOCKS5BytestreamManager::dispose( SOCKS5Bytestream* s5b )
  {
    S5BMap::iterator it = m_s5bMap.find( s5b->sid() );
    if( it != m_s5bMap.end() )
    {
      delete s5b;
      m_s5bMap.erase( it );
      return true;
    }

    return false;
  }

}
