/*
  Copyright (c) 2004-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef PARSER_H__
#define PARSER_H__

#include "gloox.h"
#include "taghandler.h"
#include "tag.h"

#include <string>

namespace gloox
{


  /**
   * @brief This class implements an XML parser.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API Parser
  {
    public:
      /**
       * Constructs a new Parser object.
       * @param ph The object to send incoming Tags to.
       */
      Parser( TagHandler *ph );

      /**
       * Virtual destructor.
       */
      virtual ~Parser();

      /**
       * Use this function to feed the parser with more XML.
       * @param data Raw xml to parse.
       * @return Returns @b true if parsing was successful, @b false otherwise.
       */
      bool feed( const std::string& data );

      /**
       * Rests the parser's internal state.
       */
      void reset() { cleanup(); }

    private:
      void addTag();
      void addAttribute();
      void addCData();
      bool closeTag();
      void cleanup();
      bool isWhitespace( unsigned char c );
      bool isValid( unsigned char c );
      void streamEvent( Tag *tag );

      enum ParserInternalState
      {
        Initial,
        TagOpening,
        TagOpeningSlash,
        TagOpeningLt,
        TagInside,
        TagNameCollect,
        TagNameComplete,
        TagAttribute,
        TagAttributeComplete,
        TagAttributeEqual,
        TagClosing,
        TagClosingSlash,
        TagValueApos,
        TagValue,
        TagPreamble
      };

      TagHandler *m_tagHandler;
      Tag *m_current;
      Tag *m_root;

      ParserInternalState m_state;
      Tag::AttributeList m_attribs;
      std::string m_tag;
      std::string m_cdata;
      std::string m_attrib;
      std::string m_value;
      int m_preamble;
      bool m_quote;

  };

}

#endif // PARSER_H__
