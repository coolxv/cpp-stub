#pragma once

#include <string>


class Obj
{
    public:
        Obj();
        virtual bool is_valid(std::string str);
        int get_number();
    private:
        int m_i;
};


