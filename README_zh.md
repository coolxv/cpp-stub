[中文](README_zh.md)|[English](README.md)


# 原理介绍
## 两个核心点
- 如何获取原函数的地址(**addr_pri.h**、**addr_any.h**)
- 如何用桩函数替换原函数(**stub.h**)

## 一些说明
- stub.h(适合 windows, linux) 基于C++98开发; 使用 inline hook 技术; 主要解决函数替换问题 (相关参考:[stub](https://github.com/3gguan/stub.git))
- addr_pri.h(适合 windows, linux) 基于C++11开发; 主要解决对象的私有方法地址获取问题 (相关参考:[access_private](https://github.com/martong/access_private))
- src_linux/addr_any.h(仅适合 linux) 基于C++98开发; 使用 elfio 库查询ELF格式文件的符号表获取函数的地址 (也可以使用 bfd 库); 主要解决静态函数地址获取问题，前提编译时得包含调试信息 (相关参考:[ELFIO](https://github.com/serge1/ELFIO)、[bfd](https://sourceware.org/binutils/docs/bfd/))
- src_win/addr_any.h(仅适合 windows) 基于C++98开发; 使用 dbghelp 库查询PDB文件的符号表获取函数的地址; 主要解决静态函数地址获取问题，前提编译时得包含调试信息 (相关参考:[symbol-files](https://docs.microsoft.com/zh-cn/windows/desktop/Debug/symbol-files)、[dbghelpexamples](http://www.debuginfo.com/examples/dbghelpexamples.html)、[pelib](http://www.pelib.com/index.php))
- 使用时linux和windows还是有差别的, 主要涉及桩函数的写法, 还有原函数地址获取的方法不同; 获取虚函数的地址方法就不同，主要是C++ABI不兼容, 编译器支持不同(相关参考: [cxx-abi](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#vtable))
- 支持的操作系统 : windows,linux
- 支持的硬件平台 : x86,x86-64,arm64,arm32
- 支持的编译器 : msvc,gcc,clang
- 未来计划支持 macOS

## GOT/PLT Hook 、 Trap Hook 对比 Inline Hook

| | GOT/PLT Hook | Trap Hook | Inline Hook |
| --- | --- | --- | --- |
| The implementation principle | Modify Delay Binding Table | SIGTRAP Breakpoint Signal | Runtime Instruction Replacement |
| granularity | method level | instruction level | instruction level |
| Scopes | Narrow | Wide | Wide |
| Performance | High | Low | High |
| Difficulty | Medium | Medium |  High|

- Inline hook
![](pic/inline.png)


- GOT/PLT hook
![](pic/pltgot.png)


## X86/X64 跳转指令
![](pic/intel.png)

## aarch32/aarch64 跳转指令
![](pic/arm32.png)

![](pic/arm64.png)

# 单元测试相关说明
## 不能打桩
- 不能对 exit 函数打桩,编译器做优化了
- 不能对纯虚函数打桩, 纯虚函数没有地址
- 不能对 lambda 函数打桩, lambda 函数获取不到地址
- 不能对静态函数打桩, 静态函数地址不可见.(但可以尝试使用 addr_any.h 接口获取地址)


## 测试替身(来自网络)
- Dummy objects are passed around but never actually used. Usually they are just used to fill parameter lists.
- Fake objects actually have working implementations, but usually take some shortcut which makes them not suitable for production (an InMemoryTestDatabase is a good example).
- Spy are stubs that also record some information based on how they were called. One form of this might be an email service that records how many messages it was sent.
- Mock are pre-programmed with expectations which form a specification of the calls they are expected to receive. They can throw an exception if they receive a call they don't expect and are checked during verification to ensure they got all the calls they were expecting.
- Stub provide canned answers to calls made during the test, usually not responding at all to anything outside what's programmed in for the test.

## 单元测试框架
- gtest、gmock https://github.com/google/googletest
- cppunit https://github.com/epronk/cppunit
- catch2 https://github.com/catchorg/Catch2
- Boost.Test https://github.com/boostorg/test
- cpputest https://github.com/cpputest/cpputest
- doctest https://github.com/onqtam/doctest
- kmtest https://github.com/SergiusTheBest/kmtest
- trompeloeil https://github.com/rollbear/trompeloeil

## 单元测试编译选项, linux g++可用的
- -fno-access-control
- -fno-inline
- -Wno-pmf-conversions
- -Wl,--allow-multiple-definition
- -no-pie -fno-stack-protector
- -fprofile-arcs
- -ftest-coverage

## 代码覆盖率, linux g++使用方法**
```
lcov -d build/ -z
lcov -d build/ -b ../../src1 --no-external -rc lcov_branch_coverage=1 -t ut -c -o ut_1.info
lcov -d build/ -b ../../src2 --no-external -rc lcov_branch_coverage=1 -t ut -c -o ut_2.info
lcov -a ut_1.info -a ut_2.info -o ut.info
genhtml -o report/ --prefix=`pwd` --branch-coverage --function-coverage ut.info
```

# 接口介绍

## stub.h
```
Stub stub
stub.set(addr, addr_stub)
stub.reset(addr)
```

## addr_pri.h
```
Declaration:
    ACCESS_PRIVATE_FIELD(ClassName, TypeName, FieldName)
    ACCESS_PRIVATE_FUN(ClassName, TypeName, FunName)
    ACCESS_PRIVATE_STATIC_FIELD(ClassName, TypeName, FieldName)
    ACCESS_PRIVATE_STATIC_FUN(ClassName, TypeName, FunName)

Use:
    access_private_field::ClassNameFieldName(object);
    access_private_static_field::ClassName::ClassNameFieldName();
    call_private_fun::ClassNameFunName(object,parameters...);
    call_private_static_fun::ClassName::ClassNameFunName(parameters...);
    get_private_fun::ClassNameFunName();
    get_private_static_fun::ClassName::ClassNameFunName();
```

## addr_any.h(linux)
```
AddrAny any //for exe
AddrAny any(libname) //for lib
int get_local_func_addr_symtab(std::string func_name_regex_str, std::map<std::string,void*>& result)
int get_global_func_addr_symtab(std::string func_name_regex_str, std::map<std::string,void*>& result)
int get_weak_func_addr_symtab(std::string func_name_regex_str, std::map<std::string,void*>& result)

int get_global_func_addr_dynsym( std::string func_name_regex_str, std::map<std::string,void*>& result)
int get_weak_func_addr_dynsym(std::string func_name_regex_str, std::map<std::string,void*>& result)

```
## addr_any.h(windows)
```
AddrAny any //for all
int get_func_addr(std::string func_name, std::map<std::string,void*>& result)
```
## addr_any.h(darwin)
```
not implement
```

# 接口使用示例

## 常规函数

```
//for linux and windows
#include<iostream>
#include "stub.h"
using namespace std;
int foo(int a)
{   
    cout<<"I am foo"<<endl;
    return 0;
}
int foo_stub(int a)
{   
    cout<<"I am foo_stub"<<endl;
    return 0;
}


int main()
{
    Stub stub;
    stub.set(foo, foo_stub);
    foo(1);
    return 0;
}

```
## 可变参函数

```
//for linux and windows
#include<iostream>
#include <stdarg.h>
#include "stub.h"
using namespace std;

double average(int num, ...)
{
 
    va_list valist;
    double sum = 0.0;
    int i;
 
    va_start(valist, num);
 
    for (i = 0; i < num; i++)
    {
       sum += va_arg(valist, int);
    }
    va_end(valist);
    cout<<"I am foo"<<endl;
    return sum/num;
}

double average_stub(int num, ...)
{   
    va_list valist;
    double sum = 0.0;
    int i;
 
    va_start(valist, num);
 
    for (i = 0; i < num; i++)
    {
       sum += va_arg(valist, int);
    }
    va_end(valist);
    cout<<"I am foo_stub"<<endl;
    return  sum/num;
}
 
int main()
{
    cout << "Average of 2, 3, 4, 5 = " << average(4, 2,3,4,5) << endl;
    Stub stub;
    stub.set(average, average_stub);
    cout << "Average of 2, 3, 4, 5 = " << average(4, 2,3,4,5) << endl;

}

```
## 类成员函数

```
//for linux，__cdecl
#include<iostream>
#include "stub.h"
using namespace std;
class A{
    int i;
public:
    int foo(int a){
        cout<<"I am A_foo"<<endl;
        return 0;
    }
};

int foo_stub(void* obj, int a)
{   
    A* o= (A*)obj;
    cout<<"I am foo_stub"<<endl;
    return 0;
}


int main()
{
    Stub stub;
    stub.set(ADDR(A,foo), foo_stub);
    A a;
    a.foo(1);
    return 0;
}

```

```
//for windows，__thiscall
#include<iostream>
#include "stub.h"
using namespace std;
class A{
    int i;
public:
    int foo(int a){
        cout<<"I am A_foo"<<endl;
        return 0;
    }
};


class B{
public:
    int foo_stub(int a){
        cout<<"I am foo_stub"<<endl;
        return 0;
    }
};

int main()
{
    Stub stub;
    stub.set(ADDR(A,foo), ADDR(B,foo_stub));
    A a;
    a.foo(1);
    return 0;
}

```
## 类的静态成员函数

```
//for linux and windows
#include<iostream>
#include "stub.h"
using namespace std;
class A{
    int i;
public:
    static int foo(int a){
        cout<<"I am A_foo"<<endl;
        return 0;
    }
};

int foo_stub(int a)
{   
    cout<<"I am foo_stub"<<endl;
    return 0;
}


int main()
{
    Stub stub;
    stub.set(ADDR(A,foo), foo_stub);

    A::foo(1);
    return 0;
}

```
## 模板函数

```
//for linux，__cdecl
#include<iostream>
#include "stub.h"
using namespace std;
class A{
public:
   template<typename T>
   int foo(T a)
   {   
        cout<<"I am A_foo"<<endl;
        return 0;
   }
};

int foo_stub(void* obj, int x)
{   
    A* o= (A*)obj;
    cout<<"I am foo_stub"<<endl;
    return 0;
}


int main()
{
    Stub stub;
    stub.set((int(A::*)(int))ADDR(A,foo), foo_stub);
    A a;
    a.foo(5);
    return 0;
}

```

```
//for windows，__thiscall
#include<iostream>
#include "stub.h"
using namespace std;
class A{
public:
   template<typename T>
   int foo(T a)
   {   
        cout<<"I am A_foo"<<endl;
        return 0;
   }
};


class B {
public:
    int foo_stub(int a) {
        cout << "I am foo_stub" << endl;
        return 0;
    }
};


int main()
{
    Stub stub;
    stub.set((int(A::*)(int))ADDR(A,foo), ADDR(B, foo_stub));
    A a;
    a.foo(5);
    return 0;
}
```

## 重载函数

```
//for linux，__cdecl
#include<iostream>
#include "stub.h"
using namespace std;
class A{
    int i;
public:
    int foo(int a){
        cout<<"I am A_foo_int"<<endl;
        return 0;
    }
    int foo(double a){
        cout<<"I am A_foo-double"<<endl;
        return 0;
    }
};

int foo_stub_int(void* obj,int a)
{   
    A* o= (A*)obj;
    cout<<"I am foo_stub_int"<< a << endl;
    return 0;
}
int foo_stub_double(void* obj,double a)
{   
    A* o= (A*)obj;
    cout<<"I am foo_stub_double"<< a << endl;
    return 0;
}

int main()
{
    Stub stub;
    stub.set((int(A::*)(int))ADDR(A,foo), foo_stub_int);
    stub.set((int(A::*)(double))ADDR(A,foo), foo_stub_double);
    A a;
    a.foo(5);
    a.foo(1.1);
    return 0;
}

```

```
//for windows，__thiscall
#include<iostream>
#include "stub.h"
using namespace std;
class A{
    int i;
public:
    int foo(int a){
        cout<<"I am A_foo_int"<<endl;
        return 0;
    }
    int foo(double a){
        cout<<"I am A_foo-double"<<endl;
        return 0;
    }
};
class B{
    int i;
public:
    int foo_stub_int(int a)
    {
        cout << "I am foo_stub_int" << a << endl;
        return 0;
    }
    int foo_stub_double(double a)
    {
        cout << "I am foo_stub_double" << a << endl;
        return 0;
    }
};
int main()
{
    Stub stub;
    stub.set((int(A::*)(int))ADDR(A,foo), ADDR(B, foo_stub_int));
    stub.set((int(A::*)(double))ADDR(A,foo), ADDR(B, foo_stub_double));
    A a;
    a.foo(5);
    a.foo(1.1);
    return 0;
}
```

## 类成员虚函数

gcc extension: https://gcc.gnu.org/onlinedocs/gcc/Bound-member-functions.html

```
//for linux gcc
#include<iostream>
#include "stub.h"
using namespace std;
class A{
public:
    virtual int foo(int a){
        cout<<"I am A_foo"<<endl;
        return 0;
    }
};

int foo_stub(void* obj,int a)
{   
    A* o= (A*)obj;
    cout<<"I am foo_stub"<<endl;
    return 0;
}


int main()
{
    typedef int (*fptr)(A*,int);
    fptr A_foo = (fptr)(&A::foo);   //obtaining an address
    Stub stub;
    stub.set(A_foo, foo_stub);
    A a;
    a.foo();
    return 0;
}

```

```
//for windows x86
#include<iostream>
#include "stub.h"
using namespace std;
class A {
public:
    virtual int foo(int a) {
        cout << "I am A_foo" << endl;
        return 0;
    }
};

class B {
public:
    int foo_stub(int a)
    {
        cout << "I am foo_stub" << endl;
        return 0;
    }
};



int main()
{
    unsigned long addr;
    _asm {mov eax, A::foo}
    _asm {mov addr, eax}
    Stub stub;
    stub.set(addr, ADDR(B, foo_stub));
    A a;
    a.foo(1);
    return 0;
}
```

```
//for windows x64, msvc x64位不支持内联汇编语法, 可以试下单独汇编文件获取
https://docs.microsoft.com/en-us/cpp/assembler/inline/inline-assembler?view=vs-2019
```

```
//for clang, the clang++ 暂时没找到支持获取虚函数地址的扩展语法
```


## 虚函数并且重载


```
//for linux gcc
#include<iostream>
#include "stub.h"
using namespace std;
class A{
    int i;
public:
    virtual int foo(int a){
        cout<<"I am A_foo"<<endl;
        return 0;
    }
    virtual int foo(double a){
        cout<<"I am A_foo"<<endl;
        return 0;
    }
};

int foo_stub(void* obj, int a)
{
    A* o= (A*)obj;
    cout<<"I am foo_stub"<<endl;
    return 0;
}


int main()
{
    typedef int (*fptr)(A*,int);
    fptr A_foo = (fptr)((int(A::*)(int))&A::foo);
    Stub stub;
    stub.set(A_foo, foo_stub);
    A a;
    a.foo(1);
    return 0;
}
```

## 仿函数


```
//for linux gcc
#include<iostream>
#include "stub.h"
using namespace std;


class Foo
{
public:
    void operator() (int a)
    {
        cout<<"I am foo"<<endl;
    }
};

int foo_stub(void* obj, int a)
{   
    Foo* o= (Foo*)obj;
    cout<<"I am foo_stub"<<endl;
    return 0;
}

int main()
{
    Stub stub;
    stub.set(ADDR(Foo,operator()), foo_stub);
    Foo foo;
    foo(1);
    return 0;
}

```

```
//for windows
#include<iostream>
#include "stub.h"
using namespace std;


class Foo
{
public:
    void operator() (int a)
    {
        cout<<"I am foo"<<endl;
    }
};

class B{
public:
    int foo_stub(int a){
        cout<<"I am foo_stub"<<endl;
        return 0;
    }
};
int main()
{
    Stub stub;
    stub.set(ADDR(Foo,operator()), ADDR(B,foo_stub));
    Foo foo;
    foo(1);
    return 0;
}

```

## lambda函数

```


```

## 内联函数

```
//for linux
//添加 -fno-inline 编译选项, 禁用内联
```

```
//for windows
//添加 /Ob0  编译选项, 禁用内联
```

## 动态库里的函数


```
//for linux
#include<iostream>
#include<cstdio>
#include "stub.h"
using namespace std;
int foo(int a)
{
    printf("I am foo\n");
    return 0;
}

int printf_stub(const char * format, ...)
{
    cout<<"I am printf_stub"<<endl;
    return 0;
}


int main()
{
    Stub stub;
    stub.set(puts, printf_stub);
    foo(1);
    return 0;
}

```

```
//for windows
#include<iostream>
#include<cstdio>
#include "stub.h"
using namespace std;
int foo(int a)
{
    printf("I am foo\n");
    return 0;
}

int printf_stub(const char * format, ...)
{
    cout<<"I am printf_stub"<<endl;
    return 0;
}


int main()
{
    Stub stub;
    stub.set(printf, printf_stub);
    foo(1);
    return 0;
}

```


## 类的私有成员函数(使用 addr_pri.h)

```
//for linux
#include<iostream>
#include "stub.h"
#include "addr_pri.h"
using namespace std;
class A{
    int a;
    int foo(int x){
        cout<<"I am A_foo "<< a << endl;
        return 0;
    }
    static int b;
    static int bar(int x){
        cout<<"I am A_bar "<< b << endl;
        return 0;
    }
};


ACCESS_PRIVATE_FIELD(A, int, a);
ACCESS_PRIVATE_FUN(A, int(int), foo);
ACCESS_PRIVATE_STATIC_FIELD(A, int, b);
ACCESS_PRIVATE_STATIC_FUN(A, int(int), bar);

int foo_stub(void* obj, int x)
{   
    A* o= (A*)obj;
    cout<<"I am foo_stub"<<endl;
    return 0;
}
int bar_stub(int x)
{   
    cout<<"I am bar_stub"<<endl;
    return 0;
}
int main()
{
    A a;
    
    auto &A_a = access_private_field::Aa(a);
    auto &A_b = access_private_static_field::A::Ab();
    A_a = 1;
    A_b = 10;
   
    call_private_fun::Afoo(a,1);
    call_private_static_fun::A::Abar(1);
   
    auto A_foo= get_private_fun::Afoo();
    auto A_bar = get_private_static_fun::A::Abar();
    
    Stub stub;
    stub.set(A_foo, foo_stub);
    stub.set(A_bar, bar_stub);
    
    call_private_fun::Afoo(a,1);
    call_private_static_fun::A::Abar(1);
    return 0;
}
```

```
//for windows，__thiscall
#include<iostream>
#include "stub.h"
using namespace std;
class A{
    int a;
    int foo(int x){
        cout<<"I am A_foo "<< a << endl;
        return 0;
    }
    static int b;
    static int bar(int x){
        cout<<"I am A_bar "<< b << endl;
        return 0;
    }
};


ACCESS_PRIVATE_FIELD(A, int, a);
ACCESS_PRIVATE_FUN(A, int(int), foo);
ACCESS_PRIVATE_STATIC_FIELD(A, int, b);
ACCESS_PRIVATE_STATIC_FUN(A, int(int), bar);
class B {
public:
    int foo_stub(int x)
    {
        cout << "I am foo_stub" << endl;
        return 0;
    }
};
int bar_stub(int x)
{   
    cout<<"I am bar_stub"<<endl;
    return 0;
}


int main()
{
    A a;
    
    auto &A_a = access_private_field::Aa(a);
    auto &A_b = access_private_static_field::A::Ab();
    A_a = 1;
    A_b = 10;
   
    call_private_fun::Afoo(a,1);
    call_private_static_fun::A::Abar(1);
   
    auto A_foo= get_private_fun::Afoo();
    auto A_bar = get_private_static_fun::A::Abar();
    
    Stub stub;
    stub.set(A_foo, ADDR(B,foo_stub));
    stub.set(A_bar, bar_stub);
    
    call_private_fun::Afoo(a,1);
    call_private_static_fun::A::Abar(1);
    return 0;
}
```


## 静态函数(使用 addr_any.h)
通过查找符号表来获取地址, 也可以查找动态库里的静态函数地址
```
//for linux
#include<iostream>
#include<cstdio>
#include "stub.h"
#include "addr_any.h"

//This static function can be in another file or in another dynamic library, needed -g -O0 compile
static int foo()
{
    printf("I am foo\n");
    return 0;
}


int foo_stub()
{
    std::cout << "I am foo_stub" << std::endl;
    return 0;
}

int printf_stub(const char * format, ...)
{
    std::cout<< "I am printf_stub" << std::endl;
    return 0;
}

int main(int argc, char **argv)
{
    //Get application static function address
    {
        AddrAny any;
        
        std::map<std::string,void*> result;
        any.get_local_func_addr_symtab("^foo()$", result);
        
        foo();
        Stub stub;
        std::map<std::string,void*>::iterator it;
        for (it=result.begin(); it!=result.end(); ++it)
        {
            stub.set(it->second ,foo_stub);
            std::cout << it->first << " => " << it->second << std::endl;
        }
        foo();  
    
    }
    //Get dynamic library static function address
    {
        AddrAny any("libc-2.27.so");// cat /proc/pid/maps
        
        std::map<std::string,void*> result;
#ifdef __clang__ 
        any.get_global_func_addr_dynsym("^printf$", result);
#else
        any.get_weak_func_addr_dynsym("^puts", result);
#endif
        
        foo();
        Stub stub;
        std::map<std::string,void*>::iterator it;
        for (it=result.begin(); it!=result.end(); ++it)
        {
            stub.set(it->second ,printf_stub);
            std::cout << it->first << " => " << it->second << std::endl;
        }
        foo();
    }
    return 0;
}



```
```
//for windows
#include<iostream>
#include<cstdio>
#include "stub.h"
#include "addr_any.h"

using namespace std;

static int foo()
{
    printf("I am foo\n");
    return 0;
}

int foo_stub()
{
    std::cout << "I am foo_stub" << std::endl;
    return 0;
}

int printf_stub(const char * format, ...)
{
    std::cout<< "I am printf_stub" << std::endl;
    return 0;
}

int main(int argc, char **argv)
{

    //Get application static function address
    {
        AddrAny any;
        
        std::map<std::string,void*> result;
        any.get_func_addr("foo", result);
        
        foo();
        Stub stub;
        std::map<std::string,void*>::iterator it;
        for (it=result.begin(); it!=result.end(); ++it)
        {
            stub.set(it->second ,foo_stub);
            std::cout << it->first << " => " << it->second << std::endl;
        }
        foo();  
    
    }
    //Get dynamic library static function address
    {
        AddrAny any;
        
        std::map<std::string,void*> result;
        any.get_func_addr("printf", result);

        
        foo();
        Stub stub;
        std::map<std::string,void*>::iterator it;
        for (it=result.begin(); it!=result.end(); ++it)
        {
            stub.set(it->second ,printf_stub);
            std::cout << it->first << " => " << it->second << std::endl;
        }
        foo();
    }
    return 0;

}



```
