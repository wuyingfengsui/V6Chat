/*
  Copyright (c) 2007-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef SIHANDLER_H__
#define SIHANDLER_H__

#include <string>

namespace gloox
{

  class Stanza;
  class Tag;
  class JID;

  /**
   * @brief An abstract base class to handle results of outgoing SI requests, i.e. you requested a stream
   * (using SIManager::requestSI()) to send a file to a remote entity.
   *
   * You should usually not need to use this class directly, unless your profile is not supported
   * by gloox.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API SIHandler
  {

    public:
      /**
       * Virtual destructor.
       */
      virtual ~SIHandler() {}

      /**
       * This function is called to handle results of outgoing SI requests, i.e. you requested a stream
       * (using SIManager::requestSI()) to send a file to a remote entity.
       * @param from The SI receiver.
       * @param sid The stream ID.
       * @param si The request's complete &lt;si/&gt; Tag.
       * @param ptag The profile-specific child of the SI request. May be 0.
       * @param fneg The &lt;feature/&gt; child of the SI request. May be 0 (but should not be).
       */
      virtual void handleSIRequestResult( const JID& from, const std::string& sid,
                                          Tag* si, Tag* ptag, Tag* fneg ) = 0;

      /**
       * This function is called to handle a request error or decline.
       * @param stanza The complete error stanza.
       * @param sid The request's SID.
       */
      virtual void handleSIRequestError( Stanza* stanza, const std::string& sid ) = 0;

  };

}

#endif // SIHANDLER_H__
