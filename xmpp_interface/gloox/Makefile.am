##
## Process this file with automake to produce Makefile.in
##

SUBDIRS = . examples tests

AM_CPPFLAGS = -Wall -pedantic -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith \
                -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual -ansi

lib_LTLIBRARIES = libgloox.la

libgloox_la_SOURCES = jid.cpp parser.cpp connectiontcpclient.cpp clientbase.cpp tag.cpp stanza.cpp logsink.cpp \
                        dns.cpp prep.cpp base64.cpp client.cpp component.cpp \
                        disco.cpp adhoc.cpp privatexml.cpp registration.cpp \
                        nonsaslauth.cpp rosteritem.cpp rostermanager.cpp privacyitem.cpp \
                        bookmarkstorage.cpp annotations.cpp privacymanager.cpp lastactivity.cpp \
                        flexoff.cpp dataform.cpp dataformfield.cpp dataformbase.cpp messagesession.cpp \
                        messageeventfilter.cpp chatstatefilter.cpp gloox.cpp \
                        inbandbytestream.cpp inbandbytestreammanager.cpp messagefilter.cpp vcard.cpp \
                        vcardmanager.cpp md5.cpp sha.cpp search.cpp dataformreported.cpp dataformitem.cpp \
                        mucroom.cpp mucmessagesession.cpp oob.cpp vcardupdate.cpp stanzaextensionfactory.cpp \
                        delayeddelivery.cpp xdelayeddelivery.cpp gpgencrypted.cpp gpgsigned.cpp \
                        uniquemucroom.cpp instantmucroom.cpp compressionzlib.cpp tlsgnutlsclient.cpp \
                        tlsopenssl.cpp connectionhttpproxy.cpp tlsgnutlsserveranon.cpp tlsgnutlsbase.cpp \
                        tlsgnutlsclientanon.cpp tlsschannel.cpp tlsdefault.cpp simanager.cpp siprofileft.cpp \
                        mutex.cpp connectionsocks5proxy.cpp socks5bytestreammanager.cpp socks5bytestream.cpp \
                        connectiontcpbase.cpp connectiontcpserver.cpp socks5bytestreamserver.cpp

libgloox_la_LDFLAGS = -version-info 7:0:0 -no-undefined -no-allow-shlib-undefined
libgloox_la_LIBADD =
libgloox_la_CFLAGS = $(CPPFLAGS)

libglooxincludedir = $(includedir)/gloox
libglooxinclude_HEADERS = adhoccommandprovider.h      privacymanager.h        tag.h \
                            adhoc.h                   iqhandler.h             privatexml.h \
                            annotations.h             client.h                privatexmlhandler.h \
                            annotationshandler.h      component.h             registration.h \
                            bookmarkhandler.h         registrationhandler.h   jid.h \
                            bookmarkstorage.h         messagehandler.h        rosteritem.h \
                            clientbase.h              rosterlistener.h        taghandler.h \
                            connectionlistener.h      rostermanager.h         gloox.h \
                            disco.h                   presencehandler.h       subscriptionhandler.h \
                            discohandler.h            privacyitem.h           stanza.h \
                            disconodehandler.h        privacylisthandler.h    loghandler.h \
                            lastactivity.h            lastactivityhandler.h   flexoff.h \
                            flexoffhandler.h          dataform.h              dataformfield.h \
                            dataformitem.h            dataformbase.h          dataformreported.h \
                            macros.h                  logsink.h               messagesession.h \
                            messageeventhandler.h     messagesessionhandler.h chatstatehandler.h \
                            chatstatefilter.h         messageeventfilter.h    inbandbytestream.h \
                            inbandbytestreamhandler.h inbandbytestreamdatahandler.h \
                            inbandbytestreammanager.h messagefilter.h         vcard.h \
                            vcardmanager.h            vcardhandler.h          adhochandler.h \
                            search.h                  searchhandler.h         statisticshandler.h \
                            resource.h                mucroom.h               mucroomhandler.h \
                            mucroomconfighandler.h    parser.h                \
                            mucinvitationhandler.h    stanzaextension.h       oob.h \
                            vcardupdate.h             delayeddelivery.h       base64.h \
                            xdelayeddelivery.h        gpgencrypted.h          gpgsigned.h \
                            md5.h                     sha.h                   uniquemucroom.h \
                            instantmucroom.h          tlsbase.h               tlshandler.h \
                            connectionbase.h          connectiondatahandler.h compressiondatahandler.h \
                            compressionbase.h         connectiontcpclient.h   connectionhttpproxy.h \
                            tlsdefault.h              simanager.h             siprofilehandler.h \
                            sihandler.h               siprofileft.h           siprofilefthandler.h \
                            socks5bytestreammanager.h connectionsocks5proxy.h socks5bytestreamdatahandler.h \
                            socks5bytestreamhandler.h socks5bytestream.h      socks5bytestreamserver.h \
                            mutex.h                   mutexguard.h            connectiontcpbase.h \
                            connectiontcpserver.h     connectionhandler.h

noinst_HEADERS = prep.h dns.h nonsaslauth.h mucmessagesession.h stanzaextensionfactory.h tlsgnutlsclient.h \
                   tlsgnutlsbase.h tlsgnutlsclientanon.h tlsgnutlsserveranon.h tlsopenssl.h tlsschannel.h \
                   compressionzlib.h

EXTRA_DIST = version.rc

test:
	make -C tests test

check: test

leaktest:
	make -C tests leaktest

leakcheck: leaktest

examples:
	make -C examples
