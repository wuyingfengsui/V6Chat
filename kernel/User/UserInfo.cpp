#include "UserInfo.h"

#include <sstream>

#include "../../db_interface/db_interface.h"
#include "../IPSearcher/IPSearcher.h"
#include "../V6Util.h"

#include "MyInfo.h"

void UserInfo::RecvText(int text_type, const std::wstring &text, const std::string &recordTime)
{
	ChatMsg msg(text, recordTime, text_type);
	unreadChatContent.push(msg);
}

bool UserInfo::RecordText(bool sendOrRecv, const std::wstring &text, std::string &recordTime)
{
	std::stringstream suid;
	char *time = NULL;

	suid << this->uid;

	std::string name;
	if(sendOrRecv)
	{
		MyInfo *my = MyInfo::Instance();
		name = V6Util::UnicodeToUtf8(my->GetName().c_str()).c_str();
	}
	else
		name = V6Util::UnicodeToUtf8(this->nick_name.c_str()).c_str();

	int iid = insert_chatItem(suid.str().c_str(), 
		name.c_str(), 
		V6Util::UnicodeToUtf8(text.c_str()).c_str(),
		&time);

	if(iid)
	{
		recordTime = time;
		release_db_data((void **)&time);
		return true;
	}
	else
		return false;
}