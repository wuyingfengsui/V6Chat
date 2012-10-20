/*
  Copyright (c) 2007-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#include "uniquemucroom.h"
#include "clientbase.h"
#include "jid.h"
#include "sha.h"

namespace gloox
{

  UniqueMUCRoom::UniqueMUCRoom( ClientBase *parent, const JID& nick, MUCRoomHandler *mrh )
    : InstantMUCRoom( parent, nick, mrh )
  {
  }

  UniqueMUCRoom::~UniqueMUCRoom()
  {
    m_parent->removeIDHandler( this );
  }

  void UniqueMUCRoom::join()
  {
    if( !m_parent || m_joined )
      return;

    const std::string& id = m_parent->getID();
    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "id", id );
    iq->addAttribute( "to", m_nick.server() );
    iq->addAttribute( "type", "get" );
    Tag *u = new Tag( iq, "unique" );
    u->addAttribute( "xmlns", XMLNS_MUC_UNIQUE );

    m_parent->trackID( this, id, RequestUniqueName );
    m_parent->send( iq );
  }

  bool UniqueMUCRoom::handleIqID( Stanza *stanza, int context )
  {
    switch( stanza->subtype() )
    {
      case StanzaIqResult:
        if( context == RequestUniqueName )
        {
          Tag *u = stanza->findChild( "unique", "xmlns", XMLNS_MUC_UNIQUE );
          if( u )
          {
            const std::string& name = u->cdata();
            if( !name.empty() )
              setName( name );
          }
        }
        break;
      case StanzaIqError:
        if( context == RequestUniqueName )
        {
          SHA s;
          s.feed( m_parent->jid().full() );
          s.feed( m_parent->getID() );
          setName( s.hex() );
        }
        break;
      default:
        break;
    }

    MUCRoom::join();

    return false;
  }

}
