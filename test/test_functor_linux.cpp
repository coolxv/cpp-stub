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
