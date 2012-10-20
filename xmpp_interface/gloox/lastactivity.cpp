/*
  Copyright (c) 2005-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#include "lastactivity.h"
#include "disco.h"
#include "discohandler.h"
#include "client.h"
#include "lastactivityhandler.h"

#include <cstdlib>

namespace gloox
{

  LastActivity::LastActivity( ClientBase *parent )
    : m_lastActivityHandler( 0 ), m_parent( parent ),
      m_active( time ( 0 ) )
  {
    if( m_parent )
      m_parent->disco()->addFeature( XMLNS_LAST );
  }

  LastActivity::~LastActivity()
  {
    if( m_parent )
    {
      m_parent->disco()->removeFeature( XMLNS_LAST );
      m_parent->removeIDHandler( this );
    }
  }

  void LastActivity::query( const JID& jid )
  {
    const std::string& id = m_parent->getID();

    Tag *t = new Tag( "iq" );
    t->addAttribute( "type", "get" );
    t->addAttribute( "id", id );
    t->addAttribute( "to", jid.full() );
    Tag *q = new Tag( t, "query" );
    q->addAttribute( "xmlns", XMLNS_LAST );

    m_parent->trackID( this, id, 0 );
    m_parent->send( t );
  }

  bool LastActivity::handleIq( Stanza *stanza )
  {
    switch( stanza->subtype() )
    {
      case StanzaIqGet:
      {
        time_t now = time( 0 );

        Tag *t = new Tag( "iq" );
        t->addAttribute( "type", "result" );
        t->addAttribute( "id", stanza->id() );
        t->addAttribute( "to", stanza->from().full() );
        Tag *q = new Tag( t, "query" );
        q->addAttribute( "seconds", (long)( now - m_active ) );
        q->addAttribute( "xmlns", XMLNS_LAST );

        m_parent->send( t );
        break;
      }

      case StanzaIqSet:
      {
        Tag *t = new Tag( "iq" );
        t->addAttribute( "id", stanza->id() );
        t->addAttribute( "to", stanza->from().full() );
        t->addAttribute( "type", "error" );
        Tag *e = new Tag( t, "error" );
        e->addAttribute( "type", "cancel" );
        Tag *f = new Tag( e, "feature-not-implemented" );
        f->addAttribute( "xmlns", XMLNS_XMPP_STANZAS );

        m_parent->send( t );
        break;
      }

      default:
        break;
    }

    return true;
  }

  bool LastActivity::handleIqID( Stanza *stanza, int /*context*/ )
  {
    if( !m_lastActivityHandler )
      return false;

    switch( stanza->subtype() )
    {
      case StanzaIqResult:
      {
        Tag *q = stanza->findChild( "query" );
        if( q )
        {
          const std::string& seconds = q->findAttribute( "seconds" );
          if( !seconds.empty() )
          {
            int secs = atoi( seconds.c_str() );
            m_lastActivityHandler->handleLastActivityResult( stanza->from(), secs );
          }
        }
        break;
      }
      case StanzaIqError:
        m_lastActivityHandler->handleLastActivityError( stanza->from(), stanza->error() );
        break;
      default:
        break;
    }

    return false;
  }

  void LastActivity::resetIdleTimer()
  {
    m_active = time( 0 );
  }

}
