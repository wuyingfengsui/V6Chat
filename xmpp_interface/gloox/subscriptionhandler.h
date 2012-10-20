/*
  Copyright (c) 2004-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef SUBSCRIPTIONHANDLER_H__
#define SUBSCRIPTIONHANDLER_H__

#include "stanza.h"

namespace gloox
{

  /**
   * @brief A virtual interface which can be reimplemented to receive incoming subscription stanzas.
   *
   * Derived classes can be registered as SubscriptionHandlers with the Client.
   * Upon an incoming Subscription packet @ref handleSubscription() will be called.
   * @author Jakob Schroeter <js@camaya.net>
   */
  class GLOOX_API SubscriptionHandler
  {
    public:
      /**
       * Virtual destructor.
       */
      virtual ~SubscriptionHandler() {}

      /**
       * Reimplement this function if you want to be notified about incoming
       * subscriptions/subscription requests.
       * @param stanza The complete Stanza.
       */
      virtual void handleSubscription( Stanza *stanza ) = 0;
  };

}

#endif // SUBSCRIPTIONHANDLER_H__
