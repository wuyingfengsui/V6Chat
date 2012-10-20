#pragma once

#include <string>

#include "gloox\client.h"
#include "gloox\socks5bytestreamserver.h"
#include "gloox\siprofileft.h"
#include "gloox\siprofilefthandler.h"
#include "gloox\socks5bytestreamdatahandler.h"

#define BLOCK_SIZE 2048

#ifdef XMPP_INTERFACE_EXPORTS
class _declspec(dllexport) IXmppFileTransferListener
#else
class _declspec(dllimport) IXmppFileTransferListener
#endif
{
public:
	virtual bool RecvFileTransferRequest(
		const std::string &serverName, 
		const std::string &peerAccount, 
		const std::string& sid,
		const std::string& fileName, 
		long size, 
		const std::string& hash,
		const std::string& date, 
		const std::string& mimeType,
		const std::string& description,
		std::string &filePath                             // ����dll��ʱ����������
		) = 0;

	virtual void FileTransferEnd(
		bool success,
		const std::string &serverName,
		const std::string &peerAccount,
		const std::string& sid
		) = 0;
};

struct FileInformation : public gloox::SOCKS5BytestreamDataHandler
{
	int type;                   // 1-���͵��ļ���2-���յ��ļ�
	int endflag;                // ������ǣ�0-������1-������
	std::string filename;
	std::string filepath;
	std::string ansifilename;    // ansi�����ʽ���ļ�����·����֧������
	std::string ansifilepath;
	FILE* pf;                    // ���ļ���ָ�룬���ڽ���ʱ
	long filesize;
	long handedsize;
	gloox::SOCKS5Bytestream* SOCKstream;
	IXmppFileTransferListener *ftListener;

	// �÷�����Ϊʵ�� SOCKS5BytestreamDataHandler �������ӿ��еĴ���ͨ��SOCKS5�ֽ����յ������ݵķ���ʵ�֡�
	virtual void handleSOCKS5Data( gloox::SOCKS5Bytestream* /*s5b*/, const std::string& data );

	// �÷�����Ϊʵ�� SOCKS5BytestreamDataHandler �������ӿ��е�SOCKS5�ֽ������ִ���ķ���ʵ�֡�
	virtual void handleSOCKS5Error( gloox::SOCKS5Bytestream* /*s5b*/, gloox::Stanza* /*stanza*/ );

	// �÷�����Ϊʵ�� SOCKS5BytestreamDataHandler �������ӿ��е�SOCKS5�ֽ����򿪵ķ���ʵ�֡�
	virtual void handleSOCKS5Open( gloox::SOCKS5Bytestream* /*s5b*/ );

	// �÷�����Ϊʵ�� SOCKS5BytestreamDataHandler �������ӿ��е�SOCKS5�ֽ����رյķ���ʵ�֡�
	virtual void handleSOCKS5Close( gloox::SOCKS5Bytestream* /*s5b*/ );
};

struct TransferThreadPkg
{
	FileInformation *fi;
	gloox::SIProfileFT* f;
};

class XmppFileTransfer : public gloox::SIProfileFTHandler
{
public:
	XmppFileTransfer(gloox::Client *c)
		:client(c), ftListener(NULL), server(NULL), hasStart(false)
	{}

	~XmppFileTransfer();

	// �÷�����Ϊʵ�� SIProfileFTHandler �������ӿ��е����µ��ļ���������ķ���ʵ�֡�
	virtual void handleFTRequest( const gloox::JID& from, const std::string& id, const std::string& sid,
		const std::string& name, long size, const std::string& hash,
		const std::string& date, const std::string& mimetype,
		const std::string& desc, int /*stypes*/, long /*offset*/, long /*length*/ );

	// �÷�����Ϊʵ�� SIProfileFTHandler �������ӿ��е��ļ�����������ִ���ķ���ʵ�֡�
	virtual void handleFTRequestError( gloox::Stanza* stanza, const std::string& sid );

	// �÷�����Ϊʵ�� SIProfileFTHandler �������ӿ��е�ͨ�������򿪵�SOCKS5�ֽ����ķ���ʵ�֡�
	virtual void handleFTSOCKS5Bytestream( gloox::SOCKS5Bytestream* s5b );

	bool Start(int listenPort);

	void Close();

	// ����������
	bool SendFile(const std::string& peerAccount, const std::string& peerResource, const std::string& filePath, std::string &sid);

	void SetFileTransferListener(IXmppFileTransferListener *ftListener)
	{
		this->ftListener = ftListener;
	}

	const bool HasStart() const
	{
		return hasStart;
	}

private:
	bool hasStart;
	gloox::Client *client;
	std::map<std::string, struct FileInformation> fileList;
	IXmppFileTransferListener *ftListener;
	gloox::SOCKS5BytestreamServer *server;
	gloox::SIProfileFT* f; // �ļ�����������ָ��

	static unsigned int __stdcall serverThread(void *parm);
	static unsigned int __stdcall sendFileThread(void *parm);
	static unsigned int __stdcall recvFileThread(void *parm);
};