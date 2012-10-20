/*
  Copyright (c) 2006-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "xdelayeddelivery.h"

#include "tag.h"

namespace gloox
{

  XDelayedDelivery::XDelayedDelivery( const JID& from, const std::string stamp, const std::string& reason )
    : StanzaExtension( ExtXDelay ), m_from( from ), m_stamp( stamp ), m_reason( reason ), m_valid( false )
  {
    if( !m_stamp.empty() )
      m_valid = true;
  }

  XDelayedDelivery::XDelayedDelivery( Tag *tag )
    : StanzaExtension( ExtXDelay ), m_valid( false )
  {
    if( !tag || tag->name() != "x" || !tag->hasAttribute( "xmlns", XMLNS_X_DELAY )
         || !tag->hasAttribute( "stamp" ) )
      return;

    m_reason = tag->cdata();
    m_stamp = tag->findAttribute( "stamp" );
    m_from.setJID( tag->findAttribute( "from" ) );
    m_valid = true;
  }

  XDelayedDelivery::~XDelayedDelivery()
  {
  }

  Tag* XDelayedDelivery::tag() const
  {
    if( !m_valid )
      return 0;

    Tag *t = new Tag( "x" );
    t->addAttribute( "xmlns", XMLNS_X_DELAY );
    if( m_from )
      t->addAttribute( "from", m_from.full() );
    if( !m_stamp.empty() )
      t->addAttribute( "stamp", m_stamp );
    if( !m_reason.empty() )
      t->setCData( m_reason );
    return t;
  }

}
