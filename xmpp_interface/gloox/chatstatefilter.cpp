/*
  Copyright (c) 2005-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "chatstatefilter.h"
#include "chatstatehandler.h"
#include "messageeventhandler.h"
#include "messagesession.h"
#include "stanza.h"

namespace gloox
{

  ChatStateFilter::ChatStateFilter( MessageSession *parent )
  : MessageFilter( parent ), m_chatStateHandler( 0 ), m_lastSent( ChatStateGone ),
      m_enableChatStates( true )
  {
  }

  ChatStateFilter::~ChatStateFilter()
  {
  }

  void ChatStateFilter::filter( Stanza *stanza )
  {
    if( m_chatStateHandler )
    {
      if( stanza->body().empty() )
      {
        m_enableChatStates = true;
        if( stanza->hasChild( "active" ) )
          m_chatStateHandler->handleChatState( stanza->from(), ChatStateActive );
        else if( stanza->hasChild( "composing" ) )
          m_chatStateHandler->handleChatState( stanza->from(), ChatStateComposing );
        else if( stanza->hasChild( "paused" ) )
          m_chatStateHandler->handleChatState( stanza->from(), ChatStatePaused );
        else if( stanza->hasChild( "inactive" ) )
          m_chatStateHandler->handleChatState( stanza->from(), ChatStateInactive );
        else if( stanza->hasChild( "gone" ) )
          m_chatStateHandler->handleChatState( stanza->from(), ChatStateGone );
        else
          m_enableChatStates = false;
      }
      else
      {
        if( stanza->hasChild( "active", "xmlns", XMLNS_CHAT_STATES )
            || stanza->hasChild( "composing", "xmlns", XMLNS_CHAT_STATES )
            || stanza->hasChild( "paused", "xmlns", XMLNS_CHAT_STATES )
            || stanza->hasChild( "inactive", "xmlns", XMLNS_CHAT_STATES )
            || stanza->hasChild( "gone", "xmlns", XMLNS_CHAT_STATES ) )
          m_enableChatStates = true;
        else
          m_enableChatStates = false;
      }
    }
    else
    {
      m_enableChatStates = false;
    }
  }

  void ChatStateFilter::setChatState( ChatStateType state )
  {
    if( !m_enableChatStates || state == m_lastSent )
      return;

    Tag *m = new Tag( "message" );
    m->addAttribute( "to", m_parent->target().full() );
    m->addAttribute( "type", "chat" );

    Tag *s = 0;
    switch( state )
    {
      case ChatStateActive:
        s = new Tag( m, "active" );
        s->addAttribute( "xmlns", XMLNS_CHAT_STATES );
        break;
      case ChatStateComposing:
        s = new Tag( m, "composing" );
        s->addAttribute( "xmlns", XMLNS_CHAT_STATES );
        break;
      case ChatStatePaused:
        s = new Tag( m, "paused" );
        s->addAttribute( "xmlns", XMLNS_CHAT_STATES );
        break;
      case ChatStateInactive:
        s = new Tag( m, "inactive" );
        s->addAttribute( "xmlns", XMLNS_CHAT_STATES );
        break;
      case ChatStateGone:
        s = new Tag( m, "gone" );
        s->addAttribute( "xmlns", XMLNS_CHAT_STATES );
        break;
    }

    m_lastSent = state;

    send( m );
  }

  void ChatStateFilter::decorate( Tag *tag )
  {
    if( !m_enableChatStates )
      return;

    Tag *s = new Tag( tag, "active" );
    s->addAttribute( "xmlns", XMLNS_CHAT_STATES );
  }

  void ChatStateFilter::registerChatStateHandler( ChatStateHandler *csh )
  {
    m_chatStateHandler = csh;
  }

  void ChatStateFilter::removeChatStateHandler()
  {
    m_chatStateHandler = 0;
  }

}
