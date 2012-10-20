/*
  Copyright (c) 2005-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "jid.h"

#include "prep.h"

namespace gloox
{

  void JID::setJID( const std::string& jid )
  {
    if ( jid.empty() )
    {
      m_bare = m_full = m_server = m_username = m_serverRaw = m_resource = "";
      return;
    }

    size_t at = jid.find( "@", 0 );
    size_t slash = jid.find( "/", 0 );

    if( at == std::string::npos )
    {
      if( slash == std::string::npos )
      {
        m_serverRaw = jid;
      }
      else
      {
        m_serverRaw = jid.substr( 0, slash );
        m_resource = prep::resourceprep( jid.substr( slash + 1 ) );
      }
    }
    else
    {
      m_username = prep::nodeprep( jid.substr( 0, at ) );
      if( slash != std::string::npos )
      {
        m_serverRaw = jid.substr( at + 1, slash - at - 1 );
        m_resource = prep::resourceprep( jid.substr( slash + 1 ) );
      }
      else
      {
        m_serverRaw = jid.substr( at + 1 );
      }
    }
    m_server = prep::nameprep( m_serverRaw );
    setStrings();
  }

  void JID::setUsername( const std::string& username )
  {
    m_username = prep::nodeprep( username );
    setStrings();
  }

  void JID::setServer( const std::string& server )
  {
    m_serverRaw = server;
    m_server = prep::nameprep( m_serverRaw );
    setStrings();
  }

  void JID::setResource( const std::string& resource )
  {
    m_resource = prep::resourceprep( resource );
    setFull();
  }

  void JID::setFull()
  {
    m_full = bare();
    if( !m_resource.empty() )
      m_full += '/' + m_resource;
  }

  void JID::setBare()
  {
    if( !m_username.empty() )
      m_bare = m_username + '@';
    else
      m_bare = "";
    m_bare += m_server;
  }

}
