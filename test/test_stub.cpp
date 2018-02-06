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
   void f2(double a)
    {
        printf("[A:f2] ===> a = %lf, b = %d\n",a,b);
        return;
    }
   template<typename T>
   void f5(T a)
   {   
       printf("[A:f5]\n");
       return;
   }

    
};
static void f4(int a)
{
    printf("[static:f4] ===> a = %d\n",a);
    return;
}

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
void f2_d(void *obj,double a)
{
    A* o= (A*)obj;
    auto &a_b = access_private_field::Ab(*o);
    printf("[f2] ===> a = %lf, b = %d\n", a, a_b);

    return;
}

void f3(int a)
{
    auto &a_c = access_private_static_field::A::Ac();
    printf("[f3] ===> a = %d, c = %d\n", a, a_c);

    return;
}

void f4_stub(int a)
{
    //printf("[stub:f4] ===> a = %d\n",a);
    return;
}

void f5(int a)
{   
    printf("[f5]\n");
    return;
}


int main()
{

    
    A a;

    auto &a_b = access_private_field::Ab(a);
    auto &a_c = access_private_static_field::A::Ac();
    a_b=555;
    a_c=666;
    
    printf("========================================\n");

    call_private_fun::Af1(a,1);
    a.f2(2);
    a.f2(2.0);
    call_private_static_fun::A::Af3(3);
    f4(4);
    a.f5(5);
    printf("========================================\n");


    //replace object member function with custom function

    Stub *stub = new Stub;

    auto a_f1= get_private_fun::Af1();
    auto a_f3 = get_private_static_fun::A::Af3();


    stub->set(a_f1, f1);
    stub->set((void(A::*)(int))ADDR(A,f2),f2);
    stub->set((void(A::*)(double))ADDR(A,f2),f2_d);

    stub->set(a_f3,f3);
    //stub->set(printf,f4_stub);
    stub->set((void(A::*)(int))ADDR(A,f5),f5);
    

    call_private_fun::Af1(a,11);
    a.f2(22);
    a.f2(22.00);
    call_private_static_fun::A::Af3(33);
    f4(44);
    f5(55);

    printf("========================================\n");

    //reset
    stub->reset(a_f1);
    stub->reset(a_f3);
    
    call_private_fun::Af1(a,111);
    a.f2(222);
    a.f2(222.000);

    call_private_static_fun::A::Af3(333);
    printf("========================================\n");

    //recover object member function
    delete stub;
    
    call_private_fun::Af1(a,1111);
    a.f2(2222);
    a.f2(2222.0000);
    call_private_static_fun::A::Af3(3333);
    a.f5(5555);
    printf("========================================\n");

    return 0;
}
