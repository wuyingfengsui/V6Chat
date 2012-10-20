/*
  Copyright (c) 2006-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#include "search.h"

#include "clientbase.h"
#include "stanza.h"

namespace gloox
{

  Search::Search( ClientBase *parent )
    : m_parent( parent )
  {
  }

  Search::~Search()
  {
    m_parent->removeIDHandler( this );
  }

  void Search::fetchSearchFields( const JID& directory, SearchHandler *sh )
  {
    if( !m_parent || !directory || !sh )
      return;

    const std::string& id = m_parent->getID();

    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "type", "get" );
    iq->addAttribute( "id", id );
    iq->addAttribute( "to", directory.full() );
    Tag *q = new Tag( iq, "query" );
    q->addAttribute( "xmlns", XMLNS_SEARCH );

    m_track[id] = sh;
    m_parent->trackID( this, id, FetchSearchFields );
    m_parent->send( iq );
  }

  void Search::search( const JID& directory, const DataForm& form, SearchHandler *sh )
  {
    if( !m_parent || !directory || !sh )
      return;

    const std::string& id = m_parent->getID();

    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "id", id );
    iq->addAttribute( "type", "set" );
    iq->addAttribute( "to", directory.full() );
    Tag *q = new Tag( iq, "query" );
    q->addAttribute( "xmlns", XMLNS_SEARCH );
    q->addChild( form.tag() );

    m_track[id] = sh;
    m_parent->trackID( this, id, DoSearch );
    m_parent->send( iq );
  }

  void Search::search( const JID& directory, int fields, const SearchFieldStruct& values, SearchHandler *sh )
  {
    if( !m_parent || !directory || !sh )
      return;

    const std::string& id = m_parent->getID();

    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "id", id );
    iq->addAttribute( "type", "set" );
    iq->addAttribute( "to", directory.full() );
    Tag *q = new Tag( iq, "query" );
    q->addAttribute( "xmlns", XMLNS_SEARCH );

    if( fields & SearchFieldFirst )
      new Tag( q, "first", values.first );
    if( fields & SearchFieldLast )
      new Tag( q, "last", values.last );
    if( fields & SearchFieldNick )
      new Tag( q, "nick", values.nick );
    if( fields & SearchFieldEmail )
      new Tag( q, "email", values.email );

    m_track[id] = sh;
    m_parent->trackID( this, id, DoSearch );
    m_parent->send( iq );
  }

  bool Search::handleIqID( Stanza *stanza, int context )
  {
    TrackMap::iterator it = m_track.find( stanza->id() );
    if( it != m_track.end() )
    {
      switch( stanza->subtype() )
      {
        case StanzaIqResult:
          switch( context )
          {
            case FetchSearchFields:
            {
              Tag *q = stanza->findChild( "query" );
              if( q && q->hasAttribute( "xmlns", XMLNS_SEARCH ) )
              {
                Tag *x = q->findChild( "x", "xmlns", XMLNS_X_DATA );
                if( x )
                {
                  DataForm *df = new DataForm( x );
                  (*it).second->handleSearchFields( stanza->from(), df );
                }
                else
                {
                  int fields = 0;
                  std::string instructions;

                  if( q->hasChild( "first" ) )
                    fields |= SearchFieldFirst;
                  if( q->hasChild( "last" ) )
                    fields |= SearchFieldLast;
                  if( q->hasChild( "nick" ) )
                    fields |= SearchFieldNick;
                  if( q->hasChild( "email" ) )
                    fields |= SearchFieldEmail;
                  if( q->hasChild( "instructions" ) )
                    instructions = q->findChild( "instructions" )->cdata();

                  (*it).second->handleSearchFields( stanza->from(), fields, instructions );
                }
              }
              break;
            }
            case DoSearch:
            {
              Tag *q = stanza->findChild( "query" );
              if( q && q->hasAttribute( "xmlns", XMLNS_SEARCH ) )
              {
                Tag *x = q->findChild( "x", "xmlns", XMLNS_X_DATA );
                if( x )
                {
                  DataForm *df = new DataForm( x );
                  (*it).second->handleSearchResult( stanza->from(), df );
                }
                else
                {
                  SearchResultList e;
                  SearchFieldStruct s;
                  const Tag::TagList &l = q->children();
                  Tag::TagList::const_iterator itl = l.begin();
                  for( ; itl != l.end(); ++itl )
                  {
                    if( (*itl)->name() == "item" )
                    {
                      s.jid.setJID( (*itl)->findAttribute( "jid" ) );
                      Tag *t = 0;
                      if( ( t = (*itl)->findChild( "first" ) ) != 0 )
                        s.first = t->cdata();
                      if( ( t = (*itl)->findChild( "last" ) ) != 0 )
                        s.last = t->cdata();
                      if( ( t = (*itl)->findChild( "nick" ) ) != 0 )
                        s.nick = t->cdata();
                      if( ( t = (*itl)->findChild( "email" ) ) != 0 )
                        s.email = t->cdata();
                      e.push_back( s );
                    }
                  }

                  (*it).second->handleSearchResult( stanza->from(), e );
                }
              }
              break;
            }
          }
          break;
        case StanzaIqError:
          (*it).second->handleSearchError( stanza->from(), stanza );
          break;

        default:
          break;
      }

      m_track.erase( it );
    }

    return false;
  }

}
