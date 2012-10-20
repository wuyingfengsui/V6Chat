#include "XmppFileTransfer.h"

#include <sys\stat.h>
#include <process.h>
#include <fstream>

#include "Util.h"

#include "gloox\disco.h"

using namespace gloox;

void FileInformation::handleSOCKS5Data( SOCKS5Bytestream* /*s5b*/, const std::string& data )
{
	if(type == 2)
	{
		if( SOCKstream && SOCKstream->isOpen() && pf)
		{
			size_t ret = fwrite(data.c_str(), data.length(), 1, pf);
			if(!ret)
			{
				fclose(pf);
				pf = NULL;
			}

			handedsize += data.length();
		}
	}
}

void FileInformation::handleSOCKS5Open( gloox::SOCKS5Bytestream* /*s5b*/ )
{
	endflag = 1;
	if(type == 2)
	{
		std::string path = ansifilepath + ansifilename;
		pf = fopen(path.c_str(), "wb");
	}
	else
		pf = NULL;
}

void FileInformation::handleSOCKS5Error( SOCKS5Bytestream* s5b, Stanza* /*stanza*/ )
{
	endflag = 0;
	if(type == 2 && pf)
	{
		fclose(pf);
		pf = NULL;
	}

	std::string server, sid;
	if(SOCKstream)
	{
		server = SOCKstream->initiator().server();
		sid = SOCKstream->sid();
	}

	if(ftListener)
		ftListener->FileTransferEnd(false, server, s5b->initiator().username(), sid);
}

void FileInformation::handleSOCKS5Close( SOCKS5Bytestream* s5b )
{
	endflag = 0;
	if(type == 2 && pf)
	{
		fclose(pf);
		pf = NULL;
	}

	std::string server, sid;
	if(SOCKstream)
	{
		server = SOCKstream->initiator().server();
		sid = SOCKstream->sid();
	}

	if(ftListener)
		ftListener->FileTransferEnd(filesize == handedsize, server, s5b->initiator().username(), sid);
}


XmppFileTransfer::~XmppFileTransfer()
{
	if(hasStart)
	{
		hasStart = false;
		server->stop();
		delete server;
		delete f;
	}
}

void XmppFileTransfer::handleFTRequest( const JID& from, const std::string& id, const std::string& sid,
	const std::string& name, long size, const std::string& hash,
	const std::string& date, const std::string& mimetype,
	const std::string& desc, int /*stypes*/, long /*offset*/, long /*length*/ )
{
	std::string filePath;
	if(ftListener && ftListener->RecvFileTransferRequest(from.server(), from.username(), sid, name, size, hash, date, mimetype, desc, filePath))
	{
		FileInformation fi;
		fi.filename = name;
		fi.filepath = filePath;
		fi.ansifilename = Utf8ToAnsi(name.c_str());
		fi.ansifilepath = Utf8ToAnsi(filePath.c_str());
		fi.filesize = size;
		fi.handedsize = 0;
		fi.type = 2;
		fi.SOCKstream = NULL;
		fi.ftListener = ftListener;
		fileList.insert(std::make_pair(sid, fi));

		f->acceptFT( from, id, SIProfileFT::FTTypeS5B );
	}
	else
		f->declineFT(from, sid, SIManager::RequestRejected);
}

void XmppFileTransfer::handleFTRequestError( Stanza* stanza, const std::string& sid )
{
	fileList.erase(sid);
	ftListener->FileTransferEnd(false, stanza->to().server(), "", sid);
}

void XmppFileTransfer::handleFTSOCKS5Bytestream( SOCKS5Bytestream* s5b )
{
	std::string sid = s5b->sid();
	if(fileList.count(sid))
	{
		FileInformation &fi = fileList[sid];
		s5b->registerSOCKS5BytestreamDataHandler( &fi );
		fi.SOCKstream = s5b;

		if(fi.type == 1 || s5b->connect())
		{
			TransferThreadPkg *tpkg = new TransferThreadPkg;
			tpkg->f = f;
			tpkg->fi = &fi;

			if(fileList[sid].type == 1)
				::CloseHandle((HANDLE)_beginthreadex(NULL, 0, sendFileThread, tpkg, 0, NULL));
			else if(fileList[sid].type == 2)
				::CloseHandle((HANDLE)_beginthreadex(NULL, 0, recvFileThread, tpkg, 0, NULL));
		}
		else
			ftListener->FileTransferEnd(false, s5b->initiator().server(), s5b->initiator().username(), sid);
	}
}

