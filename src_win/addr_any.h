#ifndef __ADDR_ANY_H__
#define __ADDR_ANY_H__

#include <windows.h>
#include <tchar.h>
// Now we have to define _NO_CVCONST_H to be able to access 
// various declarations from DbgHelp.h, which are not available by default 
#define _NO_CVCONST_H
#include <dbghelp.h>

#ifdef __ADDR_ANY_DEBUG__
#include <stdio.h>
#endif
//注意：需要这两个文件
//dbghelp.dll	
//symsrv.dll
#pragma comment(lib,"dbghelp.lib")





///////////////////////////////////////////////////////////////////////////////
// CSymbolInfoPackage class declaration 
// 
// Wrapper for SYMBOL_INFO_PACKAGE structure 
//

struct CSymbolInfoPackage : public SYMBOL_INFO_PACKAGE 
{
	CSymbolInfoPackage() 
	{
		si.SizeOfStruct = sizeof(SYMBOL_INFO); 
		si.MaxNameLen   = sizeof(name); 
	}
};




inline static bool GetFileSize( const TCHAR* pFileName, DWORD& FileSize )
{
	// Check parameters 

	if( pFileName == 0 ) 
	{
		return false; 
	}


	// Open the file 

	HANDLE hFile = ::CreateFile( pFileName, GENERIC_READ, FILE_SHARE_READ, 
	                             NULL, OPEN_EXISTING, 0, NULL ); 

	if( hFile == INVALID_HANDLE_VALUE ) 
	{
#ifdef __ADDR_ANY_DEBUG__
		_tprintf( _T("CreateFile() failed. Error: %u \n"), ::GetLastError() ); 
#endif
		return false; 
	}


	// Obtain the size of the file 

	FileSize = ::GetFileSize( hFile, NULL ); 

	if( FileSize == INVALID_FILE_SIZE ) 
	{
#ifdef __ADDR_ANY_DEBUG__
		_tprintf( _T("GetFileSize() failed. Error: %u \n"), ::GetLastError() ); 
		// and continue ...
#endif
 
	}


	// Close the file 

	if( !::CloseHandle( hFile ) ) 
	{
#ifdef __ADDR_ANY_DEBUG__
		_tprintf( _T("CloseHandle() failed. Error: %u \n"), ::GetLastError() ); 
		// and continue ... 
#endif

	}


	// Complete 

	return ( FileSize != INVALID_FILE_SIZE ); 

}

inline static bool GetFileParams( const TCHAR* pFileName, DWORD64& BaseAddr, DWORD& FileSize ) 
{
	// Check parameters 

	if( pFileName == 0 ) 
	{
		return false; 
	}


	// Determine the extension of the file 

	TCHAR szFileExt[_MAX_EXT] = {0}; 

	_tsplitpath( pFileName, NULL, NULL, NULL, szFileExt ); 

	
	// Is it .PDB file ? 

	if( _tcsicmp( szFileExt, _T(".PDB") ) == 0 ) 
	{
		// Yes, it is a .PDB file 

		// Determine its size, and use a dummy base address 

		BaseAddr = 0x10000000; // it can be any non-zero value, but if we load symbols 
		                       // from more than one file, memory regions specified 
		                       // for different files should not overlap 
		                       // (region is "base address + file size") 
		
		if( !GetFileSize( pFileName, FileSize ) ) 
		{
			return false; 
		}

	}
	else 
	{
		// It is not a .PDB file 

		// Base address and file size can be 0 

		BaseAddr = 0; 
		FileSize = 0; 
	}


	// Complete 

	return true; 

}



