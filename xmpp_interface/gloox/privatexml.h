/*
  Copyright (c) 2004-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef PRIVATEXML_H__
#define PRIVATEXML_H__

#include "iqhandler.h"
#include "privatexmlhandler.h"

#include <string>
#include <list>
#include <map>

namespace gloox
{

  class ClientBase;
  class Tag;
  class Stanza;

  /**
   * @brief This class implements XEP-0049 (Private XML Storage).
   *
   * @author Jakob Schroeter <js@camaya.net>
   */
  class GLOOX_API PrivateXML : public IqHandler
  {
    public:
      /**
       * Constructor.
       * Creates a new PrivateXML client that registers as IqHandler
       * with @c ClientBase.
       * @param parent The ClientBase used for XMPP communication
       */
      PrivateXML( ClientBase *parent );

      /**
       * Virtual destructor.
       */
      virtual ~PrivateXML();

      /**
       * Use this function to request the private XML stored in the given namespace.
       * @param tag Child element of the query element used to identify the requested XML fragment.
       * @param xmlns The namespace which qualifies the tag.
       * @param pxh The handler to receive the result.
       * @return The ID of the sent query.
       */
      std::string requestXML( const std::string& tag, const std::string& xmlns, PrivateXMLHandler *pxh );

      /**
       * Use this function to store private XML stored in the given namespace.
       * @param tag The XML to store. This is the complete tag including the unique namespace.
       * It is deleted automatically after sending it.
       * @param pxh The handler to receive the result.
       * @return The ID of the sent query.
       */
      std::string storeXML( Tag *tag, PrivateXMLHandler *pxh );

      // reimplemented from IqHandler.
      virtual bool handleIq( Stanza *stanza );

      // reimplemented from IqHandler.
      virtual bool handleIqID( Stanza *stanza, int context );

    protected:
      ClientBase *m_parent;

    private:
      enum IdType
      {
        RequestXml,
        StoreXml
      };

      typedef std::map<std::string, PrivateXMLHandler*> TrackMap;

      TrackMap m_track;
  };

}

#endif // PRIVATEXML_H__
