#ifndef __STUB_H__
#define __STUB_H__

//linux
#include <memory.h>
#include <unistd.h>
#include <sys/mman.h>
//c
#include <cstdlib>
#include <climits>
//c++
#include <exception>
#include <map>
#include <utility>





/**********************************************************
                  replace function
**********************************************************/


#define ADDR(CLASS_NAME,MEMBER_NAME) (&CLASS_NAME::MEMBER_NAME)
#define CODESIZE 5U

struct func_stub
{
    void *fn;
    unsigned char code_buf[CODESIZE];
};


class Stub
{
public:
    Stub()
    {
        m_pagesize = sysconf(_SC_PAGE_SIZE);
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
            if (-1 == mprotect(pageof(pstub->fn), m_pagesize * 2, PROT_READ | PROT_WRITE | PROT_EXEC))
            {
                throw("stub mprotect to w+r+x faild");
            }

            memcpy(pstub->fn, pstub->code_buf, CODESIZE);
            
            if (-1 == mprotect(pageof(pstub->fn), m_pagesize * 2, PROT_READ | PROT_EXEC))
            {
                throw("stub mprotect to r+x failed");
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
        memcpy(pstub->code_buf, fn, CODESIZE);
        
        if (-1 == mprotect(pageof(fn), m_pagesize * 2, PROT_READ | PROT_WRITE | PROT_EXEC))
        {
            throw("stub set mprotect to w+r+x faild");
        }

        *(unsigned char *)fn = (unsigned char)0xE9;
        *(unsigned int *)((unsigned char *)fn + 1) = (unsigned char *)fn_stub - (unsigned char *)fn - CODESIZE;
        
        if (-1 == mprotect(pageof(fn), m_pagesize * 2, PROT_READ | PROT_EXEC))
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
        

        if (-1 == mprotect(pageof(pstub->fn), m_pagesize * 2, PROT_READ | PROT_WRITE | PROT_EXEC))
        {
            throw("stub reset mprotect to w+r+x faild");
        }

        memcpy(pstub->fn, pstub->code_buf, CODESIZE);
        
        if (-1 == mprotect(pageof(pstub->fn), m_pagesize * 2, PROT_READ | PROT_EXEC))
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
        return (void *)((unsigned long)p & ~(m_pagesize - 1));
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

private:
    long m_pagesize;
    std::map<void*, func_stub*> m_result;
    
};


#endif

