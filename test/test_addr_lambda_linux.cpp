#include<iostream>
#include<cstdio>
#include "stub.h"
#include "addr_any.h"

// g++ -g test_addr_lambda_linux.cpp -std=c++11 -I../src -I../src/elfio/elfio -o test_addr_lambda_linux

//This lambda function can be in another file or in another dynamic library, needed -g -O0 compile
static int foo()
{
    int temp = 2;
    auto a =  [temp](int a){std::cout << "foo lambda:" << a + temp << std::endl;};
    a(1);
    std::cout << "I am foo" << std::endl;
    return 0;
}


void foo_lambda_stub(void *obj, int a)
{
    //__closure={__temp = 2}
    std::cout << "I am foo_lambda_stub:" << *(int*)obj + a << std::endl;
    return;
}


int main(int argc, char **argv)
{
    //Get application static function address
    {
        AddrAny any;
        
        std::map<std::string,void*> result;
        any.get_local_func_addr_symtab("^foo()::{lambda.*", result);
        
        foo();
        Stub stub;
        std::map<std::string,void*>::iterator it;
        for (it=result.begin(); it!=result.end(); ++it)
        {
            stub.set(it->second ,foo_lambda_stub);
            std::cout << it->first << " => " << it->second << std::endl;
        }
        foo();  
    
    }

    return 0;
}

