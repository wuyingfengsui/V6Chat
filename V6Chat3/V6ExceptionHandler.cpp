#include "V6ExceptionHandler.h"

#include <Shlwapi.h>
#include <sstream>

using namespace std;

LPTOP_LEVEL_EXCEPTION_FILTER V6ExceptionHandler::m_previousFilter;
wstring V6ExceptionHandler::logPathName;

V6ExceptionHandler::V6ExceptionHandler(const wchar_t* logFileName)
{
	m_previousFilter = SetUnhandledExceptionFilter(V6UnhandledExceptionFilter);

	wstring path = L"..\\";
	path += logFileName;
	WCHAR wszModulePath[MAX_PATH];
    GetModuleFileName(NULL,wszModulePath,MAX_PATH);
	PathAppend(wszModulePath, path.c_str());
	logPathName = wszModulePath;
}

V6ExceptionHandler::~V6ExceptionHandler()
{
	SetUnhandledExceptionFilter( m_previousFilter );
}

long __stdcall V6ExceptionHandler::V6UnhandledExceptionFilter( PEXCEPTION_POINTERS pExceptionInfo )
{
	HANDLE m_hReportFile = CreateFile( logPathName.c_str(),
		GENERIC_WRITE,
		0,
		0,
		OPEN_ALWAYS,
		FILE_FLAG_WRITE_THROUGH,
		0 );

	if ( m_hReportFile )
	{
		SetFilePointer( m_hReportFile, 0, 0, FILE_END );

		// Start out with a banner
		_tprintf( m_hReportFile, L"//=====================================================\n" );

		PEXCEPTION_RECORD pExceptionRecord = pExceptionInfo->ExceptionRecord;

		// First print information about the type of fault
		_tprintf( m_hReportFile, L"Exception code: %08X %s\n",
			pExceptionRecord->ExceptionCode,
			GetExceptionString(pExceptionRecord->ExceptionCode) );

		// Now print information about where the fault occured
		TCHAR szFaultingModule[MAX_PATH];
		DWORD section, offset;
		GetLogicalAddress(  pExceptionRecord->ExceptionAddress,
			szFaultingModule,
			sizeof( szFaultingModule ),
			section, offset );

		_tprintf( m_hReportFile, L"Fault address:  %08X %02X:%08X %s\n",
			pExceptionRecord->ExceptionAddress,
			section, offset, szFaultingModule );

		PCONTEXT pCtx = pExceptionInfo->ContextRecord;

		// Show the registers
#ifdef _M_IX86  // Intel Only!
		_tprintf( m_hReportFile, TEXT("\nRegisters:\n") );

		_tprintf(m_hReportFile, TEXT("EAX:%08X\nEBX:%08X\nECX:%08X\nEDX:%08X\nESI:%08X\nEDI:%08X\n"),
			pCtx->Eax, pCtx->Ebx, pCtx->Ecx, pCtx->Edx, pCtx->Esi, pCtx->Edi );

		_tprintf( m_hReportFile, TEXT("CS:EIP:%04X:%08X\n"), pCtx->SegCs, pCtx->Eip );
		_tprintf( m_hReportFile, TEXT("SS:ESP:%04X:%08X  EBP:%08X\n"),
			pCtx->SegSs, pCtx->Esp, pCtx->Ebp );
		_tprintf( m_hReportFile, TEXT("DS:%04X  ES:%04X  FS:%04X  GS:%04X\n"),
			pCtx->SegDs, pCtx->SegEs, pCtx->SegFs, pCtx->SegGs );
		_tprintf( m_hReportFile, TEXT("Flags:%08X\n"), pCtx->EFlags );

		// Walk the stack using x86 specific code
		IntelStackWalk( m_hReportFile, pCtx );

#endif

		_tprintf( m_hReportFile, TEXT("\n") );

		wstringstream error;
		error << L"非常抱歉，程序出错了！您可以将" << logPathName << L"发送到wuyingfengsui@gmail.com，以便完善，谢谢支持！" << endl;
		MessageBox(NULL, error.str().c_str(), L"非常抱歉，程序出错了！", MB_OK);

		CloseHandle( m_hReportFile );
	}

	return EXCEPTION_EXECUTE_HANDLER;     // 程序退出
}

