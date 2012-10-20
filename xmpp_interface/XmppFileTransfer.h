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
		std::string &filePath                             // 当在dll中时可能有问题
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
	int type;                   // 1-发送的文件，2-接收的文件
	int endflag;                // 结束标记，0-结束，1-传输中
	std::string filename;
	std::string filepath;
	std::string ansifilename;    // ansi编码格式的文件名及路径，支持中文
	std::string ansifilepath;
	FILE* pf;                    // 打开文件的指针，用于接收时
	long filesize;
	long handedsize;
	gloox::SOCKS5Bytestream* SOCKstream;
	IXmppFileTransferListener *ftListener;

	// 该方法即为实现 SOCKS5BytestreamDataHandler 监听器接口中的处理通过SOCKS5字节流收到的数据的方法实现。
	virtual void handleSOCKS5Data( gloox::SOCKS5Bytestream* /*s5b*/, const std::string& data );

	// 该方法即为实现 SOCKS5BytestreamDataHandler 监听器接口中的SOCKS5字节流出现错误的方法实现。
	virtual void handleSOCKS5Error( gloox::SOCKS5Bytestream* /*s5b*/, gloox::Stanza* /*stanza*/ );

	// 该方法即为实现 SOCKS5BytestreamDataHandler 监听器接口中的SOCKS5字节流打开的方法实现。
	virtual void handleSOCKS5Open( gloox::SOCKS5Bytestream* /*s5b*/ );

	// 该方法即为实现 SOCKS5BytestreamDataHandler 监听器接口中的SOCKS5字节流关闭的方法实现。
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

	// 该方法即为实现 SIProfileFTHandler 监听器接口中的有新的文件传输请求的方法实现。
	virtual void handleFTRequest( const gloox::JID& from, const std::string& id, const std::string& sid,
		const std::string& name, long size, const std::string& hash,
		const std::string& date, const std::string& mimetype,
		const std::string& desc, int /*stypes*/, long /*offset*/, long /*length*/ );

	// 该方法即为实现 SIProfileFTHandler 监听器接口中的文件传输请求出现错误的方法实现。
	virtual void handleFTRequestError( gloox::Stanza* stanza, const std::string& sid );

	// 该方法即为实现 SIProfileFTHandler 监听器接口中的通过即将打开的SOCKS5字节流的方法实现。
	virtual void handleFTSOCKS5Bytestream( gloox::SOCKS5Bytestream* s5b );

	bool Start(int listenPort);

	void Close();

	// 非阻塞方法
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
	gloox::SIProfileFT* f; // 文件传输管理对象指针

	static unsigned int __stdcall serverThread(void *parm);
	static unsigned int __stdcall sendFileThread(void *parm);
	static unsigned int __stdcall recvFileThread(void *parm);
};