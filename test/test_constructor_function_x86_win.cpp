//for windows x86(32位)
#include<iostream>
#include "stub.h"
using namespace std;


template<class T>
void * get_ctor_addr()
{
    goto Start;
Call_Constructor:
    //This line of code will not be executed.
	//The purpose of the code is to allow the compiler to generate the assembly code that calls the constructor.
    T();
Start:
    //The address of the line of code T() obtained by assembly
    char * p = nullptr;
    __asm { mov[p], offset Call_Constructor }
    /*
    __asm
    {
        MOV EAX, OFFSET Call_Constructor
        MOV DWORD PTR[p], EAX
    }
    */
    int offset = *(int *)(p + 4);
    void * ret = p + 8 + offset;
    
    return ret;
}


class A {
public:
    A(){cout << "I am A_constructor" << endl;}
};

class B {
public:
    B(){cout << "I am B_constructor" << endl;}
};


int main()
{
    Stub stub;
    auto xa = get_ctor_addr<A>();
    auto xb = get_ctor_addr<B>();
    stub.set(xa, xb);
    A aa;
    return 0;
}
//for windows x64(64位)，VS编译器不支持内嵌汇编。可以把汇编代码独立成一个文件。
//https://social.msdn.microsoft.com/Forums/vstudio/en-US/e8b13ec0-32f0-4dcd-a5a2-59fc29e824e5/true-address-of-virtual-member-function-not-thunk?forum=vclanguage