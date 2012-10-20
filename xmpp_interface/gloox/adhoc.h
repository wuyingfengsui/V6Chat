/*
  Copyright (c) 2004-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef ADHOC_H__
#define ADHOC_H__

#include "disconodehandler.h"
#include "discohandler.h"
#include "iqhandler.h"

#include <string>
#include <list>
#include <map>

namespace gloox
{

  class DataForm;
  class ClientBase;
  class Stanza;
  class AdhocHandler;
  class AdhocCommandProvider;

  /**
   * @brief This class implements a provider for XEP-0050 (Ad-hoc Commands).
   *
   * The current, not complete, implementation is probably best suited for fire-and-forget
   * type of commands. Any additional feature, like multiple stages, etc., would have to be
   * added separately.
   *
   * To offer commands to remote entities, use this class as follows:<br>
   * Create a class that will handle command execution requests and derive it from
   * AdhocCommandProvider. Instantiate an Adhoc object and register your
   * AdhocCommandProvider-derived object with the Adhoc object using
   * registerAdhocCommandProvider(). The additional parameters to that method are the internal
   * name of the command as used in the code, and the public name of the command as it
   * will be shown to an end user:
   * @code
   * MyClass::someFunc()
   * {
   *   Adhoc *m_adhoc = new Adhoc( m_client );
   *
   *   // this might be a bot monitoring a weather station, for example
   *   m_adhoc->registerAdhocCommandProvider( this, "getTemp", "Retrieve current temperature" );
   *   m_adhoc->registerAdhocCommandProvider( this, "getPressure", "Retrieve current air pressure" );
   *   [...]
   * }
   * @endcode
   * In this example, MyClass is AdhocCommandProvider-derived so it is obviously the command handler, too.
   *
   * And that's about it you can do with the Adhoc class. Of course you can have a AdhocCommandProvider
   * handle more than one command, just register it with the Adhoc object for every desired command,
   * like shown above.
   *
   * What the Adhoc object does when you install a new command is tell the supplied Disco object
   * to advertise these commands to clients using the 'Service Discovery' protocol to learn about
   * this implementation's features. These clients can then call and execute the command. Of course you
   * are free to implement access restrictions to not let anyone mess with your bot, for example.
   * However, the commands offered using Service Discovery are publically visible in any case.
   *
   * To execute commands offered by a remote entity:<br>
   * ...TBC...
   *
   * XEP version: 1.2
   * @author Jakob Schroeter <js@camaya.net>
   */
  class GLOOX_API Adhoc : public DiscoNodeHandler, public DiscoHandler, public IqHandler
  {
    public:
      /**
       * The current status of a command.
       */
      enum AdhocCommandStatus
      {
        AdhocCommandExecuting,      /**< The command is being executed. */
        AdhocCommandCompleted,      /**< The command has completed. The command session has ended. */
        AdhocCommandCanceled,       /**< The command has been canceled. The command session has ended. */
        AdhocCommandStatusUnknown   /**< None or unknown status. */
      };

      /**
       * Describes actions to jump between execution stages and dataform pages.
       */
      enum AdhocExecuteActions
      {
        ActionDefault    =  0,      /**< The default action is being executed. */
        ActionPrevious   =  1,      /**< Request previous page. */
        ActionNext       =  2,      /**< Request next page. */
        ActionComplete   =  4,      /**< Complete or finish the execution. */
        ActionCancel     =  8       /**< Cancel command execution. */
      };

      /**
       * Describes the type of a note attached to a execution stage.
       */
      enum AdhocNoteType
      {
        AdhocNoteInfo,              /**< The note is informational only. This is not really
                                     * an exceptional condition. */
        AdhocNoteWarn,              /**< The note indicates a warning. Possibly due to
                                     * illogical (yet valid) data. */
        AdhocNoteError              /**< The note indicates an error. The text should indicate
                                     * the reason for the error. */
      };

      /**
       * Constructor.
       * Creates a new Adhoc client that registers as IqHandler with a ClientBase.
       * @param parent The ClientBase used for XMPP communication.
       */
      Adhoc( ClientBase *parent );

      /**
       * Virtual destructor.
       */
      virtual ~Adhoc();

      // reimplemented from DiscoNodeHandler
      virtual StringList handleDiscoNodeFeatures( const std::string& node );

      // reimplemented from DiscoNodeHandler
      virtual StringMap handleDiscoNodeIdentities( const std::string& node, std::string& name );

      // reimplemented from DiscoNodeHandler
      virtual DiscoNodeItemList handleDiscoNodeItems( const std::string& node );

      // reimplemented from IqHandler
      virtual bool handleIq( Stanza *stanza );

      // reimplemented from IqHandler
      virtual bool handleIqID( Stanza *stanza, int context );

      // reimplemented from DiscoHandler
      virtual void handleDiscoInfoResult( Stanza *stanza, int context );

      // reimplemented from DiscoHandler
      virtual void handleDiscoItemsResult( Stanza *stanza, int context );

      // reimplemented from DiscoHandler
      virtual void handleDiscoError( Stanza *stanza, int context );

      /**
       * Using this function, you can register a AdhocCommandProvider -derived object as
       * handler for a specific Ad-hoc Command as defined in XEP-0050.
       * @param acp The obejct to register as handler for the specified command.
       * @param command The node name of the command. Will be announced in disco#items.
       * @param name The natural-language name of the command. Will be announced in disco#items.
       */
      void registerAdhocCommandProvider( AdhocCommandProvider *acp, const std::string& command,
                                         const std::string& name );

      /**
       * This function queries the given remote entity for Adhoc Commands support.
       * @param remote The remote entity's JID.
       * @param ah The object handling the result of this request.
       */
      void checkSupport( const JID& remote, AdhocHandler *ah );

      /**
       * Retrieves a list of commands from the remote entity. You should check whether the remote
       * entity actually supports Adhoc Commands by means of checkSupport().
       * @param remote The remote entity's JID.
       * @param ah The object handling the result of this request.
       */
      void getCommands( const JID& remote, AdhocHandler *ah );

      /**
       * Executes the given command on the given remote entity.
       * For initial execution requests, only the first three parameters are required. For
       * subsequent requests (of a multiple stages request) at least @b sessionid and
       * @b form should be provided (depending on the command being executed, of course).
       * @param remote The remote entity's JID.
       * @param command The command to execute.
       * @param ah The object handling the result of this request.
       * @param sessionid The sessionid identifying the command currenly being executed. Must be
       * empty on first request.
       * @param form A DataForm containing the result of a previous response. Must be left empty
       * on first request.
       * @param action The action to take, e.g. navigatte o the previous 'screen'.
       */
      void execute( const JID& remote, const std::string& command, AdhocHandler *ah,
                    const std::string& sessionid = "", DataForm *form = 0,
                    AdhocExecuteActions action = ActionDefault );

      /**
       * Use this function to unregister an adhoc command previously registered using
       * registerAdhocCommandProvider().
       * @param command The command to unregister.
       */
      void removeAdhocCommandProvider( const std::string& command );

    private:
      typedef std::map<const std::string, AdhocCommandProvider*> AdhocCommandProviderMap;
      AdhocCommandProviderMap m_adhocCommandProviders;

      enum AdhocContext
      {
        CheckAdhocSupport,
        FetchAdhocCommands,
        ExecuteAdhocCommand
      };

      struct TrackStruct
      {
        JID remote;
        AdhocContext context;
        AdhocHandler *ah;
      };
      typedef std::map<std::string, TrackStruct> AdhocTrackMap;
      AdhocTrackMap m_adhocTrackMap;

      ClientBase *m_parent;

      StringMap m_items;

  };

}

#endif // ADHOC_H__
