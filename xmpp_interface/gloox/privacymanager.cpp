/*
  Copyright (c) 2005-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#include "privacymanager.h"
#include "clientbase.h"

#ifndef _WIN32_WCE
# include <sstream>
#endif

namespace gloox
{

  PrivacyManager::PrivacyManager( ClientBase *parent )
    : m_parent( parent ), m_privacyListHandler( 0 )
  {
    if( m_parent )
      m_parent->registerIqHandler( this, XMLNS_PRIVACY );
  }

  PrivacyManager::~PrivacyManager()
  {
    if( m_parent )
    {
      m_parent->removeIqHandler( XMLNS_PRIVACY );
      m_parent->removeIDHandler( this );
    }
  }

  std::string PrivacyManager::requestListNames()
  {
    const std::string& id = m_parent->getID();

    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "type", "get" );
    iq->addAttribute( "id", id );
    Tag *q = new Tag( iq, "query" );
    q->addAttribute( "xmlns", XMLNS_PRIVACY );

    m_parent->trackID( this, id, PLRequestNames );
    m_parent->send( iq );
    return id;
  }

  std::string PrivacyManager::requestList( const std::string& name )
  {
    const std::string& id = m_parent->getID();

    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "type", "get" );
    iq->addAttribute( "id", id );
    Tag *q = new Tag( iq, "query" );
    q->addAttribute( "xmlns", XMLNS_PRIVACY );
    Tag *l = new Tag( q, "list" );
    l->addAttribute( "name", name );

    m_parent->trackID( this, id, PLRequestList );
    m_parent->send( iq );
    return id;
  }

  std::string PrivacyManager::removeList( const std::string& name )
  {
    const std::string& id = m_parent->getID();

    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "type", "set" );
    iq->addAttribute( "id", id );
    Tag *q = new Tag( iq, "query" );
    q->addAttribute( "xmlns", XMLNS_PRIVACY );
    Tag *l = new Tag( q, "list" );
    l->addAttribute( "name", name );

    m_parent->trackID( this, id, PLRemove );
    m_parent->send( iq );
    return id;
  }

  std::string PrivacyManager::setDefault( const std::string& name )
  {
    const std::string& id = m_parent->getID();

    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "type", "set" );
    iq->addAttribute( "id", id );
    Tag *q = new Tag( iq, "query" );
    q->addAttribute( "xmlns", XMLNS_PRIVACY );
    Tag *d = new Tag( q, "default" );
    d->addAttribute( "name", name );

    m_parent->trackID( this, id, PLDefault );
    m_parent->send( iq );
    return id;
  }

  std::string PrivacyManager::unsetDefault()
  {
    const std::string& id = m_parent->getID();

    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "type", "set" );
    iq->addAttribute( "id", id );
    Tag *q = new Tag( iq, "query" );
    q->addAttribute( "xmlns", XMLNS_PRIVACY );
    new Tag( q, "default" );

    m_parent->trackID( this, id, PLUnsetDefault );
    m_parent->send( iq );
    return id;
  }

  std::string PrivacyManager::setActive( const std::string& name )
  {
    const std::string& id = m_parent->getID();

    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "type", "set" );
    iq->addAttribute( "id", id );
    Tag *q = new Tag( iq, "query" );
    q->addAttribute( "xmlns", XMLNS_PRIVACY );
    Tag *a = new Tag( q, "active" );
    a->addAttribute( "name", name );

    m_parent->trackID( this, id, PLActivate );
    m_parent->send( iq );
    return id;
  }

  std::string PrivacyManager::unsetActive()
  {
    const std::string& id = m_parent->getID();

    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "type", "set" );
    iq->addAttribute( "id", id );
    Tag *q = new Tag( iq, "query" );
    q->addAttribute( "xmlns", XMLNS_PRIVACY );
    new Tag( q, "active" );

    m_parent->trackID( this, id, PLUnsetActivate );
    m_parent->send( iq );
    return id;
  }

  std::string PrivacyManager::store( const std::string& name,
                                     const PrivacyListHandler::PrivacyList& list )
  {
    if( list.empty() )
      return std::string();

    const std::string& id = m_parent->getID();

    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "type", "set" );
    iq->addAttribute( "id", id );
    Tag *q = new Tag( iq, "query" );
    q->addAttribute( "xmlns", XMLNS_PRIVACY );
    Tag *l = new Tag( q, "list" );
    l->addAttribute( "name", name );

    int count = 0;
    PrivacyListHandler::PrivacyList::const_iterator it = list.begin();
    for( ; it != list.end(); ++it )
    {
      Tag *i = new Tag( l, "item" );

      switch( (*it).type() )
      {
        case PrivacyItem::TypeJid:
          i->addAttribute( "type", "jid" );
          break;
        case PrivacyItem::TypeGroup:
          i->addAttribute( "type", "group" );
          break;
        case PrivacyItem::TypeSubscription:
          i->addAttribute( "type", "subscription" );
          break;
        default:
          break;
      }

      switch( (*it).action() )
      {
        case PrivacyItem::ActionAllow:
          i->addAttribute( "action", "allow" );
          break;
        case PrivacyItem::ActionDeny:
          i->addAttribute( "action", "deny" );
          break;
      }

      int pType = (*it).packetType();
      if( pType != 15 )
      {
        if( pType & PrivacyItem::PacketMessage )
          new Tag( i, "message" );
        if( pType & PrivacyItem::PacketPresenceIn )
          new Tag( i, "presence-in" );
        if( pType & PrivacyItem::PacketPresenceOut )
          new Tag( i, "presence-out" );
        if( pType & PrivacyItem::PacketIq )
          new Tag( i, "iq" );
      }

      i->addAttribute( "value", (*it).value() );
      i->addAttribute( "order", ++count );
    }

    m_parent->trackID( this, id, PLStore );
    m_parent->send( iq );
    return id;
  }

  bool PrivacyManager::handleIq( Stanza *stanza )
  {
    if( stanza->subtype() != StanzaIqSet || !m_privacyListHandler )
      return false;

    Tag *l = stanza->findChild( "query" )->findChild( "list" );
    if( l->hasAttribute( "name" ) )
    {
      const std::string& name = l->findAttribute( "name" );
      m_privacyListHandler->handlePrivacyListChanged( name );

      Tag *iq = new Tag( "iq" );
      iq->addAttribute( "type", "result" );
      iq->addAttribute( "id", stanza->id() );
      m_parent->send( iq );
      return true;
    }

    return false;
  }

  bool PrivacyManager::handleIqID( Stanza *stanza, int context )
  {
    if( !m_privacyListHandler )
      return false;

    switch( stanza->subtype() )
    {
      case StanzaIqResult:
        switch( context )
        {
          case PLStore:
            m_privacyListHandler->handlePrivacyListResult( stanza->id(), ResultStoreSuccess );
            break;
          case PLActivate:
            m_privacyListHandler->handlePrivacyListResult( stanza->id(), ResultActivateSuccess );
            break;
          case PLDefault:
            m_privacyListHandler->handlePrivacyListResult( stanza->id(), ResultDefaultSuccess );
            break;
          case PLRemove:
            m_privacyListHandler->handlePrivacyListResult( stanza->id(), ResultRemoveSuccess );
            break;
          case PLRequestNames:
          {
            StringList lists;
            std::string def;
            std::string active;
            Tag *q = stanza->findChild( "query" );
            const Tag::TagList& l = q->children();
            Tag::TagList::const_iterator it = l.begin();
            for( ; it != l.end(); ++it )
            {
              const std::string& name = (*it)->findAttribute( "name" );
              if( (*it)->name() == "default" )
                def = name;
              else if( (*it)->name() == "active" )
                active = name;
              else if( (*it)->name() == "list" )
                lists.push_back( name );
            }

            m_privacyListHandler->handlePrivacyListNames( def, active, lists );
            break;
          }
          case PLRequestList:
          {
            PrivacyListHandler::PrivacyList items;

            Tag *list = stanza->findChild( "query" )->findChild( "list" );
            const std::string& name = list->findAttribute( "name" );
            const Tag::TagList& l = list->children();
            Tag::TagList::const_iterator it = l.begin();
            for( ; it != l.end(); ++it )
            {
              PrivacyItem::ItemType type;
              PrivacyItem::ItemAction action;
              int packetType = 0;

              const std::string& t = (*it)->findAttribute( "type" );
              if( t == "jid" )
                type = PrivacyItem::TypeJid;
              else if( t == "group" )
                type = PrivacyItem::TypeGroup;
              else if( t == "subscription" )
                type = PrivacyItem::TypeSubscription;
              else
                type = PrivacyItem::TypeUndefined;

              const std::string& a = (*it)->findAttribute( "action" );
              if( a == "allow" )
                action = PrivacyItem::ActionAllow;
              else if( a == "deny" )
                action = PrivacyItem::ActionDeny;
              else
                action = PrivacyItem::ActionAllow;

              const std::string& value = (*it)->findAttribute( "value" );

              const Tag::TagList& c = (*it)->children();
              Tag::TagList::const_iterator it_c = c.begin();
              for( ; it_c != c.end(); ++it_c )
              {
                if( (*it_c)->name() == "iq" )
                  packetType |= PrivacyItem::PacketIq;
                else if( (*it_c)->name() == "presence-out" )
                  packetType |= PrivacyItem::PacketPresenceOut;
                else if( (*it_c)->name() == "presence-in" )
                  packetType |= PrivacyItem::PacketPresenceIn;
                else if( (*it_c)->name() == "message" )
                  packetType |= PrivacyItem::PacketMessage;
              }

              PrivacyItem item( type, action, packetType, value );
              items.push_back( item );
            }
            m_privacyListHandler->handlePrivacyList( name, items );
            break;
          }
        }
        break;

      case StanzaIqError:
      {
        switch( stanza->error() )
        {
          case StanzaErrorConflict:
            m_privacyListHandler->handlePrivacyListResult( stanza->id(), ResultConflict );
            break;
          case StanzaErrorItemNotFound:
            m_privacyListHandler->handlePrivacyListResult( stanza->id(), ResultItemNotFound );
            break;
          case StanzaErrorBadRequest:
            m_privacyListHandler->handlePrivacyListResult( stanza->id(), ResultBadRequest );
            break;
          default:
            m_privacyListHandler->handlePrivacyListResult( stanza->id(), ResultUnknownError );
            break;
        }
        break;
      }

      default:
        break;
    }
    return false;
  }

  void PrivacyManager::registerPrivacyListHandler( PrivacyListHandler *plh )
  {
    m_privacyListHandler = plh;
  }

  void PrivacyManager::removePrivacyListHandler()
  {
    m_privacyListHandler = 0;
  }

}
