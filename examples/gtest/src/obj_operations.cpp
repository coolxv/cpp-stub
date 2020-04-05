#include <iostream>
#include "obj_operations.hpp"


bool check_obj(Obj &obj)
{
    if (obj.is_valid("Kanotest"))
    {
        std::cout << "invalid" << std::endl;
        return false;
    }

    std::cout << obj.get_number() << std::endl;
    return true;
}
