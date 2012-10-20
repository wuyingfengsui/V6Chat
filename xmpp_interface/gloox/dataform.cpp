/*
  Copyright (c) 2005-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "dataform.h"
#include "dataformfield.h"
#include "dataformreported.h"
#include "dataformitem.h"
#include "tag.h"

namespace gloox
{

  DataForm::DataForm( DataFormType type, const StringList& instructions, const std::string& title )
    : m_instructions( instructions ), m_type( type ), m_title( title )
  {
  }

  DataForm::DataForm( DataFormType type, const std::string& title )
    : m_type( type ), m_title( title )
  {
  }

  DataForm::DataForm( Tag *tag )
    : m_type( FormTypeInvalid )
  {
    parse( tag );
  }

  DataForm::DataForm()
  : m_type( FormTypeInvalid )
  {
  }

  DataForm::~DataForm()
  {
  }

  bool DataForm::parse( Tag *tag )
  {
    if( !tag || !tag->hasAttribute( "xmlns", XMLNS_X_DATA ) || tag->name() != "x" )
      return false;

    if( tag->hasAttribute( "type", "form" ) )
      m_type = FormTypeForm;
    else if( tag->hasAttribute( "type", "submit" ) )
      m_type = FormTypeSubmit;
    else if( tag->hasAttribute( "type", "cancel" ) )
      m_type = FormTypeCancel;
    else if( tag->hasAttribute( "type", "result" ) )
      m_type = FormTypeResult;
    else
      return false;

    const Tag::TagList& l = tag->children();
    Tag::TagList::const_iterator it = l.begin();
    for( ; it != l.end(); ++it )
    {
      if( (*it)->name() == "title" )
        m_title = (*it)->cdata();
      else if( (*it)->name() == "instructions" )
        m_instructions.push_back( (*it)->cdata() );
      else if( (*it)->name() == "field" )
      {
        DataFormField *f = new DataFormField( (*it) );
        m_fields.push_back( f );
      }
      else if( (*it)->name() == "reported" )
      {
        DataFormReported *r = new DataFormReported( (*it) );
        m_fields.push_back( r );
      }
      else if( (*it)->name() == "item" )
      {
        DataFormItem *i = new DataFormItem( (*it) );
        m_fields.push_back( i );
      }
    }

    return true;
  }

  Tag* DataForm::tag() const
  {
    if( m_type == FormTypeInvalid )
      return 0;

    Tag *x = new Tag( "x" );
    x->addAttribute( "xmlns", XMLNS_X_DATA );
    if( !m_title.empty() )
      new Tag( x, "title", m_title );

    StringList::const_iterator it_i = m_instructions.begin();
    for( ; it_i != m_instructions.end(); ++it_i )
      new Tag( x, "instructions", (*it_i) );

    FieldList::const_iterator it = m_fields.begin();
    for( ; it != m_fields.end(); ++it )
    {
      DataFormItem *i = dynamic_cast<DataFormItem*>( (*it) );
      if( i )
      {
        x->addChild( i->tag() );
        continue;
      }

      DataFormReported *r = dynamic_cast<DataFormReported*>( (*it) );
      if( r )
      {
        x->addChild( r->tag() );
        continue;
      }

      x->addChild( (*it)->tag() );
    }

    switch( m_type )
    {
      case FormTypeForm:
        x->addAttribute( "type", "form" );
        break;
      case FormTypeSubmit:
        x->addAttribute( "type", "submit" );
        break;
      case FormTypeCancel:
        x->addAttribute( "type", "cancel" );
        break;
      case FormTypeResult:
        x->addAttribute( "type", "result" );
        break;
      default:
        break;
    }

    return x;
  }

}
