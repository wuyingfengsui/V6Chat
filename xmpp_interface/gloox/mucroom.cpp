/*
  Copyright (c) 2006-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#include "mucroom.h"
#include "clientbase.h"
#include "dataform.h"
#include "stanza.h"
#include "disco.h"
#include "mucmessagesession.h"

namespace gloox
{

  MUCRoom::MUCRoom( ClientBase *parent, const JID& nick, MUCRoomHandler *mrh,
                    MUCRoomConfigHandler *mrch )
    : m_parent( parent ), m_nick( nick ), m_joined( false ), m_roomHandler( mrh ),
      m_roomConfigHandler( mrch ), m_affiliation( AffiliationNone ), m_role( RoleNone ),
      m_historyType( HistoryUnknown ), m_historyValue( 0 ), m_flags( 0 ),
      m_creationInProgress( false ), m_configChanged( false ),
      m_publishNick( false ), m_publish( false ), m_unique( false )
  {
  }

  MUCRoom::~MUCRoom()
  {
    if( m_joined )
      leave();

    if( m_parent )
    {
      if(m_publish )
        m_parent->disco()->removeNodeHandler( this, XMLNS_MUC_ROOMS );

      m_parent->removeIDHandler( this );
      m_parent->removePresenceHandler( m_nick.bareJID(), this );
      m_parent->disco()->removeDiscoHandler( this );
    }
  }

  void MUCRoom::join()
  {
    if( m_joined || !m_parent )
      return;

    m_parent->registerPresenceHandler( m_nick.bareJID(), this );

    m_session = new MUCMessageSession( m_parent, m_nick.bareJID() );
    m_session->registerMessageHandler( this );

    Stanza *s = Stanza::createPresenceStanza( m_nick );
    Tag *x = new Tag( s, "x" );
    x->addAttribute( "xmlns", XMLNS_MUC );
    if( !m_password.empty() )
      new Tag( x, "password",  m_password );
    if( m_historyType != HistoryUnknown )
    {
      switch( m_historyType )
      {
        case HistoryMaxChars:
        {
          Tag *h = new Tag( x, "history" );
          h->addAttribute( "maxchars", m_historyValue );
          break;
        }
        case HistoryMaxStanzas:
        {
          Tag *h = new Tag( x, "history" );
          h->addAttribute( "maxstanzas", m_historyValue );
          break;
        }
        case HistorySeconds:
        {
          Tag *h = new Tag( x, "history" );
          h->addAttribute( "seconds", m_historyValue );
          break;
        }
        case HistorySince:
        {
          Tag *h = new Tag( x, "history" );
          h->addAttribute( "since", m_historySince );
          break;
        }
        default:
          break;
      }
    }

    if( m_parent )
      m_parent->send( s );

    m_joined = true;
  }

  void MUCRoom::leave( const std::string& msg )
  {
    if( !m_joined )
      return;

    Stanza *s = Stanza::createPresenceStanza( m_nick, msg, PresenceUnavailable );
    Tag *x = new Tag( s, "x" );
    x->addAttribute( "xmlns", XMLNS_MUC );

    if( m_parent )
    {
      m_parent->send( s );

      m_parent->removePresenceHandler( m_nick.bareJID(), this );
      m_parent->disposeMessageSession( m_session );
      m_session = 0;
    }

    m_joined = false;
  }

  void MUCRoom::destroy( const std::string& reason, const JID* alternate, const std::string& password )
  {
    if( !m_parent )
      return;

    Tag *d = new Tag( "destroy" );
    if( alternate )
      d->addAttribute( "jid", alternate->bare() );

    if( !reason.empty() )
      new Tag( d, "reason", reason );

    if( !password.empty() )
      new Tag( d, "password", password );

    const std::string& id = m_parent->getID();

    JID j( m_nick.bare() );
    Stanza *iq = Stanza::createIqStanza( j, id, StanzaIqSet, XMLNS_MUC_OWNER, d );

    m_parent->trackID( this, id, DestroyRoom );
    m_parent->send( iq );
  }

  void MUCRoom::send( const std::string& message )
  {
    if( m_session && m_joined )
      m_session->send( message );
  }

  void MUCRoom::setSubject( const std::string& subject )
  {
    if( m_session && m_joined )
      m_session->setSubject( subject );
  }

  void MUCRoom::setNick( const std::string& nick )
  {
    if( m_parent && m_joined )
    {
      m_newNick = nick;

      Tag *p = new Tag( "presence" );
      p->addAttribute( "to", m_nick.bare() + "/" + m_newNick );
      m_parent->send( p );
    }
    else
      m_nick.setResource( nick );
  }

  void MUCRoom::getRoomInfo()
  {
    if( m_parent )
    {
      JID j( m_nick.bare() );
      m_parent->disco()->getDiscoInfo( j, "", this, GetRoomInfo );
    }
  }

  void MUCRoom::getRoomItems()
  {
    if( m_parent )
    {
      JID j( m_nick.bare() );
      m_parent->disco()->getDiscoItems( j, "", this, GetRoomItems );
    }
  }

  void MUCRoom::setPresence( Presence presence, const std::string& msg )
  {
    if( m_parent && presence != PresenceUnavailable && m_joined )
    {
      Stanza *p = Stanza::createPresenceStanza( m_nick, msg, presence );
      m_parent->send( p );
    }
  }

  void MUCRoom::invite( const JID& invitee, const std::string& reason, bool cont )
  {
    if( !m_parent || !m_joined )
      return;

    Tag *m = new Tag( "message" );
    m->addAttribute( "to", m_nick.bare() );
    Tag *x = new Tag( m, "x" );
    x->addAttribute( "xmlns", XMLNS_MUC_USER );
    Tag *i = new Tag( x, "invite" );
    i->addAttribute( "to", invitee.bare() );
    if( !reason.empty() )
      new Tag( i, "reason", reason );
    if( cont )
      new Tag( i, "continue" );

    m_parent->send( m );
  }

  Stanza* MUCRoom::declineInvitation( const JID& room, const JID& invitor, const std::string& reason )
  {
    Stanza *m = new Stanza( "message" );
    m->addAttribute( "to", room.bare() );
    Tag *x = new Tag( m, "x" );
    x->addAttribute( "xmlns", XMLNS_MUC_USER );
    Tag *d = new Tag( x, "decline" );
    d->addAttribute( "to", invitor.bare() );
    if( !reason.empty() )
      new Tag( d, "reason", reason );

    return m;
  }

  void MUCRoom::setPublish( bool publish, bool publishNick )
  {
    m_publish = publish;
    m_publishNick = publishNick;

    if( !m_parent )
      return;

    if( m_publish )
      m_parent->disco()->registerNodeHandler( this, XMLNS_MUC_ROOMS );
    else
      m_parent->disco()->removeNodeHandler( this, XMLNS_MUC_ROOMS );
  }

  void MUCRoom::addHistory( const std::string& message, const JID& from, const std::string& stamp )
  {
    if( !m_joined || !m_parent )
      return;

    Tag *m = new Tag( "message" );
    m->addAttribute( "to", m_nick.bare() );
    m->addAttribute( "type", "groupchat" );
    new Tag( m, "body", message );
    Tag *x = new Tag( m, "x" );
    x->addAttribute( "xmlns", XMLNS_X_DELAY );
    x->addAttribute( "from", from.full() );
    x->addAttribute( "stamp", stamp );

    m_parent->send( m );
  }

  void MUCRoom::setRequestHistory( int value, MUCRoom::HistoryRequestType type )
  {
    m_historyType = type;
    m_historySince = "";
    m_historyValue = value;
  }

  void MUCRoom::setRequestHistory( const std::string& since )
  {
    m_historyType = HistorySince;
    m_historySince = since;
    m_historyValue = 0;
  }

  Stanza* MUCRoom::createDataForm( const JID& room, const DataForm& df )
  {
    Stanza *m = new Stanza( "message" );
    m->addAttribute( "to", room.bare() );
    m->addChild( df.tag() );

    return m;
  }

  void MUCRoom::requestVoice()
  {
    if( !m_parent || !m_joined )
      return;

    DataForm df( DataForm::FormTypeSubmit );
    DataFormField *field = new DataFormField( DataFormField::FieldTypeNone );
    field->setName( "FORM_TYPE" );
    field->setValue( XMLNS_MUC_REQUEST );
    df.addField( field );
    field = new DataFormField( DataFormField::FieldTypeTextSingle );
    field->setName( "muc#role" );
    field->setLabel( "Requested role" );
    field->setValue( "participant" );
    df.addField( field );

    Tag *m = new Tag( "messsage" );
    m->addAttribute( "to", m_nick.bare() );
    m->addChild( df.tag() );

    m_parent->send( m );
  }

  void MUCRoom::kick( const std::string& nick, const std::string& reason )
  {
    setRole( nick, RoleNone, reason );
  }

  void MUCRoom::grantVoice( const std::string& nick, const std::string& reason )
  {
    setRole( nick, RoleParticipant, reason );
  }

  void MUCRoom::revokeVoice( const std::string& nick, const std::string& reason )
  {
    setRole( nick, RoleVisitor, reason );
  }

  void MUCRoom::ban( const std::string& nick, const std::string& reason )
  {
    setAffiliation( nick, AffiliationOutcast, reason );
  }

  void MUCRoom::setRole( const std::string& nick, MUCRoomRole role, const std::string& reason )
  {
    modifyOccupant( nick, role, "role", reason );
  }

  void MUCRoom::setAffiliation( const std::string& nick, MUCRoomAffiliation affiliation,
                                const std::string& reason )
  {
    modifyOccupant( nick, affiliation, "affiliation", reason );
  }

  void MUCRoom::modifyOccupant( const std::string& nick, int state, const std::string roa,
                                const std::string& reason )
  {
    if( !m_parent || !m_joined || nick.empty() || roa.empty() )
      return;

    std::string newRoA;
    MUCOperation action = SetRNone;
    if( roa == "role" )
    {
      switch( state )
      {
        case RoleNone:
          newRoA = "none";
          action = SetRNone;
          break;
        case RoleVisitor:
          newRoA = "visitor";
          action = SetVisitor;
          break;
        case RoleParticipant:
          newRoA = "participant";
          action = SetParticipant;
          break;
        case RoleModerator:
          newRoA = "moderator";
          action = SetModerator;
          break;
      }
    }
    else
    {
      switch( state )
      {
        case AffiliationOutcast:
          newRoA = "outcast";
          action = SetOutcast;
          break;
        case AffiliationNone:
          newRoA = "none";
          action = SetANone;
          break;
        case AffiliationMember:
          newRoA = "member";
          action = SetMember;
          break;
        case AffiliationAdmin:
          newRoA = "admin";
          action = SetAdmin;
          break;
        case AffiliationOwner:
          newRoA = "owner";
          action = SetOwner;
          break;
      }
    }
    Tag *i = new Tag( "item" );
    i->addAttribute( "nick", nick );
    i->addAttribute( roa, newRoA );
    if( !reason.empty() )
      new Tag( i, "reason", reason );

    const std::string& id = m_parent->getID();
    JID j( m_nick.bare() );
    Stanza *k = Stanza::createIqStanza( j, id, StanzaIqSet, XMLNS_MUC_ADMIN, i );

    m_parent->trackID( this, id, action );
    m_parent->send( k );
  }

  void MUCRoom::requestList( MUCOperation operation )
  {
    if( !m_parent || !m_joined || !m_roomConfigHandler )
      return;

    Tag *i = new Tag( "item" );

    switch( operation )
    {
      case RequestVoiceList:
        i->addAttribute( "role", "participant" );
        break;
      case RequestBanList:
        i->addAttribute( "affiliation", "outcast" );
        break;
      case RequestMemberList:
        i->addAttribute( "affiliation", "member" );
        break;
      case RequestModeratorList:
        i->addAttribute( "role", "moderator" );
        break;
      case RequestOwnerList:
        i->addAttribute( "affiliation", "owner" );
        break;
      case RequestAdminList:
        i->addAttribute( "affiliation", "admin" );
        break;
      default:
        delete i;
        return;
        break;
    }

    const std::string& id = m_parent->getID();
    JID j( m_nick.bare() );
    Stanza *iq = Stanza::createIqStanza( j, id, StanzaIqGet, XMLNS_MUC_ADMIN, i );

    m_parent->trackID( this, id, operation );
    m_parent->send( iq );
  }

  void MUCRoom::storeList( const MUCListItemList items, MUCOperation operation )
  {
    if( !m_parent || !m_joined )
      return;

    std::string roa;
    std::string value;
    switch( operation )
    {
      case RequestVoiceList:
        roa = "role";
        value = "participant";
        break;
      case RequestBanList:
        roa = "affiliation";
        value = "outcast";
        break;
      case RequestMemberList:
        roa = "affiliation";
        value = "member";
        break;
      case RequestModeratorList:
        roa = "role";
        value = "moderator";
        break;
      case RequestOwnerList:
        roa = "affiliation";
        value = "owner";
        break;
      case RequestAdminList:
        roa = "affiliation";
        value = "admin";
        break;
      default:
        return;
        break;
    }

    const std::string& id = m_parent->getID();
    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "id", id );
    iq->addAttribute( "type", "set" );
    iq->addAttribute( "to", m_nick.bare() );
    Tag *q = new Tag( iq, "query" );
    q->addAttribute( "xmlns", XMLNS_MUC_ADMIN );

    MUCListItemList::const_iterator it = items.begin();
    for( ; it != items.end(); ++it )
    {
      if( (*it).nick.empty() )
        continue;

      Tag *i = new Tag( q, "item" );
      i->addAttribute( "nick", (*it).nick );
      i->addAttribute( roa, value );
      if( !(*it).reason.empty() )
        new Tag( i, "reason", (*it).reason );
    }

    m_parent->trackID( this, id, operation );
    m_parent->send( iq );
  }

  void MUCRoom::handlePresence( Stanza *stanza )
  {
    if( ( stanza->from().bare() != m_nick.bare() ) || !m_roomHandler )
      return;

    if( stanza->subtype() == StanzaPresenceError )
    {
      if( m_newNick.empty() )
      {
        m_parent->removePresenceHandler( m_nick.bareJID(), this );
        m_parent->disposeMessageSession( m_session );
        m_joined = false;
        m_session = 0;
      }
      else
        m_newNick = "";
      m_roomHandler->handleMUCError( this, stanza->error() );
    }
    else
    {
      Tag* x = 0;
      if( m_roomHandler && ( x = stanza->findChild( "x", "xmlns", XMLNS_MUC_USER ) ) != 0 )
      {
        MUCRoomParticipant party;
        party.flags = 0;
        party.nick = new JID( stanza->from() );
        party.jid = 0;
        party.actor = 0;
        party.alternate = 0;
        const Tag::TagList& l = x->children();
        Tag::TagList::const_iterator it = l.begin();
        for( ; it != l.end(); ++it )
        {
          if( (*it)->name() == "item" )
          {
            const std::string& affiliation = (*it)->findAttribute( "affiliation" );
            if( affiliation == "owner" )
              party.affiliation = AffiliationOwner;
            else if( affiliation == "admin" )
              party.affiliation = AffiliationAdmin;
            else if( affiliation == "member" )
              party.affiliation = AffiliationMember;
            else if( affiliation == "outcast" )
              party.affiliation = AffiliationOutcast;
            else
              party.affiliation = AffiliationNone;

            const std::string& role = (*it)->findAttribute( "role" );
            if( role == "moderator" )
              party.role = RoleModerator;
            else if( role == "participant" )
              party.role = RoleParticipant;
            else if( role == "visitor" )
              party.role = RoleVisitor;
            else
              party.role = RoleNone;

            const std::string& jid = (*it)->findAttribute( "jid" );
            if( !jid.empty() )
              party.jid = new JID( jid );

            if( (*it)->hasChild( "actor" ) )
            {
              const std::string& actor = (*it)->findChild( "actor" )->findAttribute( "jid" );
              if( !actor.empty() )
                party.actor = new JID( actor );
            }
            if( (*it)->hasChild( "reason" ) )
            {
              party.reason = (*it)->findChild( "reason" )->cdata();
            }
            party.newNick = (*it)->findAttribute( "nick" );
          }
          else if( (*it)->name() == "status" )
          {
            const std::string& code = (*it)->findAttribute( "code" );
            if( code == "100" )
              setNonAnonymous();
            else if( code == "101" )
            {
              // affiliation changed while not in the room. not to be handled here, I guess
            }
            else if( code == "110" )
            {
              party.flags |= UserSelf;
              m_role = party.role;
              m_affiliation = party.affiliation;
            }
            else if( code == "201" )
            {
              m_creationInProgress = true;
              if( instantRoomHook() || m_roomHandler->handleMUCRoomCreation( this ) )
                acknowledgeInstantRoom();
            }
            else if( code == "210" )
              m_nick.setResource( stanza->from().resource() );
            else if( code == "301" )
              party.flags |= UserBanned;
            else if( code == "303" )
              party.flags |= UserNickChanged;
            else if( code == "307" )
              party.flags |= UserKicked;
            else if( code == "321" )
              party.flags |= UserAffiliationChanged;
          }
          else if( (*it)->name() == "destroy" )
          {
            if( (*it)->hasAttribute( "jid" ) )
              party.alternate = new JID( (*it)->findAttribute( "jid" ) );

            if( (*it)->hasChild( "reason" ) )
              party.reason = (*it)->findChild( "reason" )->cdata();

            party.flags |= UserRoomDestroyed;
          }
        }

        if( party.flags & UserNickChanged && !party.newNick.empty()
            && m_nick.resource() == stanza->from().resource()
            && party.newNick == m_newNick )
          party.flags |= UserSelf;

        if( party.flags & UserNickChanged && party.flags & UserSelf && !party.newNick.empty() )
          m_nick.setResource( party.newNick );

        party.status = stanza->status();

        m_roomHandler->handleMUCParticipantPresence( this, party, stanza->presence() );
        delete party.jid;
        delete party.nick;
        delete party.actor;
        delete party.alternate;
      }
    }
  }

  void MUCRoom::acknowledgeInstantRoom()
  {
    if( !m_creationInProgress || !m_parent || !m_joined )
      return;

    Tag *x = new Tag( "x" );
    x->addAttribute( "xmlns", XMLNS_X_DATA );
    x->addAttribute( "type", "submit" );

    JID j( m_nick.bare() );
    const std::string& id = m_parent->getID();
    Stanza *iq = Stanza::createIqStanza( j, id, StanzaIqSet, XMLNS_MUC_OWNER, x );

    m_parent->trackID( this, id, CreateInstantRoom );
    m_parent->send( iq );

    m_creationInProgress = false;
  }

  void MUCRoom::cancelRoomCreation()
  {
    if( !m_creationInProgress || !m_parent || !m_joined )
      return;

    Tag *x = new Tag( "x" );
    x->addAttribute( "xmlns", XMLNS_X_DATA );
    x->addAttribute( "type", "cancel" );

    JID j( m_nick.bare() );
    const std::string& id = m_parent->getID();
    Stanza *iq = Stanza::createIqStanza( j, id, StanzaIqSet, XMLNS_MUC_OWNER, x );

    m_parent->trackID( this, id, CancelRoomCreation );
    m_parent->send( iq );

    m_creationInProgress = false;
  }

  void MUCRoom::requestRoomConfig()
  {
    if( !m_parent || !m_joined )
      return;

    JID j( m_nick.bare() );
    const std::string& id = m_parent->getID();
    Stanza *iq = Stanza::createIqStanza( j, id, StanzaIqGet, XMLNS_MUC_OWNER, 0 );

    m_parent->trackID( this, id, RequestRoomConfig );
    m_parent->send( iq );

    if( m_creationInProgress )
      m_creationInProgress = false;
  }

  void MUCRoom::setRoomConfig( DataForm* form )
  {
    if( !m_parent || !m_joined )
      return;

    JID j( m_nick.bare() );
    const std::string& id = m_parent->getID();
    Stanza *iq = Stanza::createIqStanza( j, id, StanzaIqSet, XMLNS_MUC_OWNER, form->tag() );
    delete form;

    m_parent->trackID( this, id, SendRoomConfig );
    m_parent->send( iq );

    if( m_creationInProgress )
      m_creationInProgress = false;
  }

  void MUCRoom::setNonAnonymous()
  {
    m_flags |= FlagNonAnonymous;
    m_flags &= ~FlagSemiAnonymous;
    m_flags &= ~FlagFullyAnonymous;
  }

  void MUCRoom::setSemiAnonymous()
  {
    m_flags &= ~FlagNonAnonymous;
    m_flags |= FlagSemiAnonymous;
    m_flags &= ~FlagFullyAnonymous;
  }

  void MUCRoom::setFullyAnonymous()
  {
    m_flags &= ~FlagNonAnonymous;
    m_flags &= ~FlagSemiAnonymous;
    m_flags |= FlagFullyAnonymous;
  }

  void MUCRoom::handleMessage( Stanza *stanza, MessageSession * /*session*/ )
  {
    if( !m_roomHandler )
      return;

    if( stanza->subtype() == StanzaMessageError )
    {
      m_roomHandler->handleMUCError( this, stanza->error() );
    }
    else
    {
      Tag *x;
      if( ( x = stanza->findChild( "x", "xmlns", XMLNS_MUC_USER ) ) != 0 )
      {
        const Tag::TagList& l = x->children();
        Tag::TagList::const_iterator it = l.begin();
        for( ; it != l.end(); ++it )
        {
          if( (*it)->name() == "status" )
          {
            const std::string& code = (*it)->findAttribute( "code" );
            if( code == "100" )
            {
              setNonAnonymous();
            }
            else if( code == "104" )
              /*m_configChanged =*/ (void)true;
            else if( code == "170" )
              m_flags |= FlagPublicLogging;
            else if( code == "171" )
              m_flags &= ~FlagPublicLogging;
            else if( code == "172" )
            {
              setNonAnonymous();
            }
            else if( code == "173" )
            {
              setSemiAnonymous();
            }
            else if( code == "174" )
            {
              setFullyAnonymous();
            }
          }
          else if( (*it)->name() == "decline" )
          {
            std::string reason;
            JID invitee( (*it)->findAttribute( "from" ) );
            if( (*it)->hasChild( "reason" ) )
              reason = (*it)->findChild( "reason" )->cdata();
            m_roomHandler->handleMUCInviteDecline( this, invitee, reason );
            return;
          }
          // call some handler?
        }
      }
      else if( m_roomConfigHandler && ( x = stanza->findChild( "x", "xmlns", XMLNS_X_DATA ) ) != 0 )
      {
        DataForm df( x );
        m_roomConfigHandler->handleMUCRequest( this, df );
        return;
      }

      if( !stanza->subject().empty() )
      {
        m_roomHandler->handleMUCSubject( this, stanza->from().resource(), stanza->subject() );
      }
      else if( !stanza->body().empty() )
      {
        JID from;
        std::string when;
        bool privMsg = false;
        bool history = false;
        if( ( x = stanza->findChild( "x", "xmlns", XMLNS_X_DELAY ) ) != 0 )
        {
          from.setJID( x->findAttribute( "from" ) );
          when = x->findAttribute( "stamp" );
          history = true;
        }
        if( stanza->subtype() == StanzaMessageChat ||
            stanza->subtype() == StanzaMessageNormal )
          privMsg = true;

        m_roomHandler->handleMUCMessage( this, stanza->from().resource(), stanza->body(),
                                          history, when, privMsg );
      }
    }
  }

  bool MUCRoom::handleIqID( Stanza *stanza, int context )
  {
    if( !m_roomConfigHandler )
      return false;

    switch( stanza->subtype() )
    {
      case StanzaIqResult:
        return handleIqResult( stanza, context );
        break;
      case StanzaIqError:
        return handleIqError( stanza, context );
        break;
      default:
        break;
    }

    return false;
  }

  bool MUCRoom::handleIqResult( Stanza *stanza, int context )
  {
    switch( context )
    {
      case SetRNone:
      case SetVisitor:
      case SetParticipant:
      case SetModerator:
      case SetANone:
      case SetOutcast:
      case SetMember:
      case SetAdmin:
      case SetOwner:
      case CreateInstantRoom:
      case CancelRoomCreation:
      case DestroyRoom:
      case StoreVoiceList:
      case StoreBanList:
      case StoreMemberList:
      case StoreModeratorList:
      case StoreAdminList:
        m_roomConfigHandler->handleMUCConfigResult( this, true, (MUCOperation)context );
        return true;
        break;
      case RequestRoomConfig:
      {
        Tag *x = 0;
        Tag *q = stanza->findChild( "query", "xmlns", XMLNS_MUC_OWNER );
        if( q )
          x = q->findChild( "x", "xmlns", XMLNS_X_DATA );
        if( x )
        {
          DataForm df( x );
          m_roomConfigHandler->handleMUCConfigForm( this, df );
        }
        return true;
        break;
      }
      case RequestVoiceList:
      case RequestBanList:
      case RequestMemberList:
      case RequestModeratorList:
      case RequestOwnerList:
      case RequestAdminList:
      {
        Tag *x = 0;
        Tag *q = stanza->findChild( "query", "xmlns", XMLNS_MUC_ADMIN );
        if( !q )
          break;
        MUCListItemList itemList;
        const Tag::TagList& items = x->findChildren( "item" );
        Tag::TagList::const_iterator it = items.begin();
        for( ; it != items.end(); ++it )
        {
          MUCListItem item;
          item.jid = 0;
          item.role = getEnumRole( (*it)->findAttribute( "role" ) );
          item.affiliation = getEnumAffiliation( (*it)->findAttribute( "affiliation" ) );
          if( (*it)->hasAttribute( "jid" ) )
            item.jid = new JID( (*it)->findAttribute( "jid" ) );
          item.nick = (*it)->findAttribute( "nick" );
          itemList.push_back( item );
        }
        m_roomConfigHandler->handleMUCConfigList( this, itemList, (MUCOperation)context );

        MUCListItemList::iterator itl = itemList.begin();
        for( ; itl != itemList.end(); ++itl )
          delete (*itl).jid;

        return true;
        break;
      }
      default:
        break;
    }
    return false;
  }

  bool MUCRoom::handleIqError( Stanza * /*stanza*/, int context )
  {
    switch( context )
    {
      case SetRNone:
      case SetVisitor:
      case SetParticipant:
      case SetModerator:
      case SetANone:
      case SetOutcast:
      case SetMember:
      case SetAdmin:
      case SetOwner:
      case CreateInstantRoom:
      case CancelRoomCreation:
      case RequestRoomConfig:
      case DestroyRoom:
      case RequestVoiceList:
      case StoreVoiceList:
      case RequestBanList:
      case StoreBanList:
      case RequestMemberList:
      case StoreMemberList:
      case RequestModeratorList:
      case StoreModeratorList:
      case RequestOwnerList:
      case StoreOwnerList:
      case RequestAdminList:
      case StoreAdminList:
        m_roomConfigHandler->handleMUCConfigResult( this, false, (MUCOperation)context );
        break;
    }
    return false;
  }

  void MUCRoom::handleDiscoInfoResult( Stanza *stanza, int context )
  {
    switch( context )
    {
      case GetRoomInfo:
      {
        int oldflags = m_flags;
        m_flags = 0;
        if( oldflags & FlagPublicLogging )
          m_flags |= FlagPublicLogging;

        std::string name;
        DataForm *df = 0;
        Tag *q = stanza->findChild( "query" );
        if( q )
        {
          const Tag::TagList& l = q->children();
          Tag::TagList::const_iterator it = l.begin();
          for( ; it != l.end(); ++it )
          {
            if( (*it)->name() == "feature" )
            {
              if( (*it)->findAttribute( "var" ) == "muc_hidden" )
                m_flags |= FlagHidden;
              else if( (*it)->findAttribute( "var" ) == "muc_membersonly" )
                m_flags |= FlagMembersOnly;
              else if( (*it)->findAttribute( "var" ) == "muc_moderated" )
                m_flags |= FlagModerated;
              else if( (*it)->findAttribute( "var" ) == "muc_nonanonymous" )
                setNonAnonymous();
              else if( (*it)->findAttribute( "var" ) == "muc_open" )
                m_flags |= FlagOpen;
              else if( (*it)->findAttribute( "var" ) == "muc_passwordprotected" )
                m_flags |= FlagPasswordProtected;
              else if( (*it)->findAttribute( "var" ) == "muc_persistent" )
                m_flags |= FlagPersistent;
              else if( (*it)->findAttribute( "var" ) == "muc_public" )
                m_flags |= FlagPublic;
              else if( (*it)->findAttribute( "var" ) == "muc_semianonymous" )
                setSemiAnonymous();
              else if( (*it)->findAttribute( "var" ) == "muc_temporary" )
                m_flags |= FlagTemporary;
              else if( (*it)->findAttribute( "var" ) == "muc_fullyanonymous" )
                setFullyAnonymous();
              else if( (*it)->findAttribute( "var" ) == "muc_unmoderated" )
                m_flags |= FlagUnmoderated;
              else if( (*it)->findAttribute( "var" ) == "muc_unsecured" )
                m_flags |= FlagUnsecured;
            }
            else if( (*it)->name() == "identity" )
            {
              name = (*it)->findAttribute( "name" );
            }
            else if( (*it)->name() == "x" && (*it)->hasAttribute( "xmlns", XMLNS_X_DATA ) )
            {
              df = new DataForm( (*it) );
            }
          }
        }
        if( m_roomHandler )
          m_roomHandler->handleMUCInfo( this, m_flags, name, df );
        break;
      }
      default:
        break;
    }
  }

  void MUCRoom::handleDiscoItemsResult( Stanza *stanza, int context )
  {
    if( !m_roomHandler )
      return;

    switch( context )
    {
      case GetRoomItems:
      {
        Tag *q = stanza->findChild( "query" );
        if( q )
        {
          StringMap items;
          const Tag::TagList& l = q->children();
          Tag::TagList::const_iterator it = l.begin();
          for( ; it != l.end(); ++it )
          {
            if( (*it)->name() == "item" && (*it)->hasAttribute( "jid" ) )
            {
              items[(*it)->findAttribute( "name" )] = (*it)->findAttribute( "jid" );
            }
          }
          m_roomHandler->handleMUCItems( this, items );
        }
        break;
      }
      default:
        break;
    }
  }

  void MUCRoom::handleDiscoError( Stanza * /*stanza*/, int context )
  {
    if( !m_roomHandler )
      return;

    switch( context )
    {
      case GetRoomInfo:
        m_roomHandler->handleMUCInfo( this, 0, "", 0 );
        break;
      case GetRoomItems:
      {
        StringMap items;
        m_roomHandler->handleMUCItems( this, items );
        break;
      }
      default:
        break;
    }
  }

  StringList MUCRoom::handleDiscoNodeFeatures( const std::string& /*node*/ )
  {
    return StringList();
  }

  StringMap MUCRoom::handleDiscoNodeIdentities( const std::string& /*node*/, std::string& /*name*/ )
  {
    return StringMap();
  }

  DiscoNodeItemList MUCRoom::handleDiscoNodeItems( const std::string& node )
  {
    DiscoNodeItemList l;

    if( node != XMLNS_MUC_ROOMS )
      return l;

    if( m_publish )
    {
      DiscoNodeItem item;
      item.jid = m_nick.bare();
      if( m_publishNick )
        item.name = m_nick.resource();
      l.push_back( item );
    }
    return l;
  }

  MUCRoomRole MUCRoom::getEnumRole( const std::string& role )
  {
    if( role == "moderator" )
      return RoleModerator;
    if( role == "participant" )
      return RoleParticipant;
    if( role == "visitor" )
      return RoleVisitor;
    return RoleNone;
  }

  MUCRoomAffiliation MUCRoom::getEnumAffiliation( const std::string& affiliation )
  {
    if( affiliation == "owner" )
      return AffiliationOwner;
    if( affiliation == "admin" )
      return AffiliationAdmin;
    if( affiliation == "member" )
      return AffiliationMember;
    if( affiliation == "outcast" )
      return AffiliationOutcast;
    return AffiliationNone;
  }

}
