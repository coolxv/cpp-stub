#include<iostream>
#include<cstdio>
#include "stub.h"
#include "addr_any.h"

static int foo()
{
    int love = 3;
    auto a =  [love](int a){std::cout << "foo lambda:" << a + love << std::endl;};
    a(4);
    std::cout << "I am foo" << std::endl;
    return 0;
}


void foo_lambda_stub(int a, int love)
{
    //void <lambda>(int a){love=0x00000003 }
    std::cout << "I am foo_lambda_stub:" << love + a << std::endl;
    return;
}

#if 1

int main(int argc, char **argv)
{

    //Get application static function address
    {
        AddrAny any;
        
        std::map<std::string,void*> result;
        any.get_func_addr("<lambda_7a2556dcb8fa4823d2787bd5788e0b01>::operator()", result);
        
        foo();
        Stub stub;
        std::map<std::string,void*>::iterator it;
        for (it=result.begin(); it!=result.end(); ++it)
        {
            stub.set(it->second ,foo_lambda_stub);
            std::cout << it->first << " => " << it->second << std::endl;
        }
        foo();  
    
    }
    return 0;
}
#endif 
#if 0

#include <windows.h>
#include <stdio.h>
#include <dbghelp.h>

BOOL CALLBACK EnumSymProc(
    PSYMBOL_INFO pSymInfo,
    ULONG SymbolSize,
    PVOID UserContext)
{
    UNREFERENCED_PARAMETER(UserContext);

    printf("%llX   %s\n",
        pSymInfo->Address,  pSymInfo->Name);
    return TRUE;
}

void main()
{
    HANDLE hProcess = GetCurrentProcess();
    DWORD64 BaseOfDll;
    char* Mask = "<lambda_*>::operator()";
    BOOL status;
    foo();
    status = SymInitialize(hProcess, NULL, FALSE);
    if (status == FALSE)
    {
        return;
    }

    BaseOfDll = SymLoadModuleEx(hProcess,
        NULL,
        "test_addr_lambda_win.exe",
        NULL,
        0,
        0,
        NULL,
        0);

    if (BaseOfDll == 0)
    {
        SymCleanup(hProcess);
        return;
    }

    if (SymEnumSymbols(hProcess,     // Process handle from SymInitialize.
        BaseOfDll,   // Base address of module.
        Mask,        // Name of symbols to match.
        EnumSymProc, // Symbol handler procedure.
        NULL))       // User context.
    {
        // SymEnumSymbols succeeded
    }
    else
    {
        // SymEnumSymbols failed
        printf("SymEnumSymbols failed: %d\n", GetLastError());
    }

    SymCleanup(hProcess);
}
#endif 