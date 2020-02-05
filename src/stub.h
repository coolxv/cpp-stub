#ifndef __STUB_H__
#define __STUB_H__


#ifdef _WIN32 
//windows
#include <windows.h>
#else
//linux
#include <memory.h>
#include <unistd.h>
#include <sys/mman.h>
#endif
//c
#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <climits>
//c++
#include <iostream>
#include <exception>
#include <map>



//Supported operating systems  : windows,linux
//Supported hardware platform  : x86,x86-64
//Supported compiler           : msvc,gcc


#define ADDR(CLASS_NAME,MEMBER_NAME) (&CLASS_NAME::MEMBER_NAME)

/**********************************************************
                  replace function
**********************************************************/


#define CODESIZE 13U
#define CODESIZE_MIN 5U
#define CODESIZE_MAX CODESIZE


//13 byte(jmp m16:64)
//movabs $0x102030405060708,%r11
//jmpq   *%r11
#define REPLACE_FAR(fn, fn_stub)\
    *fn = 0x49;\
    *(fn + 1) = 0xbb;\
    *(long long *)(fn + 2) = (long long)fn_stub;\
    *(fn + 10) = 0x41;\
    *(fn + 11) = 0xff;\
    *(fn + 12) = 0xe3;

//5 byte(jmp rel32)
#define REPLACE_NEAR(fn, fn_stub)\
    *fn = 0xE9;\
    *(int *)(fn + 1) = (int)(fn_stub - fn - CODESIZE_MIN);


struct func_stub
{
    char *fn;
    unsigned char code_buf[CODESIZE];
    bool far_jmp;
};

class Stub
{
public:
    Stub()
    {
#ifdef _WIN32
        SYSTEM_INFO sys_info;  
        GetSystemInfo(&sys_info);
        m_pagesize = sys_info.dwPageSize;
#else
        m_pagesize = sysconf(_SC_PAGE_SIZE);
#endif       

        if (m_pagesize < 0)
        {
            m_pagesize = 4096;
        }
    }
    ~Stub()
    {
        std::map<char*,func_stub*>::iterator iter;
        struct func_stub *pstub;
        for(iter=m_result.begin(); iter != m_result.end(); iter++)
        {
            pstub = iter->second;
#ifdef _WIN32
            DWORD lpflOldProtect;
            if(0 == VirtualProtect(pageof(pstub->fn), m_pagesize * 2, PAGE_EXECUTE_READWRITE, &lpflOldProtect))
#else
            if (-1 != mprotect(pageof(pstub->fn), m_pagesize * 2, PROT_READ | PROT_WRITE | PROT_EXEC))
#endif       
            {

                if(pstub->far_jmp)
                {
                    memcpy(pstub->fn, pstub->code_buf, CODESIZE_MAX);
                }
                else
                {
                    memcpy(pstub->fn, pstub->code_buf, CODESIZE_MIN);
                }

#ifdef _WIN32
                VirtualProtect(pageof(pstub->fn), m_pagesize * 2, PAGE_EXECUTE_READ, &lpflOldProtect);
#else
                mprotect(pageof(pstub->fn), m_pagesize * 2, PROT_READ | PROT_EXEC);
#endif     
            }

            iter->second  = NULL;
            delete pstub;        
        }
        
        return;
    }
    template<typename T,typename S>
    void set(T addr, S addr_stub)
    {
        char * fn;
        char * fn_stub;
        fn = addrof(addr);
        fn_stub = addrof(addr_stub);
        struct func_stub *pstub;
        pstub = new func_stub;
        //start
        pstub->fn = fn;

        if(distanceof(fn, fn_stub))
        {
            pstub->far_jmp = true;
            memcpy(pstub->code_buf, fn, CODESIZE_MAX);
        }
        else
        {
            pstub->far_jmp = false;
            memcpy(pstub->code_buf, fn, CODESIZE_MIN);
        }

#ifdef _WIN32
        DWORD lpflOldProtect;
        if(0 == VirtualProtect(pageof(pstub->fn), m_pagesize * 2, PAGE_EXECUTE_READWRITE, &lpflOldProtect))
#else
        if (-1 == mprotect(pageof(pstub->fn), m_pagesize * 2, PROT_READ | PROT_WRITE | PROT_EXEC))
#endif       
        {
            throw("stub set memory protect to w+r+x faild");
        }

        if(pstub->far_jmp)
        {
            REPLACE_FAR(fn, fn_stub);
        }
        else
        {
            REPLACE_NEAR(fn, fn_stub);
        }


#ifdef _WIN32
        if(0 == VirtualProtect(pageof(pstub->fn), m_pagesize * 2, PAGE_EXECUTE_READ, &lpflOldProtect))
#else
        if (-1 == mprotect(pageof(pstub->fn), m_pagesize * 2, PROT_READ | PROT_EXEC))
#endif     
        {
            throw("stub set memory protect to r+x failed");
        }
        m_result.insert(std::pair<char*,func_stub*>(fn,pstub));
        return;
    }

    template<typename T>
    void reset(T addr)
    {
        char * fn;
        fn = addrof(addr);
        
        std::map<char*,func_stub*>::iterator iter = m_result.find(fn);
        
        if (iter == m_result.end())
        {
            return;
        }
        struct func_stub *pstub;
        pstub = iter->second;
        
#ifdef _WIN32
        DWORD lpflOldProtect;
        if(0 == VirtualProtect(pageof(pstub->fn), m_pagesize * 2, PAGE_EXECUTE_READWRITE, &lpflOldProtect))
#else
        if (-1 == mprotect(pageof(pstub->fn), m_pagesize * 2, PROT_READ | PROT_WRITE | PROT_EXEC))
#endif       
        {
            throw("stub reset memory protect to w+r+x faild");
        }

        if(pstub->far_jmp)
        {
            memcpy(pstub->fn, pstub->code_buf, CODESIZE_MAX);
        }
        else
        {
            memcpy(pstub->fn, pstub->code_buf, CODESIZE_MIN);
        }


#ifdef _WIN32
        if(0 == VirtualProtect(pageof(pstub->fn), m_pagesize * 2, PAGE_EXECUTE_READ, &lpflOldProtect))
#else
        if (-1 == mprotect(pageof(pstub->fn), m_pagesize * 2, PROT_READ | PROT_EXEC))
#endif     
        {
            throw("stub reset memory protect to r+x failed");
        }
        m_result.erase(iter);
        delete pstub;
        
        return;
    }
private:
    char *pageof(char* addr)
    { 
#ifdef _WIN32
        return (char *)((unsigned long long)addr & ~(m_pagesize - 1));
#else
        return (char *)((unsigned long)addr & ~(m_pagesize - 1));
#endif   
    }

    template<typename T>
    char* addrof(T addr)
    {
        union 
        {
          T _s;
          char* _d;
        }ut;
        ut._s = addr;
        return ut._d;
    }

    bool distanceof(char* addr, char* addr_stub)
    {
        std::ptrdiff_t diff = addr_stub - addr;
    
        if((diff > 0 && diff > 0xffffffff)|| (diff < 0 && -diff > 0xffffffff))
        {
            return true;
        }
        else 
        {
            return false;
        }
    }

private:
#ifdef _WIN32
    //LLP64
    long long m_pagesize;
#else
    //LP64
    long m_pagesize;
#endif   
    std::map<char*, func_stub*> m_result;
    
};


#endif
