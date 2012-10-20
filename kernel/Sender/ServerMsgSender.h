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

	bool GetPhotoByHttp(const std::string &photoURI, int photoType = 0);         // 根据URI获取头像并存储，type：0-好友头像，1-搜索结果用户头像
	bool StorePhotoByStream(const std::string &stream, int photoType = 0);       // 根据数据流存储头像，type：0-好友头像，1-搜索结果用户头像

	void ChatWithTextMsg(const std::wstring &text, const std::string &sendTime);

private:
	unsigned int serverid;
	unsigned int userid;
	std::string serveruid;     // utf-8格式
};