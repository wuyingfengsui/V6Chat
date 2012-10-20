/*
  Copyright (c) 2004-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef PREP_H__
#define PREP_H__

#include "macros.h"

#include <string>

namespace gloox
{

  /**
   * @brief This namespace offers functions to stringprep the individual parts of a JID.
   *
   * You should not need to use these functions directly. All the
   * necessary prepping is done for you if you stick to the interfaces provided.
   * If you write your own enhancements, check with the spec.
   *
   * @note These functions depend on an installed LibIDN at compile time of gloox. If
   * LibIDN is not installed these functions return the string they are given
   * without any modification.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.2
   */
  namespace prep
  {
      /**
       * This function applies the Nodeprep profile of Stringprep to a string.
       * @param node The string to apply the profile to.
       * @return Returns the prepped string. In case of an error an empty string
       * is returned. If LibIDN is not available the string is returned unchanged.
       */
      std::string nodeprep( const std::string& node );

      /**
       * This function applies the Nameprep profile of Stringprep to a string.
       * @param domain The string to apply the profile to.
       * @return Returns the prepped string. In case of an error an empty string
       * is returned. If LibIDN is not available the string is returned unchanged.
       */
      std::string nameprep( const std::string& domain );

      /**
       * This function applies the Resourceprep profile of Stringprep to a std::string.
       * @param resource The string to apply the profile to.
       * @return Returns the prepped string. In case of an error an empty string
       * is returned. If LibIDN is not available the string is returned unchanged.
       */
      std::string resourceprep( const std::string& resource );

      /**
       * This function applies the idna() function to a string. I.e. it transforms
       * internationalized domain names into plain ASCII.
       * @param domain The string to convert.
       * @return Returns the converted string. In case of an error an empty string
       * is returned. If LibIDN is not available the string is returned unchanged.
       */
      std::string idna( const std::string& domain );

  }

}

#endif // PREP_H__
