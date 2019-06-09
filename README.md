

**说明：**
- stub.h(适用windows、linux)相关方法基于C++03;使用Inline Hook方式;主要完成桩函数替换功能（参考：[http://jbremer.org/x86-api-hooking-demystified/#ah-other-2](http://jbremer.org/x86-api-hooking-demystified/#ah-other-2)、[https://www.codeproject.com/Articles/70302/Redirecting-functions-in-shared-ELF-libraries](https://www.codeproject.com/Articles/70302/Redirecting-functions-in-shared-ELF-libraries)）
- addr_pri.h(适用windows、linux)相关方法基于C++11;主要完成类的私有函数地址获取（参考：[https://github.com/martong/access_private](https://github.com/martong/access_private)）
- addr_any.h(只适用linux) 相关方法基于C++11,使用elfio库;主要完成任意形式函数地址获取（参考：[https://github.com/serge1/ELFIO)）
- 只适用x86、x64架构
- windows和linux的用法会稍微不同，原因是获取不同类型函数地址的方法不同，且调用约定有时不一样

**不可以打桩的情况：**
-	不可以对exit函数打桩，编译器做了特殊优化
-	不可以对纯虚函数打桩，纯虚函数没有地址
-	static声明的普通内部函数不能打桩，内部函数地址不可见（linux可使用addr_any.h来获取地址）


![](https://github.com/coolxv/cpp-stub/blob/master/pic/mm.png)
![](https://github.com/coolxv/cpp-stub/blob/master/pic/intel.png)

***


## 普通函数打桩（非static）

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
## 实例成员函数打桩

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
## 静态成员函数打桩

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
## 模板函数打桩

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

## 重载函数打桩

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

## 虚函数打桩

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
    fptr A_foo = (fptr)(&A::foo);   //获取虚函数地址
    Stub stub;
    stub.set(A_foo, foo_stub);
    A a;
    a.foo();
    return 0;
}

```

```
//for windows x86(32位)
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
//for windows x64(64位)，VS编译器不支持内嵌汇编。有解决方案自行搜索。
```

## 内联函数打桩

```
//for linux
//添加-fno-inline编译选项，禁止内联，能获取到函数地址，打桩参考上面。
```
```
//for windows
//添加/Ob0禁用内联展开。
```


## 第三方库私有成员函数打桩,使用addr_pri.h接口

```
//for linux
//被测代码添加-fno-access-private编译选项，禁用访问权限控制，成员函数都为公有的
//无源码的动态库或静态库无法自己编译，需要特殊技巧获取函数地址
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


## 静态函数打桩,使用addr_any.h接口
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