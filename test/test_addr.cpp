#include <iostream>
#include "addr.h"
#include "stub.h"

// g++ -g test_addr.cpp -std=c++11 -I../ -o test_addr

static int test_test()
{
    std::cout << "test_test" << std::endl;
    return 0;
}

static int xxx_stub()
{
    std::cout << "xxx_stub" << std::endl;
    return 0;
}
int main(int argc, char **argv)
{
    std::map<std::string,ELFIO::Elf64_Addr> result;
    get_exe_local_func_addr(argv[0], "test_test", result);

    test_test();


    Stub stub;
    std::map<std::string,ELFIO::Elf64_Addr>::iterator it;
    for (it=result.begin(); it!=result.end(); ++it)
    {
        stub.set(it->second,xxx_stub);
        std::cout << it->first << " => " << it->second << '\n';
    }

    test_test();
    
    return 0;
}

