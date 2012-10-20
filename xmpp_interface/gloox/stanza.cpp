/*
  Copyright (c) 2005-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "stanza.h"
#include "jid.h"
#include "stanzaextension.h"
#include "stanzaextensionfactory.h"

#include <cstdlib>

namespace gloox
{

  Stanza::Stanza( const std::string& name, const std::string& cdata, const std::string& xmllang,
                  bool incoming )
    : Tag( name, cdata, incoming ), m_subtype( StanzaSubUndefined ), m_presence( PresenceUnknown ),
      m_stanzaError( StanzaErrorUndefined ), m_stanzaErrorType( StanzaErrorTypeUndefined ),
      m_stanzaErrorAppCondition( 0 ), m_xmllang( xmllang ), m_priority( -300 )
  {
  }

  Stanza::Stanza( const Tag *tag )
    : Tag( tag->name(), tag->cdata(), false ), m_presence( PresenceUnknown ),
      m_stanzaError( StanzaErrorUndefined ), m_stanzaErrorType( StanzaErrorTypeUndefined ),
      m_stanzaErrorAppCondition( 0 ), m_xmllang( "default" )
  {
    m_attribs = tag->attributes();
    const Tag::TagList& l = tag->children();
    Tag::TagList::const_iterator it = l.begin();
    for( ; it != l.end(); ++it )
    {
      addChild( (*it)->clone() );
    }

    init();
  }

  Stanza::~Stanza()
  {
    StanzaExtensionList::iterator it = m_extensionList.begin();
    for( ; it != m_extensionList.end(); ++it )
    {
      delete (*it);
    }
  }

  void Stanza::init()
  {
    m_from.setJID( findAttribute( "from" ) );
    m_to.setJID( findAttribute( "to" ) );
    m_id = findAttribute( "id" );

    if( m_name == "iq" )
    {
      m_type = StanzaIq;
      if( hasAttribute( "type", "get" ) )
        m_subtype = StanzaIqGet;
      else if( hasAttribute( "type", "set" ) )
        m_subtype = StanzaIqSet;
      else if( hasAttribute( "type", "result" ) )
        m_subtype = StanzaIqResult;
      else if( hasAttribute( "type", "error" ) )
        m_subtype = StanzaIqError;
      else
        m_subtype = StanzaSubUndefined;

      Tag *t = findChildWithAttrib( "xmlns" );
      if( t )
        m_xmlns = t->findAttribute( "xmlns" );

      const TagList& c = children();
      TagList::const_iterator it = c.begin();
      for( ; it != c.end(); ++it )
      {
        StanzaExtension *se = StanzaExtensionFactory::create( (*it) );
        if( se )
          m_extensionList.push_back( se );
      }
    }
    else if( m_name == "message" )
    {
      m_type = StanzaMessage;
      if( hasAttribute( "type", "chat" ) )
        m_subtype = StanzaMessageChat;
      else if( hasAttribute( "type", "error" ) )
        m_subtype = StanzaMessageError;
      else if( hasAttribute( "type", "headline" ) )
        m_subtype = StanzaMessageHeadline;
      else if( hasAttribute( "type", "groupchat" ) )
        m_subtype = StanzaMessageGroupchat;
      else
        m_subtype = StanzaMessageNormal;

      const TagList& c = children();
      TagList::const_iterator it = c.begin();
      for( ; it != c.end(); ++it )
      {
        if( (*it)->name() == "body" )
        {
          setLang( m_body, (*it) );
        }
        else if( (*it)->name() == "subject" )
        {
          setLang( m_subject, (*it) );
        }
        else if( (*it)->name() == "thread" )
        {
          m_thread = (*it)->cdata();
        }
        else
        {
          StanzaExtension *se = StanzaExtensionFactory::create( (*it) );
          if( se )
            m_extensionList.push_back( se );
        }
      }
    }
    else if( m_name == "presence" )
    {
      if( hasAttribute( "type", "subscribe" ) )
      {
        m_type = StanzaS10n;
        m_subtype = StanzaS10nSubscribe;
      }
      else if( hasAttribute( "type", "subscribed" ) )
      {
        m_type = StanzaS10n;
        m_subtype = StanzaS10nSubscribed;
      }
      else if( hasAttribute( "type", "unsubscribe" ) )
      {
        m_type = StanzaS10n;
        m_subtype = StanzaS10nUnsubscribe;
      }
      else if( hasAttribute( "type", "unsubscribed" ) )
      {
        m_type = StanzaS10n;
        m_subtype = StanzaS10nUnsubscribed;
      }
      else if( hasAttribute( "type", "unavailable" ) )
      {
        m_type = StanzaPresence;
        m_subtype = StanzaPresenceUnavailable;
      }
      else if( hasAttribute( "type", "probe" ) )
      {
        m_type = StanzaPresence;
        m_subtype = StanzaPresenceProbe;
      }
      else if( hasAttribute( "type", "error" ) )
      {
        m_type = StanzaPresence;
        m_subtype = StanzaPresenceError;
      }
      else if( !hasAttribute( "type" ) )
      {
        m_type = StanzaPresence;
        m_subtype = StanzaPresenceAvailable;
      }
      else
      {
        m_type = StanzaPresence;
        m_subtype = StanzaSubUndefined;
      }
    }
    else
    {
      m_type = StanzaUndefined;
      m_subtype = StanzaSubUndefined;
    }

    if( m_type == StanzaPresence )
    {
      if( !hasAttribute( "type" ) )
        m_presence = PresenceAvailable;

      if( hasChildWithCData( "show", "chat" ) )
        m_presence = PresenceChat;
      else if( hasChildWithCData( "show", "away" ) )
        m_presence = PresenceAway;
      else if( hasChildWithCData( "show", "dnd" ) )
        m_presence = PresenceDnd;
      else if( hasChildWithCData( "show", "xa" ) )
        m_presence = PresenceXa;
      else if( hasAttribute( "type", "unavailable" ) )
        m_presence = PresenceUnavailable;

      if( hasChild( "priority" ) )
        m_priority = atoi( findChild( "priority" )->cdata().c_str() );
    }

    if( m_type == StanzaPresence || m_type == StanzaS10n )
    {
      const TagList& c = children();
      TagList::const_iterator it = c.begin();
      for( ; it != c.end(); ++it )
      {
        if( (*it)->name() == "status" )
        {
          setLang( m_status, (*it) );
        }
        else
        {
          StanzaExtension *se = StanzaExtensionFactory::create( (*it) );
          if( se )
            m_extensionList.push_back( se );
        }
      }
    }

    m_xmllang = findAttribute( "xml:lang" );

    if( hasAttribute( "type", "error" ) && hasChild( "error" ) )
    {
      Tag *e = findChild( "error" );

      if( e->hasAttribute( "type", "cancel" ) )
        m_stanzaErrorType = StanzaErrorTypeCancel;
      else if( e->hasAttribute( "type", "continue" ) )
        m_stanzaErrorType = StanzaErrorTypeContinue;
      else if( e->hasAttribute( "type", "modify" ) )
        m_stanzaErrorType = StanzaErrorTypeModify;
      else if( e->hasAttribute( "type", "auth" ) )
        m_stanzaErrorType = StanzaErrorTypeAuth;
      else if( e->hasAttribute( "type", "wait" ) )
        m_stanzaErrorType = StanzaErrorTypeWait;

      const TagList& c = e->children();
      TagList::const_iterator it = c.begin();
      StanzaError err = StanzaErrorUndefined;
      for( ; it != c.end(); ++it )
      {
        if( (*it)->name() == "bad-request" )
          err = StanzaErrorBadRequest;
        else if( (*it)->name() == "conflict" )
          err = StanzaErrorConflict;
        else if( (*it)->name() == "feature-not-implemented" )
          err = StanzaErrorFeatureNotImplemented;
        else if( (*it)->name() == "forbidden" )
          err = StanzaErrorForbidden;
        else if( (*it)->name() == "gone" )
          err = StanzaErrorGone;
        else if( (*it)->name() == "internal-server-error" )
          err = StanzaErrorInternalServerError;
        else if( (*it)->name() == "item-not-found" )
          err = StanzaErrorItemNotFound;
        else if( (*it)->name() == "jid-malformed" )
          err = StanzaErrorJidMalformed;
        else if( (*it)->name() == "not-acceptable" )
          err = StanzaErrorNotAcceptable;
        else if( (*it)->name() == "not-allowed" )
          err = StanzaErrorNotAllowed;
        else if( (*it)->name() == "not-authorized" )
          err = StanzaErrorNotAuthorized;
        else if( (*it)->name() == "recipient-unavailable" )
          err = StanzaErrorRecipientUnavailable;
        else if( (*it)->name() == "redirect" )
          err = StanzaErrorRedirect;
        else if( (*it)->name() == "registration-required" )
          err = StanzaErrorRegistrationRequired;
        else if( (*it)->name() == "remote-server-not-found" )
          err = StanzaErrorRemoteServerNotFound;
        else if( (*it)->name() == "remote-server-timeout" )
          err = StanzaErrorRemoteServerTimeout;
        else if( (*it)->name() == "resource-constraint" )
          err = StanzaErrorResourceConstraint;
        else if( (*it)->name() == "service-unavailable" )
          err = StanzaErrorServiceUnavailable;
        else if( (*it)->name() == "subscription-required" )
          err = StanzaErrorSubscribtionRequired;
        else if( (*it)->name() == "undefined-condition" )
          err = StanzaErrorUndefinedCondition;
        else if( (*it)->name() == "unexpected-request" )
          err = StanzaErrorUnexpectedRequest;
        else if( (*it)->name() == "text" )
        {
          setLang( m_errorText, (*it) );
        }
        else {
          m_stanzaErrorAppCondition = (*it);
        }

        if( err != StanzaErrorUndefined && (*it)->hasAttribute( "xmlns", XMLNS_XMPP_STANZAS ) )
        {
          m_stanzaError = err;
        }
      }
    }
  }

  void Stanza::addExtension( StanzaExtension *se )
  {
    m_extensionList.push_back( se );
    addChild( se->tag() );
  }

  Stanza* Stanza::createIqStanza( const JID& to, const std::string& id,
                                  StanzaSubType subtype, const std::string& xmlns, Tag* tag )
  {
    Stanza *s = new Stanza( "iq" );
    switch( subtype )
    {
      case StanzaIqError:
        s->addAttribute( "type", "error" );
        break;
      case StanzaIqSet:
        s->addAttribute( "type", "set" );
        break;
      case StanzaIqResult:
        s->addAttribute( "type", "result" );
        break;
      case StanzaIqGet:
      default:
        s->addAttribute( "type", "get" );
        break;
    }

    if( !xmlns.empty() )
    {
      Tag *q = new Tag( s, "query" );
      q->addAttribute( "xmlns", xmlns );
      if( tag )
        q->addChild( tag );
    }
    s->addAttribute( "to", to.full() );
    s->addAttribute( "id", id );

    s->finalize();

    return s;
  }

  Stanza* Stanza::createPresenceStanza( const JID& to, const std::string& msg,
                                        Presence status, const std::string& xmllang )
  {
    Stanza *s = new Stanza( "presence" );
    switch( status )
    {
      case PresenceUnavailable:
        s->addAttribute( "type", "unavailable" );
        break;
      case PresenceChat:
        new Tag( s, "show", "chat" );
        break;
      case PresenceAway:
        new Tag( s, "show", "away" );
        break;
      case PresenceDnd:
        new Tag( s, "show", "dnd" );
        break;
      case PresenceXa:
        new Tag( s, "show", "xa" );
        break;
      default:
        break;
    }

    if( to )
      s->addAttribute( "to", to.full() );

    if( !msg.empty() )
    {
      Tag *t = new Tag( s, "status", msg );
      t->addAttribute( "xml:lang", xmllang );
    }

    s->finalize();

    return s;
  }

  Stanza* Stanza::createMessageStanza( const JID& to, const std::string& body,
                                       StanzaSubType subtype, const std::string& subject,
                                       const std::string& thread, const std::string& xmllang )
  {
    Stanza *s = new Stanza( "message" );
    switch( subtype )
    {
      case StanzaMessageError:
        s->addAttribute( "type", "error" );
        break;
      case StanzaMessageNormal:
        s->addAttribute( "type", "normal" );
        break;
      case StanzaMessageHeadline:
        s->addAttribute( "type", "headline" );
        break;
      case StanzaMessageGroupchat:
        s->addAttribute( "type", "groupchat" );
        break;
      case StanzaMessageChat:
      default:
        s->addAttribute( "type", "chat" );
        break;
    }

    s->addAttribute( "to", to.full() );

    if( !body.empty() )
    {
      Tag *b = new Tag( s, "body", body );
      b->addAttribute( "xml:lang", xmllang );
    }
    if( !subject.empty() )
    {
      Tag *su = new Tag( s, "subject", subject );
      su->addAttribute( "xml:lang", xmllang );
    }
    if( !thread.empty() )
      new Tag( s, "thread", thread );

    s->finalize();

    return s;
  }

  Stanza* Stanza::createSubscriptionStanza( const JID& to, const std::string& msg,
                                            StanzaSubType subtype, const std::string& xmllang )
  {
    Stanza *s = new Stanza( "presence" );
    switch( subtype )
    {
      case StanzaS10nSubscribed:
        s->addAttribute( "type", "subscribed" );
        break;
      case StanzaS10nUnsubscribe:
        s->addAttribute( "type", "unsubscribe" );
        break;
      case StanzaS10nUnsubscribed:
        s->addAttribute( "type", "unsubscribed" );
        break;
      case StanzaS10nSubscribe:
      default:
        s->addAttribute( "type", "subscribe" );
        break;
    }

    s->addAttribute( "to", to.full() );
    if( !msg.empty() )
    {
      Tag *t = new Tag( s, "status", msg );
      t->addAttribute( "xml:lang", xmllang );
    }

    s->finalize();

    return s;
  }

  void Stanza::setLang( StringMap& map, const Tag *tag )
  {
    const std::string& lang = tag->findAttribute( "xml:lang" );
    map[ lang.empty() ? "default" : lang ] = tag->cdata();
  }

  const std::string Stanza::findLang( const StringMap& map, const std::string& lang )
  {
    StringMap::const_iterator it = map.find( lang );
    return ( it != map.end() ) ? (*it).second : std::string();
  }

}
