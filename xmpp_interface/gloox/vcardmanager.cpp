/*
  Copyright (c) 2006-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "vcardmanager.h"
#include "vcardhandler.h"
#include "vcard.h"
#include "clientbase.h"
#include "disco.h"

namespace gloox
{

  VCardManager::VCardManager( ClientBase *parent )
    : m_parent( parent )
  {
    if( m_parent )
    {
      m_parent->registerIqHandler( this, XMLNS_VCARD_TEMP );
      m_parent->disco()->addFeature( XMLNS_VCARD_TEMP );
    }
  }

  VCardManager::~VCardManager()
  {
    if( m_parent )
    {
      m_parent->disco()->removeFeature( XMLNS_VCARD_TEMP );
      m_parent->removeIqHandler( XMLNS_VCARD_TEMP );
      m_parent->removeIDHandler( this );
    }
  }

  void VCardManager::fetchVCard( const JID& jid, VCardHandler *vch )
  {
    if( !m_parent || !vch )
      return;

    TrackMap::const_iterator it = m_trackMap.find( jid.bare() );
    if( it != m_trackMap.end() )
      return;

    const std::string& id = m_parent->getID();
    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "type", "get" );
    iq->addAttribute( "id", id );
    iq->addAttribute( "to", jid.bare() );
    Tag *v = new Tag( iq, "vCard" );
    v->addAttribute( "xmlns", XMLNS_VCARD_TEMP );

    m_parent->trackID( this, id, VCardHandler::FetchVCard );
    m_trackMap[id] = vch;
    m_parent->send( iq );
  }

  void VCardManager::cancelVCardOperations( VCardHandler *vch )
  {
    TrackMap::iterator t;
    TrackMap::iterator it = m_trackMap.begin();
    while( it != m_trackMap.end() )
    {
      t = it;
      ++it;
      if( (*t).second == vch )
        m_trackMap.erase( t );
    }
  }

  void VCardManager::storeVCard( const VCard *vcard, VCardHandler *vch )
  {
    if( !m_parent || !vch )
      return;

    const std::string& id = m_parent->getID();
    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "type", "set" );
    iq->addAttribute( "id", id );
    iq->addChild( vcard->tag() );

    m_parent->trackID( this, id, VCardHandler::StoreVCard );
    m_trackMap[id] = vch;
    m_parent->send( iq );
  }

  bool VCardManager::handleIq( Stanza * /*stanza*/ )
  {
    return false;
  }

  bool VCardManager::handleIqID( Stanza *stanza, int context )
  {
    TrackMap::iterator it = m_trackMap.find( stanza->id() );
    if( it != m_trackMap.end() )
    {
      switch( stanza->subtype() )
      {
        case StanzaIqResult:
        {
          switch( context )
          {
            case VCardHandler::FetchVCard:
            {
              Tag *v = stanza->findChild( "vCard", "xmlns", XMLNS_VCARD_TEMP );
              if( v )
                (*it).second->handleVCard( stanza->from(), new VCard( v ) );
              else
                (*it).second->handleVCard( stanza->from(), 0 );
              break;
            }
            case VCardHandler::StoreVCard:
              (*it).second->handleVCardResult( VCardHandler::StoreVCard, stanza->from() );
              break;
          }
        }
        break;
        case StanzaIqError:
        {
          switch( context )
          {
            case VCardHandler::FetchVCard:
              (*it).second->handleVCardResult( VCardHandler::FetchVCard, stanza->from(), stanza->error() );
              break;
            case VCardHandler::StoreVCard:
              (*it).second->handleVCardResult( VCardHandler::StoreVCard, stanza->from(), stanza->error() );
              break;
          }
          break;
        }
        default:
          return false;
      }

      m_trackMap.erase( it );
    }
    return false;
  }

}
