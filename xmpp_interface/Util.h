#pragma once

#include <Windows.h>

#include <string>
#include <vector>

/*
 * 一些转码方法
 */
std::string AnsiToUtf8(const char *buf)
{
	int nlen = ::MultiByteToWideChar(CP_ACP, 0, buf, -1, NULL, 0);
	if (nlen == 0) return "";
	std::vector<wchar_t> unicode(nlen);
	::MultiByteToWideChar(CP_ACP, 0, buf, -1, &unicode[0], nlen);

	nlen = ::WideCharToMultiByte(CP_UTF8, 0, &unicode[0], -1, NULL, 0, NULL, NULL);
	std::vector<char> utf8(nlen);
	::WideCharToMultiByte(CP_UTF8, 0, &unicode[0], -1, &utf8[0], nlen, NULL, NULL);
	return &utf8[0];
}

std::string Utf8ToAnsi(const char *buf)
{
	int   nLen = ::MultiByteToWideChar(CP_UTF8, 0, buf, -1, NULL, 0);
	if (nLen == 0) return "";
	std::vector<wchar_t> uBuf(nLen);
	::MultiByteToWideChar(CP_UTF8, 0, buf, -1, &uBuf[0], nLen);

	nLen = ::WideCharToMultiByte(CP_ACP, 0, &uBuf[0], -1, NULL, 0, NULL, NULL);
	std::vector<char> utf8(nLen);
	::WideCharToMultiByte(CP_ACP, 0, &uBuf[0], -1, &utf8[0], nLen, NULL, NULL);
	return &utf8[0];
}