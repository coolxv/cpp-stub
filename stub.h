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
#include <cstdlib>
#include <climits>
//c++
#include<iostream>
#include <exception>
#include <map>
#include <utility>
#if defined(_WIN32) &&  (_MSC_VER >= 1800) || (__cplusplus >= 201103L)
#include <type_traits>
#endif






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
                memcpy(pstub->fn, pstub->code_buf, CODESIZE);
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
        memcpy(pstub->code_buf, fn, CODESIZE);
#ifdef _WIN32
        DWORD lpflOldProtect;
        if(0 == VirtualProtect(pageof(pstub->fn), m_pagesize * 2, PAGE_EXECUTE_READWRITE, &lpflOldProtect))
#else
        if (-1 == mprotect(pageof(pstub->fn), m_pagesize * 2, PROT_READ | PROT_WRITE | PROT_EXEC))
#endif       
        {
            throw("stub set mprotect to w+r+x faild");
        }

        *(unsigned char *)fn = (unsigned char)0xE9;
        *(unsigned int *)((unsigned char *)fn + 1) = (unsigned char *)fn_stub - (unsigned char *)fn - CODESIZE;
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

        memcpy(pstub->fn, pstub->code_buf, CODESIZE);
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

private:
#ifdef _WIN32
    long long m_pagesize;
#else
    long m_pagesize;
#endif   
    std::map<void*, func_stub*> m_result;
    
};


/**********************************************************
             get private member
**********************************************************/

#if defined(_WIN32)  &&  (_MSC_VER >= 1800) || (__cplusplus >= 201103L)

namespace std {
  template <bool B, class T = void>
  using enable_if_t = typename enable_if<B, T>::type;
  template <class T>
  using remove_reference_t = typename remove_reference<T>::type;
} // std

// Unnamed namespace is used to avoid duplicate symbols if the macros are used
namespace {
  namespace private_access_detail {

    // @tparam TagType, used to declare different "get" funciton overloads for
    // different members/statics
    template <typename PtrType, PtrType PtrValue, typename TagType>
    struct private_access {
      // Normal lookup cannot find in-class defined (inline) friend functions.
      friend PtrType get(TagType) { return PtrValue; }
    };

  } // namespace private_access_detail
} // namespace

// Used macro naming conventions:
// The "namespace" of this macro library is PRIVATE_ACCESS, i.e. all
// macro here has this prefix.
// All implementation macro, which are not meant to be used directly have the
// PRIVATE_ACCESS_DETAIL prefix.
// Some macros have the ABCD_IMPL form, which means they contain the
// implementation details for the specific ABCD macro.

#define PRIVATE_ACCESS_DETAIL_CONCATENATE_IMPL(x, y) x##y
#define PRIVATE_ACCESS_DETAIL_CONCATENATE(x, y)                                \
  PRIVATE_ACCESS_DETAIL_CONCATENATE_IMPL(x, y)

// @param PtrTypeKind E.g if we have "class A", then it can be "A::*" in case of
// members, or it can be "*" in case of statics.
#define PRIVATE_ACCESS_DETAIL_ACCESS_PRIVATE(Tag, Class, Type, Name,           \
                                             PtrTypeKind)                      \
  namespace {                                                                  \
    namespace private_access_detail {                                          \
      /* Tag type, used to declare different get funcitons for different       \
       * members                                                               \
       */                                                                      \
      struct Tag {};                                                           \
      /* Explicit instantiation */                                             \
      template struct private_access<decltype(&Class::Name), &Class::Name,     \
                                     Tag>;                                     \
      /* We can build the PtrType only with two aliases */                     \
      /* E.g. using PtrType = int(int) *; would be illformed */                \
      using PRIVATE_ACCESS_DETAIL_CONCATENATE(Alias_, Tag) = Type;             \
      using PRIVATE_ACCESS_DETAIL_CONCATENATE(PtrType_, Tag) =                 \
          PRIVATE_ACCESS_DETAIL_CONCATENATE(Alias_, Tag) PtrTypeKind;          \
      /* Declare the friend function, now it is visible in namespace scope.    \
       * Note,                                                                 \
       * we could declare it inside the Tag type too, in that case ADL would   \
       * find                                                                  \
       * the declaration. By choosing to declare it here, the Tag type remains \
       * a                                                                     \
       * simple tag type, it has no other responsibilities. */                 \
      PRIVATE_ACCESS_DETAIL_CONCATENATE(PtrType_, Tag) get(Tag);               \
    }                                                                          \
  }

