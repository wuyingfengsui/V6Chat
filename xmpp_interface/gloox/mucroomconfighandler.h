/*
  Copyright (c) 2006-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef MUCROOMCONFIGHANDLER_H__
#define MUCROOMCONFIGHANDLER_H__

#include "gloox.h"

#include <string>
#include <list>

namespace gloox
{

  class MUCRoom;

  /**
   * An item in a list of MUC room users. Lists of these items are
   * used when manipulating the lists of members, admins, owners, etc.
   * of a room.
   */
  struct MUCListItem
  {
    JID *jid;                       /**< Pointer to the occupant's JID if available, 0 otherwise. */
    std::string nick;               /**< The occupant's nick in the room. */
    MUCRoomAffiliation affiliation; /**< The occupant's affiliation. */
    MUCRoomRole role;               /**< The occupant's role. */
    std::string reason;             /**< Use this only when **setting** the item's role/affiliation to
                                     * specify a reason for the role/affiliation change. This field is
                                     * empty in items fetched from the MUC service. */
  };

  /**
   * A list of MUCListItems.
   */
  typedef std::list<MUCListItem> MUCListItemList;

  /**
   * Available operations on a room.
   */
  enum MUCOperation
  {
    RequestUniqueName,              /**< Request a unique room name. */
    CreateInstantRoom,              /**< Create an instant room. */
    CancelRoomCreation,             /**< Cancel room creation process. */
    RequestRoomConfig,              /**< Request room configuration form. */
    SendRoomConfig,                 /**< Send room configuration */
    DestroyRoom,                    /**< Destroy room. */
    GetRoomInfo,                    /**< Fetch room info. */
    GetRoomItems,                   /**< Fetch room items (e.g., current occupants). */
    SetRNone,                       /**< Set a user's role to None. */
    SetVisitor,                     /**< Set a user's role to Visitor. */
    SetParticipant,                 /**< Set a user's role to Participant. */
    SetModerator,                   /**< Set a suer's role to Moderator. */
    SetANone,                       /**< Set a user's affiliation to None. */
    SetOutcast,                     /**< Set a user's affiliation to Outcast. */
    SetMember,                      /**< Set a user's affiliation to Member. */
    SetAdmin,                       /**< Set a user's affiliation to Admin. */
    SetOwner,                       /**< Set a user's affiliation to Owner. */
    RequestVoiceList,               /**< Request the room's Voice List. */
    StoreVoiceList,                 /**< Store the room's Voice List. */
    RequestBanList,                 /**< Request the room's Ban List. */
    StoreBanList,                   /**< Store the room's Ban List. */
    RequestMemberList,              /**< Request the room's Member List. */
    StoreMemberList,                /**< Store the room's Member List. */
    RequestModeratorList,           /**< Request the room's Moderator List. */
    StoreModeratorList,             /**< Store the room's Moderator List. */
    RequestOwnerList,               /**< Request the room's Owner List. */
    StoreOwnerList,                 /**< Store the room's Owner List. */
    RequestAdminList,               /**< Request the room's Admin List. */
    StoreAdminList                  /**< Store the room's Admin List. */
  };

  /**
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API MUCRoomConfigHandler
  {
    public:
      /**
       * Virtual Destructor.
       */
      virtual ~MUCRoomConfigHandler() {}

      /**
       * This function is called in response to MUCRoom::requestList() if the list was
       * fetched successfully.
       * @param room The room for which the list arrived.
       * @param items The requestd list's items.
       * @param operation The type of the list.
       */
      virtual void handleMUCConfigList( MUCRoom *room, const MUCListItemList& items,
                                        MUCOperation operation ) = 0;

      /**
       * This function is called when the room's configuration form arrives. This usually happens
       * after a call to MUCRoom::requestRoomConfig(). Use
       * MUCRoom::setRoomConfig() to send the configuration back to the
       * room.
       * @param room The room for which the config form arrived.
       * @param form The configuration form.
       */
      virtual void handleMUCConfigForm( MUCRoom *room, const DataForm& form ) = 0;

      /**
       * This function is called in response to MUCRoom::kick(), MUCRoom::storeList(),
       * MUCRoom::ban(), and others, to indcate the end of the operation.
       * @param room The room for which the operation ended.
       * @param success Whether or not the operation was successful.
       * @param operation The finished operation.
       */
      virtual void handleMUCConfigResult( MUCRoom *room, bool success, MUCOperation operation ) = 0;

      /**
       * This function is called when a Voice request or a Registration request arrive through
       * the room that need to be approved/rejected by the room admin. Use MUCRoom::createDataForm()
       * to have a Tag created that answers the request.
       * @param room The room the request arrived from.
       * @param form A DataForm containing the request.
       */
      virtual void handleMUCRequest( MUCRoom *room, const DataForm& form ) = 0;

  };

}

#endif // MUCROOMCONFIGHANDLER_H__
