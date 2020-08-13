#include "obj.hpp"


Obj::Obj()
{
    m_i = 1;
}


bool Obj::is_valid(std::string str)
{
    return str.find("test") == 0;
}


int Obj::get_number()
{
    return m_i;
}
