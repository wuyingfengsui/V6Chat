/*
  Copyright (c) 2006-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "mucmessagesession.h"
#include "clientbase.h"
#include "stanza.h"
#include "messagehandler.h"

namespace gloox
{

  MUCMessageSession::MUCMessageSession( ClientBase *parent, const JID& jid )
    : MessageSession( parent, jid, false, StanzaMessageGroupchat | StanzaMessageChat
                                          | StanzaMessageNormal | StanzaMessageError )
  {
  }

  MUCMessageSession::~MUCMessageSession()
  {
  }

  void MUCMessageSession::handleMessage( Stanza *stanza )
  {
    if( m_messageHandler )
      m_messageHandler->handleMessage( stanza );
  }

  void MUCMessageSession::send( const std::string& message )
  {
    Tag *m = new Tag( "message" );
    m->addAttribute( "type", "groupchat" );
    new Tag( m, "body", message );

    m->addAttribute( "from", m_parent->jid().full() );
    m->addAttribute( "to", m_target.full() );

//     decorate( m );

    m_parent->send( m );
  }

  void MUCMessageSession::setSubject( const std::string& subject )
  {
    Tag *m = new Tag( "message" );
    m->addAttribute( "to", m_target.bare() );
    m->addAttribute( "type", "groupchat" );
    new Tag( m, "subject", subject );

    m_parent->send( m );
  }

}
