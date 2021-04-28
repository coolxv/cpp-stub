#include<iostream>
#include "stub.h"
using namespace std;

template<class T>
void * get_dtor_addr()
{
	//the start vairable must be true, or the compiler will optimize out.
    goto Start;
    //This line of code will not be executed.
	//The purpose of the code is to allow the compiler to generate the assembly code that calls the constructor.
	{
		T();
Call_dtor:
		;;
	}

Start:
    //The address of the line of code T() obtained by assembly
    char * p;
	__asm { mov[p], offset Call_dtor }

    //CALL rel32
	void * ret = 0;
	char pos;
	char call = 0xe8;
	do{
		pos = *p;
		if(pos == call)
		{
			ret = p + 5 + (*(int*)(p+1));
		}
		
	}while(!ret&&(--p));
    
    return ret;
}


class A {
public:
    A(){cout << "I am A_constructor" << endl;}
	~A(){{cout << "I am A_dtor" << endl;}}
};

class B {
public:
    B(){cout << "I am B_constructor" << endl;}
    ~B(){cout << "I am B_dtor" << endl;}
};


int main()
{
    Stub stub;
    auto xa = get_dtor_addr<A>();
    auto xb = get_dtor_addr<B>();
    stub.set(xa, xb);
    A aa;
    return 0;
}
