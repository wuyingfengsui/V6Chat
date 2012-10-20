#include "RecvFileList.h"

RecvFileList *RecvFileList::fileList = NULL;
CriticalSection RecvFileList::aCritSect;

RecvFileList *RecvFileList::Instance()
{
	SimpleCriticalSection m(aCritSect);

	if(NULL == fileList)
		fileList = new RecvFileList;

	return fileList;
}

void RecvFileList::Release()
{
	SimpleCriticalSection m(aCritSect);

	if(fileList)
	{
		delete fileList;
		fileList = NULL;
	}
}

int RecvFileList::AddFileItem(const std::wstring &fileName)                 
{
	SimpleCriticalSection m(aCritSect);

	RecvFilePkg fp;
	fp.pathname = fileName;

	files[filesCount] = fp;
	return filesCount++;
}

int RecvFileList::AddFileItem(unsigned int uid, const std::string &fid, const std::wstring &fileName, bool downloaded)
{
	SimpleCriticalSection m(aCritSect);

	RecvFilePkg fp;
	fp.pathname = fileName;
	fp.fid = fid;
	fp.uid = uid;
	fp.download = downloaded;

	files[filesCount] = fp;
	return filesCount++;
}

void RecvFileList::DeleteFileItem(int number)
{
	SimpleCriticalSection m(aCritSect);

	if(files.count(number))
		files.erase(number);
}

bool RecvFileList::GetFilePath(int number, std::wstring &fileName)
{
	SimpleCriticalSection m(aCritSect);

	if(!files.count(number))
		return false;

	fileName = files[number].pathname;
	return true;
}

bool RecvFileList::GetDownloadedFilePath(int uid, const std::string fid, std::wstring &fileName)
{
	SimpleCriticalSection m(aCritSect);
	for(std::map<int, RecvFilePkg>::iterator it = files.begin(); it != files.end(); ++it)
	{
		if(it->second.uid == uid && it->second.fid == fid && it->second.download)
		{
			fileName = it->second.pathname;
			return true;
		}
	}
	return false;
}

bool RecvFileList::GetDownloadedFileNumber(int uid, const std::string fid, int& number)
{
	SimpleCriticalSection m(aCritSect);
	for(std::map<int, RecvFilePkg>::iterator it = files.begin(); it != files.end(); ++it)
	{
		if(it->second.uid == uid && it->second.fid == fid && it->second.download)
		{
			number = it->first;
			return true;
		}
	}
	return false;
}

bool RecvFileList::ChangeFileState(int uid, const std::string fid, bool download)
{
	SimpleCriticalSection m(aCritSect);
	for(std::map<int, RecvFilePkg>::iterator it = files.begin(); it != files.end(); ++it)
	{
		if(it->second.uid == uid && it->second.fid == fid)
		{
			if(it->second.download != download)
			{
				it->second.download = download;
				return true;
			}
			else
				return false;
		}
	}
	return false;
}