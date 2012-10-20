#include "MyInfo.h"
#include "../V6Util.h"

#include "../../db_interface/db_interface.h"

#include <sstream>
#include <Shlwapi.h>

using namespace std;

MyInfo *MyInfo::my = NULL;
CriticalSection MyInfo::aCritSect;

bool MyInfo::Init()
{
	dbId=0;name.clear();sex=0;age=0;introduction.clear();v4TcpPort=0;v6TcpPort=0;

	char myid[6];

	CHAR szModulePath[MAX_PATH];
    GetModuleFileNameA(NULL,szModulePath,MAX_PATH);
	PathAppendA(szModulePath, "..\\app.ini");
	::GetPrivateProfileStringA("User","id","1",myid,6,szModulePath);

	char *nameU = NULL, *introductionU = NULL, *headImagePathU = NULL;
	bool has_myInfo = get_myProfileById(myid, &nameU, &sex, &age, &introductionU, &headImagePathU, &v4TcpPort, &v6TcpPort);

	if(has_myInfo)
	{
		name = V6Util::Utf8ToUnicode(nameU);
		introduction = V6Util::Utf8ToUnicode(introductionU);
		release_db_data((void **)&nameU);
		release_db_data((void **)&introductionU);
		release_db_data((void **)&headImagePathU);
	}

	return has_myInfo;
}

bool MyInfo::Init(const std::wstring &nick_name, int sex, int age, const std::wstring &introduction)
{
	dbId=0;
	this->sex=sex;
	this->age=age;
	this->introduction = introduction;
	v4TcpPort=0;v6TcpPort=0;

	this->name = nick_name;
	char *introductionU = NULL, *headImagePathU = NULL;
	bool has_myInfo = get_myProfileByName(V6Util::UnicodeToUtf8(name.c_str()).c_str(), &dbId, &this->sex, &this->age, &introductionU, &headImagePathU, &v4TcpPort, &v6TcpPort);

	if(has_myInfo)
	{
		this->introduction = V6Util::Utf8ToUnicode(introductionU);
		release_db_data((void **)&introductionU);
		release_db_data((void **)&headImagePathU);
	}

	return true;
}

void MyInfo::Store()
{
	stringstream sid, ssex, sage, sv4port, sv6port;
	sid<<dbId; ssex<<sex, sage<<age;sv4port<<v4TcpPort;sv6port<<v6TcpPort;

	dbId = update_myProfile(sid.str().c_str(), V6Util::UnicodeToUtf8(name.c_str()).c_str(), ssex.str().c_str(), sage.str().c_str(), V6Util::UnicodeToUtf8(introduction.c_str()).c_str(), NULL, sv4port.str().c_str(), sv6port.str().c_str());
	
	sid.str("");
	sid<<dbId;
	CHAR szModulePath[MAX_PATH];
    GetModuleFileNameA(NULL,szModulePath,MAX_PATH);
	PathAppendA(szModulePath, "..\\app.ini");
	::WritePrivateProfileStringA("User","id",sid.str().c_str(),szModulePath);
}