/*
  Copyright (c) 2005-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "flexoff.h"
#include "dataform.h"
#include "disco.h"

#include <cstdlib>

namespace gloox
{

  FlexibleOffline::FlexibleOffline( ClientBase *parent )
    : m_parent( parent ), m_flexibleOfflineHandler( 0 )
  {
  }

  FlexibleOffline::~FlexibleOffline()
  {
    m_parent->removeIDHandler( this );
  }

  void FlexibleOffline::checkSupport()
  {
    m_parent->disco()->getDiscoInfo( m_parent->jid().server(), "", this, FOCheckSupport );
  }

  void FlexibleOffline::getMsgCount()
  {
    m_parent->disco()->getDiscoInfo( m_parent->jid().server(), XMLNS_OFFLINE, this, FORequestNum );
  }

  void FlexibleOffline::fetchHeaders()
  {
    m_parent->disco()->getDiscoItems( m_parent->jid().server(), XMLNS_OFFLINE, this, FORequestHeaders );
  }

  void FlexibleOffline::fetchMessages( const StringList& msgs )
  {
    const std::string& id = m_parent->getID();
    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "type", "get" );
    iq->addAttribute( "id", id );
    Tag *o = new Tag( iq, "offline" );
    o->addAttribute( "xmlns", XMLNS_OFFLINE );

    if( msgs.size() == 0 )
      new Tag( o, "fetch" );
    else
    {
      StringList::const_iterator it = msgs.begin();
      for( ; it != msgs.end(); ++it )
      {
        Tag *i = new Tag( o, "item" );
        i->addAttribute( "action", "view" );
        i->addAttribute( "node", (*it) );
      }
    }

    m_parent->trackID( this, id, FORequestMsgs );
    m_parent->send( iq );
  }

  void FlexibleOffline::removeMessages( const StringList& msgs )
  {
    const std::string& id = m_parent->getID();
    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "type", "get" );
    iq->addAttribute( "id", id );
    Tag *o = new Tag( iq, "offline" );
    o->addAttribute( "xmlns", XMLNS_OFFLINE );

    if( msgs.size() == 0 )
      new Tag( o, "purge" );
    else
    {
      StringList::const_iterator it = msgs.begin();
      for( ; it != msgs.end(); ++it )
      {
        Tag *i = new Tag( o, "item" );
        i->addAttribute( "action", "remove" );
        i->addAttribute( "node", (*it) );
      }
    }

    m_parent->trackID( this, id, FORemoveMsgs );
    m_parent->send( iq );
  }

  void FlexibleOffline::registerFlexibleOfflineHandler( FlexibleOfflineHandler *foh )
  {
    m_flexibleOfflineHandler = foh;
  }

  void FlexibleOffline::removeFlexibleOfflineHandler()
  {
    m_flexibleOfflineHandler = 0;
  }

  void FlexibleOffline::handleDiscoInfoResult( Stanza *stanza, int context )
  {
    if( !m_flexibleOfflineHandler )
      return;

    switch( context )
    {
      case FOCheckSupport:
        m_flexibleOfflineHandler->handleFlexibleOfflineSupport(
            stanza->findChild( "query" )->hasChild( "feature", "var", XMLNS_OFFLINE ) );
        break;

      case FORequestNum:
        int num = -1;
        DataForm f( stanza->findChild( "query" )->findChild( "x" ) );
        if( f.hasField( "number_of_messages" ) )
          num = atoi( f.field( "number_of_messages" )->value().c_str() );

        m_flexibleOfflineHandler->handleFlexibleOfflineMsgNum( num );
        break;
    }
  }

  void FlexibleOffline::handleDiscoItemsResult( Stanza *stanza, int context )
  {
    if( context == FORequestHeaders && m_flexibleOfflineHandler )
    {
      Tag *q = stanza->findChild( "query" );
      if( q && q->hasAttribute( "xmlns", XMLNS_DISCO_ITEMS ) && q->hasAttribute( "node", XMLNS_OFFLINE ) )
      {
        StringMap m;
        const Tag::TagList& l = q->children();
        Tag::TagList::const_iterator it = l.begin();
        for( ; it != l.end(); ++it )
        {
          m[(*it)->findAttribute( "node" )] = (*it)->findAttribute( "name" );
        }
        m_flexibleOfflineHandler->handleFlexibleOfflineMessageHeaders( m );
      }
    }
  }

  void FlexibleOffline::handleDiscoError( Stanza * /*stanza*/, int /*context*/ )
  {
  }

  bool FlexibleOffline::handleIqID( Stanza *stanza, int context )
  {
    if( !m_flexibleOfflineHandler )
      return false;

    switch( context )
    {
      case FORequestMsgs:
        switch( stanza->subtype() )
        {
          case StanzaIqResult:
            m_flexibleOfflineHandler->handleFlexibleOfflineResult( FomrRequestSuccess );
            break;
          case StanzaIqError:
            switch( stanza->error() )
            {
              case StanzaErrorForbidden:
                m_flexibleOfflineHandler->handleFlexibleOfflineResult( FomrForbidden );
                break;
              case StanzaErrorItemNotFound:
                m_flexibleOfflineHandler->handleFlexibleOfflineResult( FomrItemNotFound );
                break;
              default:
                m_flexibleOfflineHandler->handleFlexibleOfflineResult( FomrUnknownError );
                break;
            }
            break;
          default:
            break;
        }
        break;
      case FORemoveMsgs:
        switch( stanza->subtype() )
        {
          case StanzaIqResult:
            m_flexibleOfflineHandler->handleFlexibleOfflineResult( FomrRemoveSuccess );
            break;
          case StanzaIqError:
            switch( stanza->error() )
            {
              case StanzaErrorForbidden:
                m_flexibleOfflineHandler->handleFlexibleOfflineResult( FomrForbidden );
                break;
              case StanzaErrorItemNotFound:
                m_flexibleOfflineHandler->handleFlexibleOfflineResult( FomrItemNotFound );
                break;
              default:
                m_flexibleOfflineHandler->handleFlexibleOfflineResult( FomrUnknownError );
                break;
            }
            break;
          default:
            break;
        }
        break;
    }

    return false;
  }

  bool FlexibleOffline::handleIq( Stanza * /*stanza*/ )
  {
    return false;
  }

}
