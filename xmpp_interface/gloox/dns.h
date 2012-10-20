/*
  Copyright (c) 2005-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef DNS_H__
#define DNS_H__

#include "macros.h"
#include "logsink.h"

#ifdef __MINGW32__
# include <windows.h>
# include <windns.h>
#endif

#ifdef HAVE_ARPA_NAMESER_H
# include <arpa/nameser.h>
#endif

#ifdef __APPLE__
# include <arpa/nameser_compat.h>
#endif

#ifndef NS_MAXDNAME
# define NS_MAXDNAME 1025
#endif

#ifndef NS_PACKETSZ
# define NS_PACKETSZ 512
#endif

#include <string>
#include <map>

namespace gloox
{

  /**
   * @brief This class holds a number of static functions used for DNS related stuff.
   *
   * You should not need to use these functions directly.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.3
   */
  class GLOOX_API DNS
  {
    public:

      /**
       * A list of strings (used for server addresses) and ints (used for port numbers).
       */
      typedef std::map<std::string, int> HostMap;

      /**
       * This funtion resolves a service/protocol/domain tuple.
       * @param service The SRV service type.
       * @param proto The SRV protocol.
       * @param domain The domain to search for SRV records.
       * @param logInstance A LogSink to use for logging.
       * @return A list of weighted hostname/port pairs from SRV records, or A records if no SRV
       * records where found.
       */
      static HostMap resolve( const std::string& service, const std::string& proto,
                              const std::string& domain, const LogSink& logInstance );

      /**
       * This is a convenience funtion which uses @ref resolve() to resolve SRV records
       * for a given domain, using a service of @b xmpp and a proto of @b tcp.
       * @param domain The domain to resolve SRV records for.
       * @param logInstance A LogSink to use for logging.
       * @return A list of weighted hostname/port pairs from SRV records, or A records if no SRV
       * records where found.
       */
      static HostMap resolve( const std::string& domain, const LogSink& logInstance )
        { return resolve( "xmpp-client", "tcp", domain, logInstance ); }

      /**
       * This is a convenience function which uses @ref resolve() to get a list of hosts
       * and connects to one of them.
       * @param domain The domain to resolve SRV records for.
       * @param logInstance A LogSink to use for logging.
       * @return A file descriptor for the established connection.
       */
      static int connect( const std::string& domain, const LogSink& logInstance );

      /**
       * This is a convenience function which uses connects to the given host and port. No SRV
       * records are resolved. Use this function for special setups.
       * @param domain The domain to connect to.
       * @param port A custom port to connect to.
       * @param logInstance A LogSink to use for logging.
       * @return A file descriptor for the established connection.
       */
      static int connect( const std::string& domain, unsigned short port, const LogSink& logInstance );

      /**
       * Prepares and returnes a simple socket.
       * @return A simple socket.
       */
      static int getSocket();

      /**
       * Closes the given socket.
       * @param fd The socket to close.
       */
      static void closeSocket( int fd );

    private:
      static HostMap defaultHostMap( const std::string& domain, const LogSink& logInstance );
      static void cleanup();

      typedef struct buffer
      {
        unsigned char buf[NS_PACKETSZ];
        int len;
      };
      typedef unsigned char name[NS_MAXDNAME];
  };

}

#endif // DNS_H__