#define PRIVATE_ACCESS_DETAIL_ACCESS_PRIVATE_FIELD(Tag, Class, Type, Name)     \
  PRIVATE_ACCESS_DETAIL_ACCESS_PRIVATE(Tag, Class, Type, Name, Class::*)       \
  namespace {                                                                  \
    namespace access_private_field {                                                 \
      Type &Class##Name(Class &&t) { return t.*get(private_access_detail::Tag{}); }   \
      Type &Class##Name(Class &t) { return t.*get(private_access_detail::Tag{}); }    \
      /* The following usings are here to avoid duplicate const qualifier      \
       * warnings                                                              \
       */                                                                      \
      using PRIVATE_ACCESS_DETAIL_CONCATENATE(X, Tag) = Type;                  \
      using PRIVATE_ACCESS_DETAIL_CONCATENATE(Y, Tag) =                        \
          const PRIVATE_ACCESS_DETAIL_CONCATENATE(X, Tag);                     \
      PRIVATE_ACCESS_DETAIL_CONCATENATE(Y, Tag) & Class##Name(const Class &t) {\
        return t.*get(private_access_detail::Tag{});                           \
      }                                                                        \
    }                                                                          \
  }

#define PRIVATE_ACCESS_DETAIL_ACCESS_PRIVATE_FUN(Tag, Class, Type, Name)       \
  PRIVATE_ACCESS_DETAIL_ACCESS_PRIVATE(Tag, Class, Type, Name, Class::*)       \
  namespace {                                                                  \
    namespace call_private_fun {                                                   \
      /* We do perfect forwarding, but we want to restrict the overload set    \
       * only for objects which have the type Class. */                        \
      template <typename Obj,                                                  \
                std::enable_if_t<std::is_same<std::remove_reference_t<Obj>,    \
                                              Class>::value> * = nullptr,      \
                typename... Args>                                              \
      auto Class##Name(Obj &&o, Args &&... args) -> decltype(                  \
          (std::forward<Obj>(o).*                                              \
           get(private_access_detail::Tag{}))(std::forward<Args>(args)...)) {  \
        return (std::forward<Obj>(o).*get(private_access_detail::Tag{}))(      \
            std::forward<Args>(args)...);                                      \
      }                                                                        \
    }                                                                          \
    namespace get_private_fun {                                               \
      auto Class##Name()  -> decltype(                                         \
          get(private_access_detail::Tag{})) {                                 \
        return (get(private_access_detail::Tag{}));                            \
      }                                                                        \
    }                                                                          \
  }

#define PRIVATE_ACCESS_DETAIL_ACCESS_PRIVATE_STATIC_FIELD(Tag, Class, Type,    \
                                                          Name)                \
  PRIVATE_ACCESS_DETAIL_ACCESS_PRIVATE(Tag, Class, Type, Name, *)              \
  namespace {                                                                  \
    namespace access_private_static_field {                                          \
      namespace Class {                                                        \
        Type &Class##Name() { return *get(private_access_detail::Tag{}); }     \
      }                                                                        \
    }                                                                          \
  }

#define PRIVATE_ACCESS_DETAIL_ACCESS_PRIVATE_STATIC_FUN(Tag, Class, Type,      \
                                                        Name)                  \
  PRIVATE_ACCESS_DETAIL_ACCESS_PRIVATE(Tag, Class, Type, Name, *)              \
  namespace {                                                                  \
    namespace call_private_static_fun {                                            \
      namespace Class {                                                        \
        template <typename... Args>                                            \
        auto Class##Name(Args &&... args) -> decltype(                         \
            get(private_access_detail::Tag{})(std::forward<Args>(args)...)) {  \
          return get(private_access_detail::Tag{})(                            \
              std::forward<Args>(args)...);                                    \
        }                                                                      \
      }                                                                        \
    }                                                                          \
    namespace get_private_static_fun {                                        \
      namespace Class {                                                        \
        auto Class##Name() -> decltype(get(private_access_detail::Tag{})) {    \
          return get(private_access_detail::Tag{});                            \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define PRIVATE_ACCESS_DETAIL_UNIQUE_TAG                                       \
  PRIVATE_ACCESS_DETAIL_CONCATENATE(PrivateAccessTag, __COUNTER__)

#define ACCESS_PRIVATE_FIELD(Class, Type, Name)                                \
  PRIVATE_ACCESS_DETAIL_ACCESS_PRIVATE_FIELD(PRIVATE_ACCESS_DETAIL_UNIQUE_TAG, \
                                             Class, Type, Name)

#define ACCESS_PRIVATE_FUN(Class, Type, Name)                                  \
  PRIVATE_ACCESS_DETAIL_ACCESS_PRIVATE_FUN(PRIVATE_ACCESS_DETAIL_UNIQUE_TAG,   \
                                           Class, Type, Name)

#define ACCESS_PRIVATE_STATIC_FIELD(Class, Type, Name)                         \
    Type Class::Name;                                                          \
  PRIVATE_ACCESS_DETAIL_ACCESS_PRIVATE_STATIC_FIELD(                           \
      PRIVATE_ACCESS_DETAIL_UNIQUE_TAG, Class, Type, Name)

#define ACCESS_PRIVATE_STATIC_FUN(Class, Type, Name)                           \
  PRIVATE_ACCESS_DETAIL_ACCESS_PRIVATE_STATIC_FUN(                             \
      PRIVATE_ACCESS_DETAIL_UNIQUE_TAG, Class, Type, Name)

#endif
#endif
