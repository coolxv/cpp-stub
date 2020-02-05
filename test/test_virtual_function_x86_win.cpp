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
//for windows x64(64位)，VS编译器不支持内嵌汇编。可以把汇编代码独立成一个文件。
//https://social.msdn.microsoft.com/Forums/vstudio/en-US/e8b13ec0-32f0-4dcd-a5a2-59fc29e824e5/true-address-of-virtual-member-function-not-thunk?forum=vclanguage