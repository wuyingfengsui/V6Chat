// dllÈë¿Ú
#include "stdafx.h"
#include "LuaV6Chat3App.h"

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    if(ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
		//×¢²áapp
		XL_LRT_ENV_HANDLE hEnv = XLLRT_GetEnv(NULL);
		LuaV6Chat3App::RegisterObj(hEnv);
		XLLRT_ReleaseEnv(hEnv);
    }

    return TRUE;
}