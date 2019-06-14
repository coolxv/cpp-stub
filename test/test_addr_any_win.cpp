///////////////////////////////////////////////////////////////////////////////
// Include files 
//

#include <windows.h>
#include <tchar.h>

// Now we have to define _NO_CVCONST_H to be able to access 
// various declarations from DbgHelp.h, which are not available by default 
#define _NO_CVCONST_H
#include <dbghelp.h>

#include <stdio.h>
#include <iostream>
using namespace std;

static int foo()
{
    cout<<"I am foo"<<endl;
    return 0;
}

int foo_stub()
{   
    cout<<"I am foo_stub"<<endl;
    return 0;
}


///////////////////////////////////////////////////////////////////////////////
//
// SymFromName.cpp 
// 
// Author: Oleg Starodumov
// 
//


///////////////////////////////////////////////////////////////////////////////
// 
// Description: 
// 
// This example looks up a symbol by name and displays some simple information 
// about it. 
// 
// This example shows how to: 
// 
//   * Define _NO_CVCONST_H to be able to use various non-default declarations 
//     from DbgHelp.h (e.g. SymTagEnum enumeration) 
//   * Initialize DbgHelp 
//   * Load symbols for a module or from a .PDB file 
//   * Check what kind of symbols is loaded 
//   * Look up a symbol by name (supplied by the user)
//   * Display simple information about the symbol 
//   * Unload symbols 
//   * Deinitialize DbgHelp 
//
// Actions: 
// 
//   * Enable debug option 
//   * Initialize DbgHelp 
//   * If symbols should be loaded from a .PDB file, determine its size 
//   * Load symbols 
//   * Obtain and display information about loaded symbols 
//   * Look up a symbol by name 
//   * Display simple information about the symbol
//   * Unload symbols 
//   * Deinitialize DbgHelp 
//
// Command line parameters: 
// 
//   * Path to the module you want to load symbols for, 
//     or to a .PDB file to load the symbols from 
//   * Name of the symbol to search for 
//



///////////////////////////////////////////////////////////////////////////////
// Directives 
//

#pragma comment( lib, "dbghelp.lib" )


///////////////////////////////////////////////////////////////////////////////
// Declarations 
//

bool GetFileParams( const TCHAR* pFileName, DWORD64& BaseAddr, DWORD& FileSize );
bool GetFileSize( const TCHAR* pFileName, DWORD& FileSize );
void ShowSymbolInfo( DWORD64 ModBase ); 

void ShowSymbolDetails( SYMBOL_INFO& SymInfo ); 

const TCHAR* TagStr( ULONG Tag ); 


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


///////////////////////////////////////////////////////////////////////////////
// main 
//

int _tmain( int argc, const TCHAR* argv[] ) 
{
	BOOL bRet = FALSE; 


	// Check command line parameters 

	if( argc < 3 ) 
	{
		_tprintf( _T("Usage: %s <FilePathAndName> <SymbolName> \n"), argv[0] ); 
		return 0; 
	}

	const TCHAR* pSymName = argv[2]; 


	// Set options 

	DWORD Options = SymGetOptions(); 

		// SYMOPT_DEBUG option asks DbgHelp to print additional troubleshooting 
		// messages to debug output - use the debugger's Debug Output window 
		// to view the messages 

	Options |= SYMOPT_DEBUG; 

	::SymSetOptions( Options ); 


	// Initialize DbgHelp and load symbols for all modules of the current process 

	bRet = ::SymInitialize ( 
	            GetCurrentProcess(),  // Process handle of the current process 
	            NULL,                 // No user-defined search path -> use default 
	            FALSE                 // Do not load symbols for modules in the current process 
	          ); 

	if( !bRet ) 
	{
		_tprintf(_T("Error: SymInitialize() failed. Error code: %u \n"), ::GetLastError());
		return 0; 
	}


	do
	{
		// Determine the base address and the file size 

		const TCHAR* pFileName = argv[1]; 

		DWORD64   BaseAddr  = 0; 
		DWORD     FileSize  = 0; 

		if( !GetFileParams( pFileName, BaseAddr, FileSize ) ) 
		{
			_tprintf( _T("Error: Cannot obtain file parameters (internal error).\n") ); 
			break; 
		}


		// Load symbols for the module 

		_tprintf( _T("Loading symbols for: %s ... \n"), pFileName ); 

		DWORD64 ModBase = ::SymLoadModule64 ( 
								GetCurrentProcess(), // Process handle of the current process 
								NULL,                // Handle to the module's image file (not needed)
								pFileName,           // Path/name of the file 
								NULL,                // User-defined short name of the module (it can be NULL) 
								BaseAddr,            // Base address of the module (cannot be NULL if .PDB file is used, otherwise it can be NULL) 
								FileSize             // Size of the file (cannot be NULL if .PDB file is used, otherwise it can be NULL) 
							); 

		if( ModBase == 0 ) 
		{
			_tprintf(_T("Error: SymLoadModule64() failed. Error code: %u \n"), ::GetLastError());
			break; 
		}

		_tprintf( _T("Load address: %I64x \n"), ModBase ); 


		// Look up symbol by name 

		_tprintf( _T("Looking for symbol %s ... \n"), pSymName ); 

		CSymbolInfoPackage sip; // it contains SYMBOL_INFO structure plus additional 
														// space for the name of the symbol 

		bRet = ::SymFromName( 
								GetCurrentProcess(), // Process handle of the current process 
								pSymName,            // Symbol name 
								&sip.si              // Address of the SYMBOL_INFO structure (inside "sip" object) 
							);

		if( !bRet ) 
		{
			_tprintf( _T("Error: SymFromName() failed. Error code: %u \n"), ::GetLastError() ); 
		}
		else 
		{
			// Display information about the symbol 

			_tprintf( _T("Symbol found: \n") ); 

			ShowSymbolDetails( sip.si ); 
		}


		// Unload symbols for the module 

		bRet = ::SymUnloadModule64( GetCurrentProcess(), ModBase ); 

		if( !bRet )
		{
			_tprintf( _T("Error: SymUnloadModule64() failed. Error code: %u \n"), ::GetLastError() ); 
		}

	}
	while( 0 ); 


	// Deinitialize DbgHelp 

	bRet = ::SymCleanup( GetCurrentProcess() ); 

	if( !bRet ) 
	{
		_tprintf(_T("Error: SymCleanup() failed. Error code: %u \n"), ::GetLastError());
		return 0; 
	}


	// Complete 

	return 0; 
}


