/*
  Copyright (c) 2006-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef STANZAEXTENSION_H__
#define STANZAEXTENSION_H__

#include "macros.h"

namespace gloox
{

  class Tag;

  /**
   * Supported Stanza extension types.
   */
  enum StanzaExtensionType
  {
    ExtNone                =   1,   /**< Invalid StanzaExtension. */
    ExtVCardUpdate         =   2,   /**< Extension in the vcard-temp:x:update namspace, advertising
                                     * a user avatar's SHA1 hash (XEP-0153). */
    ExtOOB                 =   4,   /**< An extension in the jabber:iq:oob or jabber:x:oob namespaces
                                     * (XEP-0066). */
    ExtGPGSigned           =   8,   /**< An extension containing a GPG/PGP signature (XEP-0027). */
    ExtGPGEncrypted        =  16,   /**< An extension containing a GPG/PGP encrypted message (XEP-0027). */
    ExtXDelay              =  32,   /**< An extension containing notice of delayed delivery (XEP-0091). */
    ExtDelay               =  64    /**< An extension containing notice of delayed delivery (XEP-0203). */
  };

  /**
   * @brief This class abstracts a stanza extension, which is usually (but not necessarily) an 'x'
   * element in a specific namespace.
   *
   * You should not need to use this class directly.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API StanzaExtension
  {
    public:
      /**
       * Constructs an empty StanzaExtension.
       */
      StanzaExtension( StanzaExtensionType type ) : m_type( type ) {}

      /**
       * Virtual destructor.
       */
      virtual ~StanzaExtension() {}

      /**
       * Returns the extension's type.
       * @return The extension's type.
       */
      StanzaExtensionType type() const { return m_type; }

      /**
       * Returns a Tag representation of the extension.
       * @return A Tag representation of the extension.
       */
      virtual Tag* tag() const = 0;

    private:
      StanzaExtensionType m_type;

  };

}

#endif // STANZAEXTENSION_H__
