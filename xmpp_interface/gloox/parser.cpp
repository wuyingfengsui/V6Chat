/*
  Copyright (c) 2004-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#include "gloox.h"

#include "parser.h"

namespace gloox
{

  Parser::Parser( TagHandler *ph )
    : m_tagHandler( ph ), m_current( 0 ), m_root( 0 ), m_state( Initial ),
      m_preamble( 0 ), m_quote( false )
  {
  }

  Parser::~Parser()
  {
    delete m_root;
  }

  bool Parser::feed( const std::string& data )
  {
    std::string::const_iterator it = data.begin();
    for( ; it != data.end(); ++it )
    {
      const unsigned char c = (*it);
//       printf( "found char:   %c, ", c );

      if( !isValid( c ) )
      {
        cleanup();
        return false;
      }

      switch( m_state )
      {
        case Initial:
          m_tag = "";
          if( isWhitespace( c ) )
            break;

          switch( c )
          {
            case '<':
              m_state = TagOpening;
              break;
            case '>':
            default:
//               cleanup();
//               return false;
              break;
          }
          break;
        case TagOpening:               // opening '<' has been found before
          if( isWhitespace( c ) )
            break;

          switch( c )
          {
            case '<':
            case '>':
            case '!':
              cleanup();
              return false;
              break;
            case '/':
              m_state = TagClosingSlash;
              break;
            case '?':
              m_state = TagNameCollect;
              m_preamble = 1;
              break;
            default:
              m_tag += c;
              m_state = TagNameCollect;
              break;
          }
          break;
        case TagNameCollect:          // we're collecting the tag's name, we have at least one octet already
          if( isWhitespace( c ) )
          {
            m_state = TagNameComplete;
            break;
          }

          switch( c )
          {
            case '<':
            case '?':
            case '!':
              cleanup();
              return false;
              break;
            case '/':
              m_state = TagOpeningSlash;
              break;
            case '>':
              addTag();
              m_state = TagInside;
              break;
            default:
              m_tag += c;
              break;
          }
          break;
        case TagInside:                // we're inside a tag, expecting a child tag or cdata
          m_tag = "";
          switch( c )
          {
            case '<':
              addCData();
              m_state = TagOpening;
              break;
            default:
              m_cdata += c;
              break;
          }
          break;
        case TagOpeningSlash:         // a slash in an opening tag has been found, initing close of the tag
          if( isWhitespace( c ) )
            break;

          if( c == '>' )
          {
            addTag();
            if( !closeTag() )
            {
              cleanup();
              return false;
            }

            m_state = Initial;
          }
          else
          {
            cleanup();
            return false;
          }
          break;
        case TagClosingSlash:         // we have found the '/' of a closing tag
          if( isWhitespace( c ) )
            break;

          switch( c )
          {
            case '>':
            case '<':
            case '/':
              cleanup();
              return false;
              break;
            default:
              m_tag += c;
              m_state = TagClosing;
              break;
          }
          break;
        case TagClosing:               // we're collecting the name of a closing tag
          switch( c )
          {
            case '<':
            case '/':
              cleanup();
              return false;
              break;
            case '>':
              if( !closeTag() )
              {
                cleanup();
                return false;
              }

              m_state = Initial;
              break;
            default:
              m_tag += c;
              break;
          }
          break;
        case TagNameComplete:        // a tag name is complete, expect tag close or attribs
          if( isWhitespace( c ) )
            break;

          switch( c )
          {
            case '<':
              cleanup();
              return false;
              break;
            case '/':
              m_state = TagOpeningSlash;
              break;
            case '>':
              if( m_preamble == 1 )
              {
                cleanup();
                return false;
              }
              m_state = TagInside;
              addTag();
              break;
            case '?':
              if( m_preamble == 1 )
                m_preamble = 2;
              else
              {
                cleanup();
                return false;
              }
              break;
            default:
              m_attrib += c;
              m_state = TagAttribute;
              break;
          }
          break;
        case TagAttribute:                  // we're collecting the name of an attribute, we have at least 1 octet
          if( isWhitespace( c ) )
          {
            m_state = TagAttributeComplete;
            break;
          }

          switch( c )
          {
            case '<':
            case '/':
            case '>':
              cleanup();
              return false;
              break;
            case '=':
              m_state = TagAttributeEqual;
              break;
            default:
              m_attrib += c;
          }
          break;
        case TagAttributeComplete:         // we're expecting an equals sign or ws or the attrib value
          if( isWhitespace( c ) )
            break;

          switch( c )
          {
            case '=':
              m_state = TagAttributeEqual;
              break;
            case '<':
            case '/':
            case '>':
            default:
              cleanup();
              return false;
              break;
          }
          break;
        case TagAttributeEqual:            // we have found an equals sign
          if( isWhitespace( c ) )
            break;

          switch( c )
          {
            case '"':
              m_quote = true;
            case '\'':
              m_state = TagValue;
              break;
            case '=':
            case '<':
            case '>':
            default:
              cleanup();
              return false;
              break;
          }
          break;
        case TagValue:                 // we're expecting value data
          switch( c )
          {
            case '<':
              cleanup();
              return false;
              break;
            case '\'':
              if( m_quote )
              {
                m_value += c;
                break;
              }
            case '"':
              addAttribute();
              m_state = TagNameComplete;
              m_quote = false;
              break;
            case '>':
            default:
              m_value += c;
          }
          break;
        default:
//           printf( "default action!?\n" );
          break;
      }
//       printf( "parser state: %d\n", m_state );
    }

    return true;
  }

  void Parser::addTag()
  {
    if( !m_root )
    {
//       printf( "created Tag named %s, ", m_tag.c_str() );
      m_root = new Tag( m_tag, "", true );
      m_current = m_root;
    }
    else
    {
//       printf( "created Tag named %s, ", m_tag.c_str() );
      m_current = new Tag( m_current, m_tag, "", true );
    }

    if( m_attribs.size() )
    {
      m_current->setAttributes( m_attribs );
//       printf( "added %d attributes, ", m_attribs.size() );
      m_attribs.clear();
    }

    if( m_tag == "stream:stream" )
    {
      streamEvent( m_root );
      cleanup();
    }
//     else
//       printf( "%s, ", m_root->xml().c_str() );

    if( m_tag == "xml" && m_preamble == 2 )
      cleanup();
  }

  void Parser::addAttribute()
  {
//     printf( "adding attribute: %s='%s', ", m_attrib.c_str(), m_value.c_str() );
    m_attribs.push_back( Tag::Attribute( Tag::relax( m_attrib ), Tag::relax( m_value ) ) );
    m_attrib = "";
    m_value = "";
//     printf( "added, " );
  }

  void Parser::addCData()
  {
    if( m_current )
    {
      m_current->setCData( m_cdata );
//       printf( "added cdata %s, ", m_cdata.c_str() );
      m_cdata = "";
    }
  }

  bool Parser::closeTag()
  {
//     printf( "about to close, " );

    if( m_tag == "stream:stream" )
      return true;

    if( !m_current || m_current->name() != m_tag )
      return false;

//       printf( "m_current: %s, ", m_current->name().c_str() );
//       printf( "m_tag: %s, ", m_tag.c_str() );

    if( m_current->parent() )
      m_current = m_current->parent();
    else
    {
//       printf( "pushing upstream, " );
      streamEvent( m_root );
      cleanup();
    }

    return true;
  }

  void Parser::cleanup()
  {
    delete m_root;
    m_root = 0;
    m_current = 0;
    m_cdata = "";
    m_tag = "";
    m_attrib = "";
    m_value = "";
    m_attribs.clear();
    m_state = Initial;
    m_preamble = 0;
  }

  bool Parser::isValid( unsigned char c )
  {
    return ( c != 0xc0 || c != 0xc1 || c < 0xf5 );
  }

  bool Parser::isWhitespace( unsigned char c )
  {
    return ( c == 0x09 || c == 0x0a || c == 0x0d || c == 0x20 );
  }

  void Parser::streamEvent( Tag *tag )
  {
    if( m_tagHandler )
      m_tagHandler->handleTag( tag );
  }

}
