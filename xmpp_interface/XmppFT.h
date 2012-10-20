#include "XmppFileTransfer.h"

#ifdef XMPP_INTERFACE_EXPORTS
class _declspec(dllexport) XmppFT
#else
class __declspec(dllimport) XmppFT
#endif
{
public:
	XmppFT() : ft(NULL)
	{}

	bool StartFileTransfer(gloox::Client *c);
	void Close()
	{
		if(ft)
		{
			if(ft->HasStart())
				ft->Close();
			delete ft;
			ft = NULL;
		}
	}

	bool SendFile(const std::string& peerAccount, const std::string& filePath, std::string &sid);
	void SetFileTransferListener(IXmppFileTransferListener *ftListener);

private:
	XmppFileTransfer *ft;
};