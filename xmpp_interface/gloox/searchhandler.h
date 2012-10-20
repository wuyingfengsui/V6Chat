/*
  Copyright (c) 2006-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef SEARCHHANDLER_H__
#define SEARCHHANDLER_H__

#include "stanza.h"
#include "dataform.h"

#include <string>

namespace gloox
{

  /**
   * Holds all the possible fields a server may require for searching according
   * to Section 7, XEP-0055.
   */
  struct SearchFieldStruct
  {
    std::string first;              /**< User's first name. */
    std::string last;               /**< User's last name. */
    std::string nick;               /**< User's nickname. */
    std::string email;              /**< User's email. */
    JID jid;                        /**< User's JID. */
  };

  /**
   * The possible fields of a XEP-0055 user search.
   */
  enum SearchFieldEnum
  {
    SearchFieldFirst    = 1,        /**< Search in first names. */
    SearchFieldLast     = 2,        /**< Search in last names. */
    SearchFieldNick     = 4,        /**< Search in nicknames. */
    SearchFieldEmail    = 8         /**< Search in email addresses. */
  };

  /**
   * A list of directory entries returned by a search.
   */
  typedef std::list<SearchFieldStruct> SearchResultList;

  /**
   * @brief A virtual interface that enables objects to receive Jabber Search (XEP-0055) results.
   *
   * A class implementing this interface can receive the result of a Jabber Search.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.8.5
   */
  class GLOOX_API SearchHandler
  {
    public:
      /**
       * Virtual Destructor.
       */
      virtual ~SearchHandler() {}

      /**
       * This function is called to announce the searchable fields a directory supports. It is the result
       * of a call to @link gloox::Search::fetchSearchFields Search::fetchSearchFields() @endlink.
       * @param directory The directory that was queried.
       * @param fields Bit-wise ORed SearchFieldEnum values.
       * @param instructions Plain-text instructions for the end user.
       */
      virtual void handleSearchFields( const JID& directory, int fields,
                                       const std::string& instructions ) = 0;

      /**
       * This function is called to announce the searchable fields a directory supports. It is the result
       * of a call to @link gloox::Search::fetchSearchFields Search::fetchSearchFields() @endlink.
       * @note The SearchHandler is responsible for deleting the DataForm.
       * @param directory The directory that was queried.
       * @param form A DataForm describing the valid searchable fields.
       */
      virtual void handleSearchFields( const JID& directory, DataForm *form ) = 0;

     /**
      * This function is called to let the SearchHandler know about the results of the search.
      * @param directory The searched directory.
      * @param resultList A list of SearchFieldStructs. May be empty.
      */
      virtual void handleSearchResult( const JID& directory, const SearchResultList& resultList ) = 0;

      /**
       * This function is called to let the SearchHandler know about the result of the search.
       * @note The SearchHandler is responsible for deleting the DataForm.
       * @param directory The searched directory.
       * @param form A DataForm containing the search results.
       */
      virtual void handleSearchResult( const JID& directory, const DataForm *form ) = 0;

      /**
       * This function is called if a error occured as a result to a search or search field request.
       * @param directory The queried/searched directory.
       * @param stanza The full error stanza.
       */
      virtual void handleSearchError( const JID& directory, Stanza *stanza ) = 0;

  };

}

#endif // SEARCHHANDLER_H__
