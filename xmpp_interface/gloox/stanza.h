/*
  Copyright (c) 2005-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef STANZA_H__
#define STANZA_H__

#include "gloox.h"
#include "tag.h"
#include "jid.h"

namespace gloox
{

  class StanzaExtension;

  /**
   * A list of StanzaExtensions.
   */
  typedef std::list<StanzaExtension*> StanzaExtensionList;

  /**
   * @brief This is an abstraction of a XMPP stanza.
   *
   * You can create a new Stanza from an existing Tag (or another stanza).
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.4
   */
  class GLOOX_API Stanza : public Tag
  {
    public:
      /**
       * Creates a new Stanza from a deep copy of the given Tag.
       * @param tag The Tag to create the Stanza from.
       * @since 0.7
       * @note While the signature of this constructor in 0.7 is the the same as in older versions,
       * semantics have changed. The copy created by this constructor is no longer a shallow one. You have to
       * make sure the copy is deleted properly.
       */
      Stanza( const Tag *tag );

      /**
       * Creates a new Stanza with given name and optional CData.
       * @param name The name of the root tag.
       * @param cdata Initial XML character data for the tag.
       * @param xmllang The value of the xmllang attribute. The stanza's primary language.
       * @param incoming Indicates whether tag names, attributes, attribute values, and cdata shall
       * be escaped (false, default) or not (true).
       */
      Stanza( const std::string& name, const std::string& cdata = "",
              const std::string& xmllang = "default", bool incoming = false );

      /**
       * Virtual destructor.
       */
      virtual ~Stanza();

      /**
       * Returns the sub-type of the stanza.
       * @return The sub-type of the stanza.
       */
      virtual StanzaSubType subtype() const { return m_subtype; }

      /**
       * Returns the JID the stanza comes from.
       * @return The origin of the stanza.
       */
      virtual const JID& from() const { return m_from; }

      /**
       * Returns the receiver of the stanza.
       * @return The stanza's destination.
       */
      virtual const JID& to() const { return m_to; }

      /**
       * Returns the id of the stanza, if set.
       * @return The ID of the stanza.
       */
      virtual const std::string& id() const { return m_id; }

      /**
       * Returns the value of the xmlns attribute of the first child node.
       * @return The namespace of the IQ stanza.
       */
      virtual const std::string& xmlns() const { return m_xmlns; }

      /**
       * Returns the presence 'show' type of a presence stanza.
       * @return The presence type of the sender.
       */
      virtual Presence presence() const { return m_presence; }

      /**
       * Returns the remote entity resource's presence priority if the stanza is a presence stanza.
       * If the stanza is not a presence stanza or if no priority information was included, a value
       * below -128 is returned, which is an illegal value for the priority. Legal range is between
       * -128 and +127.
       * @return The priority information contained in the stanza, if any, or a value below -128.
       */
      virtual int priority() const { return m_priority; }

      /**
       * Returns the status text of a presence stanza for the given language if available.
       * If the requested language is not available, the default status text (without a xml:lang
       * attribute) will be returned.
       * @param lang The language identifier for the desired language. It must conform to
       * section 2.12 of the XML specification and RFC 3066. If empty, the default body
       * will be returned, if any.
       * @return The status text set by the sender.
       */
      virtual const std::string status( const std::string& lang = "default" ) const
        { return findLang( m_status, lang ); }

      /**
       * Returns the body of a message stanza for the given language if available.
       * If the requested language is not available, the default body (without a xml:lang
       * attribute) will be returned.
       * @param lang The language identifier for the desired language. It must conform to
       * section 2.12 of the XML specification and RFC 3066. If empty, the default body
       * will be returned, if any.
       * @return The body of a message stanza. Empty for non-message stanzas.
       */
      virtual const std::string body( const std::string& lang = "default" ) const
        { return findLang( m_body, lang ); }

      /**
       * Returns the subject of a message stanza for the given language if available.
       * If the requested language is not available, the default subject (without a xml:lang
       * attribute) will be returned.
       * @param lang The language identifier for the desired language. It must conform to
       * section 2.12 of the XML specification and RFC 3066. If empty, the default subject
       * will be returned, if any.
       * @return The subject of a message stanza. Empty for non-message stanzas.
       */
      virtual const std::string subject( const std::string& lang = "default" ) const
        { return findLang( m_subject, lang ); }

      /**
       * Returns the text of a error stanza for the given language if available.
       * If the requested language is not available, the default text (without a xml:lang
       * attribute) will be returned.
       * @param lang The language identifier for the desired language. It must conform to
       * section 2.12 of the XML specification and RFC 3066. If empty, the default subject
       * will be returned, if any.
       * @return The text of an error stanza. Empty for non-error stanzas.
       */
      virtual const std::string errorText( const std::string& lang = "default" ) const
        { return findLang( m_errorText, lang ); }

      /**
       * Returns the stanza error condition, if any.
       * @return The stanza error condition.
       */
      virtual StanzaError error() const { return m_stanzaError; }

      /**
       * This function can be used to retrieve the application-specific error condition of a stanza error.
       * @return The application-specific error element of a stanza error. 0 if no respective element was
       * found or no error occured.
       */
      Tag* errorAppCondition() { return m_stanzaErrorAppCondition; }

      /**
       * Returns the thread ID of a message stanza.
       * @return The thread ID of a message stanza. Empty for non-message stanzas.
       */
      virtual const std::string& thread() const { return m_thread; }

      /**
       * Sets the Stanza's thread ID. Only useful for message stanzas.
       * @param thread The thread ID.
       * @since 0.9
       */
      void setThread( const std::string& thread ) { m_thread = thread; }

      /**
       * Retrieves the value of the xml:lang attribute of this stanza.
       * Default is 'en'.
       */
      const std::string& xmlLang() const { return m_xmllang; }

      /**
       * Use this function to parse the content of the Tag and determine type, etc.
       * of the Stanza. This feels kind of hackish...
       * You only need to call this if you are constructing a bare Stanza from scratch.
       * Stanzas provided by gloox are fully parsed.
       * @deprecated
       */
      void finalize() { init(); }

      /**
       * Use this function to add a StanzaExtension to this Stanza.
       * @param se The StanzaExtension to add.
       * @note The Stanza will become the owner of the StanzaExtension and will delete it
       * after using it.
       * @since 0.9
       */
      void addExtension( StanzaExtension *se );

      /**
       * Returns the list of the Stanza's extensions.
       * @return The list of the Stanza's extensions.
       */
      const StanzaExtensionList& extensions() const { return m_extensionList; }

      /**
       * Creates a new IQ stanza.
       * @param to The receiver of the stanza.
       * @param id An ID for the stanza. Best is to use ClientBase::getID() as input.
       * @param subtype The IQ type. Only StanzaIq* types are valid.
       * @param xmlns If this is non-empty, a child element named 'query' will be included, with this
       * value as value of the 'xmlns' attribute.
       * @param tag If this if not NULL, and xmlns is not empty, this Tag will be included as child tag of
       * the 'query' element.
       * @since 0.7
       */
      static Stanza* createIqStanza( const JID& to, const std::string& id,
                                     StanzaSubType subtype = StanzaIqGet,
                                     const std::string& xmlns = "", Tag* tag = 0 );

      /**
       * Creates a new presence stanza.
       * @c to can be an empty JID. This makes the created stanza a broadcast stanza sent to all
       * contacts in the roster.
       * @param to The receiver of the stanza.
       * @param msg An optional message.
       * @param status The status.
       * @param xmllang The status message's language.
       * @since 0.7
       */
      static Stanza* createPresenceStanza( const JID& to, const std::string& msg = "",
                                           Presence status = PresenceAvailable,
                                           const std::string& xmllang = "" );

      /**
       * Creates a new message stanza.
       * @param to The receiver of the message.
       * @param body The message's body.
       * @param subtype The message type. Only StanzaMessage* types are valid.
       * @param subject The message's subject.
       * @param thread The message's conversation thread id.
       * @param xmllang The message's language.
       * @since 0.7
       */
      static Stanza* createMessageStanza( const JID& to, const std::string& body,
                                          StanzaSubType subtype = StanzaMessageChat,
                                          const std::string& subject = "", const std::string& thread = "",
                                          const std::string& xmllang = "" );

      /**
       * Creates a new subscription stanza.
       * @param to The recipient of the subscription stanza.
       * @param msg An optional message.
       * @param subtype The subscription type. Only StanzaS10n* types are vaild.
       * @param xmllang The message's language.
       * @since 0.7
       */
      static Stanza* createSubscriptionStanza( const JID& to, const std::string& msg = "",
                                               StanzaSubType subtype = StanzaS10nSubscribe,
                                               const std::string& xmllang = "" );

    protected:
      void init();

      StanzaExtensionList m_extensionList;
      StanzaSubType m_subtype;
      Presence m_presence;
      StanzaError m_stanzaError;
      StanzaErrorType m_stanzaErrorType;
      Tag *m_stanzaErrorAppCondition;
      StringMap m_errorText;
      StringMap m_body;
      StringMap m_subject;
      StringMap m_status;
      JID m_from;
      JID m_to;
      std::string m_xmlns;
      std::string m_id;
      std::string m_thread;
      std::string m_xmllang;
      int m_priority;

      static const std::string findLang( const StringMap& map, const std::string& lang );
      static void setLang( StringMap& map, const Tag *tag );
  };

}

#endif // STANZA_H__
