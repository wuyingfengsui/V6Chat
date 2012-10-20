#pragma once

#include <string>

struct V6Util
{
	static std::wstring Utf8ToUnicode(const char* buf);
	static std::string  UnicodeToUtf8(const wchar_t* buf);
	static std::string  UnicodeToAnsi(const wchar_t* buf);
	static std::wstring AnsiToUnicode(const char* buf);
	static std::string AnsiToUtf8(const char *buf);
	static std::string Utf8ToAnsi(const char *buf);

	static void V6Log(const char * fmt, ...);

	static void *GetUserDataImageHandle(const wchar_t* path);
	static void *GetImageHandle(const wchar_t* path);          // 打开图片文件（支持jpg和png格式）
	static bool SelectFile(const wchar_t* fileType, const wchar_t* lpstrTitle, std::wstring &path);
};