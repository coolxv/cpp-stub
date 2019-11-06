
**Piling mainly involves two points**
- How to get the original function address (addr_pri.h,addr_any.h,c++ method of obtaining an address)
- How to replace the original function with stub function (stub.h)

**Explanation**
- stub.h(for windows, linux) related methods based on C++03; use inline hook method; mainly complete the stub function replacement function (reference:[http://jbremer.org/x86-api-hooking-demystified/#ah-other-2](http://jbremer.org/x86-api-hooking-demystified/#ah-other-2)、[https://www.codeproject.com/Articles/70302/Redirecting-functions-in-shared-ELF-libraries](https://www.codeproject.com/Articles/70302/Redirecting-functions-in-shared-ELF-libraries)）
- addr_pri.h(for windows, linux) related methods based on C + + 11; mainly complete the class's private function address acquisition (reference:[https://github.com/martong/access_private](https://github.com/martong/access_private)）
- src_linux/addr_any.h(only for linux) related methods based on C++11, use the elfio library to query the symbol table (also use bfd parsing, centos:binutils-devel); mainly complete the arbitrary form function address acquisition (reference:[https://github.com/serge1/ELFIO](https://github.com/serge1/ELFIO)、[https://sourceware.org/binutils/docs/bfd/](https://sourceware.org/binutils/docs/bfd/)）
- src_win/addr_any.h(only for windows) related methods based on C++11, use the dbghelp library to query the symbol table of the pdb file (you can also use the pe library to parse the exported symbols); mainly complete the arbitrary form function address acquisition (reference:[https://docs.microsoft.com/zh-cn/windows/desktop/Debug/symbol-files](https://docs.microsoft.com/zh-cn/windows/desktop/Debug/symbol-files)、[http://www.debuginfo.com/examples/dbghelpexamples.html](http://www.debuginfo.com/examples/dbghelpexamples.html)、[http://www.pelib.com/index.php](http://www.pelib.com/index.php)）
- Only for x86, x64 architecture
- The usage of windows and linux will be slightly different, because the methods for getting different types of function addresses are different, and the calling conventions are sometimes different.

**Cannot piling**
- Can't piling the exit function, the compiler has made special optimizations
- Can't piling pure virtual functions, pure virtual functions have no address
- The normal internal function declared by static cannot be piling, and the internal function address is not visible (addr_any.h can be used to get the address)

**Unit test compilation option for linux g++**
- -fno-access-control
- -fno-inline
- -Wno-pmf-conversions
- -Wl,--allow-multiple-definition
- -fprofile-arcs
- -ftest-coverage

**Principle of implementation**
![](https://github.com/coolxv/cpp-stub/blob/master/pic/intel.png)

***


## normal function

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
## member function

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
## static member function

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
## template function

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

## overload function

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

## virtual function

```
//for linux
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
//for windows x64, the visual studio compiler does not support inline assembly. There are solutions to search for yourself.
```

## inline function

```
//for linux
//Add the -fno-inline compile option, disable inlining, get the function address.
```
```
//for windows
//Add /Ob0 to disable inline expansion.
```


## private member function(use addr_pri.h)

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


## static function(addr_any.h)
```
#include <iostream>
#include <string>
#include <stdio.h>

#include "addr_any.h"
#include "stub.h"


static int test_test()
{
    printf("test_test\n");
    return 0;
}

static int xxx_stub()
{
    std::cout << "xxx_stub" << std::endl;
    return 0;
}
int main(int argc, char **argv)
{
    std::string res;
    get_exe_pathname(res);
    std::cout << res << std::endl;
    unsigned long base_addr;
    get_lib_pathname_and_baseaddr("libc-2.17.so", res, base_addr);
    std::cout << res << base_addr << std::endl;
    std::map<std::string,ELFIO::Elf64_Addr> result;
    get_weak_func_addr(res, "^puts$", result);
    
    test_test();


    Stub stub;
    std::map<std::string,ELFIO::Elf64_Addr>::iterator it;
    for (it=result.begin(); it!=result.end(); ++it)
    {
        stub.set(it->second + base_addr ,xxx_stub);
        std::cout << it->first << " => " << it->second + base_addr<<std::endl;
    }

    test_test();
    
    return 0;
}

```
