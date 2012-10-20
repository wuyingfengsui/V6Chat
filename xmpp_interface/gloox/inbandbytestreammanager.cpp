/*
  Copyright (c) 2006-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "inbandbytestreammanager.h"
#include "inbandbytestreamhandler.h"
#include "inbandbytestream.h"
#include "clientbase.h"
#include "disco.h"

namespace gloox
{

  InBandBytestreamManager::InBandBytestreamManager( ClientBase *parent )
    : m_parent( parent ), m_inbandBytestreamHandler( 0 ), m_syncInbandBytestreams( true ),
      m_blockSize( 4096 )
  {
    if( m_parent )
    {
      m_parent->registerIqHandler( this, XMLNS_IBB );
      m_parent->disco()->addFeature( XMLNS_IBB );
    }
  }

  InBandBytestreamManager::~InBandBytestreamManager()
  {
    if( m_parent )
    {
      m_parent->disco()->removeFeature( XMLNS_IBB );
      m_parent->removeIqHandler( XMLNS_IBB );
      m_parent->removeIDHandler( this );
    }

    IBBMap::iterator it = m_ibbMap.begin();
    for( ; it != m_ibbMap.end(); ++it )
    {
      delete (*it).second;
    }
  }

  bool InBandBytestreamManager::requestInBandBytestream( const JID& to, InBandBytestreamHandler *ibbh,
                                                         const std::string& sid )
  {
    if( !m_parent || !ibbh )
      return false;

    const std::string& msid = sid.empty() ? m_parent->getID() : sid;
    const std::string& id = m_parent->getID();
    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "type", "set" );
    iq->addAttribute( "to", to.full() );
    iq->addAttribute( "id", id );
    Tag *o = new Tag( iq, "open" );
    o->addAttribute( "sid", msid );
    o->addAttribute( "block-size", m_blockSize );
    o->addAttribute( "xmlns", XMLNS_IBB );

    TrackItem item;
    item.sid = msid;
    item.ibbh = ibbh;
    m_trackMap[id] = item;
    m_parent->trackID( this, id, IBBOpenStream );
    m_parent->send( iq );

    return true;
  }

  bool InBandBytestreamManager::handleIq( Stanza *stanza )
  {
    Tag *o = 0;
    if( ( stanza->subtype() == StanzaIqSet ) &&
          ( ( o = stanza->findChild( "open", "xmlns", XMLNS_IBB ) ) != 0 ) )
    {
      InBandBytestream *ibb = new InBandBytestream( 0, m_parent );
      const std::string& sid = o->findAttribute( "sid" );
      ibb->setSid( sid );

      if( !m_inbandBytestreamHandler )
        rejectInBandBytestream( ibb, stanza->from(), stanza->id() );

      if( !m_syncInbandBytestreams )
      {
        AsyncIBBItem item;
        item.ibb = ibb;
        item.from = stanza->from();
        item.id = stanza->id();
        m_asyncTrackMap[sid] = item;
      }

      bool t = m_inbandBytestreamHandler->handleIncomingInBandBytestream( stanza->from(), ibb );
      if( m_syncInbandBytestreams && t )
      {
        acceptInBandBytestream( ibb, stanza->from(), stanza->id() );
      }
      else if( m_syncInbandBytestreams && !t )
      {
        rejectInBandBytestream( ibb, stanza->from(), stanza->id() );
      }
    }
    else if( ( stanza->subtype() == StanzaIqSet ) &&
               ( ( o = stanza->findChild( "close", "xmlns", XMLNS_IBB ) ) != 0 ) &&
               o->hasAttribute( "sid" ) )
    {
      IBBMap::iterator it = m_ibbMap.find( o->findAttribute( "sid" ) );
      if( it != m_ibbMap.end() )
      {
        (*it).second->closed();

        Tag *iq = new Tag( "iq" );
        iq->addAttribute( "type", "result" );
        iq->addAttribute( "to", stanza->from().full() );
        iq->addAttribute( "id", stanza->id() );

        m_parent->send( iq );
      }
    }
    else
    {
      return false;
    }

    return true;
  }

  void InBandBytestreamManager::acceptInBandBytestream( InBandBytestream *ibb )
  {
    if( m_syncInbandBytestreams )
      return;

    AsyncTrackMap::iterator it = m_asyncTrackMap.find( ibb->sid() );
    if( it != m_asyncTrackMap.end() )
    {
      acceptInBandBytestream( ibb, (*it).second.from, (*it).second.id );
      m_asyncTrackMap.erase( it );
    }
  }

  void InBandBytestreamManager::rejectInBandBytestream( InBandBytestream *ibb )
  {
    if( m_syncInbandBytestreams )
      return;

    AsyncTrackMap::iterator it = m_asyncTrackMap.find( ibb->sid() );
    if( it != m_asyncTrackMap.end() )
    {
      rejectInBandBytestream( ibb, (*it).second.from, (*it).second.id );
      m_asyncTrackMap.erase( it );
    }
  }

  void InBandBytestreamManager::acceptInBandBytestream( InBandBytestream *ibb,
      const JID& from, const std::string& id )
  {
    m_ibbMap[ibb->sid()] = ibb;
    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "type", "result" );
    iq->addAttribute( "to", from.full() );
    iq->addAttribute( "id", id );
    m_parent->send( iq );
  }

  void InBandBytestreamManager::rejectInBandBytestream( InBandBytestream *ibb,
      const JID& from, const std::string& id )
  {
    delete ibb;
    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "type", "error" );
    iq->addAttribute( "to", from.full() );
    iq->addAttribute( "id", id );
    Tag *e = new Tag( iq, "error" );
    e->addAttribute( "code", "501" );
    e->addAttribute( "type", "cancel" );
    Tag *f = new Tag( e, "feature-not-implemented" );
    f->addAttribute( "xmlns", XMLNS_XMPP_STANZAS );
    m_parent->send( iq );
  }

  bool InBandBytestreamManager::handleIqID( Stanza *stanza, int context )
  {
    switch( context )
    {
      case IBBOpenStream:
      {
        TrackMap::iterator it = m_trackMap.find( stanza->id() );
        if( it != m_trackMap.end() )
        {
          switch( stanza->subtype() )
          {
            case StanzaIqResult:
            {
              InBandBytestream *ibb = new InBandBytestream( 0, m_parent );
              ibb->setSid( (*it).second.sid );
              ibb->setBlockSize( m_blockSize );
              m_ibbMap[(*it).second.sid] = ibb;
              InBandBytestreamHandler *t = (*it).second.ibbh;
              t->handleOutgoingInBandBytestream( stanza->from(), ibb );
              break;
            }
            case StanzaIqError:
              (*it).second.ibbh->handleInBandBytestreamError( stanza->from(), stanza->error() );
              break;
            default:
              break;
          }
          m_trackMap.erase( it );
        }
        break;
      }
      default:
        break;
    }

    return false;
  }

  bool InBandBytestreamManager::dispose( InBandBytestream *ibb )
  {
    IBBMap::iterator it = m_ibbMap.find( ibb->sid() );
    if( it != m_ibbMap.end() )
    {
      delete ibb;
      m_ibbMap.erase( it );
      return true;
    }

    return false;
  }

  void InBandBytestreamManager::registerInBandBytestreamHandler( InBandBytestreamHandler *ibbh,
      bool sync )
  {
    m_inbandBytestreamHandler = ibbh;
    m_syncInbandBytestreams = sync;
  }

  void InBandBytestreamManager::removeInBandBytestreamHandler()
  {
    m_inbandBytestreamHandler = 0;
  }

}
