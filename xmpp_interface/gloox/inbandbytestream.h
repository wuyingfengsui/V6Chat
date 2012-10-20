/*
  Copyright (c) 2006-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef INBANDBYTESTREAM_H__
#define INBANDBYTESTREAM_H__

#include "messagefilter.h"
#include "iqhandler.h"
#include "gloox.h"
#include "inbandbytestreammanager.h"

namespace gloox
{

  class ClientBase;
  class InBandBytestreamDataHandler;

  /**
   * @brief An implementation of a single In-Band Bytestream (XEP-0047).
   *
   * One instance of this class handles one byte stream. You can attach as many InBandBytestream
   * objects to a MessageSession as you like.
   *
   * See InBandBytestreamManager for a detailed description on how to implement In-Band Bytestreams.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.8
   */
  class GLOOX_API InBandBytestream : public MessageFilter
  {
    friend class InBandBytestreamManager;

    public:
      /**
       * Virtual destructor.
       */
      virtual ~InBandBytestream();

      /**
       * Returns whether the bytestream is open, that is, accepted by both parties.
       * @return Whether the bytestream is open or not.
       */
      bool isOpen() const { return m_open; }

      /**
       * Use this function to send a chunk of data over an open byte stream.
       * The negotiated block size is enforced. If the block is larger, nothing is sent
       * and @b false is returned. If the stream is not open or has been closed again
       * (by the remote entity or locally), nothing is sent and @b false is returned.
       * This function does the necessary base64 encoding for you.
       * @param data The block of data to send.
       * @return @b True if the data has been sent (no guarantee of receipt), @b false
       * in case of an error.
       */
      bool sendBlock( const std::string& data );

      /**
       * Lets you retrieve the stream's ID.
       * @return The stream's ID.
       */
      const std::string& sid() const { return m_sid; }

      /**
       * Lets you retrieve this bytestream's block-size.
       * @return The bytestream's block-size.
       */
      int blockSize() const { return (int)m_blockSize; }

      /**
       * Use this function to register an object that will receive any notifications from
       * the InBandBytestream instance. Only one InBandBytestreamDataHandler can be registered
       * at any one time.
       * @param ibbdh The InBandBytestreamDataHandler-derived object to receive notifications.
       */
      void registerInBandBytestreamDataHandler( InBandBytestreamDataHandler *ibbdh );

      /**
       * Removes the registered InBandBytestreamDataHandler.
       */
      void removeInBandBytestreamDataHandler();

      // reimplemented from MessageFilter
      virtual void decorate( Tag *tag );

      // reimplemented from MessageFilter
      virtual void filter( Stanza *stanza );

    private:
      InBandBytestream( MessageSession *session, ClientBase *clientbase );
      void setBlockSize( int blockSize ) { m_blockSize = blockSize; }
      void close();  // locally
      void closed(); // by remote entity
      void setSid( const std::string& sid ) { m_sid = sid; }

      ClientBase *m_clientbase;
      InBandBytestreamManager *m_manager;
      InBandBytestreamDataHandler *m_inbandBytestreamDataHandler;
      std::string m_sid;
      std::string::size_type m_blockSize;
      int m_sequence;
      int m_lastChunkReceived;
      bool m_open;

  };

}

#endif // INBANDBYTESTREAM_H__
