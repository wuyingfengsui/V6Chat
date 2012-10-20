/*
  Copyright (c) 2004-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef ROSTERITEM_H__
#define ROSTERITEM_H__

#include "gloox.h"
#include "jid.h"
#include "resource.h"

#include <string>
#include <list>


namespace gloox
{

  /**
   * @brief An abstraction of a roster item.
   *
   * For each RosterItem all resources that are available (online in some way) are stored in
   * a ResourceMap. This map is accessible using the resources() function.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.3
   */
  class GLOOX_API RosterItem
  {
    friend class RosterManager;

    public:
      /**
       * A list of resources for the given JID.
       */
      typedef std::map<std::string, Resource*> ResourceMap;

      /**
       * Describes possible subscribtion types according to RFC 3921, Section 9.
       */
      enum SubscriptionEnum
      {
        S10nNone,            /**< Contact and user are not subscribed to each other, and
                               * neither has requested a subscription from the other. */
        S10nNoneOut,         /**< Contact and user are not subscribed to each other, and
                               * user has sent contact a subscription request but contact
                               * has not replied yet. */
        S10nNoneIn,          /**< Contact and user are not subscribed to each other, and
                               * contact has sent user a subscription request but user has
                               * not replied yet (note: contact's server SHOULD NOT push or
                               * deliver roster items in this state, but instead SHOULD wait
                               * until contact has approved subscription request from user). */
        S10nNoneOutIn,       /**< Contact and user are not subscribed to each other, contact
                               * has sent user a subscription request but user has not replied
                               * yet, and user has sent contact a subscription request but
                               * contact has not replied yet. */
        S10nTo,              /**< User is subscribed to contact (one-way). */
        S10nToIn,            /**< User is subscribed to contact, and contact has sent user a
                               * subscription request but user has not replied yet. */
        S10nFrom,            /**< Contact is subscribed to user (one-way). */
        S10nFromOut,         /**< Contact is subscribed to user, and user has sent contact a
                               * subscription request but contact has not replied yet. */
        S10nBoth              /**< User and contact are subscribed to each other (two-way). */
      };

      /**
       * Constructs a new item of the roster.
       * @param jid The JID of the contact.
       * @param name The displayed name of the contact.
       */
      RosterItem( const JID& jid, const std::string& name = "" );

      /**
       * Virtual destructor.
       */
      virtual ~RosterItem();

      /**
       * Sets the displayed name of a contact/roster item.
       * @param name The contact's new name.
       */
      virtual void setName( const std::string& name );

      /**
       * Retrieves the displayed name of a contact/roster item.
       * @return The contact's name.
       */
      virtual const std::string& name() const { return m_name; }

      /**
       * Returns the contact's bare JID.
       * @return The contact's bare JID.
       */
      virtual const std::string& jid() const { return m_jid; }

      /**
       * Returns the current subscription type between the remote and the local entity.
       * @return The subscription type.
       */
      virtual SubscriptionEnum subscription() const { return m_subscription; }

      /**
       * Sets the groups this RosterItem belongs to.
       * @param groups The groups to set for this item.
       */
      virtual void setGroups( const StringList& groups );

      /**
       * Returns the groups this RosterItem belongs to.
       * @return The groups this item belongs to.
       */
      virtual const StringList& groups() const { return m_groups; }

      /**
       * Whether the item has unsynchronized changes.
       * @return @b True if the item has unsynchronized changes, @b false otherwise.
       */
      virtual bool changed() const { return m_changed; }

      /**
       * Indicates whether this item has at least one resource online (in any state).
       * @return @b True if at least one resource is online, @b false otherwise.
       */
      virtual bool online() const;

      /**
       * Returns the contact's resources.
       * @return The contact's resources.
       */
      virtual const ResourceMap& resources() const { return m_resources; }

      /**
       * Returns the Resource for a specific resource string.
       * @param res The resource string.
       * @return The Resource if found, 0 otherwise.
       */
      virtual const Resource* resource( const std::string& res ) const;

    protected:
      /**
       * Sets the current presence of the resource.
       * @param resource The resource to set the presence for.
       * @param presence The current presence.
       */
      virtual void setPresence( const std::string& resource, Presence presence );

      /**
       * Sets the current status message of the resource.
       * @param resource The resource to set the status message for.
       * @param msg The current status message, i.e. from the presence info.
       */
      virtual void setStatus( const std::string& resource, const std::string& msg );

      /**
       * Sets the current priority of the resource.
       * @param resource The resource to set the status message for.
       * @param priority The resource's priority, i.e. from the presence info.
       */
      virtual void setPriority( const std::string& resource, int priority );

      /**
       * Sets the current subscription status of the contact.
       * @param subscription The current subscription.
       * @param ask Whether a subscription request is pending.
       */
      virtual void setSubscription( const std::string& subscription, bool ask );

      /**
       * Removes the 'changed' flag from the item.
       */
      virtual void setSynchronized() { m_changed = false; }

      /**
       * This function is called to remove subsequent resources from a RosterItem.
       * @param resource The resource to remove.
       */
      virtual void removeResource( const std::string& resource );

    private:
      StringList m_groups;
      ResourceMap m_resources;
      SubscriptionEnum m_subscription;
      std::string m_name;
      std::string m_jid;
      bool m_changed;
  };

}

#endif // ROSTERITEM_H__
