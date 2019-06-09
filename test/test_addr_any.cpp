#include <iostream>
#include <string>
#include <stdio.h>

#include "addr_any.h"
#include "stub.h"

// g++ -g test_addr_any.cpp -std=c++11 -I../ -o test_addr_any

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

