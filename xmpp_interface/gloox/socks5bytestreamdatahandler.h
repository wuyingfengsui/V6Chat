/*
  Copyright (c) 2006-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef SOCKS5BYTESTREAMDATAHANDLER_H__
#define SOCKS5BYTESTREAMDATAHANDLER_H__

#include "macros.h"

#include <string>

namespace gloox
{

  class SOCKS5Bytestream;
  class Stanza;

  /**
   * @brief A virtual interface that allows implementors to receive data
   * sent over a SOCKS5 Bytestream as defined in XEP-0066. You'll also need it for
   * sending of data.
   *
   * An SOCKS5BytestreamDataHandler is registered with a SOCKS5Bytestream.
   *
   * See SOCKS5BytestreamManager for a detailed description on how to implement SOCKS5 Bytestreams.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API SOCKS5BytestreamDataHandler
  {
    public:
      /**
       * Virtual destructor.
       */
      virtual ~SOCKS5BytestreamDataHandler() {}

      /**
       * Reimplement this function to receive data which is sent over the bytestream.
       * The data received here is (probably) only a single chunk of the complete data (depending
       * on the amount of data you want to send).
       * @param s5b The bytestream.
       * @param data The actual stream payload.
       */
      virtual void handleSOCKS5Data( SOCKS5Bytestream* s5b, const std::string& data ) = 0;

      /**
       * Notifies about an error occuring while using a bytestream.
       * When this handler is called the stream has already been closed.
       * @param s5b The bytestream.
       * @param stanza The error stanza.
       */
      virtual void handleSOCKS5Error( SOCKS5Bytestream* s5b, Stanza* stanza ) = 0;

      /**
       * Notifies the handler that the given bytestream has been acknowledged
       * and is ready to send/receive data.
       * @param s5b The opened bytestream.
       */
      virtual void handleSOCKS5Open( SOCKS5Bytestream* s5b ) = 0;

      /**
       * Notifies the handler that the given bytestream has been closed.
       * @param s5b The closed bytestream.
       */
      virtual void handleSOCKS5Close( SOCKS5Bytestream* s5b ) = 0;

  };

}

#endif // SOCKS5BYTESTREAMDATAHANDLER_H__
