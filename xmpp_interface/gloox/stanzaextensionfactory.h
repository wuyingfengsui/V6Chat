/*
  Copyright (c) 2006-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef STANZAEXTENSIONFACTORY_H__
#define STANZAEXTENSIONFACTORY_H__

#include "macros.h"

#include <string>

namespace gloox
{

  class StanzaExtension;
  class Tag;

  /**
   * @brief A Factory that creates StanzaExtensions from Tags.
   *
   * You should not need to use this class directly.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class StanzaExtensionFactory
  {
    public:
      /**
       * This function tries to create a valid StanzaExtension (i.e., an object derived from
       * StanzaExtension) from the given Tag.
       * @param tag The Tag to parse and create the StanzaExtension from.
       * @return A StanzaExtension-derived object if the Tag was recognized, or 0.
       * @note To get rif of a StanzaExtension easily, you may use dispose().
       */
      static StanzaExtension* create( Tag* tag );

  };

}

#endif // STANZAEXTENSIONFACTORY_H__