LPTSTR V6ExceptionHandler::GetExceptionString( DWORD dwCode )
{
#define EXCEPTION( x ) case EXCEPTION_##x: return TEXT(#x);

	switch ( dwCode )
	{
		EXCEPTION( ACCESS_VIOLATION )
			EXCEPTION( DATATYPE_MISALIGNMENT )
			EXCEPTION( BREAKPOINT )
			EXCEPTION( SINGLE_STEP )
			EXCEPTION( ARRAY_BOUNDS_EXCEEDED )
			EXCEPTION( FLT_DENORMAL_OPERAND )
			EXCEPTION( FLT_DIVIDE_BY_ZERO )
			EXCEPTION( FLT_INEXACT_RESULT )
			EXCEPTION( FLT_INVALID_OPERATION )
			EXCEPTION( FLT_OVERFLOW )
			EXCEPTION( FLT_STACK_CHECK )
			EXCEPTION( FLT_UNDERFLOW )
			EXCEPTION( INT_DIVIDE_BY_ZERO )
			EXCEPTION( INT_OVERFLOW )
			EXCEPTION( PRIV_INSTRUCTION )
			EXCEPTION( IN_PAGE_ERROR )
			EXCEPTION( ILLEGAL_INSTRUCTION )
			EXCEPTION( NONCONTINUABLE_EXCEPTION )
			EXCEPTION( STACK_OVERFLOW )
			EXCEPTION( INVALID_DISPOSITION )
			EXCEPTION( GUARD_PAGE )
			EXCEPTION( INVALID_HANDLE )
	}

	// If not one of the "known" exceptions, try to get the string
	// from NTDLL.DLL's message table.

	static TCHAR szBuffer[512] = { 0 };

	FormatMessage(  FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
		GetModuleHandle( TEXT("NTDLL.DLL") ),
		dwCode, 0, szBuffer, sizeof( szBuffer ), 0 );

	return szBuffer;
}

BOOL V6ExceptionHandler::GetLogicalAddress(  PVOID addr, PTSTR szModule, DWORD len,
                                    DWORD& section, DWORD& offset )
{
	MEMORY_BASIC_INFORMATION mbi;

	if ( !VirtualQuery( addr, &mbi, sizeof(mbi) ) )
		return FALSE;

	DWORD hMod = (DWORD)mbi.AllocationBase;

	if ( !GetModuleFileName( (HMODULE)hMod, szModule, len ) )
		return FALSE;

	// Point to the DOS header in memory
	PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)hMod;

	// From the DOS header, find the NT (PE) header
	PIMAGE_NT_HEADERS pNtHdr = (PIMAGE_NT_HEADERS)(hMod + pDosHdr->e_lfanew);

	PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION( pNtHdr );

	DWORD rva = (DWORD)addr - hMod; // RVA is offset from module load address

	// Iterate through the section table, looking for the one that encompasses
	// the linear address.
	for (   unsigned i = 0;
		i < pNtHdr->FileHeader.NumberOfSections;
		i++, pSection++ )
	{
		DWORD sectionStart = pSection->VirtualAddress;
		DWORD sectionEnd = sectionStart
			+ max(pSection->SizeOfRawData, pSection->Misc.VirtualSize);

		// Is the address in this section???
		if ( (rva >= sectionStart) && (rva <= sectionEnd) )
		{
			// Yes, address is in the section.  Calculate section and offset,
			// and store in the "section" & "offset" params, which were
			// passed by reference.
			section = i+1;
			offset = rva - sectionStart;
			return TRUE;
		}
	}

	return FALSE;   // Should never get here!
}

void V6ExceptionHandler::IntelStackWalk( HANDLE m_hReportFile, PCONTEXT pContext )
{
	_tprintf( m_hReportFile, TEXT("\nCall stack:\n") );

	_tprintf( m_hReportFile, TEXT("Address Frame Logical addr Module\n") );

	DWORD pc = pContext->Eip;
	PDWORD pFrame, pPrevFrame;

	pFrame = (PDWORD)pContext->Ebp;

	do
	{
		TCHAR szModule[MAX_PATH] = {0};
		DWORD section = 0, offset = 0;

		GetLogicalAddress((PVOID)pc, szModule,sizeof(szModule),section,offset );

		_tprintf( m_hReportFile, TEXT("%08X  %08X  %04X:%08X %s\n"),
			pc, pFrame, section, offset, szModule );

		pc = pFrame[1];

		pPrevFrame = pFrame;

		pFrame = (PDWORD)pFrame[0]; // precede to next higher frame on stack

		if ( (DWORD)pFrame & 3 )    // Frame pointer must be aligned on a
			break;                  // DWORD boundary.  Bail if not so.

		if ( pFrame <= pPrevFrame )
			break;

		// Can two DWORDs be read from the supposed frame address?          
		if ( IsBadWritePtr(pFrame, sizeof(PVOID)*2) )
			break;

	} while ( 1 );
}

int __cdecl V6ExceptionHandler::_tprintf(HANDLE m_hReportFile, const TCHAR * format, ...)
{
	TCHAR szBuff[1024];
	int retValue;
	DWORD cbWritten;
	va_list argptr;

	va_start( argptr, format );
	retValue = wvsprintf( szBuff, format, argptr );
	va_end( argptr );

	WriteFile( m_hReportFile, szBuff, retValue * sizeof(TCHAR), &cbWritten, 0 );

	return retValue;
}