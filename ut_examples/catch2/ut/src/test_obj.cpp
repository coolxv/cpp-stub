/**
 * Example tests of the Obj() class
 */


// Test framework
#include "catch.hpp"

//Stub
#include "stub.h"


// Functions to test
#include "obj.hpp"
#include "obj.cpp"
#include "obj_operations.hpp"
#include "obj_operations.cpp"



//stub function
int get_number_stub(void* obj)
{
    return 5;
}


TEST_CASE( "get_number_test", "[get_number]" ) {
    
    Stub stub;
    stub.set(ADDR(Obj, get_number), get_number_stub);
    Obj obj;
    REQUIRE(obj.get_number()== 5);

}