inline  PVOID get_func_addr(const TCHAR* pFileExeName, const TCHAR* pFilePdbName, const TCHAR* pSymName) 
{
	BOOL bRet = FALSE; 
	PVOID address = 0; 

	// Set options 

	DWORD Options = SymGetOptions(); 

		// SYMOPT_DEBUG option asks DbgHelp to print additional troubleshooting 
		// messages to debug output - use the debugger's Debug Output window 
		// to view the messages 

	Options |= SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_FAVOR_COMPRESSED; 

	::SymSetOptions( Options ); 


	// Initialize DbgHelp and load symbols for all modules of the current process 

	bRet = ::SymInitialize ( 
	            GetCurrentProcess(),  // Process handle of the current process 
	            NULL,                 // No user-defined search path -> use default 
	            FALSE                 // Do not load symbols for modules in the current process 
	          ); 

	if( !bRet ) 
	{
#ifdef __ADDR_ANY_DEBUG__
		_tprintf(_T("Error: SymInitialize() failed. Error code: %u \n"), ::GetLastError());
#endif
		return 0; 
	}


	do
	{
		// Determine the base address and the file size 


		DWORD64   BaseAddr  = 0; 
		DWORD     FileSize  = 0; 

		if( !GetFileParams( pFilePdbName, BaseAddr, FileSize ) ) 
		{
#ifdef __ADDR_ANY_DEBUG__
			_tprintf( _T("Error: Cannot obtain file parameters (internal error).\n") ); 
#endif
			break; 
		}


		// Load symbols for the module 
#ifdef __ADDR_ANY_DEBUG__
		_tprintf( _T("Loading symbols for: %s ... \n"), pFilePdbName ); 
#endif
		
		BaseAddr = (DWORD64)GetModuleHandle(pFileExeName);

		DWORD64 ModBase = ::SymLoadModule64 ( 
								GetCurrentProcess(), // Process handle of the current process 
								NULL,                // Handle to the module's image file (not needed)
								pFilePdbName,           // Path/name of the file 
								NULL,                // User-defined short name of the module (it can be NULL) 
								BaseAddr,            // Base address of the module (cannot be NULL if .PDB file is used, otherwise it can be NULL) 
								FileSize             // Size of the file (cannot be NULL if .PDB file is used, otherwise it can be NULL) 
							); 

		if( ModBase == 0 ) 
		{
#ifdef __ADDR_ANY_DEBUG__
			_tprintf(_T("Error: SymLoadModule64() failed. Error code: %u \n"), ::GetLastError());
#endif	
			break; 
		}
#ifdef __ADDR_ANY_DEBUG__
			_tprintf( _T("Load address: %I64x \n"), ModBase ); 
		// Look up symbol by name 
		_tprintf( _T("Looking for symbol %s ... \n"), pSymName ); 
#endif	


		CSymbolInfoPackage sip; // it contains SYMBOL_INFO structure plus additional 
														// space for the name of the symbol 

		bRet = ::SymFromName( 
								GetCurrentProcess(), // Process handle of the current process 
								pSymName,            // Symbol name 
								&sip.si              // Address of the SYMBOL_INFO structure (inside "sip" object) 
							);

		if( !bRet ) 
		{
#ifdef __ADDR_ANY_DEBUG__
			_tprintf( _T("Error: SymFromName() failed. Error code: %u \n"), ::GetLastError() ); 

#endif	
		}
		else 
		{
#ifdef __ADDR_ANY_DEBUG__
			// Display information about the symbol 
			_tprintf( _T("Symbol found: \n") ); 

#endif	
		


			 address = (PVOID)sip.si.Address; 
		}


		// Unload symbols for the module 

		bRet = ::SymUnloadModule64( GetCurrentProcess(), ModBase ); 

		if( !bRet )
		{
#ifdef __ADDR_ANY_DEBUG__
			_tprintf( _T("Error: SymUnloadModule64() failed. Error code: %u \n"), ::GetLastError() ); 
			
#endif	
		}

	}
	while( 0 ); 


	// Deinitialize DbgHelp 

	bRet = ::SymCleanup( GetCurrentProcess() ); 

	if( !bRet ) 
	{
#ifdef __ADDR_ANY_DEBUG__
		_tprintf(_T("Error: SymCleanup() failed. Error code: %u \n"), ::GetLastError());
#endif
	}


	// Complete 

	return address; 
}









//获取函数地址PDB
inline static ULONG_PTR GetFunctionAddressFromPDB(HMODULE hMod, const TCHAR* szApiName)
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
#ifdef __ADDR_ANY_DEBUG__
		_tprintf(_T("SymFromName %ws returned error : %d\n"), szApiName, GetLastError());
#endif
		return 0;
	}
 
	return (ULONG_PTR)syminfo->Address;
}
 
//符号获取函数地址
inline PVOID get_func_addr_by_remote(const TCHAR* szDllName, const TCHAR* szApiName)
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
 
	HMODULE hDll = GetModuleHandle(szDllName);
	PVOID lpRet = NULL;
	lpRet = (PVOID)GetFunctionAddressFromPDB(hDll, szApiName);
	SymCleanup(GetCurrentProcess());
 
	return lpRet;
}
 
 #endif

 
