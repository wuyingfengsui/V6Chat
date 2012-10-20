/*
  Copyright (c) 2005-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef DATAFORMFIELD_H__
#define DATAFORMFIELD_H__

#include "gloox.h"

#include <utility>
#include <string>

namespace gloox
{

  class Tag;
  /**
   * @brief An abstraction of a single field in a XEP-0004 Data Form.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.7
   */
  class GLOOX_API DataFormField
  {
    public:

      /**
       * Describes the possible type of a Data Form Field.
       */
      enum DataFormFieldType
      {
        FieldTypeBoolean,           /**< The field enables an entity to gather or provide an either-or choice
                                     * between two options. The default value is "false". */
        FieldTypeFixed,             /**< The field is intended for data description (e.g., human-readable text
                                     * such as "section" headers) rather than data gathering or provision. The
                                     * &lt;value/&gt; child SHOULD NOT contain newlines (the \\n and \\r
                                     * characters); instead an application SHOULD generate multiple fixed
                                     * fields, each with one &lt;value/&gt; child. */
        FieldTypeHidden,            /**< The field is not shown to the entity providing information, but
                                     * instead is returned with the form. */
        FieldTypeJidMulti,          /**< The field enables an entity to gather or provide multiple Jabber
                                     * IDs.*/
        FieldTypeJidSingle,         /**< The field enables an entity to gather or provide a single Jabber
                                     * ID.*/
        FieldTypeListMulti,         /**< The field enables an entity to gather or provide one or more options
                                     * from among many. */
        FieldTypeListSingle,        /**< The field enables an entity to gather or provide one option from
                                     * among many. */
        FieldTypeTextMulti,         /**< The field enables an entity to gather or provide multiple lines of
                                     * text. */
        FieldTypeTextPrivate,       /**< The field enables an entity to gather or provide a single line or
                                     * word of text, which shall be obscured in an interface (e.g., *****). */
        FieldTypeTextSingle,        /**< The field enables an entity to gather or provide a single line or
                                     * word of text, which may be shown in an interface. This field type is
                                     * the default and MUST be assumed if an entity receives a field type it
                                     * does not understand.*/
        FieldTypeItem,              /**< The field really is an element of type 'item' which allows for
                                     * addition of sub-fields. See the explanantion in XEP-0004 (Data Forms)
                                     * Section 3.4 for more information */
        FieldTypeReported,          /**< The field really is an element of type 'reported' which allows for
                                     * addition of sub-fields. See the explanantion in XEP-0004 (Data Forms)
                                     * Section 3.4 for more information */
        FieldTypeInvalid,           /**< The field is invalid. Only possible if the field was created from
                                     * a Tag not correctly describing a Data Form Field. */
        FieldTypeNone               /**< The field is child of either a &lt;reported&gt; or &lt;item&gt;
                                     * element or has no type attribute. */
      };

      /**
       * Constructs a new DataForm field.
       * @param type The type of the field. Default: text-single.
       */
      DataFormField( DataFormFieldType type = FieldTypeTextSingle );

      /**
       * Constructs a new DataForm field and fills it with the given values.
       * @param name The field's name (the value of the 'var' attribute).
       * @param value The field's value.
       * @param label The field's label.
       * @param type The field's type.
       * @since 0.9
       */
      DataFormField( const std::string& name, const std::string& value = "",
                     const std::string& label = "", DataFormFieldType type = FieldTypeTextSingle );

      /**
       * Constructs a new Data Form Field from an existing tag that describes a field.
       * @param tag The tag to parse.
       */
      DataFormField( Tag *tag );

      /**
       * Virtual destructor.
       */
      virtual ~DataFormField();

      /**
       * Use this function to retrieve the optional values of a field.
       * @return The options of a field.
       */
      virtual StringMap& options() { return m_options; }

      /**
       * Use this function to create a Tag representation of the form field. This is usually called by
       * DataForm.
       * @return A Tag hierarchically describing the form field, or NULL if the field is invalid (i.e.
       * created from a Tag not correctly describing a Data Form Field).
       */
      virtual Tag* tag() const;

      /**
       * Use this function to retrieve the name of the field (the content of the 'var' attribute).
       * @return The name of the field.
       */
      virtual const std::string& name() const { return m_name; }

      /**
       * Sets the name (the content of the 'var' attribute) of the field. The name identifies the
       * field uniquely in the form.
       * @param name The new name of the field.
       * @note Fields of type other than 'fixed' MUST have a name, if it is 'fixed', it MAY.
       */
      virtual void setName( const std::string& name ) { m_name = name; }

      /**
       * Use this function to set the optional values of the field. The key of the map
       * will be used as the label of the option, while the value will be used as ... the
       * value. ;)
       * @param options The optional values of a list* or *multi type of field.
       */
      virtual void setOptions( const StringMap& options ) { m_options = options; }

      /**
       * Adds a single option to the list of options.
       * @param label The label of the option.
       * @param value The value of the option.
       * @since 0.9.4
       */
      virtual void addOption( const std::string& label, const std::string& value )
        { m_options.insert( std::make_pair( label, value ) ); }

      /**
       * Use this function to determine whether or not this field is required.
       * @return Whether or not this field is required.
       */
      virtual bool required() const { return m_required; }

      /**
       * Use this field to set this field to be required.
       * @param required Whether or not this field is required.
       */
      virtual void setRequired( bool required ) { m_required = required; }

      /**
       * Use this function to retrieve the type of this field.
       * @return The type of this field.
       */
      virtual DataFormFieldType type() const { return m_type; }

      /**
       * Use this function to retrieve the describing label of this field.
       * @return The describing label of this field.
       */
      virtual const std::string& label() const { return m_label; }

      /**
       * Use this function to set the describing label of this field.
       * @param label The describing label of this field.
       */
      virtual void setLabel( const std::string& label ) { m_label = label; }

      /**
       * Use this function to retrieve the description of this field.
       * @return The description of this field
       */
      virtual const std::string& description() const { return m_desc; }

      /**
       * Use this function to set the description of this field.
       * @param desc The description of this field.
       */
      virtual void setDescription( const std::string& desc ) { m_desc = desc; }

      /**
       * Use this function to retrieve the value of this field.
       * @return The value of this field.
       */
      virtual const std::string& value() const { return m_values.front(); }

      /**
       * Use this function to set the value of this field.
       * @param value The new value of this field.
       */
      virtual void setValue( const std::string& value ) { m_values.clear(); addValue( value ); }

      /**
       * Use this function to retrieve the values of this field, if its of type 'text-multi'.
       * @return The value of this field.
       */
      virtual const StringList& values() const { return m_values; }

      /**
       * Use this function to set multiple values of this field, if it is of type 'text-multi'. If its not,
       * use @ref setValue() instead.
       * @param values The new values of this field.
       */
      virtual void setValues( const StringList& values ) { m_values = values; }

      /**
       * Adds a single value to the list of values.
       * @param value The value to add.
       */
      virtual void addValue( const std::string& value ) { m_values.push_back( value ); }

    private:
      StringMap m_options;
      StringList m_values;

      std::string m_name;
      std::string m_desc;
      std::string m_label;
      DataFormFieldType m_type;
      bool m_required;
  };

}

#endif // DATAFORMFIELD_H__
