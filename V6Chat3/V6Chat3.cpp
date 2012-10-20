#include "stdafx.h"

#include <string>
#include <Shlwapi.h>

#include "V6ExceptionHandler.h"

using namespace std;

const WCHAR* GetResDir()
{
    static WCHAR wszModulePath[MAX_PATH];
    GetModuleFileNameW(NULL,wszModulePath,MAX_PATH);
	PathAppend(wszModulePath, L"..\\..\\XAR");
    return wszModulePath;
}

int __stdcall LuaErrorHandle(lua_State* luaState,const wchar_t* pExtInfo,const wchar_t* wcszLuaErrorString,PXL_LRT_ERROR_STACK pStackInfo)
{
    static bool s_bEnter = false;
    if (!s_bEnter)
    {
        s_bEnter = true;
        if(pExtInfo != NULL)
        {
			wstring str = wcszLuaErrorString ? wcszLuaErrorString : L"";
            str += L" @ ";
            str += pExtInfo;

            MessageBox(0,str.c_str(),L"��������ˣ�",MB_ICONERROR | MB_OK);

        }
        else
        {
			MessageBox(0,wcszLuaErrorString ? wcszLuaErrorString : L"" ,L"��������ˣ�",MB_ICONERROR | MB_OK);
        }
        s_bEnter = false;
    }
    return 0;
}



bool InitXLUE()
{
    //��ʼ��ͼ�ο�
    XLGraphicParam param;
    XL_PrepareGraphicParam(&param);
	param.textType = XLTEXT_TYPE_FREETYPE;           //��ʼ��freetype����Ⱦ�ı�
	long result = XL_InitGraphicLib(&param);
	result = XL_SetFreeTypeEnabled(TRUE);            //��ʼ��freetype����Ⱦ�ı�
    //��ʼ��XLUE,�⺯����һ�����ϳ�ʼ������
    //����˳�ʼ��Lua����,��׼����,XLUELoader�Ĺ���
    result = XLUE_InitLoader(NULL);

	//����һ���򵥵Ľű�������ʾ
    XLLRT_ErrorHandle(LuaErrorHandle);
    return true; 
}

void UninitXLUE()
{
    //�˳�����
    XLUE_Uninit(NULL);
    XLUE_UninitLuaHost(NULL);
    XL_UnInitGraphicLib();
    XLUE_UninitHandleMap(NULL);
}

void RunLuaFunc(XL_LRT_RUNTIME_HANDLE hRuntime, const wchar_t* chunkName, const wchar_t* fileName, const char* funcName)
{
	lua_State* luaState = XLLRT_GetLuaState(hRuntime);
	XL_LRT_CHUNK_HANDLE hThisChunk = NULL;
	XLLRT_CreateChunkFromModule(chunkName, fileName, funcName, &hThisChunk);
	if ( hThisChunk != NULL )
	{
		long lRet = XLLRT_PrepareChunk(hRuntime, hThisChunk);
		if (lRet == 0)
		{
			if (XLLRT_LuaCall(luaState, 0, 0, fileName))
			{
				MessageBoxA(NULL, XLLRT_GetLastError(luaState), 0, 0);
			}
		}
		else
		{
			MessageBoxA(NULL, XLLRT_GetLastError(luaState), 0, 0);
		}

		XLLRT_ReleaseChunk(hThisChunk);
	}
}

void InitLuaHelper()                                // ����һЩlua�������ɿ������
{
	XL_LRT_ENV_HANDLE hEnv = XLLRT_GetEnv(NULL);
	XL_LRT_RUNTIME_HANDLE hRuntime = XLLRT_GetRuntime(hEnv, NULL);



	wstring strLuaCodePathName = GetResDir();
	strLuaCodePathName += L"\\Util\\layout\\LuaCode\\UtilFunc.lua";

	
	RunLuaFunc(hRuntime,
			L"UtilFunc.lua", 
			strLuaCodePathName.c_str(), 
			"RegisterObject");


	XLLRT_ReleaseRunTime(hRuntime);
	XLLRT_ReleaseEnv(hEnv);
}

bool LoadBeginXAR()
{
    long result = 0;
    //����XAR������·��
    result = XLUE_AddXARSearchPath(GetResDir());
    //������XAR,��ʱ��ִ�и�XAR�������ű�onload.lua
    result = XLUE_LoadXAR("Begin");
    if(result != 0)
    {
        return false;
    }
    return true;
}



int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: �ڴ˷��ô��롣
	::CoInitialize(NULL);
	::OleInitialize(NULL);

	if(!InitXLUE())
    {
        MessageBoxW(NULL,L"��ʼ��XLUE ʧ��!",L"����",MB_OK);
        return 1;
    }

	LoadLibrary(TEXT("kernel.dll"));                //����kernel.dll

	InitLuaHelper();

	// ���������¼
	wstring fileName = L"dump";
	SYSTEMTIME  sysTime;
	::GetLocalTime(&sysTime);
	wchar_t st[15];
	wsprintf(st, L"%4d%02d%02d%02d%02d%02d", sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
	fileName = fileName + st + L".txt";
	V6ExceptionHandler exceptionHandler(fileName.c_str());

    if(!LoadBeginXAR())
    {
        MessageBoxW(NULL,L"Load XARʧ��!",L"����",MB_OK);
        return 1;
    }

	
	MSG msg;
	
	
	// ����Ϣѭ��:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UninitXLUE();
	::OleUninitialize();
	::CoUninitialize();

	return (int) msg.wParam;
}