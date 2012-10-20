/*
  Copyright (c) 2004-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef DISCONODEHANDLER_H__
#define DISCONODEHANDLER_H__

#include "gloox.h"

#include <list>
#include <map>
#include <string>

namespace gloox
{

  /**
   * A struct describing an item of a Service Discovery Item.
   */
  struct DiscoNodeItem
  {
    DiscoNodeItem() {}
    DiscoNodeItem( const std::string& _node,
                   const std::string& _jid,
                   const std::string& _name )
      : node( _node ), jid( _jid ), name( _name ) {}
    std::string node;               /**< Content of the item's node attribute. */
    std::string jid;                /**< Content of the item's jid attribute. */
    std::string name;               /**< Content of the item's name attribute. */
  };

  /**
   * A list of DiscoNodeItems.
   */
  typedef std::list<DiscoNodeItem> DiscoNodeItemList;

  /**
   * @brief Derived classes can be registered as NodeHandlers for certain nodes with the Disco object.
   *
   * Incoming disco#info and disco#items queries are delegated to their respective handlers.
   *
   * @author Jakob Schroeter <js@camaya.net>
   */
  class GLOOX_API DiscoNodeHandler
  {
    public:
      /**
       * Virtual Destructor.
       */
      virtual ~DiscoNodeHandler() {}

      /**
       * In addition to @c handleDiscoNodeIdentities, this function is used to gather
       * more information on a specific node. It is called when a disco#info query
       * arrives with a node attribute that matches the one registered for this handler.
       * @param node The node this handler is supposed to handle.
       * @return A list of features supported by this node.
       */
      virtual StringList handleDiscoNodeFeatures( const std::string& node ) = 0;

      /**
       * In addition to @c handleDiscoNodeFeatures, this function is used to gather
       * more information on a specific node. It is called when a disco#info query
       * arrives with a node attribute that matches the one registered for this handler.
       * @param node The node this handler is supposed to handle.
       * @param name This parameter is currently used as additional return value.  Just fill in the
       * name of the node.
       * @return A map of identities for the given node. The first string is the
       * category specifier, the second string is the type specifier.
       */
      virtual StringMap handleDiscoNodeIdentities( const std::string& node, std::string& name ) = 0;

      /**
       * This function is used to gather more information on a specific node.
       * It is called when a disco#items query arrives with a node attribute that
       * matches the one registered for this handler. If node is empty, items for the
       * root node (no node) shall be returned.
       * @param node The node this handler is supposed to handle.
       * @return A list of items supported by this node.
       */
      virtual DiscoNodeItemList handleDiscoNodeItems( const std::string& node = "" ) = 0;

  };

}

#endif // DISCONODEHANDLER_H__
