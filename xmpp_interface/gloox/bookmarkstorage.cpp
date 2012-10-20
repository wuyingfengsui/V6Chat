/*
  Copyright (c) 2005-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#include "bookmarkstorage.h"
#include "clientbase.h"


namespace gloox
{

  BookmarkStorage::BookmarkStorage( ClientBase *parent )
    : PrivateXML( parent ),
      m_bookmarkHandler( 0 )
  {
  }

  BookmarkStorage::~BookmarkStorage()
  {
  }

  void BookmarkStorage::storeBookmarks( const BookmarkList& bList, const ConferenceList& cList )
  {
    Tag *s = new Tag( "storage" );
    s->addAttribute( "xmlns", XMLNS_BOOKMARKS );

    if( bList.size() )
    {
      BookmarkList::const_iterator it = bList.begin();
      for( ; it != bList.end(); ++it )
      {
        Tag *i = new Tag( s, "url" );
        i->addAttribute( "name", (*it).name );
        i->addAttribute( "url", (*it).url );
      }
    }

    if( cList.size() )
    {
      ConferenceList::const_iterator it = cList.begin();
      for( ; it != cList.end(); ++it )
      {
        Tag *i = new Tag( s, "conference" );
        i->addAttribute( "name", (*it).name );
        i->addAttribute( "jid", (*it).jid );
        if( (*it).autojoin )
          i->addAttribute( "autojoin", "true" );
        else
          i->addAttribute( "autojoin", "false" );

        new Tag( i, "nick", (*it).nick );
        new Tag( i, "password", (*it).password );
      }
    }

    storeXML( s, this );
  }

  void BookmarkStorage::requestBookmarks()
  {
    requestXML( "storage", XMLNS_BOOKMARKS, this );
  }

  void BookmarkStorage::handlePrivateXML( const std::string& /*tag*/, Tag *xml )
  {
    BookmarkList bList;
    ConferenceList cList;
    const Tag::TagList& l = xml->children();
    Tag::TagList::const_iterator it = l.begin();
    for( ; it != l.end(); ++it )
    {
      if( (*it)->name() == "url" )
      {
        const std::string& url = (*it)->findAttribute( "url" );
        const std::string& name = (*it)->findAttribute( "name" );

        if( !url.empty() && !name.empty() )
        {
          BookmarkListItem item;
          item.url = url;
          item.name = name;
          bList.push_back( item );
        }
      }
      else if( (*it)->name() == "conference" )
      {
        bool autojoin = false;
        const std::string& jid = (*it)->findAttribute( "jid" );
        const std::string& name = (*it)->findAttribute( "name" );
        const std::string& join = (*it)->findAttribute( "autojoin" );
        if( ( join == "true" ) || ( join == "1" ) )
          autojoin = true;
        std::string nick;
        const Tag* nickname = (*it)->findChild( "nick" );
        if( nickname )
          nick = nickname->cdata();
        std::string pwd;
        const Tag* password = (*it)->findChild( "password" );
        if( password )
          pwd = password->cdata();

        if( !jid.empty() && !name.empty() )
        {
          ConferenceListItem item;
          item.jid = jid;
          item.name = name;
          item.nick = nick;
          item.password = pwd;
          item.autojoin = autojoin;
          cList.push_back( item );
        }
      }
    }

    if( m_bookmarkHandler )
      m_bookmarkHandler->handleBookmarks( bList, cList );
  }

  void BookmarkStorage::handlePrivateXMLResult( const std::string& /*uid*/, PrivateXMLResult /*result*/ )
  {
  }

  void BookmarkStorage::registerBookmarkHandler( BookmarkHandler *bmh )
  {
    m_bookmarkHandler = bmh;
  }

  void BookmarkStorage::removeBookmarkHandler()
  {
    m_bookmarkHandler = 0;
  }

}
