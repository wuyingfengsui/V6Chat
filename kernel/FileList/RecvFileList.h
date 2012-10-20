#pragma once

#include <map>
#include <string>

#include "../Lock/SimpleCriticalSection.h"

struct RecvFilePkg
{
	bool download;
	unsigned int uid;
	std::string fid;
	std::wstring pathname;               // 文件名，非路径
};

class RecvFileList
{
public:
	RecvFileList():filesCount(1)
	{}
	~RecvFileList()
	{}

	static RecvFileList *Instance();
	static void Release();

	int AddFileItem(const std::wstring &fileName);
	int AddFileItem(unsigned int uid, const std::string &fid, const std::wstring &fileName, bool downloaded = false);
	void DeleteFileItem(int number);
	bool GetFilePath(int number, std::wstring &fileName);
	bool GetDownloadedFilePath(int uid, const std::string fid, std::wstring &fileName);
	bool GetDownloadedFileNumber(int uid, const std::string fid, int& number);
	bool ChangeFileState(int uid, const std::string fid, bool download);

private:
	static RecvFileList *fileList;
	std::map<int, RecvFilePkg> files;
	int filesCount;
	static CriticalSection aCritSect;
};