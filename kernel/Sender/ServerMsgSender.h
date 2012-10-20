#pragma once

#include "MsgSender.h"

class ServerMsgSender : public MsgSender
{
public:
	ServerMsgSender(unsigned int serverid, const std::string &serveruid, unsigned int userid)
		:serverid(serverid), serveruid(serveruid), userid(userid)
	{}

	void AddUserAskMsg();

	void ReplyAddUserMsg(bool agree);

	void Offline()
	{}

    void UpdateProfileMsg();

	bool GetPhotoByHttp(const std::string &photoURI, int photoType = 0);         // ����URI��ȡͷ�񲢴洢��type��0-����ͷ��1-��������û�ͷ��
	bool StorePhotoByStream(const std::string &stream, int photoType = 0);       // �����������洢ͷ��type��0-����ͷ��1-��������û�ͷ��

	void ChatWithTextMsg(const std::wstring &text, const std::string &sendTime);

private:
	unsigned int serverid;
	unsigned int userid;
	std::string serveruid;     // utf-8��ʽ
};