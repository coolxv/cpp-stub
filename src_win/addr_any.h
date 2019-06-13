//通过符号文件获取函数地址
 
#include <windows.h>
#include <stdio.h>
#include <Dbghelp.h>
#include <tchar.h>
#pragma comment(lib,"dbghelp.lib")
 
//注意：需要这两个文件
//dbghelp.dll	
//symsrv.dll
 
//获取函数地址PDB
inline ULONG_PTR GetFunctionAddressPDB(HMODULE hMod, const CHAR * szApiName)
{
	//定义变量
	BYTE memory[0x2000] = {0};
 
	//参数效验
	if (hMod == NULL)return NULL;
	if (szApiName == NULL)return NULL;
 
 
	ZeroMemory(memory, sizeof(memory));
	SYMBOL_INFO * syminfo = (SYMBOL_INFO *)memory;
	syminfo->SizeOfStruct = sizeof(SYMBOL_INFO);
	syminfo->MaxNameLen = MAX_SYM_NAME;
	syminfo->ModBase = (ULONG_PTR)hMod;
 
	if (!SymFromName(GetCurrentProcess(), szApiName, syminfo))
	{
		printf("SymFromName %ws returned error : %d\n", szApiName, GetLastError());
		return 0;
	}
 
	return (ULONG_PTR)syminfo->Address;
}
 
//符号获取函数地址
inline PVOID SymGetProcAddress(LPCSTR szDllName, LPCSTR szApiName)
{
	//变量定义
	CHAR symbolPath[0x2000] = { 0 };
	CHAR szPath[MAX_PATH] = { 0 };
 
	//参数效验
	if (szDllName == NULL)return NULL;
	if (szApiName == NULL)return NULL;
 
 
	GetModuleFileName(0, szPath, ARRAYSIZE(szPath));
	CHAR * temp = strchr(szPath, '\\');
	if (temp == NULL)return NULL;
	*temp = 0;
	strcat(symbolPath, "SRV*");
	strcat(symbolPath, szPath);
	strcat(symbolPath, "*http://msdl.microsoft.com/download/symbols");
	SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_FAVOR_COMPRESSED);
	HANDLE hProcess;
	hProcess = GetCurrentProcess();
	
	if (!SymInitialize(hProcess, symbolPath, TRUE))
	{
		return NULL;
	}
	symbolPath[0x2000] = { 0 };
	GetCurrentDirectory(sizeof(symbolPath),symbolPath);
	SymSetSearchPath(hProcess,symbolPath);
 
 
 
	HMODULE hDll = GetModuleHandle(szDllName);
	PVOID lpRet = NULL;
	lpRet = (PVOID)GetFunctionAddressPDB(hDll, szApiName);
	SymCleanup(GetCurrentProcess());
 
	return lpRet;
}
 
 
 