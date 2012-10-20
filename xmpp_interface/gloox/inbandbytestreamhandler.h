/*
  Copyright (c) 2006-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef INBANDBYTESTREAMHANDLER_H__
#define INBANDBYTESTREAMHANDLER_H__

#include "macros.h"
#include "jid.h"
#include "inbandbytestream.h"

namespace gloox
{

  /**
   * @brief A virtual interface that allows to receive new incoming In-Band Bytestream requests
   * from remote entities.
   *
   * See InBandBytestreamManager for a detailed description on how to implement In-Band Bytestreams.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.8
   */
  class GLOOX_API InBandBytestreamHandler
  {
    public:
      /**
       * Virtual destructor.
       */
      virtual ~InBandBytestreamHandler() {}

      /**
       * Notifies the implementor of a new incoming IBB request.
       * Attach the IBB to a MessageSession using InBandBytestream::attachTo().<br>
       * For @b synchronous InBandBytestreamHandler:<br>
       * If the return value is @b true the bytestream holds as accepted and the
       * InBandBytestreamHandler becomes the owner of the InBandBytestream object. If
       * @b false is returned, the InBandBytestream is deleted by the InBandBytestreamManager
       * and the bytestream request will be declined.<br>
       * For @b asynchronous InBandBytestreamHandler:<br>
       * The return value is ignored. You have to call either
       * InBandBytestreamManager::acceptInBandBytestream() or
       * InBandBytestreamManager::rejectInBandBytestream(), respectively.
       * @param from The remote initiator of the bytestream request.
       * @param ibb The bytestream.
       * @return @b True to accept the byte stream, @b false to reject.
       * @note You should @b not send any data over this bytestream from within this function.
       * The bytestream will only be accepted after this function returned.
       */
      virtual bool handleIncomingInBandBytestream( const JID& from, InBandBytestream *ibb ) = 0;

      /**
       * Notifies the implementor of successful establishing of an outgoing IBB request.
       * Attach the IBB to a MessageSession using InBandBytestream::attachTo().
       * The stream has been accepted by the remote entity and is ready to send data.
       * The InBandBytestreamHandler becomes the owner of the InBandBytestream object.
       * @param to The remote entity's JID.
       * @param ibb The new bytestream.
       */
      virtual void handleOutgoingInBandBytestream( const JID& to, InBandBytestream *ibb ) = 0;

      /**
       * Notifies the handler of errors occuring when a bytestream was requested.
       * For example, if the remote entity does not implement IBB.
       * @param remote The remote entity's JID which relates to the error.
       * @param se The error.
       */
      virtual void handleInBandBytestreamError( const JID& remote, StanzaError se ) = 0;

  };

}

#endif // INBANDBYTESTREAMHANDLER_H__
