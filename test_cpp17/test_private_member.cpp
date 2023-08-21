//http://www.purecpp.cn/detail?id=2372
//g++ prog.cc -Wall -Wextra -std=c++17 -I../src
#include <tuple>
#include <string>
#include <iostream>
#include "stub.h"

template <typename T, auto... field>
struct ThiefMember {
    friend auto steal_impl(T&) {
        return std::make_tuple(field...);
    }
};

class Bank_t{
    int id;
    std::string name;
 
    std::string money(){
        return "100$";
    }
public:
    Bank_t(int i, std::string str) : id(i), name(str){}
    void set_id(int i){id = i;}
};

auto steal_impl(Bank_t& t);
template struct ThiefMember<Bank_t, &Bank_t::id, &Bank_t::name, &Bank_t::money>;



std::string money_stub(void *)
{
    return "10000$";
}

int main(void) {
    Bank_t bank(1, "ok");
    auto tp = steal_impl(bank);

    auto& id = bank.*(std::get<0>(tp)); // 1
    auto name = bank.*(std::get<1>(tp)); //ok

    auto money = (bank.*(std::get<2>(tp)))(); // 100$
    std::cout << "id: " << id << std::endl;
    std::cout << "name: " << name << std::endl;
    std::cout << "money: " << money << std::endl;
    bank.set_id(2);
    std::cout << "id: " << id << std::endl;
    Stub stub;
    auto money_ptr = (void*)(bank.*(std::get<2>(tp)));
    stub.set(money_ptr, money_stub);
    auto money_stub = (bank.*(std::get<2>(tp)))(); // 10000$
    std::cout << "money_stub: " << money_stub << std::endl;
	
    return 0;
}
