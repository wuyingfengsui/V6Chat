/*
  Copyright (c) 2007-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef UNIQUEMUCROOM_H__
#define UNIQUEMUCROOM_H__

#include "instantmucroom.h"

namespace gloox
{

  /**
   * @brief This class implements a unique MUC room.
   *
   * A unique MUC room is a room with a non-human-readable name. It is primarily intended
   * to be used when converting one-to-one chats to multi-user chats.
   *
   * XEP version: 1.21
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API UniqueMUCRoom : public InstantMUCRoom
  {
    public:
      /**
       * Creates a new abstraction of a @b unique Multi-User Chat room. The room is not joined
       * automatically. Use join() to join the room, use leave() to leave it. See MUCRoom for
       * detailed info.
       * @param parent The ClientBase object to use for the communication.
       * @param nick The room's name and service plus the desired nickname in the form
       * @b service/nick.
       * @param mrh The MUCRoomHandler that will listen to room events. May be 0 and may be specified
       * later using registerMUCRoomHandler(). However, without one, MUC is no joy.
       * @note To subsequently configure the room, use MUCRoom::registerMUCRoomConfigHandler().
       */
      UniqueMUCRoom( ClientBase *parent, const JID& nick, MUCRoomHandler *mrh );

      /**
       * Virtual Destructor.
       */
      virtual ~UniqueMUCRoom();

      // reimplemented from MUCRoom
      virtual void join();

    private:
      // reimplemented from MUCRoom (IqHandler)
      bool handleIqID( Stanza *stanza, int context );

  };

}

#endif // UNIQUEMUCROOM_H__
