#include "XmppFT.h"

#include <time.h>


bool XmppFT::StartFileTransfer(gloox::Client *c)
{
	if(ft == NULL)
		ft = new XmppFileTransfer(c);

	if(!ft->HasStart())
	{
		srand((unsigned int)time(NULL));

		int trytime;
		for(trytime = 0; trytime < 3; ++trytime)
		{
			int port = rand()%64512 + 1024;
			if(ft->Start(port))
				break;
		}

		if(3 == trytime)
			return false;
	}
	return true;
}

bool XmppFT::SendFile(const std::string& peerAccount, const std::string& filePath, std::string &sid)
{
	if(!ft->HasStart())
	{
		srand((unsigned int)time(NULL));

		int trytime;
		for(trytime = 0; trytime < 3; ++trytime)
		{
			int port = rand()%64512 + 1024;
			if(ft->Start(port))
				break;
		}

		if(3 == trytime)
			return false;
	}

	return ft->SendFile(peerAccount, "gloox", filePath, sid);
}

void XmppFT::SetFileTransferListener(IXmppFileTransferListener *ftListener)
{
	ft->SetFileTransferListener(ftListener);
}