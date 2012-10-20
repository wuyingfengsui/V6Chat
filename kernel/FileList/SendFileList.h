#pragma once

#include <map>
#include <string>

#include "../Lock/SimpleCriticalSection.h"

class SendFileList
{
public:
	SendFileList():filesCount(1)
	{}
	~SendFileList()
	{}

	static SendFileList *Instance();
	static void Release();

	int AddFileItem(const std::wstring &filePath);
	void DeleteFileItem(int number);
	bool GetFilePath(int number, std::wstring &filePath);

private:
	static SendFileList *fileList;
	std::map<int, std::wstring> files;
	int filesCount;
	static CriticalSection aCritSect;
};