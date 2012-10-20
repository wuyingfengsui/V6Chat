/*
  Copyright (c) 2005-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#include "dataformfield.h"
#include "dataformbase.h"
#include "tag.h"

namespace gloox
{

  DataFormField::DataFormField( DataFormFieldType type )
    : m_type( type ), m_required( false )
  {
  }

  DataFormField::DataFormField( const std::string& name, const std::string& value,
                                const std::string& label, DataFormFieldType type )
    : m_name( name ), m_label( label ), m_type( type ), m_required( false )
  {
    m_values.push_back( value );
  }

  DataFormField::DataFormField( Tag *tag )
    : m_type( FieldTypeInvalid ), m_required( false )
  {
    if( !tag )
      return;

    if( tag->hasAttribute( "type", "boolean" ) )
      m_type = FieldTypeBoolean;
    else if( tag->hasAttribute( "type", "fixed" ) )
      m_type = FieldTypeFixed;
    else if( tag->hasAttribute( "type", "hidden" ) )
      m_type = FieldTypeHidden;
    else if( tag->hasAttribute( "type", "jid-multi" ) )
      m_type = FieldTypeJidMulti;
    else if( tag->hasAttribute( "type", "jid-single" ) )
      m_type = FieldTypeJidSingle;
    else if( tag->hasAttribute( "type", "list-multi" ) )
      m_type = FieldTypeListMulti;
    else if( tag->hasAttribute( "type", "list-single" ) )
      m_type = FieldTypeListSingle;
    else if( tag->hasAttribute( "type", "text-multi" ) )
      m_type = FieldTypeTextMulti;
    else if( tag->hasAttribute( "type", "text-private" ) )
      m_type = FieldTypeTextPrivate;
    else if( tag->hasAttribute( "type", "text-single" ) )
      m_type = FieldTypeTextSingle;
    else if( !tag->hasAttribute( "type" ) && !tag->name().empty() )
      m_type = FieldTypeNone;

    if( tag->hasAttribute( "var" ) )
      m_name = tag->findAttribute( "var" );

    if( tag->hasAttribute( "label" ) )
      m_label = tag->findAttribute( "label" );

    const Tag::TagList& l = tag->children();
    Tag::TagList::const_iterator it = l.begin();
    for( ; it != l.end(); ++it )
    {
      if( (*it)->name() == "desc" )
        m_desc = (*it)->cdata();
      else if( (*it)->name() == "required" )
        m_required = true;
      else if( (*it)->name() == "value" )
      {
        if( m_type == FieldTypeTextMulti || m_type == FieldTypeListMulti || m_type == FieldTypeJidMulti )
          addValue( (*it)->cdata() );
        else
          setValue( (*it)->cdata() );
      }
      else if( (*it)->name() == "option" )
      {
        Tag *v = (*it)->findChild( "value" );
        if( v )
          m_options[(*it)->findAttribute( "label" )] = v->cdata();
      }
    }

  }

  DataFormField::~DataFormField()
  {
  }

  Tag* DataFormField::tag() const
  {
    if( m_type == FieldTypeInvalid )
      return 0;

    Tag *field = new Tag( "field" );

    switch( m_type )
    {
      case FieldTypeBoolean:
        field->addAttribute( "type", "boolean" );
        break;
      case FieldTypeFixed:
        field->addAttribute( "type", "fixed" );
        break;
      case FieldTypeHidden:
        field->addAttribute( "type", "hidden" );
        break;
      case FieldTypeJidMulti:
        field->addAttribute( "type", "jid-multi" );
        break;
      case FieldTypeJidSingle:
        field->addAttribute( "type", "jid-single" );
        break;
      case FieldTypeListMulti:
        field->addAttribute( "type", "list-multi" );
        break;
      case FieldTypeListSingle:
        field->addAttribute( "type", "list-single" );
        break;
      case FieldTypeTextMulti:
        field->addAttribute( "type", "text-multi" );
        break;
      case FieldTypeTextPrivate:
        field->addAttribute( "type", "text-private" );
        break;
      case FieldTypeTextSingle:
        field->addAttribute( "type", "text-single" );
        break;
      default:
        break;
    }

    field->addAttribute( "var", m_name );
    field->addAttribute( "label", m_label );
    if( m_required )
      new Tag( field, "required" );

    if( !m_desc.empty() )
      new Tag( field, "desc", m_desc );

    if( m_type == FieldTypeListSingle || m_type == FieldTypeListMulti )
    {
      StringMap::const_iterator it = m_options.begin();
      for( ; it != m_options.end(); ++it )
      {
        Tag *option = new Tag( field, "option" );
        option->addAttribute( "label", (*it).first );
        new Tag( option, "value", (*it).second );
      }
    }
    else if( m_type == FieldTypeBoolean )
    {
      if( m_values.size() == 0 || m_values.front() == "false" || m_values.front() == "0" )
        new Tag( field, "value", "0" );
      else
        new Tag( field, "value", "1" );
    }

    if( m_type == FieldTypeTextMulti || m_type == FieldTypeListMulti || m_type == FieldTypeJidMulti )
    {
      StringList::const_iterator it = m_values.begin();
      for( ; it != m_values.end() ; ++it )
        new Tag( field, "value", (*it) );
    }

    if( m_values.size() && !( m_type == FieldTypeTextMulti || m_type == FieldTypeListMulti
                              || m_type == FieldTypeBoolean || m_type == FieldTypeJidMulti ) )
      new Tag( field, "value", m_values.front() );

    return field;
  }

}
