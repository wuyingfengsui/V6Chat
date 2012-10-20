/*
  Copyright (c) 2006-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "inbandbytestream.h"
#include "inbandbytestreamdatahandler.h"
#include "messagesession.h"
#include "disco.h"
#include "clientbase.h"
#include "base64.h"

#include <sstream>

namespace gloox
{

  InBandBytestream::InBandBytestream( MessageSession *session, ClientBase *clientbase )
    : MessageFilter( session ), m_clientbase( clientbase ),
      m_inbandBytestreamDataHandler( 0 ), m_blockSize( 4096 ), m_sequence( -1 ),
      m_lastChunkReceived( -1 ), m_open( true )
  {
  }

  InBandBytestream::~InBandBytestream()
  {
    if( m_open )
      close();

    if( m_parent )
      m_parent->removeMessageFilter( this );
  }

  void InBandBytestream::decorate( Tag * /*tag*/ )
  {
  }

  void InBandBytestream::filter( Stanza *stanza )
  {
    if( !m_inbandBytestreamDataHandler || !m_open )
      return;

    if( stanza->subtype() == StanzaMessageError )
    {
      m_inbandBytestreamDataHandler->handleInBandError( m_sid, stanza->from(), stanza->error() );
      m_open = false;
    }

    Tag *data = 0;
    if( ( data = stanza->findChild( "data", "xmlns", XMLNS_IBB ) ) == 0 )
      return;

    const std::string& sid = data->findAttribute( "sid" );
    if( sid.empty() || sid != m_sid )
      return;

    const std::string& seq = data->findAttribute( "seq" );
    if( seq.empty() )
    {
      m_open = false;
      return;
    }

    std::stringstream str;
    int sequence = 0;
    str << seq;
    str >> sequence;

    if( m_lastChunkReceived + 1 != sequence )
    {
      m_open = false;
      return;
    }
    m_lastChunkReceived = sequence;

    if( !data->cdata().length() )
    {
      m_open = false;
      return;
    }

    m_inbandBytestreamDataHandler->handleInBandData( Base64::decode64( data->cdata() ), sid );
  }

  bool InBandBytestream::sendBlock( const std::string& data )
  {
    if( !m_open || !m_parent || !m_clientbase || data.length() > m_blockSize )
      return false;

    Tag *m = new Tag( "message" );
    m->addAttribute( "to", m_parent->target().full() );
    m->addAttribute( "id", m_clientbase->getID() );
    Tag *d = new Tag( m, "data", Base64::encode64( data ) );
    d->addAttribute( "sid", m_sid );
    d->addAttribute( "seq", ++m_sequence );
    d->addAttribute( "xmlns", XMLNS_IBB );

    // FIXME: hard-coded AMP
    Tag *a = new Tag( m, "amp" );
    a->addAttribute( "xmlns", XMLNS_AMP );
    Tag *r = new Tag( a, "rule" );
    r->addAttribute( "condition", "deliver-at" );
    r->addAttribute( "value", "stored" );
    r->addAttribute( "action", "error" );
    r = new Tag( a, "rule" );
    r->addAttribute( "condition", "match-resource" );
    r->addAttribute( "value", "exact" );
    r->addAttribute( "action", "error" );

    m_clientbase->send( m );
    return true;
  }

  void InBandBytestream::closed()
  {
    m_open = false;

    if( m_inbandBytestreamDataHandler )
      m_inbandBytestreamDataHandler->handleInBandClose( m_sid, m_parent->target() );
  }

  void InBandBytestream::close()
  {
    m_open = false;

    if( !m_parent )
      return;

    const std::string& id = m_clientbase->getID();
    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "type", "set" );
    iq->addAttribute( "to", m_parent->target().full() );
    iq->addAttribute( "id", id );
    Tag *c = new Tag( iq, "close" );
    c->addAttribute( "sid", m_sid );
    c->addAttribute( "xmlns", XMLNS_IBB );

    m_clientbase->send( iq );
  }

  void InBandBytestream::registerInBandBytestreamDataHandler( InBandBytestreamDataHandler *ibbdh )
  {
    m_inbandBytestreamDataHandler = ibbdh;
  }

  void InBandBytestream::removeInBandBytestreamDataHandler()
  {
    m_inbandBytestreamDataHandler = 0;
  }

}