bool XmppFileTransfer::Start(int listenPort)
{
	if(hasStart)
		return true;

	server = new SOCKS5BytestreamServer( client->logInstance(), listenPort );  // 建立文件传输服务器，端口号:listenPort
	if( server->listen() != ConnNoError )
	{
		delete server;
		server = NULL;
		return false;
	}

	f = new SIProfileFT( client, this );
	f->registerSOCKS5BytestreamServer( server );
	f->addStreamHost( client->jid(), "localhost", listenPort );   // 添加文件传输服务器到SOCKS5字节流主机列表，由于gloox库没有支持Proxy代理服务发现机制
	f->addStreamHost( JID(client->jid().server()), client->jid().server(), 7777 ); // 添加Proxy代理服务器到SOCKS5字节流主机列表

	client->disco()->addFeature("http://jabber.org/protocol/bytestreams");   // 将字节流服务加入服务列表

	::CloseHandle((HANDLE)_beginthreadex(NULL, 0, serverThread, this, 0, NULL));

	hasStart = true;
	return true;
}

void XmppFileTransfer::Close()
{
	if(hasStart)
	{
		server->stop();
		delete server;
		delete f;
		server = NULL;
		f = NULL;
		hasStart = false;
	}
}

bool XmppFileTransfer::SendFile(const std::string& peerAccount, const std::string& peerResource, const std::string& filePath, std::string &sid)
{
	if(!hasStart)
		return false;

	std::string anFilePath = Utf8ToAnsi(filePath.c_str());

	struct stat f_stat;
	if( stat( anFilePath.c_str(), &f_stat ) )
		return false;
	long m_size = f_stat.st_size;                     // 获得文件大小

	std::string filename = filePath.substr(filePath.find_last_of("\\") + 1);

	sid = f->requestFT( JID(peerAccount + "@" + client->jid().server() + "/" + peerResource), filename, m_size );  // 请求发送文件

	FileInformation fi;
	fi.type = 1;
	fi.filename = filename;
	fi.filepath = filePath;
	fi.ansifilename = Utf8ToAnsi(filename.c_str());
	fi.ansifilepath = anFilePath;
	fi.filesize = m_size;
	fi.handedsize = 0;
	fi.SOCKstream = NULL;
	fi.ftListener = ftListener;
	fileList.insert(std::make_pair(sid, fi));
	return true;
}

unsigned int __stdcall XmppFileTransfer::serverThread(void *parm)
{
	XmppFileTransfer *ftt = (XmppFileTransfer *)parm;

	ConnectionError se = ConnNoError;
	while(ftt->hasStart && ftt->server)
	{
		se = ftt->server->recv(1);
		if( se != ConnNoError )
			break;
	}

	return 0;
}

unsigned int __stdcall XmppFileTransfer::sendFileThread(void *parm)
{
	TransferThreadPkg *ftp = (TransferThreadPkg *)parm;
	FileInformation *fi = ftp->fi;
	std::string server = fi->SOCKstream->initiator().server();
	std::string sid = fi->SOCKstream->sid();

	std::ifstream ifile( fi->ansifilepath.c_str(), std::ios_base::in | std::ios_base::binary );
	if( !ifile )
	{
		fi->SOCKstream->close();
		return 0;
	}

	char input[BLOCK_SIZE];
	bool hasFinish = false;
	while(!hasFinish)
	{
		if( fi->SOCKstream && !ifile.eof() )
		{
			if( fi->SOCKstream->isOpen() )
			{
				ifile.read( input, BLOCK_SIZE );
				std::string t( input, ifile.gcount() );
				if( !fi->SOCKstream->send( t ) )
					break;
				fi->handedsize += ifile.gcount();
			}
		}
		else if( fi->SOCKstream )
		{
			fi->SOCKstream->close();
			hasFinish = true;
		}
	}

	if(fi->SOCKstream)
	{
		ftp->f->dispose( fi->SOCKstream );   // 调用此方法表示当发送完成后f会自动销毁此字节流，故无需 delete fi->SOCKstream
		fi->SOCKstream = NULL;
	}

	delete ftp;
	return 0;
}

unsigned int __stdcall XmppFileTransfer::recvFileThread(void *parm)
{
	TransferThreadPkg *ftp = (TransferThreadPkg *)parm;
	FileInformation *fi = ftp->fi;
	std::string server = fi->SOCKstream->initiator().server();
	std::string sid = fi->SOCKstream->sid();

	ConnectionError ce = ConnNoError;
	while(fi->SOCKstream && (ce == ConnNoError))
	{
		ce = fi->SOCKstream->recv(100);
	}

	if(fi->SOCKstream)
	{
		ftp->f->dispose( fi->SOCKstream );   // 调用此方法表示当发送完成后f会自动销毁此字节流，故无需 delete fi->SOCKstream
		fi->SOCKstream = NULL;
	}

	delete ftp;
	return 0;
}