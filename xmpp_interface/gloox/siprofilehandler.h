/*
  Copyright (c) 2007-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef SIPROFILEHANDLER_H__
#define SIPROFILEHANDLER_H__

#include "jid.h"

#include <string>

namespace gloox
{

  class Tag;
  class JID;

  /**
   * @brief An abstract base class to handle SI requests for a specific profile, e.g. file transfer.
   *
   * You should usually not need to use this class directly, unless your profile is not supported
   * by gloox.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API SIProfileHandler
  {

    public:
      /**
       * Virtual destructor.
       */
      virtual ~SIProfileHandler() {}

      /**
       * This function is called to handle incoming SI requests, i.e. a remote entity requested
       * a stream to send a file to you. You should use either SIManager::acceptSI() or
       * SIManager::declineSI() to accept or reject the request, respectively.
       * @param from The SI requestor.
       * @param id The request's id (@b not the stream's id). This id MUST be supplied to either
       * SIManager::acceptSI() or SIManager::declineSI().
       * @param profile The requested stream profile.
       * @param si The request's complete &lt;si/&gt; Tag.
       * @param ptag The profile-specific child of the SI request. May be 0, but should not be.
       * @param fneg The &lt;feature/&gt; child of the SI request. May be 0.
       */
      virtual void handleSIRequest( const JID& from, const std::string& id, const std::string& profile,
                                    Tag* si, Tag* ptag, Tag* fneg ) = 0;

  };

}

#endif // SIPROFILEHANDLER_H__
