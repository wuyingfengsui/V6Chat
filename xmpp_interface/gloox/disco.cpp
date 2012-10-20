/*
  Copyright (c) 2004-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "disco.h"
#include "discohandler.h"
#include "clientbase.h"
#include "disconodehandler.h"


namespace gloox
{

  Disco::Disco( ClientBase *parent )
    : m_parent( parent )
  {
    addFeature( XMLNS_VERSION );
    addFeature( XMLNS_DISCO_INFO );
    addFeature( XMLNS_DISCO_ITEMS );
    if( m_parent )
    {
      m_parent->registerIqHandler( this, XMLNS_DISCO_INFO );
      m_parent->registerIqHandler( this, XMLNS_DISCO_ITEMS );
      m_parent->registerIqHandler( this, XMLNS_VERSION );
    }
  }

  Disco::~Disco()
  {
    if( m_parent )
    {
      m_parent->removeIqHandler( XMLNS_DISCO_INFO );
      m_parent->removeIqHandler( XMLNS_DISCO_ITEMS );
      m_parent->removeIqHandler( XMLNS_VERSION );
      m_parent->removeIDHandler( this );
    }
  }

  bool Disco::handleIq( Stanza *stanza )
  {
    switch( stanza->subtype() )
    {
      case StanzaIqGet:
        if( stanza->xmlns() == XMLNS_VERSION )
        {
          Tag *iq = new Tag( "iq" );
          iq->addAttribute( "id", stanza->id() );
          iq->addAttribute( "from", m_parent->jid().full() );
          iq->addAttribute( "to", stanza->from().full() );
          iq->addAttribute( "type", "result" );
          Tag *query = new Tag( iq, "query" );
          query->addAttribute( "xmlns", XMLNS_VERSION );
          new Tag( query, "name", m_versionName );
          new Tag( query, "version", m_versionVersion );
          new Tag( query, "os", m_versionOs );

          m_parent->send( iq );
        }
        else if( stanza->xmlns() == XMLNS_DISCO_INFO && stanza->hasChild( "query" ) )
        {
          Tag *iq = new Tag( "iq" );
          iq->addAttribute( "id", stanza->id() );
          iq->addAttribute( "from", m_parent->jid().full() );
          iq->addAttribute( "to", stanza->from().full() );
          iq->addAttribute( "type", "result" );
          Tag *query = new Tag( iq, "query" );
          query->addAttribute( "xmlns", XMLNS_DISCO_INFO );

          Tag *q = stanza->findChild( "query" );
          const std::string& node = q->findAttribute( "node" );
          if( !node.empty() )
          {
            query->addAttribute( "node", node );
            new Tag( query, "feature", "var", XMLNS_DISCO_INFO );

            DiscoNodeHandlerMap::const_iterator it = m_nodeHandlers.find( node );
            if( it != m_nodeHandlers.end() )
            {
              std::string name;
              DiscoNodeHandlerList::const_iterator in = (*it).second.begin();
              for( ; in != (*it).second.end(); ++in )
              {
                const StringMap& identities = (*in)->handleDiscoNodeIdentities( node, name );
                StringMap::const_iterator im = identities.begin();
                for( ; im != identities.end(); ++im )
                {
                  Tag *i = new Tag( query, "identity" );
                  i->addAttribute( "category", (*im).first );
                  i->addAttribute( "type", (*im).second );
                  i->addAttribute( "name", name );
                }
                const StringList& features = (*in)->handleDiscoNodeFeatures( node );
                StringList::const_iterator fi = features.begin();
                for( ; fi != features.end(); ++fi )
                {
                  Tag *f = new Tag( query, "feature" );
                  f->addAttribute( "var", (*fi) );
                }
              }
            }
          }
          else
          {
            Tag *i = new Tag( query, "identity" );
            i->addAttribute( "category", m_identityCategory );
            i->addAttribute( "type", m_identityType );
            i->addAttribute( "name", m_versionName );

            StringList::const_iterator it = m_features.begin();
            for( ; it != m_features.end(); ++it )
            {
              Tag *f = new Tag( query, "feature" );
              f->addAttribute( "var", (*it) );
            }
          }

          m_parent->send( iq );
        }
        else if( stanza->xmlns() == XMLNS_DISCO_ITEMS && stanza->hasChild( "query" ) )
        {
          Tag *iq = new Tag( "iq" );
          iq->addAttribute( "id", stanza->id() );
          iq->addAttribute( "to", stanza->from().full() );
          iq->addAttribute( "from", m_parent->jid().full() );
          iq->addAttribute( "type", "result" );
          Tag *query = new Tag( iq, "query" );
          query->addAttribute( "xmlns", XMLNS_DISCO_ITEMS );

          DiscoNodeHandlerMap::const_iterator it;
          Tag *q = stanza->findChild( "query" );
          const std::string& node = q->findAttribute( "node" );
          query->addAttribute( "node", node );
          it = m_nodeHandlers.find( node );
          if( it != m_nodeHandlers.end() )
          {
            DiscoNodeHandlerList::const_iterator in = (*it).second.begin();
            for( ; in != (*it).second.end(); ++in )
            {
              const DiscoNodeItemList& items = (*in)->handleDiscoNodeItems( node );
              DiscoNodeItemList::const_iterator it = items.begin();
              for( ; it != items.end(); ++it )
              {
                Tag *i = new Tag( query, "item" );
                i->addAttribute( "jid",  (*it).jid.empty() ? m_parent->jid().full() : (*it).jid );
                i->addAttribute( "node", (*it).node );
                i->addAttribute( "name", (*it).name );
              }
            }
          }

          m_parent->send( iq );
        }
        return true;
        break;

      case StanzaIqSet:
      {
        bool res = false;
        DiscoHandlerList::const_iterator it = m_discoHandlers.begin();
        for( ; it != m_discoHandlers.end(); ++it )
        {
          if( (*it)->handleDiscoSet( stanza ) )
            res = true;
        }
        return res;
        break;
      }

      default:
        break;
    }
    return false;
  }

  bool Disco::handleIqID( Stanza *stanza, int context )
  {
    DiscoHandlerMap::iterator it = m_track.find( stanza->id() );
    if( it != m_track.end() )
    {
      switch( stanza->subtype() )
      {
        case StanzaIqResult:
          switch( context )
          {
            case GET_DISCO_INFO:
              (*it).second.dh->handleDiscoInfoResult( stanza, (*it).second.context );
              break;
            case GET_DISCO_ITEMS:
              (*it).second.dh->handleDiscoItemsResult( stanza, (*it).second.context );
              break;
           }
        break;

        case StanzaIqError:
          (*it).second.dh->handleDiscoError( stanza, (*it).second.context );
          break;

        default:
          break;
      }

      m_track.erase( it );
    }

    return false;
  }

  void Disco::addFeature( const std::string& feature )
  {
    m_features.push_back( feature );
  }

  void Disco::removeFeature( const std::string& feature )
  {
    m_features.remove( feature );
  }

  void Disco::getDiscoInfo( const JID& to, const std::string& node, DiscoHandler *dh, int context,
                            const std::string& tid )
  {
    const std::string& id = tid.empty() ? m_parent->getID() : tid;

    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "id", id );
    iq->addAttribute( "to", to.full() );
    iq->addAttribute( "from", m_parent->jid().full() );
    iq->addAttribute( "type", "get" );
    Tag *q = new Tag( iq, "query" );
    q->addAttribute( "xmlns", XMLNS_DISCO_INFO );
    if( !node.empty() )
      q->addAttribute( "node", node );

    DiscoHandlerContext ct;
    ct.dh = dh;
    ct.context = context;
    m_track[id] = ct;
    m_parent->trackID( this, id, GET_DISCO_INFO );
    m_parent->send( iq );
  }

  void Disco::getDiscoItems( const JID& to, const std::string& node, DiscoHandler *dh, int context,
                             const std::string& tid )
  {
    const std::string& id = tid.empty() ? m_parent->getID() : tid;

    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "id", id );
    iq->addAttribute( "to", to.full() );
    iq->addAttribute( "from", m_parent->jid().full() );
    iq->addAttribute( "type", "get" );
    Tag *q = new Tag( iq, "query" );
    q->addAttribute( "xmlns", XMLNS_DISCO_ITEMS );
    if( !node.empty() )
      q->addAttribute( "node", node );

    DiscoHandlerContext ct;
    ct.dh = dh;
    ct.context = context;
    m_track[id] = ct;
    m_parent->trackID( this, id, GET_DISCO_ITEMS );
    m_parent->send( iq );
  }

  void Disco::setVersion( const std::string& name, const std::string& version, const std::string& os )
  {
    m_versionName = name;
    m_versionVersion = version;
    m_versionOs = os;
  }

  void Disco::setIdentity( const std::string& category, const std::string& type )
  {
    m_identityCategory = category;
    m_identityType = type;
  }

  void Disco::registerDiscoHandler( DiscoHandler *dh )
  {
    m_discoHandlers.push_back( dh );
  }

  void Disco::removeDiscoHandler( DiscoHandler *dh )
  {
    m_discoHandlers.remove( dh );
    DiscoHandlerMap::iterator t;
    DiscoHandlerMap::iterator it = m_track.begin();
    while( it != m_track.end() )
    {
      t = it;
      ++it;
      if( dh == (*t).second.dh )
      {
        m_track.erase( t );
      }
    }
  }

  void Disco::registerNodeHandler( DiscoNodeHandler *nh, const std::string& node )
  {
    m_nodeHandlers[node].push_back( nh );
  }

  void Disco::removeNodeHandler( DiscoNodeHandler *nh, const std::string& node )
  {
    DiscoNodeHandlerMap::iterator it = m_nodeHandlers.find( node );
    if( it != m_nodeHandlers.end() )
    {
      (*it).second.remove( nh );
      if( !(*it).second.size() )
      {
        m_nodeHandlers.erase( it );
      }
    }
  }

}