///////////////////////////////////////////////////////////////////////////////
// Functions 
//

bool GetFileParams( const TCHAR* pFileName, DWORD64& BaseAddr, DWORD& FileSize ) 
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

bool GetFileSize( const TCHAR* pFileName, DWORD& FileSize )
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
		_tprintf( _T("CreateFile() failed. Error: %u \n"), ::GetLastError() ); 
		return false; 
	}


	// Obtain the size of the file 

	FileSize = ::GetFileSize( hFile, NULL ); 

	if( FileSize == INVALID_FILE_SIZE ) 
	{
		_tprintf( _T("GetFileSize() failed. Error: %u \n"), ::GetLastError() ); 
		// and continue ... 
	}


	// Close the file 

	if( !::CloseHandle( hFile ) ) 
	{
		_tprintf( _T("CloseHandle() failed. Error: %u \n"), ::GetLastError() ); 
		// and continue ... 
	}


	// Complete 

	return ( FileSize != INVALID_FILE_SIZE ); 

}

void ShowSymbolInfo( DWORD64 ModBase ) 
{
	// Get module information 

	IMAGEHLP_MODULE64 ModuleInfo; 

	memset(&ModuleInfo, 0, sizeof(ModuleInfo) ); 

	ModuleInfo.SizeOfStruct = sizeof(ModuleInfo); 

	BOOL bRet = ::SymGetModuleInfo64( GetCurrentProcess(), ModBase, &ModuleInfo ); 

	if( !bRet ) 
	{
		_tprintf(_T("Error: SymGetModuleInfo64() failed. Error code: %u \n"), ::GetLastError());
		return; 
	}


	// Display information about symbols 

		// Kind of symbols 

	switch( ModuleInfo.SymType ) 
	{
		case SymNone: 
			_tprintf( _T("No symbols available for the module.\n") ); 
			break; 

		case SymExport: 
			_tprintf( _T("Loaded symbols: Exports\n") ); 
			break; 

		case SymCoff: 
			_tprintf( _T("Loaded symbols: COFF\n") ); 
			break; 

		case SymCv: 
			_tprintf( _T("Loaded symbols: CodeView\n") ); 
			break; 

		case SymSym: 
			_tprintf( _T("Loaded symbols: SYM\n") ); 
			break; 

		case SymVirtual: 
			_tprintf( _T("Loaded symbols: Virtual\n") ); 
			break; 

		case SymPdb: 
			_tprintf( _T("Loaded symbols: PDB\n") ); 
			break; 

		case SymDia: 
			_tprintf( _T("Loaded symbols: DIA\n") ); 
			break; 

		case SymDeferred: 
			_tprintf( _T("Loaded symbols: Deferred\n") ); // not actually loaded 
			break; 

		default: 
			_tprintf( _T("Loaded symbols: Unknown format.\n") ); 
			break; 
	}

		// Image name 

	if( _tcslen( ModuleInfo.ImageName ) > 0 ) 
	{
		_tprintf( _T("Image name: %s \n"), ModuleInfo.ImageName ); 
	}

		// Loaded image name 

	if( _tcslen( ModuleInfo.LoadedImageName ) > 0 ) 
	{
		_tprintf( _T("Loaded image name: %s \n"), ModuleInfo.LoadedImageName ); 
	}

		// Loaded PDB name 

	if( _tcslen( ModuleInfo.LoadedPdbName ) > 0 ) 
	{
		_tprintf( _T("PDB file name: %s \n"), ModuleInfo.LoadedPdbName ); 
	}

		// Is debug information unmatched ? 
		// (It can only happen if the debug information is contained 
		// in a separate file (.DBG or .PDB) 

	if( ModuleInfo.PdbUnmatched || ModuleInfo.DbgUnmatched ) 
	{
		_tprintf( _T("Warning: Unmatched symbols. \n") ); 
	}

		// Contents 

			// Line numbers available ? 

	_tprintf( _T("Line numbers: %s \n"), ModuleInfo.LineNumbers ? _T("Available") : _T("Not available") ); 

			// Global symbols available ? 

	_tprintf( _T("Global symbols: %s \n"), ModuleInfo.GlobalSymbols ? _T("Available") : _T("Not available") ); 

			// Type information available ? 

	_tprintf( _T("Type information: %s \n"), ModuleInfo.TypeInfo ? _T("Available") : _T("Not available") ); 

			// Source indexing available ? 

	_tprintf( _T("Source indexing: %s \n"), ModuleInfo.SourceIndexed ? _T("Yes") : _T("No") ); 

			// Public symbols available ? 

	_tprintf( _T("Public symbols: %s \n"), ModuleInfo.Publics ? _T("Available") : _T("Not available") ); 


}

