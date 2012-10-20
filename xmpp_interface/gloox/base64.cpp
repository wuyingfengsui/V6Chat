/*
  Copyright (c) 2005-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "base64.h"

namespace gloox
{

  const std::string Base64::alphabet64( "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" );
  const char Base64::pad = '=';

  const std::string::size_type Base64::np = std::string::npos;
  const std::string::size_type Base64::table64[] =
  {
    np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np,
    np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np,
    np, np, np, 62, np, np, np, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, np, np,
    np, np, np, np, np,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, np, np, np, np, np, np, 26, 27, 28,
    29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
    49, 50, 51, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np,
    np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np,
    np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np,
    np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np,
    np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np,
    np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np,
    np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np
  };

  const std::string Base64::encode64( const std::string& input )
  {
    std::string encoded;
    char c;
    const std::string::size_type length = input.length();

    encoded.reserve( length * 2 );

    for( std::string::size_type i = 0; i < length; ++i )
    {
      c = ( input[i] >> 2 ) & 0x3f;
      encoded.append( 1, alphabet64[c] );

      c = ( input[i] << 4 ) & 0x3f;
      if( ++i < length )
        c |= ( ( input[i] >> 4 ) & 0x0f );
      encoded.append( 1, alphabet64[c] );

      if( i < length )
      {
        c = ( input[i] << 2 ) & 0x3c;
        if( ++i < length )
          c |= ( input[i] >> 6 ) & 0x03;
        encoded.append( 1, alphabet64[c] );
      }
      else
      {
        ++i;
        encoded.append( 1, pad );
      }

      if( i < length )
      {
        c = input[i] & 0x3f;
        encoded.append( 1, alphabet64[c] );
      }
      else
      {
        encoded.append( 1, pad );
      }
    }

    return encoded;
  }

  const std::string Base64::decode64( const std::string& input )
  {
    char c, d;
    const std::string::size_type length = input.length();
    std::string decoded;

    decoded.reserve( length );

    for( std::string::size_type i = 0; i < length; ++i )
    {
      c = (char)table64[(unsigned char)input[i]];
      ++i;
      d = (char)table64[(unsigned char)input[i]];
      c = ( c << 2 ) | ( ( d >> 4 ) & 0x3 );
      decoded.append( 1, c );
      if( ++i < length )
      {
        c = input[i];
        if( pad == c )
          break;

        c = (char)table64[(unsigned char)input[i]];
        d = ( ( d << 4 ) & 0xf0 ) | ( ( c >> 2 ) & 0xf );
        decoded.append( 1, d );
      }

      if( ++i < length )
      {
        d = input[i];
        if( pad == d )
          break;

        d = (char)table64[(unsigned char)input[i]];
        c = ( ( c << 6 ) & 0xc0 ) | d;
        decoded.append( 1, c );
      }
    }

    return decoded;
  }

}
