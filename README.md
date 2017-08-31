c++单元测试打桩接口
================



测试代码--->被测代码--->被测代码依赖库（无源码）

第一步、编译被测代码，编译成静态库.a( 使用-Dprivate=public -Dprotect=public -Dclass=struct )

第二步、编译测试代码，结合gtest或者cppunit，（测试代码包含头文件stub.h）

说明：stub类用来打桩使用；private相关方法主要用来获取类的私有成员（主要针对无源码的依赖库内的接口打桩时使用）



示例代码
================
```cpp

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "stub.h"

// g++ -g test_stub.cpp -std=c++11  -o test_stub


class A
{
    void f1(int a)
    {
        printf("[A:f1] ===> a = %d, b = %d\n",a,b);
        return;
    }
    int b;
    static int c;
    static void f3(int a)
    {
        printf("[A:f3] ===> a = %d, c = %d\n",a,c);
        return;        
    }
    
public:
   void f2(int a)
    {
        printf("[A:f2] ===> a = %d, b = %d\n",a,b);
        return;
    }

    
};


ACCESS_PRIVATE_STATIC_FIELD(A, int, c);
ACCESS_PRIVATE_STATIC_FUN(A, void(int), f3);
ACCESS_PRIVATE_FIELD(A, int, b);
ACCESS_PRIVATE_FUN(A, void(int), f1);






void f1(void *obj,int a)
{
    A* o= (A*)obj;
    auto &a_b = access_private_field::Ab(*o);
    printf("[f1] ===> a = %d, b = %d\n", a, a_b);
    return;
}

void f2(void *obj,int a)
{
    A* o= (A*)obj;
    auto &a_b = access_private_field::Ab(*o);
    printf("[f2] ===> a = %d, b = %d\n", a, a_b);

    return;
}
void f3(int a)
{
    auto &a_c = access_private_field_static::A::Ac();
    printf("[f3] ===> a = %d, c = %d\n", a, a_c);

    return;
}


int main()
{

    
    A a;

    auto &a_b = access_private_field::Ab(a);
    auto &a_c = access_private_field_static::A::Ac();
    a_b=555;
    a_c=666;

    call_private_fun::Af1(a,1);
    a.f2(2);
    call_private_fun_static::A::Af3(3);


    //replace object member function with custom function

    Stub *stub = new Stub;

    auto a_f1= get_private_fun::Af1();
    auto a_f3 = get_private_fun_static::A::Af3();


    stub->set(a_f1, f1);
    stub->set(ADDR(A,f2),f2);
    stub->set(a_f3,f3);
    

    call_private_fun::Af1(a,11);
    a.f2(22);
    call_private_fun_static::A::Af3(33);


    return 0;
}



```


示例代码运行结果
================
```

./test_stub 
========================================
[A:f1] ===> a = 1, b = 555
[A:f2] ===> a = 2, b = 555
[A:f3] ===> a = 3, c = 666
========================================
[f1] ===> a = 11, b = 555
[f2] ===> a = 22, b = 555
[f3] ===> a = 33, c = 666
========================================
[A:f1] ===> a = 111, b = 555
[f2] ===> a = 222, b = 555
[A:f3] ===> a = 333, c = 666
========================================
[A:f1] ===> a = 1111, b = 555
[A:f2] ===> a = 2222, b = 555
[A:f3] ===> a = 3333, c = 666
========================================

```