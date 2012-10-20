/*
  Copyright (c) 2006-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef MUCINVITATIONHANDLER_H__
#define MUCINVITATIONHANDLER_H__

#include "macros.h"
#include "jid.h"

#include <string>

namespace gloox
{

  /**
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API MUCInvitationHandler
  {
    public:
      /**
       * Virtual Destructor.
       */
      virtual ~MUCInvitationHandler() {}

      /**
       *
       */
      virtual void handleMUCInvitation( const JID& room, const JID& invitee, const std::string& reason,
                                        const std::string& body, const std::string& password,
                                        bool cont ) = 0;

  };

}

#endif // MUCINVITATIONHANDLER_H__
