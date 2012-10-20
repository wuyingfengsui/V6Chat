/*
  Copyright (c) 2005-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef DATAFORMBASE_H__
#define DATAFORMBASE_H__

#include "dataformfield.h"

#include <string>
#include <list>

namespace gloox
{

  /**
   * @brief An abstract base class for a XEP-0004 Data Form.
   *
   * You shouldn't need to use this class directly. Use DataForm instead.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.7
   */
  class GLOOX_API DataFormBase
  {
    public:
      /**
       * Creates a new DataFormBase.
       */
      DataFormBase();

      /**
       * Virtual destructor.
       */
      virtual ~DataFormBase();

      /**
       * A list of XEP-0004 Data Form Fields.
       */
      typedef std::list<DataFormField*> FieldList;

      /**
       * Use this function to check whether this form contains a field with the given name.
       * @param field The name of the field (the content of the 'var' attribute).
       * @return Whether or not the form contains the named field.
       */
      bool hasField( const std::string& field );

      /**
       * Use this function to fetch a pointer to a field of the form. If no such field exists,
       * 0 is returned.
       * @param field The name of the field (the content of the 'var' attribute).
       * @return A copy of the field with the given name if it exists, 0 otherwise.
       */
      DataFormField* field( const std::string& field );

      /**
       * Use this function to retrieve the list of fields of a form.
       * @return The list of fields the form contains.
       */
      FieldList& fields() { return m_fields; }

      /**
       * Use this function to set the fields the form contains.
       * @param fields The list of fields.
       * @note Any previously set fields will be deleted. Always set all fields, not a delta.
       */
      virtual void setFields( FieldList& fields ) { m_fields = fields; }

      /**
       * Use this function to add a single field to the list of existing fields.
       * @param field The field to add.
       * @since 0.9
       */
      virtual void addField( DataFormField *field ) { m_fields.push_back( field ); }

      /**
       * Adds a single new Field and returns a pointer to that field.
       * @param type The field's type.
       * @param name The field's name (the value of the 'var' attribute).
       * @param value The field's value.
       * @param label The field's label.
       * @since 0.9.4
       */
      DataFormField* addField( DataFormField::DataFormFieldType type, const std::string& name,
                               const std::string& value = "", const std::string& label = "" )
      { m_fields.push_back( new DataFormField( name, value, label, type ) ); return m_fields.back(); }

    protected:
      FieldList m_fields;

  };

}

#endif // DATAFORMBASE_H__
