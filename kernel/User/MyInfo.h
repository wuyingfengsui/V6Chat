#pragma once
#include <string>
#include <windows.h>

#include "../Lock/SimpleCriticalSection.h"

class MyInfo
{
public:
	MyInfo(): dbId(0), sex(0), age(0), v4TcpPort(0), v6TcpPort(0), hasChange(false)
	{}
	~MyInfo()
	{}

	static MyInfo *Instance()
	{
		SimpleCriticalSection m(aCritSect);
		if(NULL == my)
		    my = new MyInfo;

	    return my;
	}
	static void Release()
	{
		SimpleCriticalSection m(aCritSect);
		if(my)
	    {
		    delete my;
		    my = NULL;
	    }
	}

	bool Init();
	bool Init(const std::wstring &nick_name, int sex, int age, const std::wstring &introduction);
	void Store();

	void SetName(const std::wstring &name)
	{
		SimpleCriticalSection m(aCritSect);
		this->name = name;

		this->hasChange = true;
	}
	std::wstring &GetName()
	{
		SimpleCriticalSection m(aCritSect);
		return name;
	}
	void SetSex(unsigned short sex)
	{
		SimpleCriticalSection m(aCritSect);
		this->sex = sex;

		this->hasChange = true;
	}
	unsigned short GetSex()
	{
		SimpleCriticalSection m(aCritSect);
		return sex;
	}
	void SetAge(unsigned int age)
	{
		SimpleCriticalSection m(aCritSect);
		this->age = age;

		this->hasChange = true;
	}
	unsigned int GetAge()
	{
		SimpleCriticalSection m(aCritSect);
		return age;
	}
	void SetIntroduction(const std::wstring &introduction)
	{
		SimpleCriticalSection m(aCritSect);
		this->introduction = introduction;

		this->hasChange = true;
	}
	std::wstring &GetIntroduction()
	{
		SimpleCriticalSection m(aCritSect);
		return introduction;
	}
	void SetIpv4(const std::string &ipv4)
	{
		SimpleCriticalSection m(aCritSect);
		this->ipv4 = ipv4;

		this->hasChange = true;
	}
	std::string GetIpv4()
	{
		SimpleCriticalSection m(aCritSect);
		return ipv4;
	}
	void SetIpv6(const std::string &ipv6)
	{
		SimpleCriticalSection m(aCritSect);
		this->ipv6 = ipv6;

		this->hasChange = true;
	}
	std::string GetIpv6()
	{
		SimpleCriticalSection m(aCritSect);
		return ipv6;
	}
	void SetIPv4TcpPort(int tcpPort)
	{
		SimpleCriticalSection m(aCritSect);
		this->v4TcpPort = tcpPort;

		this->hasChange = true;
	}
	int GetIPv4TcpPort()
	{
		SimpleCriticalSection m(aCritSect);
		return v4TcpPort;
	}
	void SetIPv6TcpPort(int tcpPort)
	{
		SimpleCriticalSection m(aCritSect);
		this->v6TcpPort = tcpPort;

		this->hasChange = true;
	}
	int GetIPv6TcpPort()
	{
		SimpleCriticalSection m(aCritSect);
		return v6TcpPort;
	}

private:
	static MyInfo *my;
	static CriticalSection aCritSect;

	unsigned int dbId;
	std::wstring name;
	unsigned short sex;
	unsigned int age;
	std::wstring introduction;

	std::string ipv4, ipv6;
	int v4TcpPort, v6TcpPort;

	bool hasChange;
};