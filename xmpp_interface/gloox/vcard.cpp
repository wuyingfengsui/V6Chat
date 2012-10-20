/*
  Copyright (c) 2006-2008 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "vcard.h"
#include "tag.h"
#include "base64.h"

namespace gloox
{

  VCard::VCard()
    : m_class( ClassNone ), m_prodid( "gloox" + GLOOX_VERSION ),
      m_N( false ), m_PHOTO( false ), m_LOGO( false )
  {
  }

  VCard::VCard( Tag *vcard )
    : m_class( ClassNone ), m_prodid( "gloox" + GLOOX_VERSION ),
      m_N( false ), m_PHOTO( false ), m_LOGO( false )
  {
    checkField( vcard, "FN", m_formattedname );
    checkField( vcard, "NICKNAME", m_nickname );
    checkField( vcard, "URL", m_url );
    checkField( vcard, "BDAY", m_bday );
    checkField( vcard, "JABBERID", m_jabberid );
    checkField( vcard, "TITLE", m_title );
    checkField( vcard, "ROLE", m_role );
    checkField( vcard, "NOTE", m_note );
    checkField( vcard, "DESC", m_desc );
    checkField( vcard, "MAILER", m_mailer );
    checkField( vcard, "TZ", m_tz );
    checkField( vcard, "PRODID", m_prodid );
    checkField( vcard, "REV", m_rev );
    checkField( vcard, "SORT-STRING", m_sortstring );
    checkField( vcard, "UID", m_uid );

    Tag::TagList::const_iterator it = vcard->children().begin();
    for( ; it != vcard->children().end(); ++it )
    {
      if( (*it)->name() == "N" )
      {
        m_N = true;
        if( (*it)->hasChild( "FAMILY" ) )
          m_name.family = (*it)->findChild( "FAMILY" )->cdata();
        if( (*it)->hasChild( "GIVEN" ) )
          m_name.given = (*it)->findChild( "GIVEN" )->cdata();
        if( (*it)->hasChild( "MIDDLE" ) )
          m_name.middle = (*it)->findChild( "MIDDLE" )->cdata();
        if( (*it)->hasChild( "PREFIX" ) )
          m_name.prefix = (*it)->findChild( "PREFIX" )->cdata();
        if( (*it)->hasChild( "SUFFIX" ) )
          m_name.suffix = (*it)->findChild( "SUFFIX" )->cdata();
      }
      else if( (*it)->name() == "PHOTO" )
      {
        if( (*it)->hasChild( "EXTVAL" ) )
        {
          m_photo.extval = (*it)->findChild( "EXTVAL" )->cdata();
          m_PHOTO = true;
        }
        else if( (*it)->hasChild( "TYPE" ) && (*it)->hasChild( "BINVAL" ) )
        {
          std::string binval = (*it)->findChild( "BINVAL" )->cdata();
          std::string::size_type pos = 0;
          while( ( pos = binval.find( '\n' ) ) != std::string::npos )
            binval.erase( pos, 1 );
          m_photo.type = (*it)->findChild( "TYPE" )->cdata();
          m_photo.binval = Base64::decode64( binval );
          m_PHOTO = true;
        }
      }
      else if( (*it)->name() == "LOGO" )
      {
        if( (*it)->hasChild( "EXTVAL" ) )
        {
          m_logo.extval = (*it)->findChild( "EXTVAL" )->cdata();
          m_LOGO = true;
        }
        else if( (*it)->hasChild( "TYPE" ) && (*it)->hasChild( "BINVAL" ) )
        {
          std::string binval = (*it)->findChild( "BINVAL" )->cdata();
          std::string::size_type pos = 0;
          while( ( pos = binval.find( '\n' ) ) != std::string::npos )
            binval.erase( pos, 1 );
          m_logo.type = (*it)->findChild( "TYPE" )->cdata();
          m_logo.binval = Base64::decode64( binval );
          m_LOGO = true;
        }
      }
      else if( (*it)->name() == "EMAIL" && (*it)->hasChild( "USERID" ) )
      {
        Email item;
        item.userid = (*it)->findChild( "USERID" )->cdata();
        item.internet = (*it)->hasChild( "INTERNET" );
        item.x400 = (*it)->hasChild( "X400" );
        item.work = (*it)->hasChild( "WORK" );
        item.home = (*it)->hasChild( "HOME" );
        item.pref = (*it)->hasChild( "PREF" );
        m_emailList.push_back( item );
      }
      else if( (*it)->name() == "ADR" )
      {
        Address item;
        checkField( (*it), "POBOX", item.pobox );
        checkField( (*it), "EXTADD", item.extadd );
        checkField( (*it), "STREET", item.street );
        checkField( (*it), "LOCALITY", item.locality );
        checkField( (*it), "REGION", item.region );
        checkField( (*it), "PCODE", item.pcode );
        checkField( (*it), "CTRY", item.ctry );
        item.postal = (*it)->hasChild( "POSTAL" );
        item.parcel = (*it)->hasChild( "PARCEL" );
        item.work = (*it)->hasChild( "WORK" );
        item.home = (*it)->hasChild( "HOME" );
        item.pref = (*it)->hasChild( "PREF" );
        item.dom = (*it)->hasChild( "DOM" );
        item.intl = !item.dom && (*it)->hasChild( "INTL" );
        m_addressList.push_back( item );
      }
      else if( (*it)->name() == "LABEL" )
      {
        Label item;
        Tag::TagList::const_iterator it2 = (*it)->children().begin();
        for( ; it2 != (*it)->children().end(); ++it2 )
        {
          if( (*it2)->name() == "LINE" )
            item.lines.push_back( (*it)->cdata() );
          item.postal = (*it2)->name() == "POSTAL";
          item.parcel = (*it2)->name() == "PARCEL";
          item.work = (*it2)->name() == "WORK";
          item.home = (*it2)->name() == "HOME";
          item.pref = (*it2)->name() == "PREF";
          item.dom = (*it2)->name() == "DOM";
          item.intl = !item.dom && (*it2)->name() == "INTL";
        }
        m_labelList.push_back( item );
      }
      else if( (*it)->name() == "TEL" && (*it)->hasChild( "NUMBER" ) )
      {
        Telephone item;
        item.number = (*it)->findChild( "NUMBER" )->cdata();
        item.work = (*it)->hasChild( "WORK" );
        item.home = (*it)->hasChild( "HOME" );
        item.voice = (*it)->hasChild( "VOICE" );
        item.fax = (*it)->hasChild( "FAX" );
        item.pager = (*it)->hasChild( "PAGER" );
        item.msg = (*it)->hasChild( "MSG" );
        item.cell = (*it)->hasChild( "CELL" );
        item.video = (*it)->hasChild( "VIDEO" );
        item.bbs = (*it)->hasChild( "BBS" );
        item.modem = (*it)->hasChild( "MODEM" );
        item.isdn = (*it)->hasChild( "ISDN" );
        item.pcs = (*it)->hasChild( "PCS" );
        item.pref = (*it)->hasChild( "PREF" );
        m_telephoneList.push_back( item );
      }
      else if( (*it)->name() == "ORG" )
      {
        Tag::TagList::const_iterator ito = (*it)->children().begin();
        for( ; ito != (*it)->children().end(); ++ito )
        {
          if( (*ito)->name() == "ORGNAME" )
            m_org.name = (*ito)->cdata();
          else if( (*ito)->name() == "ORGUNIT" )
            m_org.units.push_back( (*ito)->cdata() );
        }
      }
      else if( (*it)->name() == "GEO" )
      {
        checkField( (*it), "LON", m_geo.longitude );
        checkField( (*it), "LAT", m_geo.latitude );
      }
      else if( (*it)->name() == "CLASS" )
      {
        if( (*it)->hasChild( "PRIVATE" ) )
          m_class = ClassPrivate;
        else if( (*it)->hasChild( "PUBLIC" ) )
          m_class = ClassPublic;
        else if( (*it)->hasChild( "CONFIDENTIAL" ) )
          m_class = ClassConfidential;
      }

    }

  }

  void VCard::checkField( Tag *vcard, const std::string& field, std::string& var )
  {
    if( vcard->hasChild( field ) )
      var = vcard->findChild( field )->cdata();
  }

  void VCard::setName( const std::string& family, const std::string& given, const std::string& middle,
                       const std::string& prefix, const std::string& suffix )
  {
    m_name.family = family;
    m_name.given = given;
    m_name.middle = middle;
    m_name.prefix = prefix;
    m_name.suffix = suffix;
    m_N = true;
  }

  void VCard::setPhoto( const std::string& extval )
  {
    if( !extval.empty() )
    {
      m_photo.extval= extval;
      m_PHOTO = true;
    }
  }

  void VCard::setPhoto( const std::string& type, const std::string& binval )
  {
    if( !type.empty() && !binval.empty() )
    {
      m_photo.type = type;
      m_photo.binval = binval;
      m_PHOTO = true;
    }
  }

  void VCard::setLogo( const std::string& extval )
  {
    if( !extval.empty() )
    {
      m_logo.extval = extval;
      m_LOGO = true;
    }
  }

  void VCard::setLogo( const std::string& type, const std::string& binval )
  {
    if( !type.empty() && !binval.empty() )
    {
      m_logo.type = type;
      m_logo.binval = binval;
      m_LOGO = true;
    }
  }

  void VCard::addEmail( const std::string& userid, int type )
  {
    if( userid.empty() )
      return;

    Email item;
    item.userid = userid;
    item.internet = type & AddrTypeInet ? true : false;
    item.x400 = type & AddrTypeX400 ? true : false;
    item.work = type & AddrTypeWork ? true : false;
    item.home = type & AddrTypeHome ? true : false;
    item.pref = type & AddrTypePref ? true : false;

    m_emailList.push_back( item );
  }

  void VCard::addAddress( const std::string& pobox, const std::string& extadd,
                          const std::string& street, const std::string& locality,
                          const std::string& region, const std::string& pcode,
                          const std::string& ctry, int type )
  {
    if( pobox.empty() && extadd.empty() && street.empty() &&
        locality.empty() && region.empty() && pcode.empty() && ctry.empty() )
      return;

    Address item;
    item.pobox = pobox;
    item.extadd = extadd;
    item.street = street;
    item.locality = locality;
    item.region = region;
    item.pcode = pcode;
    item.ctry = ctry;
    item.home = type & AddrTypeHome ? true : false;
    item.work = type & AddrTypeWork ? true : false;
    item.parcel = type & AddrTypeParcel ? true : false;
    item.postal = type & AddrTypePostal ? true : false;
    item.dom = type & AddrTypeDom ? true : false;
    item.intl = !item.dom && type & AddrTypeIntl ? true : false;
    item.pref = type & AddrTypePref ? true : false;

    m_addressList.push_back( item );
  }

  void VCard::addLabel( const StringList& lines, int type )
  {
    if( !lines.size() )
      return;

    Label item;
    item.lines = lines;
    item.work = type & AddrTypeWork ? true : false;
    item.home = type & AddrTypeHome ? true : false;
    item.postal = type & AddrTypePostal ? true : false;
    item.parcel = type & AddrTypeParcel ? true : false;
    item.pref = type & AddrTypePref ? true : false;
    item.dom = type & AddrTypeDom ? true : false;
    item.intl = !item.dom && type & AddrTypeIntl;

    m_labelList.push_back( item );
  }

  void VCard::addTelephone( const std::string& number, int type )
  {
    if( number.empty() )
      return;

    Telephone item;
    item.number = number;
    item.work = type & AddrTypeWork ? true : false;
    item.home = type & AddrTypeHome ? true : false;
    item.voice = type & AddrTypeVoice ? true : false;
    item.fax = type & AddrTypeFax ? true : false;
    item.pager = type & AddrTypePager ? true : false;
    item.msg = type & AddrTypeMsg ? true : false;
    item.cell = type & AddrTypeCell ? true : false;
    item.video = type & AddrTypeVideo ? true : false;
    item.bbs = type & AddrTypeBbs ? true : false;
    item.modem = type & AddrTypeModem ? true : false;
    item.isdn = type & AddrTypeIsdn ? true : false;
    item.pcs = type & AddrTypePcs ? true : false;
    item.pref = type & AddrTypePref ? true : false;

    m_telephoneList.push_back( item );
  }

  void VCard::setGeo( const std::string& lat, const std::string& lon )
  {
    if( !lat.empty() && !lon.empty() )
    {
      m_geo.latitude = lat;
      m_geo.longitude = lon;
    }
  }

  void VCard::setOrganization( const std::string& orgname, const StringList& orgunits )
  {
    if( !orgname.empty() )
    {
      m_org.name = orgname;
      m_org.units = orgunits;
    }
  }

  Tag* VCard::tag() const
  {
    Tag *v = new Tag( "vCard" );
    v->addAttribute( "xmlns", XMLNS_VCARD_TEMP );
    v->addAttribute( "version", "3.0" );

    insertField( v, "FN", m_formattedname );
    insertField( v, "NICKNAME", m_nickname );
    insertField( v, "URL", m_url );
    insertField( v, "BDAY", m_bday );
    insertField( v, "JABBERID", m_jabberid );
    insertField( v, "TITLE", m_title );
    insertField( v, "ROLE", m_role );
    insertField( v, "NOTE", m_note );
    insertField( v, "DESC", m_desc );
    insertField( v, "MAILER", m_mailer );
    insertField( v, "TZ", m_tz );
    insertField( v, "REV", m_rev );
    insertField( v, "SORT_STRING", m_sortstring );
    insertField( v, "UID", m_uid );

    if( m_N )
    {
      Tag *n = new Tag( v, "N" );
      insertField( n, "FAMILY", m_name.family );
      insertField( n, "GIVEN", m_name.given );
      insertField( n, "MIDDLE", m_name.middle );
      insertField( n, "PREFIX", m_name.prefix );
      insertField( n, "SUFFIX", m_name.suffix );
    }

    if( m_PHOTO )
    {
      Tag *p = new Tag( v, "PHOTO" );
      if( !m_photo.extval.empty() )
      {
        new Tag( p, "EXTVAL", m_photo.extval );
      }
      else if( !m_photo.type.empty() && !m_photo.binval.empty() )
      {
        new Tag( p, "TYPE", m_photo.type );
        new Tag( p, "BINVAL", Base64::encode64( m_photo.binval ) );
      }
    }

    if( m_LOGO )
    {
      Tag *l = new Tag( v, "LOGO" );
      if( !m_logo.extval.empty() )
      {
        new Tag( l, "EXTVAL", m_logo.extval );
      }
      else if( !m_logo.type.empty() && !m_logo.binval.empty() )
      {
        new Tag( l, "TYPE", m_logo.type );
        new Tag( l, "BINVAL", Base64::encode64( m_logo.binval ) );
      }
    }

    EmailList::const_iterator ite = m_emailList.begin();
    for( ; ite != m_emailList.end(); ++ite )
    {
      Tag *e = new Tag( v, "EMAIL" );
      insertField( e, "INTERNET", (*ite).internet );
      insertField( e, "WORK", (*ite).work );
      insertField( e, "HOME", (*ite).home );
      insertField( e, "X400", (*ite).x400 );
      insertField( e, "PREF", (*ite).pref );
      insertField( e, "USERID", (*ite).userid );
    }

    AddressList::const_iterator ita = m_addressList.begin();
    for( ; ita != m_addressList.end(); ++ita )
    {
      Tag *a = new Tag( v, "ADR" );
      insertField( a, "POSTAL", (*ita).postal );
      insertField( a, "PARCEL", (*ita).parcel );
      insertField( a, "HOME", (*ita).home );
      insertField( a, "WORK", (*ita).work );
      insertField( a, "PREF", (*ita).pref );
      insertField( a, "DOM", (*ita).dom );
      if( !(*ita).dom )
        insertField( a, "INTL", (*ita).intl );

      insertField( a, "POBOX", (*ita).pobox );
      insertField( a, "EXTADD", (*ita).extadd );
      insertField( a, "STREET", (*ita).street );
      insertField( a, "LOCALITY", (*ita).locality );
      insertField( a, "REGION", (*ita).region );
      insertField( a, "PCODE", (*ita).pcode );
      insertField( a, "CTRY", (*ita).ctry );
    }

    TelephoneList::const_iterator itt = m_telephoneList.begin();
    for( ; itt != m_telephoneList.end(); ++itt )
    {
      Tag *t = new Tag( v, "TEL" );
      insertField( t, "NUMBER", (*itt).number );
      insertField( t, "HOME", (*itt).home );
      insertField( t, "WORK", (*itt).work );
      insertField( t, "VOICE", (*itt).voice );
      insertField( t, "FAX", (*itt).fax );
      insertField( t, "PAGER", (*itt).pager );
      insertField( t, "MSG", (*itt).msg );
      insertField( t, "CELL", (*itt).cell );
      insertField( t, "VIDEO", (*itt).video );
      insertField( t, "BBS", (*itt).bbs );
      insertField( t, "MODEM", (*itt).modem );
      insertField( t, "ISDN", (*itt).isdn );
      insertField( t, "PCS", (*itt).pcs );
      insertField( t, "PREF", (*itt).pref );
    }

    if( !m_geo.latitude.empty() && !m_geo.longitude.empty() )
    {
      Tag *g = new Tag( v, "GEO" );
      new Tag( g, "LAT", m_geo.latitude );
      new Tag( g, "LON", m_geo.longitude );
    }

    if( !m_org.name.empty() )
    {
      Tag *o = new Tag( v, "ORG" );
      new Tag( o, "ORGNAME", m_org.name );
      StringList::const_iterator ito = m_org.units.begin();
      for( ; ito != m_org.units.end(); ++ito )
        new Tag( o, "ORGUNITS", (*ito) );
    }

    if( m_class != ClassNone )
    {
      Tag *c = new Tag( v, "CLASS" );
      switch( m_class )
      {
        case ClassPublic:
          new Tag( c, "PUBLIC" );
          break;
        case ClassPrivate:
          new Tag( c, "PRIVATE" );
          break;
        case ClassConfidential:
          new Tag( c, "CONFIDENTIAL" );
          break;
        default:
          break;
      }
    }

    return v;
  }

  void VCard::insertField( Tag *vcard, const std::string& field, const std::string& var ) const
  {
    if( !var.empty() )
      new Tag( vcard, field, var );
  }

  void VCard::insertField( Tag *vcard, const std::string& field, bool var ) const
  {
    if( var )
      new Tag( vcard, field );
  }

}
