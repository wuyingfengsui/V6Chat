/*
  Copyright (c) 2006-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef INBANDBYTESTREAMMANAGER_H__
#define INBANDBYTESTREAMMANAGER_H__

#include "iqhandler.h"
#include "jid.h"

namespace gloox
{

  class InBandBytestreamHandler;
  class InBandBytestream;
  class ClientBase;

  /**
   * @brief An InBandBytestreamManager dispatches In-Band Bytestreams.
   *
   * @section init_ibb Initiating a bytestream
   *
   * To initiate a new bytestream, you need an InBandBytestreamManager object. You will
   * also need an InBandBytestreamHandler-derived object which will receive incoming and
   * outgoing bytestreams (not the data but the InBandBytestream objects).
   * @code
   * class MyClass : public InBandBytestreamHandler
   * {
   *   ...
   *   private:
   *     InBandBytestreamManager *m_ibbManager;
   *   ...
   * };
   * @endcode
   *
   * Create a new InBandBytestreamManager and request a new bytestream:
   * @code
   * MyClass::MyClass()
   * {
   *   m_ibbManager = new InBandBytestreamManager( m_client );
   * }
   *
   * void MyClass::myFunc()
   * {
   *   JID jid( "entity@server/resource" );
   *   m_ibbManager->requestInBandBytestream( jid, this );
   * }
   * @endcode
   *
   * After the bytestream has been negotiated with the peer,
   * InBandBytestreamHandler::handleOutgoingInBandBytestream() is called. Here you should
   * attach the bytestream to a MessageSession associated with the remote entity. This is
   * necessary since message stanzas are used for the actual data exchange and the
   * MessageSession class offers a convenient interface to filter these out.
   * In this example, there is a map of JID/MessageSession pairs and a map of
   * JID/InBandBytestreams.
   * @code
   * void MyClass::handleOutgoingInBandBytestream( const JID& to, InBandBytestream *ibb )
   * {
   *   MessageSessionList::iterator it = m_messageSessions.find( to.full() );
   *   if( it != m_messageSessions.end() )
   *   {
   *     ibb->attachTo( (*it).second );
   *   }
   *   else
   *   {
   *     MessageSession *session = new MessageSession( m_client, to );
   *     ibb->attachTo( session );
   *     m_messageSessions[to.full()] = session;
   *   }
   *
   *   m_ibbs[to.full()] = ibb;
   * }
   * @endcode
   * If you want to receive data from the bytestream (In-Band Bytestreams can be bi-directional),
   * you have to register a InBandBytestreamDataHandler here, similar to the folowing example.
   *
   * When sending data you should make sure you never try to send a block larger than the
   * negotiated blocksize (which defaults to 4096 bytes). If a block is larger than this it will
   * not be sent.
   *
   * @section recv_ibb Receiving a bytestream
   *
   * To receive a bytestream you need a InBandBytestreamManager, too, and you have to
   * register an InBandBytestreamHandler which will receive the incoming bytestreams.
   * @code
   *   m_ibbManager->registerInBandBytestreamHandler( this );
   * @endcode
   *
   * Upon an incoming request the InBandBytestreamManager notifies the registered
   * InBandBytestreamHandler by calling
   * @link InBandBytestreamHandler::handleIncomingInBandBytestream() handleIncomingInBandBytestream() @endlink.
   * The return value of the handler determines whether the stream shall be accepted or not.
   * @code
   * bool MyClass::handleIncomingInBandBytestream( const JID& from, InBandBytestream *ibb )
   * {
   *   // Check whether you want to accept the bytestream
   *
   *   // add an InBandBytestreamDataHandler
   *   ibb->registerInBandBytestreamDataHandler( this );
   *
   *   // The rest of this function probaly looks similar to the implementation of
   *   // handleOutgoingInBandBytestream() above.
   *   // You should *not* start to send blocks of data from within this
   *   // function, though.
   *
   *   // return true to accept the bytestream, false to reject it
   *   return true;
   * }
   * @endcode
   *
   * @section send_ibb Sending data
   *
   * To actually send data, you should utilise some kind of mainloop integration that allows
   * to call a function periodically. It is important to remember the following:
   * @li chunks of data passed to InBandBytestream::sendBlock() may not exceed the negotiated block-size
   * @li neither InBandBytestreamManager nor InBandBytestream will ask for data to send.
   *
   * The following is an example of a primitive mainloop integration:
   * @code
   * void MyClass::mainloop()
   * {
   *   if( m_client->connect(false) )
   *   {
   *     ConnectionError ce = ConnNoError;
   *     while( ce == ConnNoError )
   *     {
   *       ce = m_client->recv();
   *       sendIBBData();
   *     }
   *     printf( "disconnected. reason: %d\n", ce );
   *   }
   * }
   * @endcode
   *
   * In sendIBBData() you would then iterate over your bytestreams and send a block of data
   * where appropriate.
   * @code
   * void MyClass::sendIBBData()
   * {
   *   IBBList::iterator it = m_ibbs.begin();
   *   for( ; it != m_ibbs.end(); ++it )
   *   {
   *     (*it).second->sendBlock( "some data" );
   *   }
   * }
   * @endcode
   *
   * @section del_ibb Getting rid of bytestreams
   *
   * When you're done with a bytestream you can get rid of it by calling the dispose() function. The
   * bytestream will be recycled and you should no longer use it.
   * @code
   *   m_ibbManager->dispose( ibb );
   *   ibb = 0;
   * @endcode
   *
   * @note You should have only one InBandBytestreamManager per Client/ClientBase lying around.
   * One is enough for receiving and initiating bytestreams.
   *
   * @note In the excerpts above, only one bytestream per remote entity is possible (without leaking).
   * However, gloox in general does not impose such a limitation, nor does the In-Band Bytestreams
   * specification.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.8
   * @deprecated Functionality will be merged with SI-based file transfer (SIProfileFT).
   * InBandBytestreamManager will go away in 1.0.
   */
  class GLOOX_API InBandBytestreamManager : public IqHandler
  {
    public:
      /**
       * Constructs a new InBandBytestreamManager.
       * @param parent The ClientBase to use for sending data.
       * @deprecated Functionality will be merged with SI-based file transfer (SIProfileFT).
       * InBandBytestreamManager will go away in 1.0.
       */
      GLOOX_DEPRECATED_CTOR InBandBytestreamManager( ClientBase *parent );

      /**
       * Virtual destructor.
       */
      virtual ~InBandBytestreamManager();

      /**
       * This function initiates opening of a bytestream with the MessageSession's remote entity.
       * Data can only be sent over an open stream. Use isOpen() to find out what the stream's
       * current state is. However, successful opening/initiation will be announced by means of the
       * InBandBytestreamHandler interface. Multiple bytestreams (even per JID) can be initiated
       * without waiting for success.
       * @param to The recipient of the requested bytestream.
       * @param ibbh The InBandBytestreamHandler to send the new bytestream to.
       * @param sid The bytestream's stream ID, if previously negotiated e.g. using SI (XEP-0095).
       * @return @b False in case of an error, @b true otherwise. A return value of @b true does
       * @b not indicate that the bytestream has been opened. This is announced by means of the
       * InBandBytestreamHandler.
       */
      bool requestInBandBytestream( const JID& to, InBandBytestreamHandler *ibbh,
                                    const std::string& sid = "" );

      /**
       * Sets the default block-size. Default: 4096
       * @param blockSize The default block-size in byte.
       */
      void setBlockSize( int blockSize ) { m_blockSize = blockSize; }

      /**
       * Returns the currently set block-size.
       * @return The currently set block-size.
       */
      int blockSize() const { return m_blockSize; }

      /**
       * To get rid of a bytestream (i.e., close and delete it), call this function. You
       * should not use the bytestream any more.
       * The remote entity will be notified about the closing of the stream.
       * @param ibb The bytestream to dispose. It will be deleted here.
       */
      bool dispose( InBandBytestream *ibb );

      /**
       * When using asynchronous InBandBytestream notifications (if you used
       * registerInBandBytestreamHandler() with a second argument of @b true) you have to
       * call either this function or rejectInBandBytestream() after receiving an InBandBytestream
       * from the InBandBytestreamHandler. Else the initiator will never know whether the
       * request was actually received.
       * @param ibb The InBandBytestream (as received from InBandBytestreamHandler) to accept.
       * @since 0.8.1
       */
      void acceptInBandBytestream( InBandBytestream *ibb );

      /**
       * When using asynchronous InBandBytestream notifications (if you used
       * registerInBandBytestreamHandler() with a second argument of @b true) you have to
       * call either this function or acceptInBandBytestream() after receiving an InBandBytestream
       * from the InBandBytestreamHandler. Else the initiator will never know whether the
       * request was actually received.
       * @param ibb The InBandBytestream (as received from InBandBytestreamHandler) to reject.
       * @since 0.8.1
       */
      void rejectInBandBytestream( InBandBytestream *ibb );

      /**
       * Use this function to register an object that will receive new @b incoming bytestream
       * requests from the InBandBytestreamManager. Only one InBandBytestreamHandler can be
       * registered at any one time.
       * @param ibbh The InBandBytestreamHandler derived object to receive notifications.
       * @param sync Whether incoming bytestrams shall be announced syncronously.
       * Default: true (syncronous)
       */
      void registerInBandBytestreamHandler( InBandBytestreamHandler *ibbh, bool sync = true );

      /**
       * Removes the registered InBandBytestreamHandler.
       */
      void removeInBandBytestreamHandler();

      // reimplemented from IqHandler
      virtual bool handleIq( Stanza *stanza );

      // reimplemented from IqHandler
      virtual bool handleIqID( Stanza *stanza, int context );

    private:
      void acceptInBandBytestream( InBandBytestream *ibb, const JID& from, const std::string& id );
      void rejectInBandBytestream( InBandBytestream *ibb, const JID& from, const std::string& id );

      enum IBBActionType
      {
        IBBOpenStream,
        IBBCloseStream
      };

      typedef std::map<std::string, InBandBytestream*> IBBMap;
      IBBMap m_ibbMap;

      struct TrackItem
      {
        std::string sid;
        InBandBytestreamHandler *ibbh;
      };
      typedef std::map<std::string, TrackItem> TrackMap;
      TrackMap m_trackMap;

      struct AsyncIBBItem
      {
        InBandBytestream *ibb;
        JID from;
        std::string id;
      };
      typedef std::map<std::string, AsyncIBBItem> AsyncTrackMap;
      AsyncTrackMap m_asyncTrackMap;

      ClientBase *m_parent;
      InBandBytestreamHandler *m_inbandBytestreamHandler;
      bool m_syncInbandBytestreams;
      int m_blockSize;

  };

}

#endif // INBANDBYTESTREAMMANAGER_H__
