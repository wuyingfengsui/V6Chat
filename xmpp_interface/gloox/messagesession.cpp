/*
  Copyright (c) 2005-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "messagesession.h"

#include "messagefilter.h"
#include "messagehandler.h"
#include "clientbase.h"
#include "disco.h"
#include "stanza.h"
#include "tag.h"

namespace gloox
{

  MessageSession::MessageSession( ClientBase *parent, const JID& jid, bool wantUpgrade, int types )
    : m_parent( parent ), m_target( jid ), m_messageHandler( 0 ),
      m_types( types ), m_wantUpgrade( wantUpgrade ), m_hadMessages( false )
  {
    if( m_parent )
      m_parent->registerMessageSession( this );
  }

  MessageSession::~MessageSession()
  {
    MessageFilterList::const_iterator it = m_messageFilterList.begin();
    for( ; it != m_messageFilterList.end(); ++it )
      delete (*it);
  }

  void MessageSession::handleMessage( Stanza *stanza )
  {
    if( m_wantUpgrade && stanza->from().bare() == m_target.full() )
      setResource( stanza->from().resource() );

    if( !m_hadMessages )
    {
      m_hadMessages = true;
      if( stanza->thread().empty() )
      {
        m_thread = "gloox" + m_parent->getID();
        stanza->setThread( m_thread );
      }
      else
        m_thread = stanza->thread();
    }

    MessageFilterList::const_iterator it = m_messageFilterList.begin();
    for( ; it != m_messageFilterList.end(); ++it )
    {
      (*it)->filter( stanza );
    }

    if( m_messageHandler && !stanza->body().empty() )
      m_messageHandler->handleMessage( stanza, this );
  }

  void MessageSession::send( const std::string& message, const std::string& subject )
  {
    if( !m_hadMessages )
    {
      m_thread = "gloox" + m_parent->getID();
      m_hadMessages = true;
    }

    Tag *m = new Tag( "message" );
    m->addAttribute( "type", "chat" );
    new Tag( m, "body", message );
    if( !subject.empty() )
      new Tag( m, "subject", subject );

    m->addAttribute( "from", m_parent->jid().full() );
    m->addAttribute( "to", m_target.full() );
    m->addAttribute( "id", m_parent->getID() );
    new Tag( m, "thread", m_thread );

    decorate( m );

    m_parent->send( m );
  }

  void MessageSession::send( Tag *tag )
  {
    m_parent->send( tag );
  }

  void MessageSession::decorate( Tag *tag )
  {
    MessageFilterList::const_iterator it = m_messageFilterList.begin();
    for( ; it != m_messageFilterList.end(); ++it )
    {
      (*it)->decorate( tag );
    }
  }

  void MessageSession::resetResource()
  {
    m_wantUpgrade = true;
    m_target.setResource( "" );
  }

  void MessageSession::setResource( const std::string& resource )
  {
    m_target.setResource( resource );
  }

  void MessageSession::registerMessageHandler( MessageHandler *mh )
  {
    m_messageHandler = mh;
  }

  void MessageSession::removeMessageHandler()
  {
    m_messageHandler = 0;
  }

  void MessageSession::registerMessageFilter( MessageFilter *mf )
  {
    m_messageFilterList.push_back( mf );
  }

  void MessageSession::removeMessageFilter( MessageFilter *mf )
  {
    m_messageFilterList.remove( mf );
  }

  void MessageSession::disposeMessageFilter( MessageFilter *mf )
  {
    removeMessageFilter( mf );
    delete mf;
  }

}
