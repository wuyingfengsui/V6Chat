/*
  Copyright (c) 2004-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "clientbase.h"
#include "rostermanager.h"
#include "disco.h"
#include "rosteritem.h"
#include "rosterlistener.h"
#include "privatexml.h"
#include "base64.h"          // 有修改，添加解码头像


namespace gloox
{

  RosterManager::RosterManager( ClientBase *parent )
    : m_rosterListener( 0 ), m_parent( parent ), m_privateXML( 0 ),
      m_syncSubscribeReq( false )
  {
    if( m_parent )
    {
      m_parent->registerIqHandler( this, XMLNS_ROSTER );
      m_parent->registerPresenceHandler( this );
      m_parent->registerSubscriptionHandler( this );

      m_self = new RosterItem( m_parent->jid().bare() );

      m_privateXML = new PrivateXML( m_parent );
    }
  }

  RosterManager::~RosterManager()
  {
    if( m_parent )
    {
      m_parent->removeIqHandler( XMLNS_ROSTER );
      m_parent->removeIDHandler( this );
      m_parent->removePresenceHandler( this );
      m_parent->removeSubscriptionHandler( this );
      delete m_self;
      delete m_privateXML;
    }

    Roster::iterator it = m_roster.begin();
    for( ; it != m_roster.end(); ++it )
      delete (*it).second;
    m_roster.clear();
  }

  Roster* RosterManager::roster()
  {
    return &m_roster;
  }

  void RosterManager::fill()
  {
    Roster::iterator it2;
    Roster::iterator it = m_roster.begin();
    while( it != m_roster.end() )
    {
      it2 = it++;
      delete (*it2).second;
      m_roster.erase( it2 );
    }

    m_privateXML->requestXML( "roster", XMLNS_ROSTER_DELIMITER, this );

    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "type", "get" );
    iq->addAttribute( "id", m_parent->getID() );
    Tag *q = new Tag( iq, "query" );
    q->addAttribute( "xmlns", XMLNS_ROSTER );
    m_parent->send( iq );
  }

  bool RosterManager::handleIq( Stanza *stanza )
  {
    if( stanza->subtype() == StanzaIqResult ) // initial roster
    {
      extractItems( stanza, false );

      if( m_rosterListener )
        m_rosterListener->handleRoster( m_roster );

      m_parent->rosterFilled();

      return true;
    }
    else if( stanza->subtype() == StanzaIqSet ) // roster item push
    {
      extractItems( stanza, true );

      Tag *iq = new Tag( "iq" );
      iq->addAttribute( "id", stanza->id() );
      iq->addAttribute( "type", "result" );
      m_parent->send( iq );

      return true;
    }
    else if( stanza->subtype() == StanzaIqError )
    {
      if( m_rosterListener )
        m_rosterListener->handleRosterError( stanza );
    }

    return false;
  }

  bool RosterManager::handleIqID( Stanza * /*stanza*/, int /*context*/ )
  {
    return false;
  }

  void RosterManager::handlePresence( Stanza *stanza )
  {
    if( stanza->subtype() == StanzaPresenceError )
      return;

    StringList caps;
	int photoType = 0;
	std::string photo;
    const Tag::TagList& l = stanza->children();
    Tag::TagList::const_iterator it_c = l.begin();
    for( ; it_c != l.end(); ++it_c )
    {
      if( (*it_c)->name() == "c" )
      {
        std::string cap;
        cap.append( (*it_c)->findAttribute( "node" ).c_str() );
        cap.append( "#" );
        cap.append( (*it_c)->findAttribute( "ver" ).c_str() );
        if( (*it_c)->findAttribute( "ext" ).size ())
        {
          cap.append( "#" );
          cap.append( (*it_c)->findAttribute( "ext" ).c_str() );
        }
        caps.push_back( cap );
      }

	  // 修改，添加查找照片
	  if( (*it_c)->name() == "x" )
	  {
		  Tag *pc = (*it_c)->findChild( "PHOTO" );             // URI格式
		  if(pc)
		  {
			  Tag *ec = pc->findChild("EXTVAL");
			  if(ec)
			  {
				  photo = ec->cdata();
				  photoType = 1;
			  }
		  }
		  else
		  {
			  pc = (*it_c)->findChild( "photo" );              // base64格式
			  if(pc)
			  {
				  photo = Base64::decode64(pc->cdata());
				  photoType = 2;
			  }
		  }
	  }
    }

    Roster::iterator it = m_roster.find( stanza->from().bare() );
    if( it != m_roster.end() )
    {
      if( stanza->presence() == PresenceUnavailable )
        (*it).second->removeResource( stanza->from().resource() );
      else
      {
        (*it).second->setPresence( stanza->from().resource(), stanza->presence() );
        (*it).second->setStatus( stanza->from().resource(), stanza->status() );
        (*it).second->setPriority( stanza->from().resource(), stanza->priority() );
  //       (*it).second->setCaps ( caps );
      }

      if( m_rosterListener )
        m_rosterListener->handleRosterPresence( (*(*it).second), stanza->from().resource(),
                                                stanza->presence(), stanza->status(), photoType, photo );
    }
    else if( stanza->from().bare() == m_self->jid() )
    {
      if( stanza->presence() == PresenceUnavailable )
        m_self->removeResource( stanza->from().resource() );
      else
      {
        m_self->setPresence( stanza->from().resource(), stanza->presence() );
        m_self->setStatus( stanza->from().resource(), stanza->status() );
        m_self->setPriority( stanza->from().resource(), stanza->priority() );
  //       (*it).second->setCaps ( caps );
      }

      if( m_rosterListener )
        m_rosterListener->handleSelfPresence( *m_self, stanza->from().resource(),
                                              stanza->presence(), stanza->status(), photo );
    }
    else
    {
      if( m_rosterListener )
        m_rosterListener->handleNonrosterPresence( stanza );
    }
  }

  void RosterManager::subscribe( const JID& jid, const std::string& name,
                                 const StringList& groups, const std::string& msg )
  {
    if( !jid )
      return;

    add( jid, name, groups );

    Tag *s = new Tag( "presence" );
    s->addAttribute( "type", "subscribe" );
    s->addAttribute( "to", jid.bare() );
    s->addAttribute( "from", m_parent->jid().full() );
    if( !msg.empty() )
      new Tag( s, "status", msg );

    m_parent->send( s );
  }


  void RosterManager::add( const JID& jid, const std::string& name, const StringList& groups )
  {
    if( !jid )
      return;

    const std::string& id = m_parent->getID();

    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "type", "set" );
    iq->addAttribute( "id", id );
    Tag *q = new Tag( iq, "query" );
    q->addAttribute( "xmlns", XMLNS_ROSTER );
    Tag *i = new Tag( q, "item" );
    i->addAttribute( "jid", jid.bare() );
    if( !name.empty() )
      i->addAttribute( "name", name );

    if( groups.size() != 0 )
    {
      StringList::const_iterator it = groups.begin();
      for( ; it != groups.end(); ++it )
        new Tag( i, "group", (*it) );
    }

    m_parent->send( iq );
  }

  void RosterManager::unsubscribe( const JID& jid, const std::string& msg )
  {
    Tag *s = new Tag( "presence" );
    s->addAttribute( "type", "unsubscribe" );
    s->addAttribute( "to", jid.bare() );
    if( !msg.empty() )
      new Tag( s, "status", msg );

    m_parent->send( s );

  }

  void RosterManager::cancel( const JID& jid, const std::string& msg )
  {
    Tag *s = new Tag( "presence" );
    s->addAttribute( "type", "unsubscribed" );
    s->addAttribute( "to", jid.bare() );
    if( !msg.empty() )
      new Tag( s, "status", msg );

    m_parent->send( s );

  }

  void RosterManager::remove( const JID& jid )
  {
    const std::string& id = m_parent->getID();

    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "type", "set" );
    iq->addAttribute( "id", id );
    Tag *q = new Tag( iq, "query" );
    q->addAttribute( "xmlns", XMLNS_ROSTER );
    Tag *i = new Tag( q, "item" );
    i->addAttribute( "jid", jid.bare() );
    i->addAttribute( "subscription", "remove" );

    m_parent->send( iq );
  }

  void RosterManager::synchronize()
  {
    Roster::const_iterator it = m_roster.begin();
    for( ; it != m_roster.end(); ++it )
    {
      if( (*it).second->changed() )
      {
        const std::string& id = m_parent->getID();

        Tag *iq = new Tag( "iq" );
        iq->addAttribute( "type", "set" );
        iq->addAttribute( "id", id );
        Tag *q = new Tag( iq, "query" );
        q->addAttribute( "xmlns", XMLNS_ROSTER );
        Tag *i = new Tag( q, "item" );
        i->addAttribute( "jid", (*it).second->jid() );
        if( !(*it).second->name().empty() )
          i->addAttribute( "name", (*it).second->name() );

        if( (*it).second->groups().size() != 0 )
        {
          StringList::const_iterator g_it = (*it).second->groups().begin();
          for( ; g_it != (*it).second->groups().end(); ++g_it )
            new Tag( i, "group", (*g_it) );
        }

        m_parent->send( iq );
      }
    }
  }

  void RosterManager::ackSubscriptionRequest( const JID& to, bool ack )
  {
    Tag *p = new Tag( "presence" );
    if( ack )
      p->addAttribute( "type", "subscribed" );
    else
      p->addAttribute( "type", "unsubscribed" );

    p->addAttribute( "to", to.bare() );
    m_parent->send( p );
  }

  void RosterManager::handleSubscription( Stanza *stanza )
  {
    if( !m_rosterListener )
      return;

    switch( stanza->subtype() )
    {
      case StanzaS10nSubscribe:
      {
        bool answer = m_rosterListener->handleSubscriptionRequest( stanza->from(), stanza->status() );
        if( m_syncSubscribeReq )
        {
          ackSubscriptionRequest( stanza->from(), answer );
        }
        break;
      }
      case StanzaS10nSubscribed:
      {
//         Tag *p = new Tag( "presence" );
//         p->addAttribute( "type", "subscribe" );
//         p->addAttribute( "to", stanza->from().bare() );
//         m_parent->send( p );

        m_rosterListener->handleItemSubscribed( stanza->from() );
        break;
      }

      case StanzaS10nUnsubscribe:
      {
        Tag *p = new Tag( "presence" );
        p->addAttribute( "type", "unsubscribed" );
        p->addAttribute( "to", stanza->from().bare() );
        m_parent->send( p );

        bool answer = m_rosterListener->handleUnsubscriptionRequest( stanza->from(), stanza->status() );
        if( m_syncSubscribeReq && answer )
          remove( stanza->from().bare() );
        break;
      }

      case StanzaS10nUnsubscribed:
      {
//         Tag *p = new Tag( "presence" );
//         p->addAttribute( "type", "unsubscribe" );
//         p->addAttribute( "to", stanza->from().bare() );
//         m_parent->send( p );

        m_rosterListener->handleItemUnsubscribed( stanza->from() );
        break;
      }

      default:
        break;
    }
  }

  void RosterManager::registerRosterListener( RosterListener *rl, bool syncSubscribeReq )
  {
    m_syncSubscribeReq = syncSubscribeReq;
    m_rosterListener = rl;
  }

  void RosterManager::removeRosterListener()
  {
    m_syncSubscribeReq = false;
    m_rosterListener = 0;
  }

  void RosterManager::extractItems( Tag *tag, bool isPush )
  {
    Tag *t = tag->findChild( "query" );
    const Tag::TagList& l = t->children();
    Tag::TagList::const_iterator it = l.begin();
    for( ; it != l.end(); ++it )
    {
      if( (*it)->name() == "item" )
      {
        StringList gl;
        if( (*it)->hasChild( "group" ) )
        {
          const Tag::TagList& g = (*it)->children();
          Tag::TagList::const_iterator it_g = g.begin();
          for( ; it_g != g.end(); ++it_g )
          {
            gl.push_back( (*it_g)->cdata() );
          }
        }

        const JID& jid = (*it)->findAttribute( "jid" );
        Roster::iterator it_d = m_roster.find( jid.bare() );
        if( it_d != m_roster.end() )
        {
          (*it_d).second->setName( (*it)->findAttribute( "name" ) );
          const std::string& sub = (*it)->findAttribute( "subscription" );
          if( sub == "remove" )
          {
            delete (*it_d).second;
            m_roster.erase( it_d );
            if( m_rosterListener )
              m_rosterListener->handleItemRemoved( jid );
            continue;
          }
          const std::string& ask = (*it)->findAttribute( "ask" );
          bool a = false;
          if( !ask.empty() )
            a = true;
          (*it_d).second->setSubscription( sub, a );
          (*it_d).second->setGroups( gl );
          (*it_d).second->setSynchronized();

          if( isPush && m_rosterListener )
            m_rosterListener->handleItemUpdated( jid );
        }
        else
        {
          const std::string& sub = (*it)->findAttribute( "subscription" );
          if( sub == "remove" )
            continue;
          const std::string& name = (*it)->findAttribute( "name" );
          const std::string& ask = (*it)->findAttribute( "ask" );
          bool a = false;
          if( !ask.empty() )
            a = true;

          StringList caps;
          add( jid.bare(), name, gl, caps, sub, a );
          if( isPush && m_rosterListener )
            m_rosterListener->handleItemAdded( jid );
        }
      }
    }
  }

  void RosterManager::add( const std::string& jid, const std::string& name,
                           const StringList& groups, const StringList& caps,
                           const std::string& sub, bool ask )
  {
    if( m_roster.find( jid ) == m_roster.end() )
      m_roster[jid] = new RosterItem( jid, name );

    m_roster[jid]->setSubscription( sub, ask );
    m_roster[jid]->setGroups( groups );
//     m_roster[jid]->setCaps( caps );
    m_roster[jid]->setSynchronized();
  }

  void RosterManager::setDelimiter( const std::string& delimiter )
  {
    m_delimiter = delimiter;
    Tag *t = new Tag( "roster", m_delimiter );
    t->addAttribute( "xmlns", XMLNS_ROSTER_DELIMITER );
    m_privateXML->storeXML( t, this );
  }

  void RosterManager::handlePrivateXML( const std::string& /*tag*/, Tag *xml )
  {
    m_delimiter = xml->cdata();
  }

  void RosterManager::handlePrivateXMLResult( const std::string& /*uid*/, PrivateXMLResult /*result*/ )
  {
  }

  RosterItem* RosterManager::getRosterItem( const JID& jid )
  {
    Roster::const_iterator it = m_roster.find( jid.bare() );
    if( it != m_roster.end() )
      return (*it).second;
    else
      return 0;
  }

}
