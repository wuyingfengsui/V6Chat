/*
  Copyright (c) 2006-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef INBANDBYTESTREAMDATAHANDLER_H__
#define INBANDBYTESTREAMDATAHANDLER_H__

#include "macros.h"

#include <string>

namespace gloox
{

  /**
   * @brief A virtual interface that allows implementors to receive data
   * sent over a In-Band Bytestream as defined in XEP-0047.
   *
   * An InBandBytestreamDataHandler is registered with an InBandBytestream.
   *
   * See InBandBytestreamManager for a detailed description on how to implement In-Band Bytestreams.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.8
   */
  class GLOOX_API InBandBytestreamDataHandler
  {
    public:
      /**
       * Virtual destructor.
       */
      virtual ~InBandBytestreamDataHandler() {}

      /**
       * Reimplement this function to receive data which is sent over the bytestream.
       * The data received here is (probably) only a single chunk of the complete data (depending
       * on the amount of data you want to send).
       * In any case, its size is at maximum equal to the bytestream's negotiated blocksize.
       * @param data The actual stream payload. Not base64 encoded.
       * @param sid The stream's ID.
       */
      virtual void handleInBandData( const std::string& data, const std::string& sid ) = 0;

      /**
       * Notifies about an error occuring while using a bytestream.
       * When this handler is called the stream has already been closed.
       * @param sid The stream's ID.
       * @param remote The remote entity.
       * @param se The error.
       */
      virtual void handleInBandError( const std::string& sid, const JID& remote, StanzaError se ) = 0;

      /**
       * Notifies the handler that the bytestream for the given JID has been closed by
       * the peer.
       * @param sid The closed bytestream's ID.
       * @param from The remote entity's JID which closed the bytestream.
       */
      virtual void handleInBandClose( const std::string& sid, const JID& from ) = 0;

  };

}

#endif // INBANDBYTESTREAMDATAHANDLER_H__
