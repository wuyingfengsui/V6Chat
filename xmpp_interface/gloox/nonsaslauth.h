/*
  Copyright (c) 2005-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef NONSASLAUTH_H__
#define NONSASLAUTH_H__

#include "iqhandler.h"

#include <string>

namespace gloox
{

  class Client;
  class Stanza;
  class Tag;

  /**
   * @brief This class is an implementation of XEP-0078 (Non-SASL Authentication).
   *
   * It is invoked by @ref Client automatically if supported by the server and if SASL authentication
   * is not supported.
   * You should not need to use this class manually.
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.3
   */
  class GLOOX_API NonSaslAuth : public IqHandler
  {
    public:
      /**
       * Constructor.
       * @param parent The @ref ClientBase which is used to authenticate.
       */
      NonSaslAuth( Client *parent );

      /**
       * Virtual Destructor.
       */
      virtual ~NonSaslAuth();

      /**
       * Starts authentication by querying the server for the required authentication fields.
       * Digest authentication is preferred over plain text passwords.
       * @param sid The session ID given by the server with the stream opening tag.
       */
      void doAuth( const std::string& sid );

      // reimplemented from IqHandler
      virtual bool handleIq( Stanza *stanza );

      // reimplemented from IqHandler
      virtual bool handleIqID( Stanza *stanza, int context );

    private:
      enum NonSaslAuthTrack
      {
        TRACK_REQUEST_AUTH_FIELDS,
        TRACK_SEND_AUTH
      };

      Client *m_parent;
      std::string m_sid;

  };

}

#endif // NONSASLAUTH_H__
