/*
  Copyright (c) 2004-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#include "prep.h"

#include <cstdlib>
#include <string>
#include <string.h>

#ifdef _WIN32
# include "config.h.win"
#elif defined( _WIN32_WCE )
# include "../config.h.win"
#else
# include "config.h"
#endif

#ifdef HAVE_LIBIDN
# include <stringprep.h>
# include <idna.h>
#endif

#define JID_PORTION_SIZE 1023

namespace gloox
{

  namespace prep
  {

#ifdef HAVE_LIBIDN
    /**
     * Applies a Stringprep profile to a string. This function does the actual
     * work behind nodeprep, nameprep and resourceprep.
     * @param s The string to apply the profile to.
     * @param profile The Stringprep profile to apply.
     * @return Returns the prepped string. In case of an error an empty string
     * is returned. If LibIDN is not available the string is returned unchanged.
     */
    static std::string prepare( const std::string& s, const Stringprep_profile* profile )
    {
      if( s.empty() || s.length() > JID_PORTION_SIZE )
        return std::string();

      std::string preppedString;
      char* p = static_cast<char*>( calloc( JID_PORTION_SIZE, sizeof( char ) ) );
      strncpy( p, s.c_str(), s.length() );
      if( stringprep( p, JID_PORTION_SIZE, (Stringprep_profile_flags)0, profile ) == STRINGPREP_OK )
        preppedString = p;
      free( p );
      return preppedString;
    }
#endif

    std::string nodeprep( const std::string& node )
    {
#ifdef HAVE_LIBIDN
      return prepare( node, stringprep_xmpp_nodeprep );
#else
      return node;
#endif
    }

    std::string nameprep( const std::string& domain )
    {
#ifdef HAVE_LIBIDN
      return prepare( domain, stringprep_nameprep );
#else
      return domain;
#endif
    }

    std::string resourceprep( const std::string& resource )
    {
#ifdef HAVE_LIBIDN
      return prepare( resource, stringprep_xmpp_resourceprep );
#else
      return resource;
#endif
    }

    std::string idna( const std::string& domain )
    {
#ifdef HAVE_LIBIDN
      if( domain.empty() || domain.length() > JID_PORTION_SIZE )
        return std::string();

      std::string preppedString;
      char* prepped;
      int rc = idna_to_ascii_8z( domain.c_str(), &prepped, (Idna_flags)0 );
      if( rc == IDNA_SUCCESS )
        preppedString = prepped;
      if( rc != IDNA_MALLOC_ERROR )
        free( prepped );
      return preppedString;
#else
      return domain;
#endif
    }
  }
}
