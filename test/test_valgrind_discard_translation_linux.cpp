#include<iostream>
#include<cstdio>
#include "stub.h"

/*
1. Build test_valgrind_discard_translation_linux without -D__VALGRIND__ by below command:

g++ test_valgrind_discard_translation_linux.cpp -o test_valgrind_discard_translation_linux -std=c++11 -m32 -I../src -fno-pie -fno-stack-protector -Wall -Wno-unused-function  -Wno-unused-variable -Wno-pmf-conversions

then run the compiled elf with "valgrind ./test_valgrind_discard_translation_linux" and you
will see the output as below, you failed to set the stub to foo_stub2.
"
set stub function to 0x8048b86
I am foo_stub1
set stub function to 0x8048bb7
I am foo_stub1
"

2. Build test_valgrind_discard_translation_linux with -D__VALGRIND__ by below command:

g++ test_valgrind_discard_translation_linux.cpp -o test_valgrind_discard_translation_linux -std=c++11 -m32 -I../src -D__VALGRIND__ -fno-pie -fno-stack-protector -Wall -Wno-unused-function -Wno-unused-variable -Wno-pmf-conversions

then run the compiled elf with "valgrind ./test_valgrind_discard_translation_linux" and you
will see the output as below, you succeeded to set the stub to foo_stub2.
"
set stub function to 0x8048c36
I am foo_stub1
set stub function to 0x8048c67
I am foo_stub2
"
*/


static int foo()
{
    printf("I am foo\n");
    return 0;
}

int foo_stub1()
{
    std::cout << "I am foo_stub1" << std::endl;
    return 0;
}

int foo_stub2()
{
    std::cout << "I am foo_stub2" << std::endl;
    return 0;
}

typedef int (*stub_f)();

Stub stub;

void test(stub_f f)
{
    printf("set stub function to %p\n", f);

    stub.set(foo, f);
    foo();
    stub.reset(foo);
}

int main(int argc, char **argv)
{
    test(foo_stub1);

    test(foo_stub2);

    return 0;
}
