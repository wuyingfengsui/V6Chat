/*
  Copyright (c) 2006-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "delayeddelivery.h"

#include "tag.h"

namespace gloox
{

  DelayedDelivery::DelayedDelivery( const JID& from, const std::string stamp, const std::string& reason )
    : StanzaExtension( ExtDelay ), m_from( from ), m_stamp( stamp ), m_reason( reason ), m_valid( false )
  {
    if( !m_stamp.empty() )
      m_valid = true;
  }


  DelayedDelivery::DelayedDelivery( Tag *tag )
    : StanzaExtension( ExtDelay ), m_valid( false )
  {
    if( !tag || tag->name() != "delay" || !tag->hasAttribute( "xmlns", XMLNS_DELAY )
         || !tag->hasAttribute( "stamp" ) )
      return;

    m_reason = tag->cdata();
    m_stamp = tag->findAttribute( "stamp" );
    m_from = tag->findAttribute( "from" );
    m_valid = true;
  }

  DelayedDelivery::~DelayedDelivery()
  {
  }

  Tag* DelayedDelivery::tag() const
  {
    if( !m_valid )
      return 0;

    Tag *t = new Tag( "delay" );
    t->addAttribute( "xmlns", XMLNS_DELAY );
    if( m_from )
      t->addAttribute( "from", m_from.full() );
    if( !m_stamp.empty() )
      t->addAttribute( "stamp", m_stamp );
    if( !m_reason.empty() )
      t->setCData( m_reason );
    return t;
  }

}
