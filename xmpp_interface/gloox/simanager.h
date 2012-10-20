/*
  Copyright (c) 2007-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef SIMANAGER_H__
#define SIMANAGER_H__

#include "iqhandler.h"

namespace gloox
{

  class ClientBase;
  class SIProfileHandler;
  class SIHandler;

  /**
   * @brief This class manages streams initiated using XEP-0095.
   *
   * You need only one SIManager object per ClientBase instance.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API SIManager : public IqHandler
  {

    public:
      /**
       * SI error conditions.
       */
      enum SIError
      {
        NoValidStreams,             /**< None of the stream types are acceptable */
        BadProfile,                 /**< Profile is not understood. */
        RequestRejected             /**< SI request was rejected. */
      };

      /**
       * Constructor.
       * @param parent The ClientBase to use for communication.
       * @param advertise Whether to advertise SI capabilities by disco. Defaults to true.
       */
      SIManager( ClientBase* parent, bool advertise = true );

      /**
       * Virtual destructor.
       */
      virtual ~SIManager();

      /**
       * Starts negotiating a stream with a remote entity.
       * @param sih The SIHandler to handle the result of this request.
       * @param to The entity to talk to.
       * @param profile The SI profile to use. See XEP-0095 for more info.
       * @param child1 The first of the two allowed children of the SI offer. See
       * XEP-0095 for more info.
       * @param child2 The second of the two allowed children of the SI offer. See
       * XEP-0095 for more info. Defaults to 0.
       * @param mimetype The stream's/file's mime-type. Defaults to 'binary/octet-stream'.
       * @return The requested stream's ID (SID). Empty if SIHandler or ClientBase are invalid.
       * @note The SIManager claims ownership of the Tags supplied to this function, and will
       * delete them after use.
       */
      const std::string requestSI( SIHandler* sih, const JID& to, const std::string& profile, Tag* child1,
                                   Tag* child2 = 0, const std::string& mimetype = "binary/octet-stream" );

      /**
       * Call this function to accept an SI request previously announced by means of
       * SIProfileHandler::handleSIRequest().
       * @param to The requestor.
       * @param id The request's id, as passed to SIProfileHandler::handleSIRequest().
       * @param child1 The &lt;feature/&gt; child of the SI request. See XEP-0095 for details.
       * @param child2 The profile-specific child of the SI request. May be 0. See XEP-0095
       * for details.
       * @note The SIManager claims ownership of the Tags supplied to this function, and will
       * delete them after use.
       */
      void acceptSI( const JID& to, const std::string& id, Tag* child1, Tag* child2 = 0 );

      /**
       * Call this function to decline an SI request previously announced by means of
       * SIProfileHandler::handleSIRequest().
       * @param to The requestor.
       * @param id The request's id, as passed to SIProfileHandler::handleSIRequest().
       * @param reason The reason for the reject.
       * @param text An optional human-readable text explaining the decline.
       */
      void declineSI( const JID& to, const std::string& id, SIError reason, const std::string& text = "" );

      /**
       * Registers the given SIProfileHandler to handle requests for the
       * given SI profile namespace. The profile will be advertised by disco (unless disabled in
       * the ctor).
       * @param profile The complete profile namespace, e.g.
       * http://jabber.org/protocol/si/profile/file-transfer.
       * @param sih The profile handler.
       */
      void registerProfile( const std::string& profile, SIProfileHandler* sih );

      /**
       * Un-registers the given profile.
       * @param profile The profile's namespace to un-register.
       */
      void removeProfile( const std::string& profile );

      // re-implemented from IqHandler
      virtual bool handleIq( Stanza *stanza );

      // re-implemented from IqHandler
      virtual bool handleIqID( Stanza *stanza, int context );

    private:
      enum TrackContext
      {
        OfferSI
      };

      struct TrackStruct
      {
        std::string sid;
        std::string profile;
        SIHandler* sih;
      };
      typedef std::map<std::string, TrackStruct> TrackMap;
      TrackMap m_track;

      ClientBase* m_parent;

      typedef std::map<std::string, SIProfileHandler*> HandlerMap;
      HandlerMap m_handlers;

      bool m_advertise;

  };

}

#endif // SIMANAGER_H__
