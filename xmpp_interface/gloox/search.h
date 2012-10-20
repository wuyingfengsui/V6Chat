/*
  Copyright (c) 2006-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef SEARCH_H__
#define SEARCH_H__

#include "gloox.h"
#include "searchhandler.h"
#include "discohandler.h"
#include "iqhandler.h"

#include <string>

namespace gloox
{

  class ClientBase;
  class Stanza;
  class Disco;

  /**
   * @brief An implementation of XEP-0055 (Jabber Search)
   *
   * To perform a search in a directory (e.g., a User Directory):
   *
   * @li Inherit from SearchHandler and implement the virtual functions.
   * @li Create a new Search object.
   * @li Ask the directory for the supported fields using fetchSearchFields(). Depending on the directory,
   * the result can be either an integer (bit-wise ORed supported fields) or a DataForm.
   * @li Search by either using a DataForm or the SearchFieldStruct.
   * @li The results can be either a (empty) list of SearchFieldStructs or a DataForm.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.8.5
   */
  class GLOOX_API Search : public IqHandler
  {

    public:
      /**
       * Creates a new Search object.
       * @param parent The ClientBase to use.
       */
      Search( ClientBase *parent );

      /**
       * Virtual Destructor.
       */
      ~Search();

      /**
       * Use this function to check which fields the directory supports.
       * @param directory The (user) directory to fetch the available/searchable fields from.
       * @param sh The SearchHandler to notify about the results.
       */
      void fetchSearchFields( const JID& directory, SearchHandler *sh );

      /**
       * Initiates a search on the given directory, with the given data form. The given SearchHandler
       * is notified about the results.
       * @param directory The (user) directory to search.
       * @param form The DataForm contains the phrases the user wishes to search for.
       * @param sh The SearchHandler to notify about the results.
       */
      void search( const JID& directory, const DataForm& form, SearchHandler *sh );

      /**
       * Initiates a search on the given directory, with the given phrases. The given SearchHandler
       * is notified about the results.
       * @param directory The (user) directory to search.
       * @param fields Bit-wise ORed FieldEnum values describing the valid (i.e., set) fields in
       * the @b values parameter.
       * @param values Contains the phrases to search for.
       * @param sh The SearchHandler to notify about the results.
       */
      void search( const JID& directory, int fields, const SearchFieldStruct& values, SearchHandler *sh );

      // reimplemented from IqHandler
      virtual bool handleIq( Stanza *stanza ) { (void) stanza; return false; }

      // reimplemented from IqHandler
      virtual bool handleIqID( Stanza *stanza, int context );

    protected:
      enum IdType
      {
        FetchSearchFields,
        DoSearch
      };

      typedef std::map<std::string, SearchHandler*> TrackMap;
      TrackMap m_track;

      ClientBase *m_parent;
      Disco *m_disco;

  };

}

#endif // SEARCH_H__
