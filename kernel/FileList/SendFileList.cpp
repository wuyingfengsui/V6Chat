#include "SendFileList.h"

SendFileList *SendFileList::fileList = NULL;
CriticalSection SendFileList::aCritSect;

SendFileList *SendFileList::Instance()
{
	SimpleCriticalSection m(aCritSect);

	if(NULL == fileList)
		fileList = new SendFileList;

	return fileList;
}

void SendFileList::Release()
{
	SimpleCriticalSection m(aCritSect);

	if(fileList)
	{
		delete fileList;
		fileList = NULL;
	}
}

int SendFileList::AddFileItem(const std::wstring &filePath)
{
	SimpleCriticalSection m(aCritSect);

	files[filesCount] = filePath;
	return filesCount++;
}

void SendFileList::DeleteFileItem(int number)
{
	SimpleCriticalSection m(aCritSect);

	if(files.count(number))
		files.erase(number);
}

bool SendFileList::GetFilePath(int number, std::wstring &filePath)
{
	SimpleCriticalSection m(aCritSect);

	if(!files.count(number))
		return false;

	filePath = files[number];
	return true;
}