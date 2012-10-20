/*
  Copyright (c) 2006-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef XDELAYEDDELIVERY_H__
#define XDELAYEDDELIVERY_H__

#include "stanzaextension.h"
#include "jid.h"

#include <string>

namespace gloox
{

  class Tag;

  /**
   * @brief This is an implementation of XEP-0091 (Delayed Delivery).
   *
   * XEP Version: 1.2
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class XDelayedDelivery : public StanzaExtension
  {

    public:
      /**
       * Constructs a new object and fills it according to the parameters.
       * @param from The JID of the original sender or the entity that delayed the sending.
       * @param stamp The datetime stamp of the original send.
       * @param reason A natural language reason for the delay.
       */
      GLOOX_DEPRECATED_CTOR XDelayedDelivery( const JID& from, const std::string stamp,
                                              const std::string& reason );

      /**
       * Constructs a new object from the given Tag.
       * @param tag The Tag to parse.
       */
      XDelayedDelivery( Tag *tag );

      /**
       * Virtual Destructor.
       */
      virtual ~XDelayedDelivery();

      /**
       * Returns the datetime when the stanza was originally sent.
       * The format SHOULD be "CCYYMMDDThh:mm:ss" and is to be understood as UTC.
       * @return The original datetime.
       */
      const std::string& stamp() const { return m_stamp; }

      /**
       * Returns the JID of the original sender of the stanza or of the entity that
       * delayed the sending.
       * @return The JID.
       */
      const JID& from() const { return m_from; }

      /**
       * Returns a natural language reason for the delay.
       * @return A natural language reason for the delay.
       */
      const std::string& reason() const { return m_reason; }

      // reimplemented from StanzaExtension
      virtual Tag* tag() const;

    private:
      JID m_from;
      std::string m_stamp;
      std::string m_reason;
      bool m_valid;

  };

}

#endif // XDELAYEDDELIVERY_H__
