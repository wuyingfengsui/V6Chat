/*
  Copyright (c) 2006-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef MUCROOM_H__
#define MUCROOM_H__

#include "discohandler.h"
#include "disconodehandler.h"
#include "presencehandler.h"
#include "iqhandler.h"
#include "messagehandler.h"
#include "mucroomhandler.h"
#include "mucroomconfighandler.h"
#include "jid.h"

#include <string>

namespace gloox
{

  class ClientBase;
  class MUCMessageSession;
  class Stanza;

  /**
   * @brief This is an implementation of XEP-0045 (Multi-User Chat).
   *
   * Usage is pretty simple:
   *
   * Derrive an object from MUCRoomHandler and implement its virtuals:
   * @code
   * class MyClass : public MUCRoomHandler
   * {
   *   ...
   * };
   * @endcode
   *
   * Then create a new MUCRoom object and pass it a valid ClientBase, the desired full room JID,
   * your MUCRoomHandler-derived object, and an optional MUCRoomConfigHandler-derived object.
   * @code
   * void MyOtherClass::joinRoom( const std::string& room, const std::string& service,
   *                              const std::string& nick )
   * {
   *   MyClass *myHandler = new MyClass(...);
   *   JID roomJID( room + "@" + service + "/" + nick );
   *   m_room = new MUCRoom( m_clientbase, roomJID, myHandler, 0 );
   *   m_room->join();
   * }
   * @endcode
   *
   * When joining the room was successful, the various MUCRoomHandler functions will start to
   * be called. If joining was not successful, MUCRoomHandler::handleMUCError() will be called,
   * giving a hint at the reason for the failure.
   *
   * To set up your own room, or to configure an existing room, you should also derive a
   * class from MUCRoomConfigHandler and register it with the MUCRoom (either by using it
   * with MUCRoom's constructor, or by calling registerMUCRoomConfigHandler()).
   *
   * To quickly create an instant room, see InstantMUCRoom.
   *
   * To quickly create an instant room to turn a one-to-one chat into a multi-user chat,
   * see UniqueMUCRoom.
   *
   * To send a private message to a room participant, use
   * @link MessageSession gloox::MessageSession @endlink with the participant's full room JID
   * (room\@service/nick).
   *
   * XEP version: 1.21
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API MUCRoom : private DiscoHandler, private PresenceHandler,
                            public IqHandler, private MessageHandler, private DiscoNodeHandler
  {
    public:
      /**
       * Allowable history request types. To disable sending of history, use any value except
       * HistoryUnknown and specify a zero-length time span (using setRequestHistory()).
       */
      enum HistoryRequestType
      {
        HistoryUnknown,             /**< It is up to the service to decide how much history to send.
                                     * This is the default. */
        HistoryMaxChars,            /**< Limit the total number of characters in the history to "X"
                                     * (where the character count is the characters of the complete
                                     * XML stanzas, not only their XML character data). */
        HistoryMaxStanzas,          /**< Limit the total number of messages in the history to "X". */
        HistorySeconds,             /**< Send only the messages received in the last "X" seconds. */
        HistorySince                /**< Send only the messages received since the datetime specified
                                     * (which MUST conform to the DateTime profile specified in Jabber
                                     * Date and Time Profiles (XEP-0082)). */
      };

      /**
       * Creates a new abstraction of a Multi-User Chat room. The room is not joined automatically.
       * Use join() to join the room, use leave() to leave it.
       * @param parent The ClientBase object to use for the communication.
       * @param nick The room's name and service plus the desired nickname in the form
       * room\@service/nick.
       * @param mrh The MUCRoomHandler that will listen to room events. May be 0 and may be specified
       * later using registerMUCRoomHandler(). However, without one, MUC is no joy.
       * @param mrch The MUCRoomConfigHandler that will listen to room config result. Defaults to 0
       * initially. However, at the latest you need one when you create a new room which is not an
       * instant room. You can set a MUCRoomConfigHandler using registerMUCRoomConfigHandler().
       */
      MUCRoom( ClientBase *parent, const JID& nick, MUCRoomHandler *mrh, MUCRoomConfigHandler *mrch = 0 );

      /**
       * Virtual Destructor.
       */
      virtual ~MUCRoom();

      /**
       * Use this function to set a password to use when joining a (password protected)
       * room.
       * @param password The password to use for this room.
       * @note This function does not password-protect a room.
       */
      void setPassword( const std::string& password ) { m_password = password; }

      /**
       * A convenience function that returns the room's name.
       * @return The room's name.
       */
      const std::string name() const { return m_nick.username(); }

      /**
       * A convenience function that returns the name/address of the MUC service the room is running on
       * (e.g., conference.jabber.org).
       * @return The MUC service's name/address.
       */
      const std::string service() const { return m_nick.server(); }

      /**
       * A convenience function that returns the user's nickname in the room.
       * @return The user's nickname.
       */
      const std::string nick() const { return m_nick.resource(); }

      /**
       * Join this room.
       */
      virtual void join();

      /**
       * Leave this room.
       * @param msg An optional msg indicating the reason for leaving the room. Default: empty.
       */
      void leave( const std::string& msg = "" );

      /**
       * Sends a chat message to the room.
       * @param message The message to send.
       */
      void send( const std::string& message );

      /**
       * Sets the subject of the room to the given string.
       * The MUC service may decline the request to set a new subject. You should
       * not assume the subject was set successfully util it is acknowledged via the MUCRoomHandler.
       * @param subject The new subject.
       */
      void setSubject( const std::string& subject );

      /**
       * Returns the user's current affiliation with this room.
       * @return The user's current affiliation.
       */
      MUCRoomAffiliation affiliation() const { return m_affiliation; }

      /**
       * Returns the user's current role in this room.
       * @return The user's current role.
       */
      MUCRoomRole role() const { return m_role; }

      /**
       * Use this function to change the user's nickname in the room.
       * The MUC service may decline the request to set a new nickname. You should not assume
       * the nick change was successful until it is acknowledged via the MUCRoomHandler.
       * @param nick The user's new nickname.
       */
      void setNick( const std::string& nick );

      /**
       * Use this function to set the user's presence in this room. It is not possible to
       * use PresenceUnavailable with this function.
       * @param presence The user's new presence.
       * @param msg An optional status message. Default: empty.
       */
      void setPresence( Presence presence, const std::string& msg = "" );

      /**
       * Use this function to invite another user to this room.
       * @param invitee The (bare) JID of the user to invite.
       * @param reason The user-supplied reason for the invitation.
       * @param cont Whether this invitation is part of a transformation of a
       * one-to-one chat to a MUC. Default: false.
       */
      void invite( const JID& invitee, const std::string& reason, bool cont = false );

      /**
       * Use this function to request basic room info, possibly prior to joining it.
       * Results are announced using the MUCRoomHandler.
       */
      void getRoomInfo();

      /**
       * Use this function to request information about the current room occupants,
       * possibly prior to joining it. The room ay be configured not to disclose such
       * information.
       * Results are announced using the MUCRoomHandler.
       */
      void getRoomItems();

      /**
       * The MUC spec enables other entities to discover via Service Discovery which rooms
       * an entity is in. By default, gloox does not publish such info for privacy reasons.
       * This function can be used to enable publishing the info for @b this room.
       * @param publish Whether to enable other entities to discover the user's presence in
       * @b this room.
       * @param publishNick Whether to publish the nickname used in the room. This parameter
       * is ignored if @c publish is @b false.
       */
      void setPublish( bool publish, bool publishNick );

      /**
       * Use this function to register a (new) MUCRoomHandler with this room. There can be only one
       * MUCRoomHandler per room at any one time.
       * @param mrl The MUCRoomHandler to register.
       */
      void registerMUCRoomHandler( MUCRoomHandler *mrl ) { m_roomHandler = mrl; }

      /**
       * Use this function to remove the registered MUCRoomHandler.
       */
      void removeMUCRoomHandler() { m_roomHandler = 0; }

      /**
       * Use this function to register a (new) MUCRoomConfigHandler with this room. There can
       * be only one MUCRoomConfigHandler per room at any one time.
       * @param mrch The MUCRoomConfigHandler to register.
       */
      void registerMUCRoomConfigHandler( MUCRoomConfigHandler *mrch ) { m_roomConfigHandler = mrch; }

      /**
       * Use this function to remove the registered MUCRoomConfigHandler.
       */
      void removeMUCRoomConfigHandler() { m_roomConfigHandler = 0; }

      /**
       * Use this function to add history to a (newly created) room. The use case from the MUC spec
       * is to add history to a room that was created in the process of a transformation of a
       * one-to-one chat to a multi-user chat.
       * @param message A reason for declining the invitation.
       * @param from The JID of the original author of this part of the history.
       * @param stamp The datetime of the original message in the format: 20061224T12:15:23Z
       * @note You should not attempt to use this function before
       * MUCRoomHandler::handleMUCParticipantPresence() was called for the first time.
       */
      void addHistory( const std::string& message, const JID& from, const std::string& stamp );

      /**
       * Use this function to request room history. Set @c value to zero to disable the room
       * history request. You should not use HistorySince type with this function.
       * History is sent only once after entering a room. You should use this function before joining.
       * @param value Represents either the number of requested characters, the number of requested
       * message stanzas, or the number seconds, depending on the value of @c type.
       * @param type
       * @note If this function is not used to request a specific amount of room history, it is up
       * to the MUC service to decide how much history to send.
       */
      void setRequestHistory( int value, HistoryRequestType type );

      /**
       * Use this function to request room history since specific datetime.
       * History is sent only once after entering a room. You should use this function before joining.
       * @param since A string representing a datetime conforming to the DateTime profile specified
       * in Jabber Date and Time Profiles (XEP-0082).
       * @note If this function is not used to request a specific amount of room history, it is up
       * to the MUC service to decide how much history to send.
       */
      void setRequestHistory( const std::string& since );

      /**
       * This static function allows to formally decline a MUC invitation received via the
       * MUCInvitationListener.
       * @param room The JID of the room the invitation came from.
       * @param invitor The JID of the invitor.
       * @param reason An optional reason for the decline.
       */
      static Stanza* declineInvitation( const JID& room, const JID& invitor,
                                        const std::string& reason = "");

      /**
       * It is not possible for a visitor to speak in a moderated room. Use this function to request
       * voice from the moderator.
       */
      void requestVoice();

      /**
       * Use this function to kick a user from the room.
       * Depending on service and/or room configuration and role/affiliation
       * this may not always succeed. Usually, a role of 'moderator' is necessary.
       * @note This is a convenience function. It directly uses setRole() with a MUCRoomRole of RoleNone.
       * @param nick The nick of the user to be kicked.
       * @param reason An optional reason for the kick.
       */
      void kick( const std::string& nick, const std::string& reason = "" );

      /**
       * Use this function to ban a user from the room.
       * Depending on service and/or room configuration and role/affiliation
       * this may not always succeed. Usually, an affiliation of admin is necessary.
       * @note This is a convenience function. It directly uses setAffiliation() with a MUCRoomAffiliation
       * of RoleOutcast.
       * @param nick The nick of the user to be banned.
       * @param reason An optional reason for the ban.
       */
      void ban( const std::string& nick, const std::string& reason );

      /**
       * Use this function to grant voice to a user in a moderated room.
       * Depending on service and/or room configuration and role/affiliation
       * this may not always succeed. Usually, a role of 'moderator' is necessary.
       * @note This is a convenience function. It directly uses setRole() with a MUCRoomRole
       * of RoleParticipant.
       * @param nick The nick of the user to be granted voice.
       * @param reason An optional reason for the grant.
       */
      void grantVoice( const std::string& nick, const std::string& reason );

      /**
       * Use this function to create a Tag that approves a voice request or registration request
       * delivered via MUCRoomConfigHandler::handleMUCVoiceRequest(). You will need to send this
       * Tag off manually using Client/ClientBase.
       * @param room The room's JID. This is needed because you can use this function outside of
       * room context (e.g, if the admin is not in the room).
       * @param df The filled-in DataForm from the voice/registration request.
       */
      static Stanza* createDataForm( const JID& room, const DataForm& df );

      /**
       * Use this function to revoke voice from a user in a moderated room.
       * Depending on service and/or room configuration and role/affiliation
       * this may not always succeed. Usually, a role of 'moderator' is necessary.
       * @note This is a convenience function. It directly uses setRole() with a MUCRoomRole
       * of RoleVisitor.
       * @param nick The nick of the user.
       * @param reason An optional reason for the revoke.
       */
      void revokeVoice( const std::string& nick, const std::string& reason );

      /**
       * Use this function to change the role of a user in the room.
       * Usually, at least moderator privileges are required to succeed.
       * @param nick The nick of the user who's role shall be modfified.
       * @param role The user's new role in the room.
       * @param reason An optional reason for the role change.
       */
      void setRole( const std::string& nick, MUCRoomRole role, const std::string& reason = "" );

      /**
       * Use this function to change the affiliation of a user in the room.
       * Usually, at least admin privileges are required to succeed.
       * @param nick The nick of the user who's affiliation shall be modfified.
       * @param affiliation The user's new affiliation in the room.
       * @param reason An optional reason for the affiliation change.
       */
      void setAffiliation( const std::string& nick, MUCRoomAffiliation affiliation,
                           const std::string& reason );

      /**
       * Use this function to request the room's configuration form.
       * It can be used either after MUCRoomHandler::handleMUCRoomCreation() was called,
       * or at any later time.
       *
       * Usually owner privileges are required for this action to succeed.
       */
      void requestRoomConfig();

      /**
       * After requesting (using requestRoomConfig()) and
       * editing/filling in the room's configuration,
       * use this function to send it back to the server.
       * @param form The form to send. The function will delete the
       * object pointed to.
       */
      void setRoomConfig( DataForm* form );

      /**
       * Use this function to accept the room's default configuration. This function is useful
       * only after MUCRoomHandler::handleMUCRoomCreation() was called. This is a NOOP at
       * any other time.
       */
      void acknowledgeInstantRoom();

      /**
       * Use this function to cancel the creation of a room. This function is useful only after
       * MUCRoomHandler::handleMUCRoomCreation() was called. This is a NOOP at any other time.
       */
      void cancelRoomCreation();

      /**
       * Use this function to destroy the room. All the occupants will be removed from the room.
       * @param reason An optional reason for the destruction.
       * @param alternate A pointer to a JID of an alternate venue (e.g., another MUC room).
       * May be 0.
       * @param password An optional password for the alternate venue.
       *
       * Usually owner privileges are required for this action to succeed.
       */
      void destroy( const std::string& reason = "",
                    const JID* alternate = 0, const std::string& password = "" );

      /**
       * Use this function to request a particluar list of room occupants.
       * @note There must be a MUCRoomConfigHandler registered with this room for this
       * function to be executed.
       * @param operation The following types of lists are available:
       * @li Voice List: List of people having voice in a moderated room. Use RequestVoiceList.
       * @li Members List: List of members of a room. Use RequestMemberList.
       * @li Ban List: List of people banned from the room. Use RequestBanList.
       * @li Moderator List: List of room moderators. Use RequestModeratorList.
       * @li Admin List: List of room admins. Use RequestAdminList.
       * @li Owner List: List of room owners. Use RequestOwnerList.
       * Any other value of @c operation will be ignored.
       */
      void requestList( MUCOperation operation );

      /**
       * Use this function to store a (modified) list for the room.
       * @param items The list of items. Example:<br>
       * You want to set the Voice List. The privilege of Voice refers to the role of Participant.
       * Furthermore, you only store the delta of the original (Voice)List. (Optionally, you could
       * probably store the whole list, however, remeber to include those items that were modified,
       * too.)
       * You want to, say, add one occupant to the Voice List, and remove another one.
       * Therefore you store:
       * @li GuyOne, role participant -- this guy gets voice granted, he/she is now a participant.
       * @li GuyTwo, role visitor -- this guy gets voice revoked, he/she is now a mere visitor
       * (Visitor is the Role "below" Participant in the privileges hierarchy).
       *
       * For operations modifying Roles, you should specifiy only the new Role in the MUCListItem
       * structure, for those modifying Affiliations, you should only specify the new Affiliation,
       * respectively. The nickname is mandatory in the MUCListItem structure. Items without nickname
       * will be ignored.
       *
       * You may specify a reason for the role/affiliation change in the MUCListItem structure.
       * You should not specify a JID in the MUCListItem structure, it will be ignored.
       *
       * @param operation See requestList() for a list of available list types. Any other value will
       * be ignored.
       */
      void storeList( const MUCListItemList items, MUCOperation operation );

      /**
       * Returns the currently know room flags.
       * @return ORed MUCRoomFlag's describing the current room configuration.
       */
      int flags() const { return m_flags; }

      // reimplemented from DiscoHandler
      virtual void handleDiscoInfoResult( Stanza *stanza, int context );

      // reimplemented from DiscoHandler
      virtual void handleDiscoItemsResult( Stanza *stanza, int context );

      // reimplemented from DiscoHandler
      virtual void handleDiscoError( Stanza *stanza, int context );

      // reimplemented from PresenceHandler
      virtual void handlePresence( Stanza *stanza );

      // reimplemented from MessageHandler
      virtual void handleMessage( Stanza *stanza, MessageSession *session = 0 );

      // reimplemented from IqHandler
      virtual bool handleIq( Stanza* /*stanza*/ ) { return false; }

      // reimplemented from IqHandler
      virtual bool handleIqID( Stanza *stanza, int context );

      // reimplemented from DiscoNodeHandler
      virtual StringList handleDiscoNodeFeatures( const std::string& node );

      // reimplemented from DiscoNodeHandler
      virtual StringMap handleDiscoNodeIdentities( const std::string& node, std::string& name );

      // reimplemented from DiscoNodeHandler
      virtual DiscoNodeItemList handleDiscoNodeItems( const std::string& node = "" );

    protected:
      void setName( const std::string& name ) { m_nick.setUsername( name ); }
      virtual bool instantRoomHook() const { return false; }

      ClientBase *m_parent;
      JID m_nick;

      bool m_joined;

    private:
      bool handleIqResult( Stanza *stanza, int context );
      bool handleIqError( Stanza *stanza, int context );
      void setNonAnonymous();
      void setSemiAnonymous();
      void setFullyAnonymous();
      void modifyOccupant( const std::string& nick, int state, const std::string roa,
                           const std::string& reason );
      void acknowledgeRoomCreation();
      MUCRoomAffiliation getEnumAffiliation( const std::string& affiliation );
      MUCRoomRole getEnumRole( const std::string& role );

      MUCRoomHandler *m_roomHandler;
      MUCRoomConfigHandler *m_roomConfigHandler;
      MUCMessageSession *m_session;

      typedef std::list<MUCRoomParticipant> ParticipantList;
      ParticipantList m_participants;

      std::string m_password;
      std::string m_newNick;

      MUCRoomAffiliation m_affiliation;
      MUCRoomRole m_role;

      HistoryRequestType m_historyType;

      std::string m_historySince;
      int m_historyValue;
      int m_flags;
      bool m_creationInProgress;
      bool m_configChanged;
      bool m_publishNick;
      bool m_publish;
      bool m_unique;

  };

}

#endif // MUCROOM_H__
