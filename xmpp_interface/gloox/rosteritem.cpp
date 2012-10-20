/*
  Copyright (c) 2004-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#include "rosteritem.h"

namespace gloox
{

  RosterItem::RosterItem( const JID& jid, const std::string& name )
    : m_subscription( S10nNone ), m_name( name ), m_jid( jid.bare() ), m_changed( false )
  {
  }

  RosterItem::~RosterItem()
  {
    ResourceMap::iterator it = m_resources.begin();
    for( ; it != m_resources.end(); ++it )
    {
      delete (*it).second;
      (*it).second = 0;
    }
  }

  void RosterItem::setName( const std::string& name )
  {
    m_name = name;
    m_changed = true;
  }

  void RosterItem::setPresence( const std::string& resource, Presence presence )
  {
    if( m_resources.find( resource ) == m_resources.end() )
    {
      m_resources[resource] = new Resource( 0, std::string(), presence );
    }
    else
      m_resources[resource]->setStatus( presence );
  }

  void RosterItem::setStatus( const std::string& resource, const std::string& msg )
  {
    if( m_resources.find( resource ) == m_resources.end() )
    {
      m_resources[resource] = new Resource( 0, msg, PresenceUnavailable );
    }
    else
      m_resources[resource]->setMessage( msg );
  }

  void RosterItem::setPriority( const std::string& resource, int priority )
  {
    if( m_resources.find( resource ) == m_resources.end() )
    {
      m_resources[resource] = new Resource( priority, std::string(), PresenceUnavailable );
    }
    else
      m_resources[resource]->setPriority( priority );
  }

  void RosterItem::setSubscription( const std::string& subscription, bool ask )
  {
    if( subscription == "from" && !ask )
      m_subscription = S10nFrom;
    else if( subscription == "from" && ask )
      m_subscription = S10nFromOut;
    else if( subscription == "to" && !ask )
      m_subscription = S10nTo;
    else if( subscription == "to" && ask )
      m_subscription = S10nToIn;
    else if( subscription == "none" && !ask )
      m_subscription = S10nNone;
    else if( subscription == "none" && ask )
      m_subscription = S10nNoneOut;
    else if( subscription == "both" )
      m_subscription = S10nBoth;
  }

  void RosterItem::setGroups( const StringList& groups )
  {
    m_groups = groups;
    m_changed = true;
  }

  void RosterItem::removeResource( const std::string& resource )
  {
    ResourceMap::iterator it = m_resources.find( resource );
    if( it != m_resources.end() )
    {
      delete (*it).second;
      m_resources.erase( it );
    }
  }

  bool RosterItem::online() const
  {
    return !m_resources.empty();
  }

  const Resource* RosterItem::resource( const std::string& res ) const
  {
    ResourceMap::const_iterator it = m_resources.find( res );
    return it != m_resources.end() ? (*it).second : 0;
  }

}
