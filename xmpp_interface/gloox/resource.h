/*
  Copyright (c) 2004-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef RESOURCE_H__
#define RESOURCE_H__

#include <string>

namespace gloox
{

  /**
    * @brief Holds resource attributes.
    *
    * This holds the information of a single resource of a contact that is online.
    *
    * @author Jakob Schroeter <js@caaya.net>
    * @since 0.8
    */
  class GLOOX_API Resource
  {

    friend class RosterItem;

    public:
      /**
        * Constructor.
        * @param priority The resource's priority.
        * @param msg The resource's status message.
        * @param presence The resource's presence status.
        */
      Resource( int priority, const std::string& msg, Presence presence )
        : m_priority( priority ), m_message( msg ), m_presence( presence ) {}

      /**
        * Virtual destrcutor.
        */
      virtual ~Resource() {}

      /**
        * Lets you fetch the resource's priority.
        * @return The resource's priority.
        */
      int priority() const { return m_priority; }

      /**
        * Lets you fetch the resource's status message.
        * @return The resource's status message.
        */
      const std::string& message() const { return m_message; }

      /**
        * Lets you fetch the resource's presence status.
        * @return The resource's presence status.
        */
      Presence presence() const { return m_presence; }

    private:
      void setPriority( int priority ) { m_priority = priority; }
      void setMessage( std::string message ) { m_message = message; }
      void setStatus( Presence presence ) { m_presence = presence; }

      int m_priority;
      std::string m_message;
      std::string m_name;
      Presence m_presence;

  };

}

#endif // RESOURCE_H__
