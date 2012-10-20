#pragma once

#include <Windows.h>
#include <string>

class V6ExceptionHandler
{
public:
	V6ExceptionHandler(const wchar_t* logFileName);
	~V6ExceptionHandler();

private:
	static long __stdcall V6UnhandledExceptionFilter(
                                PEXCEPTION_POINTERS pExceptionInfo );

	static LPTSTR GetExceptionString( DWORD dwCode );
	static BOOL GetLogicalAddress(  PVOID addr, PTSTR szModule, DWORD len,
                                    DWORD& section, DWORD& offset );
	static void IntelStackWalk( HANDLE m_hReportFile, PCONTEXT pContext );

	static int __cdecl _tprintf(HANDLE m_hReportFile, const TCHAR * format, ...);

	static LPTOP_LEVEL_EXCEPTION_FILTER m_previousFilter;
	static std::wstring logPathName;
};