void ShowSymbolDetails( SYMBOL_INFO& SymInfo ) 
{
	// Kind of symbol (tag) 

	_tprintf( _T("Symbol: %s  "), TagStr(SymInfo.Tag) ); 

	// Address 

	_tprintf( _T("Address: %x  "), SymInfo.Address ); 

	// Size 

	_tprintf( _T("Size: %u  "), SymInfo.Size ); 

	// Name 

	_tprintf( _T("Name: %s"), SymInfo.Name ); 

	_tprintf( _T("\n") ); 

}

const TCHAR* TagStr( ULONG Tag ) 
{
	switch( Tag ) 
	{
    case SymTagNull:
			return _T("Null"); 
    case SymTagExe:
			return _T("Exe"); 
    case SymTagCompiland:
			return _T("Compiland"); 
    case SymTagCompilandDetails:
			return _T("CompilandDetails"); 
    case SymTagCompilandEnv:
			return _T("CompilandEnv"); 
    case SymTagFunction:
			return _T("Function"); 
    case SymTagBlock:
			return _T("Block"); 
    case SymTagData:
			return _T("Data"); 
    case SymTagAnnotation:
			return _T("Annotation"); 
    case SymTagLabel:
			return _T("Label"); 
    case SymTagPublicSymbol:
			return _T("PublicSymbol"); 
    case SymTagUDT:
			return _T("UDT"); 
    case SymTagEnum:
			return _T("Enum"); 
    case SymTagFunctionType:
			return _T("FunctionType"); 
    case SymTagPointerType:
			return _T("PointerType"); 
    case SymTagArrayType:
			return _T("ArrayType"); 
    case SymTagBaseType:
			return _T("BaseType"); 
    case SymTagTypedef:
			return _T("Typedef"); 
    case SymTagBaseClass:
			return _T("BaseClass"); 
    case SymTagFriend:
			return _T("Friend"); 
    case SymTagFunctionArgType:
			return _T("FunctionArgType"); 
    case SymTagFuncDebugStart:
			return _T("FuncDebugStart"); 
    case SymTagFuncDebugEnd:
			return _T("FuncDebugEnd"); 
    case SymTagUsingNamespace:
			return _T("UsingNamespace"); 
    case SymTagVTableShape:
			return _T("VTableShape"); 
    case SymTagVTable:
			return _T("VTable"); 
    case SymTagCustom:
			return _T("Custom"); 
    case SymTagThunk:
			return _T("Thunk"); 
    case SymTagCustomType:
			return _T("CustomType"); 
    case SymTagManagedType:
			return _T("ManagedType"); 
    case SymTagDimension:
			return _T("Dimension"); 
		default: 
			return _T("Unknown"); 
	}

	return _T(""); 

}

