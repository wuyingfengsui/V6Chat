/*
  Copyright (c) 2007-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "simanager.h"

#include "siprofilehandler.h"
#include "sihandler.h"
#include "clientbase.h"
#include "disco.h"

namespace gloox
{

  SIManager::SIManager( ClientBase* parent, bool advertise )
    : m_parent( parent ), m_advertise( advertise )
  {
    if( m_parent && m_advertise )
    {
      m_parent->registerIqHandler( this, XMLNS_SI );
      if( m_parent->disco() )
        m_parent->disco()->addFeature( XMLNS_SI );
    }
  }

  SIManager::~SIManager()
  {
    if( m_parent && m_advertise )
    {
      m_parent->removeIqHandler( XMLNS_SI );
      m_parent->removeIDHandler( this );
      if( m_parent->disco() )
        m_parent->disco()->removeFeature( XMLNS_SI );
    }
  }

  const std::string SIManager::requestSI( SIHandler* sih, const JID& to, const std::string& profile,
                                          Tag* child1, Tag* child2, const std::string& mimetype )
  {
    if( !m_parent || !sih )
      return std::string();

    const std::string& id = m_parent->getID();
    const std::string& id2 = m_parent->getID();

    Tag* iq = new Tag( "iq" );
    iq->addAttribute( "type", "set" );
    iq->addAttribute( "id", id );
    iq->addAttribute( "to", to.full() );
    Tag* si = new Tag( iq, "si" );
    si->addAttribute( "xmlns", XMLNS_SI );
    si->addAttribute( "id", id2 );
    if( mimetype.empty() )
      si->addAttribute( "mime-type", "binary/octet-stream" );
    else
      si->addAttribute( "mime-type", mimetype );
    si->addAttribute( "profile", profile );

    si->addChild( child1 );
    si->addChild( child2 );

    TrackStruct t;
    t.sid = id2;
    t.profile = profile;
    t.sih = sih;
    m_track[id] = t;
    m_parent->trackID( this, id, OfferSI );
    m_parent->send( iq );

    return id2;
  }

  void SIManager::acceptSI( const JID& to, const std::string& id, Tag* child1, Tag* child2 )
  {
    Tag* iq = new Tag( "iq" );
    iq->addAttribute( "id", id );
    iq->addAttribute( "to", to.full() );
    iq->addAttribute( "type", "result" );
    Tag* si = new Tag( iq, "si" );
    si->addAttribute( "xmlns", XMLNS_SI );

    si->addChild( child1 );
    si->addChild( child2 );

    m_parent->send( iq );
  }

  void SIManager::declineSI( const JID& to, const std::string& id, SIError reason, const std::string& text )
  {
    Tag* iq = new Tag( "iq" );
    iq->addAttribute( "id", id );
    iq->addAttribute( "to", to.full() );
    iq->addAttribute( "type", "error" );
    Tag* error = new Tag( iq, "error" );
    if( reason == NoValidStreams || reason == BadProfile )
    {
      error->addAttribute( "code", "400" );
      error->addAttribute( "type", "cancel" );
      new Tag( error, "bad-request", "xmlns", XMLNS_XMPP_STANZAS );
      if( reason == NoValidStreams )
        new Tag( error, "no-valid-streams", "xmlns", XMLNS_SI );
      else if( reason == BadProfile )
        new Tag( error, "bad-profile", "xmlns", XMLNS_SI );
    }
    else
    {
      error->addAttribute( "code", "403" );
      error->addAttribute( "type", "cancel" );
      new Tag( error, "forbidden", "xmlns", XMLNS_XMPP_STANZAS );
      if( !text.empty() )
      {
        Tag* t = new Tag( error, "text", "xmlns", XMLNS_XMPP_STANZAS );
        t->setCData( text );
      }
    }

    m_parent->send( iq );
  }

  void SIManager::registerProfile( const std::string& profile, SIProfileHandler* sih )
  {
    if( !sih || profile.empty() )
      return;

    m_handlers[profile] = sih;

    if( m_parent && m_advertise && m_parent->disco() )
      m_parent->disco()->addFeature( profile );
  }

  void SIManager::removeProfile( const std::string& profile )
  {
    if( profile.empty() )
      return;

    m_handlers.erase( profile );

    if( m_parent && m_advertise && m_parent->disco() )
      m_parent->disco()->removeFeature( profile );
  }

  bool SIManager::handleIq( Stanza *stanza )
  {
    TrackMap::iterator it = m_track.find( stanza->id() );
    if( it != m_track.end() )
      return false;

    Tag *si = stanza->findChild( "si", "xmlns", XMLNS_SI );
    if( si && si->hasAttribute( "profile" ) )
    {
      const std::string& profile = si->findAttribute( "profile" );
      HandlerMap::const_iterator it = m_handlers.find( profile );
      if( it != m_handlers.end() && (*it).second )
      {
        Tag* p = si->findChildWithAttrib( "xmlns", profile );
        Tag* f = si->findChild( "feature", "xmlns", XMLNS_FEATURE_NEG );
        (*it).second->handleSIRequest( stanza->from(), stanza->id(), profile, si, p, f );
        return true;
      }
    }

    return false;
  }

  bool SIManager::handleIqID( Stanza *stanza, int context )
  {
    switch( stanza->subtype() )
    {
      case StanzaIqResult:
        if( context == OfferSI )
        {
          TrackMap::iterator it = m_track.find( stanza->id() );
          if( it != m_track.end() )
          {
            Tag* si = stanza->findChild( "si", "xmlns", XMLNS_SI );
            Tag* ptag = 0;
            Tag* fneg = 0;
            if( si )
            {
              ptag = si->findChildWithAttrib( "xmlns", (*it).second.profile );
              fneg = si->findChild( "feature", "xmlns", XMLNS_FEATURE_NEG );
            }
            (*it).second.sih->handleSIRequestResult( stanza->from(), (*it).second.sid, si, ptag, fneg );
            m_track.erase( it );
          }
          return true;
        }
        break;
      case StanzaIqError:
        if( context == OfferSI )
        {
          TrackMap::iterator it = m_track.find( stanza->id() );
          if( it != m_track.end() )
          {
            (*it).second.sih->handleSIRequestError( stanza, (*it).second.sid );
            m_track.erase( it );
          }
          return true;
        }
        break;
      default:
        break;
    }

    return false;
  }

}
