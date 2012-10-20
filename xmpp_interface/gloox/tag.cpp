/*
  Copyright (c) 2005-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "tag.h"

#include <stdlib.h>

#ifdef _WIN32_WCE
# include <cmath>
#else
# include <sstream>
#endif

#include <algorithm>
#include <cstring>

namespace gloox
{
  Tag::Tag()
    : m_parent( 0 ), m_type( StanzaUndefined ), m_incoming( false ), m_valid( false )
  {
  }

  Tag::Tag( const std::string& name, const std::string& cdata, bool incoming )
    : m_name( incoming ? relax( name ) : name ),
      m_cdata( incoming ? relax( cdata ) : cdata ),
      m_parent( 0 ), m_type( StanzaUndefined ), m_incoming( incoming ), m_valid( true )
  {
    m_valid = !m_name.empty();
  }

  Tag::Tag( Tag *parent, const std::string& name, const std::string& cdata, bool incoming )
    : m_name( incoming ? relax( name ) : name ),
      m_cdata( incoming ? relax( cdata ) : cdata ),
      m_parent( parent ), m_type( StanzaUndefined ), m_incoming( incoming ), m_valid( true )
  {
    if( m_parent )
      m_parent->addChild( this );
    m_valid = !m_name.empty();
  }

  Tag::Tag( const std::string& name, const std::string& attrib, const std::string& value, bool incoming )
    : m_name( incoming ? relax( name ) : name ),
      m_parent( 0 ), m_type( StanzaUndefined ), m_incoming( incoming ), m_valid( true )
  {
    addAttribute( attrib, value );
    m_valid = !m_name.empty();
  }

  Tag::Tag( Tag *parent, const std::string& name, const std::string&  attrib, const std::string& value,
            bool incoming )
    : m_name( incoming ? relax( name ) : name ),
      m_parent( parent ), m_type( StanzaUndefined ), m_incoming( incoming ), m_valid( true )
  {
    if( m_parent )
      m_parent->addChild( this );
    addAttribute( attrib, value );
    m_valid = !m_name.empty();
  }

  Tag::~Tag()
  {
    TagList::iterator it = m_children.begin();
    for( ; it != m_children.end(); ++it )
    {
      delete (*it);
    }
    m_parent = 0;
  }

  bool Tag::operator==( const Tag &right ) const
  {
    if( m_name != right.m_name || m_attribs != right.m_attribs
         || m_children.size() != right.m_children.size() )
      return false;

    TagList::const_iterator it = m_children.begin();
    TagList::const_iterator it_r = right.m_children.begin();
    while( it != m_children.end() && it_r != right.m_children.end() && *(*it) == *(*it_r) )
    {
      ++it;
      ++it_r;
    }
    return it == m_children.end();
  }

  const std::string Tag::xml() const
  {
    std::string xml = "<";
    xml += escape( m_name );
    if( !m_attribs.empty() )
    {
      AttributeList::const_iterator it_a = m_attribs.begin();
      for( ; it_a != m_attribs.end(); ++it_a )
      {
        xml += " ";
        xml += escape( (*it_a).first );
        xml += "='";
        xml += escape( (*it_a).second );
        xml += "'";
      }
    }

    if( m_cdata.empty() && !m_children.size() )
      xml += "/>";
    else if( m_children.size() )
    {
      xml += ">";
      TagList::const_iterator it_c = m_children.begin();
      for( ; it_c != m_children.end(); ++it_c )
      {
        xml += (*it_c)->xml();
      }
      xml += "</";
      xml += escape( m_name );
      xml += ">";
    }
    else if( !m_cdata.empty() )
    {
      xml += ">";
      xml += escape( m_cdata );
      xml += "</";
      xml += escape( m_name );
      xml += ">";
    }

    return xml;
  }

  static const char escape_chars[] = { '&', '<', '>', '\'', '"', '<', '>',
  '\'', '"', '<', '>', '<', '>', '\'', '"', '<', '>', '<', '>', '\'', '"' };

  static const std::string escape_seqs[] = { "amp;", "lt;", "gt;", "apos;",
  "quot;", "#60;", "#62;", "#39;", "#34;", "#x3c;", "#x3e;", "#x3C;",
  "#x3E;", "#x27;", "#x22;", "#X3c;", "#X3e;", "#X3C;", "#X3E;", "#X27;",
  "#X22;" };

  static const unsigned nb_escape = sizeof(escape_chars)/sizeof(char);
  static const unsigned escape_size = 5;

  const std::string Tag::escape( std::string esc )
  {
    for( unsigned val, i = 0; i < esc.length(); ++i )
    {
      for( val = 0; val < escape_size; ++val )
      {
        if( esc[i] == escape_chars[val] )
        {
          esc[i] = '&';
          esc.insert( i+1, escape_seqs[val] );
          i += escape_seqs[val].length();
          break;
        }
      }
    }
    return esc;
  }

  /*
   * When a sequence is found, do not repack the string directly, just set
   * the new symbol and mark the rest for deletation (0).
   */
  const std::string Tag::relax( std::string esc )
  {
    const unsigned int l = esc.length();
    unsigned int p = 0;
    unsigned int i = 0;

    for( unsigned int val; i < l; ++i )
    {
      if( esc[i] != '&' )
        continue;

      for( val = 0; val < nb_escape; ++val )
      {
        if( ( i + escape_seqs[val].length() <= l )
        && !strncmp( esc.data()+i+1, escape_seqs[val].data(),
                                          escape_seqs[val].length() ) )
        {
          esc[i] = escape_chars[val];
          for( p=1; p <= escape_seqs[val].length(); ++p )
            esc[i+p] = 0;
          i += p-1;
          break;
        }
      }
    }
    if( p )
    {
      for( p = 0, i = 0; i < l; ++i )
      {
        if( esc[i] != 0 )
        {
          if( esc[p] == 0 )
          {
            esc[p] = esc[i];
            esc[p+1] = 0;
          }
          ++p;
        }
      }
      esc.resize( p );
    }
    return esc;
  }

  void Tag::addAttribute( const std::string& name, const std::string& value )
  {
    if( name.empty() || value.empty() )
      return;

    AttributeList::iterator it = m_attribs.begin();
    for( ; it != m_attribs.end(); ++it )
    {
      if( (*it).first == ( m_incoming ? relax( name ) : name ) )
      {
        (*it).second = m_incoming ? relax( value ) : value;
        return;
      }
    }

    m_attribs.push_back( Attribute( m_incoming ? relax( name ) : name,
                                    m_incoming ? relax( value ) : value ) );
  }

  void Tag::addAttribute( const std::string& name, int value )
  {
    if( !name.empty() )
    {
#ifdef _WIN32_WCE
      const int len = 4 + (int)std::log10( value ) + 1;
      char *tmp = new char[len];
      sprintf( tmp, "%d", value );
      std::string ret( tmp, len );
      addAttribute( name, ret );
      delete[] tmp;
#else
      std::ostringstream oss;
      oss << value;
      addAttribute( name, oss.str() );
#endif
    }
  }

  void Tag::addAttribute( const std::string& name, long value )
  {
    if( !name.empty() )
    {
#ifdef _WIN32_WCE
      const int len = 4 + (int)std::log10( value ) + 1;
      char *tmp = new char[len];
      sprintf( tmp, "%ld", value );
      std::string ret( tmp, len );
      addAttribute( name, ret );
      delete[] tmp;
#else
      std::ostringstream oss;
      oss << value;
      addAttribute( name, oss.str() );
#endif
    }
  }

  void Tag::addChild( Tag *child )
  {
    if( child )
    {
      m_children.push_back( child );
      child->m_parent = this;
    }
  }

  void Tag::addChildCopy( const Tag *child )
  {
    if( child )
    {
      Tag *t = child->clone();
      m_children.push_back( t );
      t->m_parent = this;
    }
  }

  const std::string Tag::findAttribute( const std::string& name ) const
  {
    AttributeList::const_iterator it = m_attribs.begin();
    for( ; it != m_attribs.end(); ++it )
      if( (*it).first == ( m_incoming ? relax( name ) : name ) )
        return (*it).second;

    return std::string();
  }

  bool Tag::hasAttribute( const std::string& name, const std::string& value ) const
  {
    if( name.empty() )
      return true;

    AttributeList::const_iterator it = m_attribs.begin();
    for( ; it != m_attribs.end(); ++it )
      if( (*it).first == ( m_incoming ? relax( name ) : name )
            && ( value.empty() || (*it).second == ( m_incoming ? relax( value ) : value ) ) )
        return true;

    return false;
  }

  Tag* Tag::findChild( const std::string& name ) const
  {
    TagList::const_iterator it = m_children.begin();
    while( it != m_children.end() && (*it)->name() != ( m_incoming ? relax( name ) : name ) )
      ++it;
    return it != m_children.end() ? (*it) : 0;
  }

  Tag* Tag::findChild( const std::string& name, const std::string& attr,
                       const std::string& value ) const
  {
    if( name.empty() )
      return 0;

    TagList::const_iterator it = m_children.begin();
    while( it != m_children.end()
           && ( (*it)->name() != ( m_incoming ? relax( name ) : name )
                || ! (*it)->hasAttribute( attr, value ) ) )
      ++it;
    return it != m_children.end() ? (*it) : 0;
  }

  bool Tag::hasChildWithCData( const std::string& name, const std::string& cdata ) const
  {
    TagList::const_iterator it = m_children.begin();
    while( it != m_children.end() && ( (*it)->name() != ( m_incoming ? relax( name ) : name )
            || ( !cdata.empty() && (*it)->cdata() != ( m_incoming ? relax( cdata ) : cdata ) ) ) )
      ++it;
    return it != m_children.end();
  }

  Tag* Tag::findChildWithAttrib( const std::string& attr, const std::string& value ) const
  {
    TagList::const_iterator it = m_children.begin();
    while( it != m_children.end() && ! (*it)->hasAttribute( attr, value ) )
      ++it;
    return it != m_children.end() ? (*it) : 0;
  }

  Tag* Tag::clone() const
  {
    Tag *t = new Tag( name(), cdata(), m_incoming );
    t->m_attribs = m_attribs;
    t->m_type = m_type;

    Tag::TagList::const_iterator it = m_children.begin();
    for( ; it != m_children.end(); ++it )
    {
      t->addChild( (*it)->clone() );
    }

    return t;
  }

  Tag::TagList Tag::findChildren( const std::string& name ) const
  {
    return findChildren( m_children, name );
  }

  Tag::TagList Tag::findChildren( const Tag::TagList& list, const std::string& name ) const
  {
    Tag::TagList ret;
    Tag::TagList::const_iterator it = list.begin();
    for( ; it != list.end(); ++it )
    {
      if( (*it)->name() == ( m_incoming ? relax( name ) : name ) )
        ret.push_back( (*it) );
    }
    return ret;
  }

  Tag* Tag::findTag( const std::string& expression )
  {
    const Tag::TagList& l = findTagList( expression );
    return !l.empty() ? l.front() : 0;
  }

  Tag::TagList Tag::findTagList( const std::string& expression )
  {
    Tag::TagList l;
    if( expression == "/" || expression == "//" )
      return l;

    if( m_parent && expression.length() >= 2 && expression.substr( 0, 1 ) == "/"
                                                  && expression.substr( 1, 1 ) != "/" )
      return m_parent->findTagList( expression );

    unsigned len = 0;
    Tag *p = parse( expression, len );
//     if( p )
//       printf( "parsed tree: %s\n", p->xml().c_str() );
    l = evaluateTagList( p );
    delete p;
    return l;
  }

  Tag::TagList Tag::evaluateTagList( Tag *token )
  {
    Tag::TagList result;
    if( !token )
      return result;

//     printf( "evaluateTagList called in Tag %s and Token %s (type: %s)\n", name().c_str(),
//             token->name().c_str(), token->findAttribute( "type" ).c_str() );

    TokenType tokenType = (TokenType)atoi( token->findAttribute( "type" ).c_str() );
    switch( tokenType )
    {
      case XTUnion:
        add( result, evaluateUnion( token ) );
        break;
      case XTElement:
      {
//         printf( "in XTElement, token: %s\n", token->name().c_str() );
        if( token->name() == name() || token->name() == "*" )
        {
//           printf( "found %s\n", name().c_str() );
          const Tag::TagList& tokenChildren = token->children();
          if( tokenChildren.size() )
          {
            bool predicatesSucceeded = true;
            Tag::TagList::const_iterator cit = tokenChildren.begin();
            for( ; cit != tokenChildren.end(); ++cit )
            {
              if( (*cit)->hasAttribute( "predicate", "true" ) )
              {
                predicatesSucceeded = evaluatePredicate( (*cit) );
                if( !predicatesSucceeded )
                  return result;
              }
            }

            bool hasElementChildren = false;
            cit = tokenChildren.begin();
            for( ; cit != tokenChildren.end(); ++cit )
            {
              if( (*cit)->hasAttribute( "predicate", "true" ) ||
                  (*cit)->hasAttribute( "number", "true" ) )
                continue;

              hasElementChildren = true;

//               printf( "checking %d children of token %s\n", tokenChildren.size(), token->name().c_str() );
              if( !m_children.empty() )
              {
                Tag::TagList::const_iterator it = m_children.begin();
                for( ; it != m_children.end(); ++it )
                {
                  add( result, (*it)->evaluateTagList( (*cit) ) );
                }
              }
              else if( atoi( (*cit)->findAttribute( "type" ).c_str() ) == XTDoubleDot && m_parent )
              {
                (*cit)->addAttribute( "type", XTDot );
                add( result, m_parent->evaluateTagList( (*cit) ) );
              }
            }

            if( !hasElementChildren )
              result.push_back( this );
          }
          else
          {
//             printf( "adding %s to result set\n", name().c_str() );
            result.push_back( this );
          }
        }
//         else
//           printf( "found %s != %s\n", token->name().c_str(), name().c_str() );

        break;
      }
      case XTDoubleSlash:
      {
//         printf( "in XTDoubleSlash\n" );
        Tag *t = token->clone();
//         printf( "original token: %s\ncloned token: %s\n", token->xml().c_str(), n->xml().c_str() );
        t->addAttribute( "type", XTElement );
        add( result, evaluateTagList( t ) );
        const Tag::TagList& res2 = allDescendants();
        Tag::TagList::const_iterator it = res2.begin();
        for( ; it != res2.end(); ++it )
        {
          add( result, (*it)->evaluateTagList( t ) );
        }
        delete t;
        break;
      }
      case XTDot:
      {
        const Tag::TagList& tokenChildren = token->children();
        if( !tokenChildren.empty() )
        {
          add( result, evaluateTagList( tokenChildren.front() ) );
        }
        else
          result.push_back( this );
        break;
      }
      case XTDoubleDot:
      {
//         printf( "in XTDoubleDot\n" );
        if( m_parent )
        {
          const Tag::TagList& tokenChildren = token->children();
          if( tokenChildren.size() )
          {
            Tag *testtoken = tokenChildren.front();
            if( testtoken->name() == "*" )
            {
              add( result, m_parent->evaluateTagList( testtoken ) );
            }
            else
            {
              Tag *t = token->clone();
              t->addAttribute( "type", XTElement );
              t->m_name = m_parent->m_name;
              add( result, m_parent->evaluateTagList( t ) );
              delete t;
            }
          }
          else
          {
            result.push_back( m_parent );
          }
        }
      }
      case XTInteger:
      {
        const Tag::TagList& l = token->children();
        if( !l.size() )
          break;

        const Tag::TagList& res = evaluateTagList( l.front() );

        int pos = atoi( token->name().c_str() );
//         printf( "checking index %d\n", pos );
        if( pos > 0 && pos <= (int)res.size() )
        {
          Tag::TagList::const_iterator it = res.begin();
          while ( --pos )
          {
            ++it;
          }
          result.push_back( *it );
        }
        break;
      }
      default:
        break;
    }
    return result;
  }

  bool Tag::evaluateBoolean( Tag *token )
  {
    if( !token )
      return false;

    bool result = false;
    TokenType tokenType = (TokenType)atoi( token->findAttribute( "type" ).c_str() );
    switch( tokenType )
    {
      case XTAttribute:
        if( token->name() == "*" && m_attribs.size() )
          result = true;
        else
          result = hasAttribute( token->name() );
        break;
      case XTOperatorEq:
        result = evaluateEquals( token );
        break;
      case XTOperatorLt:
        break;
      case XTOperatorLtEq:
        break;
      case XTOperatorGtEq:
        break;
      case XTOperatorGt:
        break;
      case XTUnion:
      case XTElement:
      {
        Tag *t = new Tag( "." );
        t->addAttribute( "type", XTDot );
        t->addChild( token );
        result = !evaluateTagList( t ).empty();
        t->removeChild( token );
        delete t;
        break;
      }
      default:
        break;
    }

    return result;
  }

  bool Tag::evaluateEquals( Tag *token )
  {
    if( !token || token->children().size() != 2 )
      return false;

    bool result = false;
    Tag::TagList::const_iterator it = token->children().begin();
    Tag *ch1 = (*it);
    Tag *ch2 = (*++it);

    TokenType tt1 = (TokenType)atoi( ch1->findAttribute( "type" ).c_str() );
    TokenType tt2 = (TokenType)atoi( ch2->findAttribute( "type" ).c_str() );
    switch( tt1 )
    {
      case XTAttribute:
        switch( tt2 )
        {
          case XTInteger:
          case XTLiteral:
            result = ( findAttribute( ch1->name() ) == ch2->name() );
            break;
          case XTAttribute:
            result = ( hasAttribute( ch1->name() ) && hasAttribute( ch2->name() ) &&
                      findAttribute( ch1->name() ) == findAttribute( ch2->name() ) );
            break;
          default:
            break;
        }
        break;
      case XTInteger:
      case XTLiteral:
        switch( tt2 )
        {
          case XTAttribute:
            result = ( ch1->name() == findAttribute( ch2->name() ) );
            break;
          case XTLiteral:
          case XTInteger:
            result = ( ch1->name() == ch2->name() );
            break;
          default:
            break;
        }
        break;
      default:
        break;
    }

    return result;
  }

  Tag::TagList Tag::allDescendants()
  {
    Tag::TagList result;
    Tag::TagList::const_iterator it = m_children.begin();
    for( ; it != m_children.end(); ++it )
    {
      result.push_back( (*it) );
      add( result, (*it)->allDescendants() );
    }
    return result;
  }

  Tag::TagList Tag::evaluateUnion( Tag *token )
  {
    Tag::TagList result;
    if( !token )
      return result;

    const Tag::TagList& l = token->children();
    Tag::TagList::const_iterator it = l.begin();
    for( ; it != l.end(); ++it )
    {
      add( result, evaluateTagList( (*it) ) );
    }
    return result;
  }

  void Tag::closePreviousToken( Tag** root, Tag** current, Tag::TokenType& type, std::string& tok )
  {
    if( !tok.empty() )
    {
      addToken( root, current, type, tok );
      type = XTElement;
      tok = "";
    }
  }

  Tag* Tag::parse( const std::string& expression, unsigned& len, Tag::TokenType border )
  {
    Tag *root = 0;
    Tag *current = root;
    std::string token;

//     XPathError error = XPNoError;
//     XPathState state = Init;
//     int expected = 0;
//     bool run = true;
//     bool ws = false;

    Tag::TokenType type  = XTElement;

    char c;
    for( ; len < expression.length(); ++len )
    {
      c = expression[len];
      if( type == XTLiteralInside && c != '\'' )
      {
        token += c;
        continue;
      }

      switch( c )
      {
        case '/':
          closePreviousToken( &root, &current, type, token );

          if( len < expression.length()-1 && expression[len+1] == '/' )
          {
//             addToken( &root, &current, XTDoubleSlash, "//" );
            type = XTDoubleSlash;
            ++len;
          }
//           else
//           {
//             if( !current )
//             addToken( &root, &current, XTSlash, "/" );
//           }
          break;
        case ']':
          closePreviousToken( &root, &current, type, token );
          ++len;
          return root;
        case '[':
        {
          closePreviousToken( &root, &current, type, token );
          Tag *t = parse( expression, ++len, XTRightBracket );
          if( !addPredicate( &root, &current, t ) )
            delete t;
          break;
        }
        case '(':
        {
          closePreviousToken( &root, &current, type, token );
          Tag *t = parse( expression, ++len, XTRightParenthesis );
          if( current )
          {
//             printf( "added %s to %s\n", t->xml().c_str(), current->xml().c_str() );
            t->addAttribute( "argument", "true" );
            current->addChild( t );
          }
          else
          {
            root = t;
//             printf( "made %s new root\n", t->xml().c_str() );
          }
          break;
        }
        case ')':
          closePreviousToken( &root, &current, type, token );
          ++len;
          return root;
        case '\'':
          if( type == XTLiteralInside )
            if( expression[len - 2] == '\\' )
              token[token.length() - 2] = c;
            else
              type = XTLiteral;
          else
            type = XTLiteralInside;
          break;
        case '@':
          type = XTAttribute;
          break;
        case '.':
          token += c;
          if( token.size() == 1 )
          {
            if( len < expression.length()-1 && expression[len+1] == '.' )
            {
              type = XTDoubleDot;
              ++len;
              token += c;
            }
            else
            {
              type = XTDot;
            }
          }
          break;
        case '*':
//           if( !root || ( current && ( current->tokenType() == XTSlash
//                                       || current->tokenType() == XTDoubleSlash ) ) )
//           {
//             addToken( &root, &current, type, "*" );
//             break;
//           }
          addToken( &root, &current, type, "*" );
          type = XTElement;
          break;
        case '+':
        case '>':
        case '<':
        case '=':
        case '|':
        {
          closePreviousToken( &root, &current, type, token );
          std::string s( 1, c );
          Tag::TokenType ttype = getType( s );
          if( ttype <= border )
            return root;
          Tag *t = parse( expression, ++len, ttype );
          addOperator( &root, &current, t, ttype, s );
          if( border == XTRightBracket )
            return root;
          break;
        }
        default:
          token += c;
      }
    }

    if( !token.empty() )
      addToken( &root, &current, type, token );

//     if( error != XPNoError )
//       printf( "error: %d\n", error );
    return root;
  }

  void Tag::addToken( Tag **root, Tag **current, Tag::TokenType type,
                      const std::string& token )
  {
    Tag *t = new Tag( token );
    if( t->isNumber() && !t->children().size() )
      type = XTInteger;
    t->addAttribute( "type", type );

    if( *root )
    {
//       printf( "new current %s, type: %d\n", token.c_str(), type );
      (*current)->addChild( t );
      *current = t;
    }
    else
    {
//       printf( "new root %s, type: %d\n", token.c_str(), type );
      *current = *root = t;
    }
  }

  void Tag::addOperator( Tag **root, Tag **current, Tag *arg,
                           Tag::TokenType type, const std::string& token )
  {
    Tag *t = new Tag( token );
    t->addAttribute( "type", type );
//     printf( "new operator: %s (arg1: %s, arg2: %s)\n", t->name().c_str(), (*root)->xml().c_str(),
//                                                                           arg->xml().c_str() );
    t->addAttribute( "operator", "true" );
    t->addChild( *root );
    t->addChild( arg );
    *current = *root = t;
  }

  bool Tag::addPredicate( Tag **root, Tag **current, Tag *token )
  {
    if( !*root || !*current )
      return false;

    if( ( token->isNumber() && !token->children().size() ) || token->name() == "+" )
    {
//       printf( "found Index %s, full: %s\n", token->name().c_str(), token->xml().c_str() );
      if( !token->hasAttribute( "operator", "true" ) )
      {
        token->addAttribute( "type", XTInteger );
      }
      if( *root == *current )
      {
        *root = token;
//         printf( "made Index new root\n" );
      }
      else
      {
        (*root)->removeChild( *current );
        (*root)->addChild( token );
//         printf( "added Index somewhere between root and current\n" );
      }
      token->addChild( *current );
//       printf( "added Index %s, full: %s\n", token->name().c_str(), token->xml().c_str() );
    }
    else
    {
      token->addAttribute( "predicate", "true" );
      (*current)->addChild( token );
    }

    return true;
  }

  Tag::TokenType Tag::getType( const std::string& c )
  {
    if( c == "|" )
      return XTUnion;
    if( c == "<" )
      return XTOperatorLt;
    if( c == ">" )
      return XTOperatorGt;
    if( c == "*" )
      return XTOperatorMul;
    if( c == "+" )
      return XTOperatorPlus;
    if( c == "=" )
      return XTOperatorEq;

    return XTNone;
  }

  bool Tag::isWhitespace( const char c )
  {
    return ( c == 0x09 || c == 0x0a || c == 0x0d || c == 0x20 );
  }

  bool Tag::isNumber()
  {
    if( m_name.empty() )
      return false;

    std::string::size_type l = m_name.length();
    std::string::size_type i = 0;
    while( i < l && isdigit( m_name[i] ) )
      ++i;
    return i == l;
  }

  void Tag::add( Tag::TagList& one, const Tag::TagList& two )
  {
    Tag::TagList::const_iterator it = two.begin();
    for( ; it != two.end(); ++it )
      if( std::find( one.begin(), one.end(), (*it) ) == one.end() )
        one.push_back( (*it) );
  }

}
