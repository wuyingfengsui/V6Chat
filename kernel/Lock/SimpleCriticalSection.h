#pragma once
#include <windows.h>

class SimpleCriticalSection;

class CriticalSection
{
public:
	friend class SimpleCriticalSection;

	CriticalSection()
	{ ::InitializeCriticalSection(&myLock); }

	~CriticalSection()
	{ ::DeleteCriticalSection(&myLock); }

private:
	CRITICAL_SECTION myLock;
};

class SimpleCriticalSection
{
public:
	SimpleCriticalSection(CriticalSection &myLocker):myLock(myLocker.myLock)
	{ ::EnterCriticalSection(&myLock); }
	~SimpleCriticalSection()
	{ ::LeaveCriticalSection(&myLock); }
private:
	CRITICAL_SECTION &myLock;
};