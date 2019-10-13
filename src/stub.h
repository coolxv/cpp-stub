#ifndef __STUB_H__
#define __STUB_H__


#ifdef _WIN32 
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
#include <climits>
//c++
#include <iostream>
#include <exception>
#include <map>



//base on C++03
/**********************************************************
                  replace function
**********************************************************/
#define ADDR(CLASS_NAME,MEMBER_NAME) (&CLASS_NAME::MEMBER_NAME)
#ifdef __x86_64__
#define CODESIZE 13U
#define CODESIZE_MIN 5U
#define CODESIZE_MAX CODESIZE
#else
#define CODESIZE 5U
#endif


struct func_stub
{
    void *fn;
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
        
        std::map<void*,func_stub*>::iterator iter;
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
#ifdef __x86_64__
            if(pstub->far_jmp)
            {
                memcpy(pstub->fn, pstub->code_buf, CODESIZE_MAX);
            }
            else
            {
                memcpy(pstub->fn, pstub->code_buf, CODESIZE_MIN);
            }
#else
            memcpy(pstub->fn, pstub->code_buf, CODESIZE);
#endif
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
        void * fn;
        void * fn_stub;
        fn = addrof(addr);
        fn_stub = addrof(addr_stub);
        struct func_stub *pstub;
        pstub = new func_stub;
        //start
        pstub->fn = fn;
#ifdef __x86_64__
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
#else
        memcpy(pstub->code_buf, fn, CODESIZE);
#endif
#ifdef _WIN32
        DWORD lpflOldProtect;
        if(0 == VirtualProtect(pageof(pstub->fn), m_pagesize * 2, PAGE_EXECUTE_READWRITE, &lpflOldProtect))
#else
        if (-1 == mprotect(pageof(pstub->fn), m_pagesize * 2, PROT_READ | PROT_WRITE | PROT_EXEC))
#endif       
        {
            throw("stub set mprotect to w+r+x faild");
        }

#ifdef __x86_64__
        if(pstub->far_jmp)
        {
            //13 byte
            *(unsigned char*)fn = 0x49;
            *((unsigned char*)fn + 1) = 0xbb;
            *(unsigned long long *)((unsigned char *)fn + 2) = (unsigned long long)fn_stub;
            *(unsigned char *)((unsigned char *)fn + 10) = 0x41;
            *(unsigned char *)((unsigned char *)fn + 11) = 0x53;
            *(unsigned char *)((unsigned char *)fn + 12) = 0xc3;
        }
        else
        {
            //5 byte
            *(unsigned char *)fn = (unsigned char)0xE9;
            *(unsigned int *)((unsigned char *)fn + 1) = (unsigned char *)fn_stub - (unsigned char *)fn - CODESIZE_MIN;
        }
#else
         //5 byte
         *(unsigned char *)fn = (unsigned char)0xE9;
         *(unsigned int *)((unsigned char *)fn + 1) = (unsigned char *)fn_stub - (unsigned char *)fn - CODESIZE;
#endif





#ifdef _WIN32
        if(0 == VirtualProtect(pageof(pstub->fn), m_pagesize * 2, PAGE_EXECUTE_READ, &lpflOldProtect))
#else
        if (-1 == mprotect(pageof(pstub->fn), m_pagesize * 2, PROT_READ | PROT_EXEC))
#endif     
        {
            throw("stub set mprotect to r+x failed");
        }
        m_result.insert(std::pair<void*,func_stub*>(fn,pstub));
        return;
    }

    template<typename T>
    void reset(T addr)
    {
        void * fn;
        fn = addrof(addr);
        
        std::map<void*,func_stub*>::iterator iter = m_result.find(fn);
        
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
            throw("stub reset mprotect to w+r+x faild");
        }
#ifdef __x86_64__
        if(pstub->far_jmp)
        {
            memcpy(pstub->fn, pstub->code_buf, CODESIZE_MAX);
        }
        else
        {
            memcpy(pstub->fn, pstub->code_buf, CODESIZE_MIN);
        }
#else
        memcpy(pstub->fn, pstub->code_buf, CODESIZE);
#endif

#ifdef _WIN32
        if(0 == VirtualProtect(pageof(pstub->fn), m_pagesize * 2, PAGE_EXECUTE_READ, &lpflOldProtect))
#else
        if (-1 == mprotect(pageof(pstub->fn), m_pagesize * 2, PROT_READ | PROT_EXEC))
#endif     
        {
            throw("stub reset mprotect to r+x failed");
        }
        m_result.erase(iter);
        delete pstub;
        
        return;
    }
private:
    void *pageof(const void* p)
    { 
#ifdef _WIN32
        return (void *)((unsigned long long)p & ~(m_pagesize - 1));
#else
        return (void *)((unsigned long)p & ~(m_pagesize - 1));
#endif   
    }

    template<typename T>
    void* addrof(T src)
    {
        union 
        {
          T _s;
          void* _d;
        }ut;
        ut._s = src;
        return ut._d;
    }
#ifdef __x86_64__
	bool distanceof(void* addr, void* addr_stub)
	{
		unsigned long long addr_tmp = (unsigned long long)addr;
		unsigned long long addr_stub_tmp = (unsigned long long)addr_stub;
		unsigned int int_addr_tmp = (unsigned int)(addr_tmp >> 32);
		unsigned int int_addr_stub_tmp = (unsigned int)(addr_stub_tmp >> 32);
		if((int_addr_tmp > 0 && int_addr_stub_tmp > 0) || (int_addr_tmp == 0 && int_addr_stub_tmp == 0))
		{
			return false;
		}
		else 
		{
			return true;
		}
	}
#endif
private:
#ifdef _WIN32
    long long m_pagesize;
#else
    long m_pagesize;
#endif   
    std::map<void*, func_stub*> m_result;
    
};


#endif
