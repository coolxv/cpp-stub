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

